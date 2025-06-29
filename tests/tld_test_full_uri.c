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
 * \brief Test a full URI with tld_check_uri() function.
 *
 * This file implements various tests to verify that the
 * tld_check_uri() function works as expected.
 */

// libtld
//
#include    <libtld/tld.h>
#include    <libtld/tld_data.h>
#include    <libtld/tld_file.h>


// C
//
#include    <string.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <limits.h>




int err_count = 0;
int verbose = 0;



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





/// \brief Structure used to define many tests to run against the tld_check_uri() function
struct test_info
{
    /// The URI to be tested
    const char *        f_uri;
    /// The valid protocols for that test
    const char *        f_protocols;
    /// Flags used to call the tld_check_uri() function
    int                 f_flags;
    /// The expected result
    enum tld_result     f_result;
    /// The expected info values
    struct tld_info     f_info;
};

/* WARNING: the first and last protocols need to be tested for full
 *          validity of the library; if you add more, add them in the
 *          middle or update the list of entries below to include the
 *          new start/end protocols.
 */
#define PROTOCOLS "ftp,sftp,http,https,gopher"

const struct test_info test_info_entries[] =
{
    /* NULL entries */
    {
      NULL,
      PROTOCOLS,
      0,
      TLD_RESULT_NULL,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "",
      PROTOCOLS,
      0,
      TLD_RESULT_NULL,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    /* invalid entries */
    {
      "bad://www.m2osw.th/bad-protocol",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "b!d://www.m2osw.cg/bad-protocol/character",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https:/www.m2osw.sx.cn/missing/slash",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http:q/www.m2osw.lezajsk.pl/q/instead/of/slash",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https::/www.m2osw.museum.mv/two/colons",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http:///www.m2osw.rec.co/3/slashes",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://@www.m2osw.co.zm/bad?user=info",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://@www.m2osw.mil.py@/bad?user=info",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://:password1@www.m2osw.net.tn/bad?user=info",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://alexis:@www.m2osw.int.rw/bad?user=info",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://:@www.m2osw.l.lc/bad?user=info",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://www.m2osw.mil.sh:/bad?port=empty",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://www.m2osw.krasnoyarsk.ru:abc/bad?port=invalid&p=number",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "sftp://www.m2osw.cat.tt:-33/bad?port=unexpected number",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "https://www.m2osw.cat.tt/bad/variable/name?=value",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.edu.pt/bar%xx/percent",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.tjeldsund.no/con\trol/character",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.edu.pt/con%09rol/character", /* %09 = \t */
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.other.nf/space character",
      PROTOCOLS,
      VALID_URI_NO_SPACES,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.co.mp/space+character",
      PROTOCOLS,
      VALID_URI_NO_SPACES,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.bv.nl/space%20character",
      PROTOCOLS,
      VALID_URI_NO_SPACES,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.gov.pk/bad/variable/name?good=value&=at&the=beginning",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.rakpetroleum.om/bad/variable/name?good=value&at=the&=beginning",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.orange.om/missing/question/mark&good=values&here=perfect",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.orange.om/invalid%9ftoo/large",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.orange.om/invalid%9ztoo/large",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.orange.om/invalid%z9too/large",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.backspace.\010.no/forbid/control-characters",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.backspace.no/forbid/control-characters/\010/in/path/too",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.and\xF8y.no/forbid/special-characters",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.and%F8y.no/forbid/special-characters",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.and .no/forbid/spaces",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.and+.no/forbid/spaces",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.and%20.no/forbid/spaces",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.%1F.no/escape/forbidden",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://www.m2osw.no/large/characters\xF0/forbidden",
      PROTOCOLS,
      VALID_URI_ASCII_ONLY,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    {
      "http://a--really--long--domain-name--is-actually--forbidden.this--is--done-by--checking--the--length--which--is--limited--to--two--hundred--and--fifty--six--characters.note--that--the--buffer--must--include--a--null--terminator--which--means--you--really--are--limited--to--255--characters.www.m2osw.no/large/characters\xF0/forbidden",
      PROTOCOLS,
      0,
      TLD_RESULT_BAD_URI,
      { TLD_CATEGORY_UNDEFINED, TLD_STATUS_UNDEFINED, "", NULL, -1, 0 }
    },
    /* valid entries */
    {
      "https://www.m2osw.co.uk/simple",
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "United Kingdom", ".co.uk/", 17, 0 }
    },
    {
      "ftp://www.m2osw.ltd%2Euk/encoded/period", /* still encoded */
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "United Kingdom", ".ltd%2Euk/", 15, 0 }
    },
    {
      "http://www.m2osw.orange.edu.om/valid%9fcharacter",
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "Oman", ".edu.om", 23, 0 }
    },
    {
      "http://www.m2osw.orange.edu.om/valid\x9f/character",
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "Oman", ".edu.om", 23, 0 }
    },
    {
      "ftp://www.m2osw.ltd%2euk/encoded/period", /* still encoded */
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "United Kingdom", ".ltd%2euk/", 15, 0 }
    },
    {
      "http://m2osw.org.nr/encoded/period?#&=55", /* looks like an empty variable name */
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "Nauru", ".org.nr/", 12, 0 }
    },
    {
      "http://snap.m2osw.blogspot.mx:888/longer/address",
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_ENTREPRENEURIAL, TLD_STATUS_VALID, "", ".blogspot.mx:", 17, 0 }
    },
    {
      "HTTP://alexis:password1@www.m2osw.com:888/correct/uri?with=variable",
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_INTERNATIONAL, TLD_STATUS_VALID, "", ".com:", 33, 0 }
    },
    {
      "ftp://ftp.m2osw.me/freeware/download/snap/snap-1.0.tar.gz",
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "Montenegro", ".me/", 15, 0 }
    },
    {
      "gopher://news.m2osw.ca.us:71/protocols",
      PROTOCOLS,
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "United States", ".ca.us:", 19, 0 }
    },
    {
      "random://news.m2osw.edu.mm/asterisk/protocols",
      "*",
      0,
      TLD_RESULT_SUCCESS,
      { TLD_CATEGORY_COUNTRY, TLD_STATUS_VALID, "Republic of the Union of Myanmar", ".edu.mm/", 19, 0 }
    },
};
#define    test_info_entries_length (sizeof(test_info_entries) / sizeof(test_info_entries[0]))


void test_uri()
{
    struct tld_info info;
    enum tld_result result;
    size_t i;

    for(i = 0; i < test_info_entries_length; ++i)
    {
        //fprintf(stderr, "testing [%s]\n", test_info_entries[i].f_uri);
        result = tld_check_uri(test_info_entries[i].f_uri, &info, test_info_entries[i].f_protocols, test_info_entries[i].f_flags);
        if(result != test_info_entries[i].f_result)
        {
            fprintf(stderr, "error:%s: URI failed with the wrong result: %d, expected %d\n", test_info_entries[i].f_uri, result, test_info_entries[i].f_result);
            ++err_count;
        }

        if(test_info_entries[i].f_info.f_tld == NULL)
        {
            if(info.f_tld != NULL)
            {
                fprintf(stderr, "error:%s: TLD was expected to be NULL, got \"%s\" instead.\n", test_info_entries[i].f_uri, info.f_tld);
                ++err_count;
            }
        }
        else
        {
            if(info.f_tld == NULL)
            {
                fprintf(stderr, "error:%s: TLD was not expected to be NULL but \"%s\" instead.\n", test_info_entries[i].f_uri, test_info_entries[i].f_info.f_tld);
                ++err_count;
            }
            else if(strncmp(test_info_entries[i].f_info.f_tld, info.f_tld, strlen(test_info_entries[i].f_info.f_tld)) != 0)
            {
                fprintf(stderr, "error:%s: TLD was not properly extracted: \"%s\".\n", test_info_entries[i].f_uri, info.f_tld);
                ++err_count;
            }
        }

        if(info.f_category != test_info_entries[i].f_info.f_category)
        {
            fprintf(stderr, "error:%s: category was not properly extracted, got %d, expected %d.\n",
                    test_info_entries[i].f_uri, info.f_category, test_info_entries[i].f_info.f_category);
            ++err_count;
        }

        if(info.f_status != test_info_entries[i].f_info.f_status)
        {
            fprintf(stderr, "error:%s: status was not properly extracted.\n", test_info_entries[i].f_uri);
            ++err_count;
        }

        if(strcmp(info.f_country, test_info_entries[i].f_info.f_country) != 0)
        {
            fprintf(stderr, "error:%s: country was not properly extracted.\n", test_info_entries[i].f_uri);
            ++err_count;
        }

        if(info.f_offset != test_info_entries[i].f_info.f_offset)
        {
            fprintf(stderr, "error:%s: TLD offset is not the expected value (%d).\n", test_info_entries[i].f_uri, info.f_offset);
            ++err_count;
        }
    }
}









int main(int argc, char *argv[])
{
    fprintf(stderr, "testing tld full URI version %s: tld_valid_uri() function\n", tld_version());

    if(argc > 1)
    {
        if(strcmp(argv[1], "-v") == 0)
        {
            verbose = 1;
        }
    }

    /* call all the tests, one by one
     * failures are "recorded" in the err_count global variable
     * and the process stops with an error message and exit(1)
     * if err_count is not zero.
     */
    load_tlds();
    test_uri();

    if(err_count)
    {
        fprintf(stderr, "%d error%s occured.\n",
                    err_count, err_count != 1 ? "s" : "");
    }
    exit(err_count ? 1 : 0);
}

/* vim: ts=4 sw=4 et
 */
