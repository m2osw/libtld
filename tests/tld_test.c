/* TLD library -- test the TLD interface
 * Copyright (c) 2011-2023  Made to Order Software Corp.  All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** \file
 * \brief Test the tld() function like an end user.
 *
 * This file implements various tests verifying the tld() function.
 * The tests defined here are not for coverage but rather edge
 * cases which could be hard to expect in a full coverage test.
 */

// libtld lib
//
#include    <libtld/tld.h>
#include    <libtld/tld_data.h>
#include    <libtld/tld_file.h>


// C lib
//
#include    <string.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <limits.h>






int err_count = 0;
int verbose = 0;

/*
 * This test calls the tld() function with all the TLDs and then
 * with wrong TLDs to make sure that the tld() functions works as
 * expected.
 *
 * extern enum tld_result tld(const char *uri, struct tld_info *info);
 */


char *              g_filename1 = "../../BUILD/Debug/contrib/libtld/libtld/tlds.tld";
char *              g_filename2 = "../libtld/tlds.tld";
struct tld_file *   g_tld_file = NULL;


/* we get access to the table with all the TLDs so we can go through them all
 * the library does not give direct access by default... (although maybe we
 * could give users access to the data)
 */
void load_tlds()
{
    enum tld_file_error err;

    err = tld_file_load(g_filename1, &g_tld_file);
    if(err == TLD_FILE_ERROR_NONE)
    {
        return;
    }

    err = tld_file_load(g_filename2, &g_tld_file);
    if(err == TLD_FILE_ERROR_NONE)
    {
        return;
    }

    fprintf(stderr, "fatal error: could not read TLD files \"%s\" or \"%s\".\n",
                        g_filename1, g_filename2);
    exit(1);
}


void free_tlds()
{
    tld_file_free(&g_tld_file);
}



/** \brief Build an extension from any offset.
 *
 * Create a domain name extensions from any entry in the TLD
 * descriptions.
 *
 * \param[in] offset  The offset in the tld_descriptions table
 * \param[in] uri  The URI buffer
 *
 * \return true if the first TLD is a star ("*").
 */
int cat_ext(int offset, char *uri)
{
    int                             o, has_star;
    uint32_t                        k, l;
    const struct tld_description *  tld;
    const char *                    name;

    tld = tld_file_description(g_tld_file, offset);
    name = tld_file_string(g_tld_file, tld->f_tld, &l);

    has_star = l == 1 && name[0] == '*';

    if(!has_star)
    {
        strcat(uri, ".");
        strncat(uri, name, l);
    }
    o = offset;
    for(k = offset + 1; k < g_tld_file->f_descriptions_count; ++k)
    {
        tld = tld_file_description(g_tld_file, k);
        if(o >= tld->f_start_offset
        && o < tld->f_end_offset)
        {
            /* found a parent */
            name = tld_file_string(g_tld_file, tld->f_tld, &l);
            if(l != 1 || name[0] != '*')
            {
                strcat(uri, ".");
                strncat(uri, name, l);
            }
            else
            {
                fprintf(stderr, "fatal error: found \"*\" at the wrong place; it's only supported as the very first segment.\n");
                exit(1);
            }
            o = k;
            k = tld->f_end_offset;
        }
    }

    return has_star;
}




struct test_uris
{
    const char *        f_uri;
    enum tld_result     f_result;
    int                 f_offset;
};


const struct test_uris g_uris[] =
{
    {
        "advisor-z2-ngprod-1997768525.us-west-2.elb.amazonaws.com",
        TLD_RESULT_SUCCESS,
        28,
    },
    {
        "us-west-2.elb.amazonaws.com",
        TLD_RESULT_SUCCESS,
        0,
    },
    {
        "m2osw.com",
        TLD_RESULT_SUCCESS,
        5,
    },
    {
        ".com",
        TLD_RESULT_SUCCESS,
        0,
    },
    {
        "com",
        TLD_RESULT_NO_TLD,
        -1,
    },
    {
        ".ar",
        TLD_RESULT_INVALID,
        0,
    },
    {
        "int.ar",
        TLD_RESULT_SUCCESS,
        0,
    },
    {
        "blah.int.ar",
        TLD_RESULT_SUCCESS,
        4,
    },
    {
        "orange.blah.int.ar",
        TLD_RESULT_SUCCESS,
        11,
    },
    {
        "congresodelalengua3.ar",   /* congresodelalengua3 is an exceptional 2nd level */
        TLD_RESULT_SUCCESS,
        19,
    },
    {
        "special.congresodelalengua3.ar",   /* congresodelalengua3 is an exceptional 2nd level */
        TLD_RESULT_SUCCESS,
        27,
    },
    {
        "night-club.kawasaki.jp",
        TLD_RESULT_SUCCESS,
        0,
    },
    {
        "orange.night-club.kawasaki.jp",
        TLD_RESULT_SUCCESS,
        6,
    },
};


/*
 * This tests various ad hoc domains with expected results.
 *
 * This way we can verify specific things we want to check.
 */
void test_specific()
{
    for(size_t idx = 0; idx < sizeof(g_uris) / sizeof(g_uris[0]); ++idx)
    {
        struct tld_info info;
        enum tld_result r = tld(g_uris[idx].f_uri, &info);
        if(verbose)
        {
            fprintf(
                  stderr
                , "info: URI \"%s\" returned %d and TLD is \"%s\"\n"
                , g_uris[idx].f_uri
                , r
                , g_uris[idx].f_uri + info.f_offset);
        }

        if(r != g_uris[idx].f_result)
        {
            fprintf(stderr, "error: testing URI \"%s\" got result %d, expected %d and TLD of \"%s\"\n",
                        g_uris[idx].f_uri, r, g_uris[idx].f_result,
                        g_uris[idx].f_uri + g_uris[idx].f_offset);
            ++err_count;
        }
        else if(info.f_offset != g_uris[idx].f_offset)
        {
            fprintf(stderr, "error: testing URI \"%s\" got offset %d, expected %d and TLD of \"%s\"\n",
                        g_uris[idx].f_uri, info.f_offset, g_uris[idx].f_offset,
                        g_uris[idx].f_uri + info.f_offset);
            ++err_count;
        }
    }
}


/*
 * This test goes through all the domain names and extracts the domain,
 * sub-domains and TLDs. (Or at least verifies that we get the correct
 * information in order to do so.)
 *
 * It builds a URI with zero to many sub-domain names, adds a specific
 * domain name, then append a complete TLD. The result is then checked
 * with the tld() function from the library. The tld() is expected to
 * either return VALID or INVALID but nothing else (since all those
 * TLDs exist in our table.) Then we verify that the returned offset is
 * a perfect match.
 */
void test_all()
{
    const char *sub_domains[] = {
        "",
        "www.",
        "tld.",
        "george.snap.",
        "very.long.sub.domain.ext.en.sion.here."
        "host.%20.space."
        "host.%fa.u-acute."
        "host.%FA.U-acute."
    };
    struct tld_info                 info;
    char                            uri[256], extension_uri[256];
    const char *                    name;
    uint32_t                        i, j, l, p, max_subdomains;
    int                             has_star, sub_has_star;
    enum tld_result                 r;
    const struct tld_description    *tld_desc, *sub_tld;

    max_subdomains = sizeof(sub_domains) / sizeof(sub_domains[0]);

    for(i = 0; i < g_tld_file->f_descriptions_count; ++i)
    {
        for(j = 0; j < max_subdomains; ++j)
        {
            strcpy(uri, sub_domains[j]);
            strcat(uri, "domain-name");
            has_star = cat_ext(i, uri);

            /* just in case make sure that we did not overflow the buffer */
            if(strlen(uri) >= sizeof(uri))
            {
                fprintf(stderr, "fatal error: the URI \"%s\" is longer than the uri[] array.\n", uri);
                exit(1);
            }

            /* reset the structure so we can verify it gets initialized */
            memset(&info, 0xFE, sizeof(info));
            r = tld(uri, &info);
            /*
            for(size_t l = 0; l < sizeof(info); ++l)
            {
                fprintf(stderr, "0x%02X ", ((unsigned char*)&info)[l]);
            }
            fprintf(stderr, "\nresult for [%s]: category[%d], status[%d/%d], country[%s],"
                                " tld[%s], offset[%d]\n",
                    uri,
                    (int)info.f_category,
                    (int)info.f_status, (int)tld_descriptions[i].f_status,
                    info.f_country,
                        info.f_tld, (int)info.f_offset);
            */
            p = i;
            tld_desc = tld_file_description(g_tld_file, i);
            if(tld_desc->f_status == TLD_STATUS_EXCEPTION)
            {
                if(tld_desc->f_exception_apply_to == USHRT_MAX)
                {
                    fprintf(stderr, "error: domain name for \"%s\" (%d) is said to be an exception but it has no apply-to parameter. (result: %d)\n",
                            uri, i, r);
                    ++err_count;
                }
                else
                {
                    p = tld_desc->f_exception_apply_to;
                }
            }
            if(tld_desc->f_status == TLD_STATUS_VALID)
            {
                if(r != TLD_RESULT_SUCCESS)
                {
                    fprintf(stderr, "error: valid domain name for \"%s\" (%d) could not be extracted successfully (returned: %d)\n",
                            uri, i, r);
                    ++err_count;
                }
                else if(has_star)
                {
                    /* the "domain-name" is absorbed as part of the TLD */
                    int expected = strlen(sub_domains[j]);
                    if(expected != 0)
                    {
                        --expected;     /* ignore the "." */
                    }
                    if(info.f_offset != expected)
                    {
                        fprintf(stderr, "error: valid domain name for \"%s\" (%d) could not be extracted successfully (offset: %d, expected: %d)\n",
                                uri, i,
                                info.f_offset, expected);
                        ++err_count;
                    }
                }
                else
                {
                    /* verify the top domain name */
                    if(info.f_offset < 11)
                    {
                        fprintf(stderr, "error: somehow the top domain name in \"%s\" (%d) cannot properly be extracted\n",
                                uri, i);
                        ++err_count;
                    }
                    else if(strncmp(uri + info.f_offset - 11, "domain-name", 11) != 0)
                    {
                        fprintf(stderr, "error: valid domain name for \"%s\" (%d) could not be extracted successfully (offset: %d)\n",
                                uri, i, info.f_offset);
                        ++err_count;
                    }
                    /*
                    else
                        fprintf(stderr, "valid: \"%s\" -> \"%s\"\n", uri, info.f_tld);
                    */
                }
            }
            else if(tld_desc->f_status == TLD_STATUS_EXCEPTION)
            {
                if(r != TLD_RESULT_SUCCESS)
                {
                    fprintf(stderr, "error: exceptional domain name for \"%s\" (%d) could not be extracted successfully (returned: %d)\n",
                            uri, i, r);
                    ++err_count;
                }
                else
                {
                    extension_uri[0] = '\0';
                    cat_ext(p, extension_uri);
                    if(strcmp(info.f_tld, extension_uri) != 0)
                    //if(strncmp(uri + info.f_offset - 11, "domain-name", 11) != 0)
                    {
                        fprintf(stderr, "error: exceptional domain name for \"%s\" (%d/%d) could not be extracted successfully as \"%s\" (offset: %d)\n",
                                uri, i, p, extension_uri, info.f_offset);
                        ++err_count;
                    }
                    /*
                    else
                        fprintf(stderr, "valid: \"%s\" -> \"%s\"\n", uri, info.f_tld);
                    */
                }
            }
            else
            {
                sub_has_star = 0;
                if(tld_desc->f_status == TLD_STATUS_UNUSED
                && tld_desc->f_start_offset != USHRT_MAX)
                {
                    sub_tld = tld_file_description(g_tld_file, tld_desc->f_start_offset);
                    name = tld_file_string(g_tld_file, sub_tld->f_tld, &l);
                    sub_has_star = l == 1 && name[0] == '*';
                }
                if(sub_has_star)
                {
                    /* this is a special case, an entry such as:
                     *
                     *     *.blah.com
                     *
                     * and that means the result is going to be SUCCESS
                     * and VALID...
                     */
                    if(r != TLD_RESULT_SUCCESS
                    || info.f_status != TLD_STATUS_VALID)
                    {
                        fprintf(stderr, "error: domain name for \"%s\" (%d) could not be extracted as expected (returned: %d) [1]\n",
                                uri, i, r);
                        ++err_count;
                    }
                }
                else if(r != TLD_RESULT_INVALID)
                {
                    fprintf(stderr, "error: domain name for \"%s\" (%d) could not be extracted as expected (returned: %d) [2]\n",
                            uri, i, r);
                    ++err_count;
                }
                else if(p != i)
                {
                    extension_uri[0] = '\0';
                    cat_ext(p, extension_uri);
                    if(strcmp(info.f_tld, extension_uri) != 0)
                    {
                        fprintf(stderr, "error: other domain name for \"%s\" (%d) could not be extracted successfully (returned: %d/%s != %s) [1]\n",
                                uri, i, r, info.f_tld, extension_uri);
                        ++err_count;
                    }
                    /*
                    else
                        fprintf(stderr, "?? invalid: \"%s\" -> \"%s\"\n", uri, info.f_tld); 
                    */
                }
                else
                {
                    if(strncmp(uri + info.f_offset - 11, "domain-name", 11) != 0)
                    {
                        fprintf(stderr, "error: other domain name for \"%s\" (%d) could not be extracted successfully (returned: %d/%s) [2]\n",
                                uri, i, r, info.f_tld);
                        ++err_count;
                    }
                    /*
                    else
                        fprintf(stderr, "?? invalid: \"%s\" -> \"%s\"\n", uri, info.f_tld); 
                    */
                }
            }
        }
    }
}


/*
 * This test checks out URIs that end with an invalid TLD. This is
 * expected to return an error every single time.
 */
void test_unknown()
{
    struct bad_data
    {
        const char *        f_uri;
    };
    struct bad_data d[] =
    {
        { "this.is.wrong" },
        { "missing.tld" },
        { ".net.absolutely.com.no.info.on.this" }
    };
    struct tld_info info;
    int i, max;
    enum tld_result r;

    max = sizeof(d) / sizeof(d[0]);
    for(i = 0; i < max; ++i)
    {
        memset(&info, 0xFE, sizeof(info));
        r = tld(d[i].f_uri, &info);
        if(r != TLD_RESULT_NOT_FOUND)
        {
            fprintf(stderr, "error: the invalid URI \"%s\" was found by tld()!\n", d[i].f_uri);
            ++err_count;
        }
    }
}




void test_invalid()
{
    struct tld_info undefined_info;
    struct tld_info clear_info;
    struct tld_info info;
    enum tld_result r;

    /*
     * We reset the undefined_info the same way we reset the info
     * structure because the alignment on 64bits may add another
     * 4 bytes at the end of the structure that are not otherwise
     * accessible.
     */
    memset(&undefined_info, 0xFE, sizeof(undefined_info));
    undefined_info.f_category  = TLD_CATEGORY_UNDEFINED;
    undefined_info.f_status    = TLD_STATUS_UNDEFINED;
    memset(undefined_info.f_country, 0, sizeof(undefined_info.f_country));
    undefined_info.f_tld       = (const char *) 0;
    undefined_info.f_offset    = -1;
    undefined_info.f_tld_index = -1;

    memset(&clear_info, 0xFE, sizeof(clear_info));

    /* test: NULL */
    info = clear_info;
    r = tld(NULL, &info);
    if(r != TLD_RESULT_NULL)
    {
        fprintf(stderr, "error: the NULL URI did not return the TLD_RESULT_NULL result.\n");
        ++err_count;
    }
    if(memcmp(&info, &undefined_info, sizeof(info)) != 0)
    {
        fprintf(stderr, "error: the NULL URI did not return a reset info structure.\n");
        ++err_count;
    }

    /* test: "" */
    info = clear_info;
    r = tld("", &info);
    if(r != TLD_RESULT_NULL)
    {
        fprintf(stderr, "error: the \"\" URI did not return the TLD_RESULT_NULL result.\n");
        ++err_count;
    }
    if(memcmp(&info, &undefined_info, sizeof(info)) != 0)
    {
        fprintf(stderr, "error: the \"\" URI did not return a reset info structure.\n");
        ++err_count;
    }

    /* test: ".." (two periods one after another) */
    info = clear_info;
    r = tld("test..com", &info);
    if(r != TLD_RESULT_BAD_URI)
    {
        fprintf(stderr, "error: the \"test..com\" URI did not return the TLD_RESULT_BAD_URI result.\n");
        ++err_count;
    }
    if(memcmp(&info, &undefined_info, sizeof(info)) != 0)
    {
        fprintf(stderr, "error: the \"test..com\" URI did not return a reset info structure.\n");
        ++err_count;
    }

    /* test: ".." (two periods one after another) */
    info = clear_info;
    r = tld("more..test.com", &info);
    if(r != TLD_RESULT_BAD_URI)
    {
        fprintf(stderr, "error: the \"more..test.com\" URI did not return the TLD_RESULT_BAD_URI result.\n");
        ++err_count;
    }
    if(memcmp(&info, &undefined_info, sizeof(info)) != 0)
    {
        fprintf(stderr, "error: the \"more..test.com\" URI did not return a reset info structure.\n");
        ++err_count;
    }

    /* test: "noperiodanywhere" (no periods anywhere) */
    info = clear_info;
    r = tld("noperiodanywhere", &info);
    if(r != TLD_RESULT_NO_TLD)
    {
        fprintf(stderr, "error: the \"noperiodanywhere\" URI did not return the TLD_RESULT_NO_TLD result.\n");
        ++err_count;
    }
    if(memcmp(&info, &undefined_info, sizeof(info)) != 0)
    {
        fprintf(stderr, "error: the \"noperiodanywhere\" URI did not return a reset info structure.\n");
        ++err_count;
    }
}


void test_tags()
{
    struct tld_info info;
    enum tld_result result;
    const char * tld_name = "info.%e6%be%b3%e9%96%80";
    int tag_count;
    struct tld_tag_definition tag_info;

    result = tld(tld_name, &info);
    if(result != TLD_RESULT_SUCCESS)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI did not return a valid info structure (error: %d).\n",
                result);
        ++err_count;
        return;
    }

    if(info.f_status != TLD_STATUS_VALID)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI did not return a VALID status as expected.\n");
        ++err_count;
        return;
    }

    if(info.f_category != TLD_CATEGORY_COUNTRY)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI did not return a country category as expected, got %d instead.\n",
                        info.f_category);
        ++err_count;
        return;
    }

    if(strcmp(info.f_country, "Macao") != 0)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI did not return \"Macao\" as the country name.\n");
        ++err_count;
        return;
    }

    if(info.f_tld != tld_name + 4)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI did not return the expected f_tld pointer.\n");
        ++err_count;
        return;
    }

    if(info.f_offset != 4)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI did not return the expected f_offset.\n");
        ++err_count;
        return;
    }

    // info.f_tld_index -- we assume this will change, there is no real point
    //                     in testing this specifically

    tag_count = tld_tag_count(&info);
    if(tag_count < 0)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI returned a negative tag_count.\n");
        ++err_count;
        return;
    }
    if(tag_count != 6)
    {
        fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI was expected to have 6 tags. Edit test if new tags were added.\n");
        ++err_count;
    }

    for(int idx = 0; idx < tag_count; ++idx)
    {
        result = tld_get_tag(&info, idx, &tag_info);
        if(result != TLD_RESULT_SUCCESS)
        {
            fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI returned an error retrieving tag %d.\n", idx);
            ++err_count;
            return;
        }

        // the order in which tags are saved in not guaranteed, so we must
        // test the name and decide what to do next
        //
        if(tag_info.f_name_length == 7
        && memcmp(tag_info.f_name, "country", 7) == 0)
        {
            if(tag_info.f_value_length != 5
            || memcmp(tag_info.f_value, "Macao", 5) != 0)
            {
                fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI \"country\" name was expected to be \"Macao\".\n");
                ++err_count;
                return;
            }
        }
        else if(tag_info.f_name_length == 17
             && memcmp(tag_info.f_name, "country_full_name", 17) == 0)
        {
            if(tag_info.f_value_length != 76
            || memcmp(tag_info.f_value, "Macao Special Administrative Region of the People's Republic of China (MSAR)", 76) != 0)
            {
                fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI \"country_full_name\" was expected to be \"Macao Special Administrative Region of the People's Republic of China (MSAR)\".\n");
                ++err_count;
                return;
            }
        }
        else if(tag_info.f_name_length == 11
             && memcmp(tag_info.f_name, "description", 11) == 0)
        {
            if(tag_info.f_value_length != 12
            || memcmp(tag_info.f_value, "Macao Island", 12) != 0)
            {
                fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI \"description\" was expected to be \"Macao Island\".\n");
                ++err_count;
                return;
            }
        }
        else if(tag_info.f_name_length == 4
             && memcmp(tag_info.f_name, "note", 4) == 0)
        {
            if(tag_info.f_value_length != 5
            || memcmp(tag_info.f_value, "Macao", 5) != 0)
            {
                fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI \"note\" was expected to be \"Macao\".\n");
                ++err_count;
                return;
            }
        }
        else if(tag_info.f_name_length == 8
             && memcmp(tag_info.f_name, "language", 8) == 0)
        {
            if(tag_info.f_value_length != 20
            || memcmp(tag_info.f_value, "Chinese, Traditional", 20) != 0)
            {
                fprintf(stderr, "error: the \"info.\xE6\xBE\xB3\xE9\x96\x80\" URI \"language\" was expected to be \"Chinese, Traditional\".\n");
                ++err_count;
                return;
            }
        }
    }
}




int main(int argc, char *argv[])
{
    fprintf(stderr, "testing tld version %s\n", tld_version());

    if(argc > 1)
    {
        if(strcmp(argv[1], "-v") == 0)
        {
            verbose = 1;
        }
        else
        {
            fprintf(stderr, "error: unknown command line option \"%s\"\n", argv[1]);
            exit(1);
        }
    }

    /* call all the tests, one by one
     * failures are "recorded" in the err_count global variable
     * and the process stops with an error message and exit(1)
     * if err_count is not zero.
     */
    load_tlds();
    test_specific();
    test_all();
    test_unknown();
    test_invalid();
    test_tags();
    free_tlds();

    if(err_count)
    {
        fprintf(stderr, "%d error%s occured.\n",
                    err_count, err_count != 1 ? "s" : "");
    }
    exit(err_count ? 1 : 0);
}

/* vim: ts=4 sw=4 et
 */
