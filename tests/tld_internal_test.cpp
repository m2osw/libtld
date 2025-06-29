/* TLD library -- Test the TLD library by including the tld.c file.
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
 * \brief Test the tld.c, tld_data.c, and tld_domain_to_lowercase.c functions.
 *
 * This file implements various tests that can directly access the internal
 * functions of the tld.c, tld_data.c, and tld_domain_to_lowercase.c
 * files.
 *
 * For that purpose we directly include those files in this test. This
 * is why the test is not actually linked against the library, it
 * includes it within itself.
 */

// libtld sources
//
#include    <libtld/tld.cpp>
#include    <libtld/tld_file.cpp>

extern "C" {
#include    <libtld/tld_data.c>
#include    <libtld/tld_domain_to_lowercase.c>
#include    <libtld/tld_strings.c>
}


// C
//
#include    <stdlib.h>
#include    <string.h>



extern "C" {


int err_count = 0;
int verbose = 0;

void test_compare()
{
    struct data
    {
        const char *a;
        const char *b;
        int n;
        int r;
    };
    struct data d[] = {
        { "uj", "uk", 2, -1 },
        { "uk", "uk", 2,  0 },
        { "ul", "uk", 2,  1 },

        { "uj", "ukmore",  2, -1 },
        { "uk", "ukstuff", 2,  0 },
        { "ul", "ukhere",  2,  1 },

        { "uk1", "ukmore",  2, 1 },
        { "uk2", "ukstuff", 2, 1 },
        { "uk3", "ukhere",  2, 1 },

        { "uk1", "uk.", 3, 1 },
        { "uk2", "uk.", 3, 1 },
        { "uk3", "uk.", 3, 1 },

        { "uk1", ".uk", 3, 1 },
        { "uk2", ".uk", 3, 1 },
        { "uk3", ".uk", 3, 1 },

        { "uk", "uk1",   3, -1 },
        { "uk", "uk22",  4, -1 },
        { "uk", "uk333", 5, -1 },

        { "uk1",   "uk", 2, 1 },
        { "uk22",  "uk", 2, 1 },
        { "uk333", "uk", 2, 1 },
    };
    int i, r, max;
    char *s, *vd, *u;

    max = sizeof(d) / sizeof(d[0]);
    for(i = 0; i < max; ++i)
    {
        r = cmp(d[i].a, strlen(d[i].a), d[i].b, d[i].n);
        if(r != d[i].r) {
            fprintf(stderr, "error: cmp() failed with \"%s\" / \"%s\", expected %d and got %d [1]\n",
                    d[i].a, d[i].b, d[i].r, r);
            ++err_count;
        }

        // create a version with uppercase and try again
        s = strdup(d[i].b);
        for(u = s; *u != '\0'; ++u)
        {
            if(*u >= 'a' && *u <= 'z')
            {
                *u &= 0x5F;
            }
        }
        vd = tld_domain_to_lowercase(s);
        r = cmp(d[i].a, strlen(d[i].a), d[i].b, d[i].n);
        if(r != d[i].r) {
            fprintf(stderr, "error: cmp() failed with \"%s\" / \"%s\", expected %d and got %d (with domain to lowercase) [2]\n",
                    d[i].a, d[i].b, d[i].r, r);
            ++err_count;
        }
        free(vd);
        free(s);
    }
}

void test_search()
{
    struct search_info
    {
        int                f_start;
        int                f_end;
        const char *    f_tld;
        int                f_length;
        int                f_result;
    };
    struct search_info d[] = {
        /*
         * This table is very annoying since each time the data changes
         * it gets out of sync. On the other hand that's the best way
         * to make sure our tests work like in the real world.
         */

        /* get the .uk offset */
        { 8857, 10481, "uk", 2, 10363 },

        /* get each offset of the .uk 2nd level domain */
        { 8608, 8636, "ac", 2,                         8608 },
        { 8608, 8636, "barsy", 5,                      8609 },
        { 8608, 8636, "co", 2,                         8610 },
        { 8608, 8636, "conn", 4,                       8611 },
        { 8608, 8636, "copro", 5,                      8612 },
        { 8608, 8636, "gov", 3,                        8613 },
        { 8608, 8636, "govt", 4,                       8614 },
        { 8608, 8636, "hosp", 4,                       8615 },
        { 8608, 8636, "icnet", 5,                      8616 },
        { 8608, 8636, "independent-commission", 22,    8617 },
        { 8608, 8636, "independent-inquest", 19,       8618 },
        { 8608, 8636, "independent-inquiry", 19,       8619 },
        { 8608, 8636, "independent-panel", 17,         8620 },
        { 8608, 8636, "independent-review", 18,        8621 },
        { 8608, 8636, "lea", 3,                        8622 },
        { 8608, 8636, "ltd", 3,                        8623 },
        { 8608, 8636, "me", 2,                         8624 },
        { 8608, 8636, "mil", 3,                        8625 },
        { 8608, 8636, "net", 3,                        8626 },
        { 8608, 8636, "nhs", 3,                        8627 },
        { 8608, 8636, "org", 3,                        8628 },
        { 8608, 8636, "orgn", 4,                       8629 },
        { 8608, 8636, "plc", 3,                        8630 },
        { 8608, 8636, "police", 6,                     8631 },
        { 8608, 8636, "public-inquiry", 14,            8632 },
        { 8608, 8636, "pymnt", 5,                      8633 },
        { 8608, 8636, "royal-commission", 16,          8634 },
        { 8608, 8636, "sch", 3,                        8635 },

        /* test with a few invalid TLDs for .uk */
        { 8608, 8635, "com", 3, -1 },
        { 8608, 8635, "aca", 3, -1 },
        { 8608, 8635, "aac", 3, -1 },
        { 8608, 8635, "bl", 2, -1 },
        { 8608, 8635, "british-library", 15, -1 },
        { 8608, 8635, "ca", 2, -1 },
        { 8608, 8635, "cn", 2, -1 },
        { 8608, 8635, "cp", 2, -1 },
        { 8608, 8635, "cz", 2, -1 },
        { 8608, 8635, "jet", 3, -1 },
        { 8608, 8635, "mod", 3, -1 },
        { 8608, 8635, "national-library-scotland", 25, -1 },
        { 8608, 8635, "nel", 3, -1 },
        { 8608, 8635, "nic", 3, -1 },
        { 8608, 8635, "nls", 3, -1 },
        { 8608, 8635, "parliament", 10, -1 },
        { 8608, 8635, "school", 2, -1 },

        /* get the .vu offset */
        { 8857, 10481, "vu", 2, 10410 },

        /* get the 2nd level .vu offsets */
        { 8769, 8777, "blog", 4, 8769 },
        { 8769, 8777, "cn", 2,   8770 },
        { 8769, 8777, "com", 3,  8771 },
        { 8769, 8777, "dev", 3,  8772 },
        { 8769, 8777, "edu", 3,  8773 },
        { 8769, 8777, "me", 2,   8774 },
        { 8769, 8777, "net", 3,  8775 },
        { 8769, 8777, "org", 3,  8776 },

        /* test with a few .vu 2nd level domains that do not exist */
        { 8769, 8777, "gov", 3, -1 },
        { 8769, 8777, "nom", 3, -1 },
        { 8769, 8777, "sch", 3, -1 },
        { 8769, 8777, "zero", 4, -1 },

        /* verify ordering of mari, mari-el, and marine (from .ru) */
        { 7925, 8075, "mari",    4, 7992 },
        { 7925, 8075, "mari-el", 7, 7993 },
        { 7925, 8075, "marine",  6, 7994 },
    };

    size_t i;

    size_t const max = sizeof(d) / sizeof(d[0]);
    for(i = 0; i < max; ++i)
    {
        int const r = search(d[i].f_start, d[i].f_end, d[i].f_tld, d[i].f_length);
        if(r != d[i].f_result)
        {
            fprintf(stderr, "error: test_search() failed with \"%s\", expected %d and got %d [3]\n",
                    d[i].f_tld, d[i].f_result, r);
            ++err_count;
        }
    }
}


void test_search_array(int start, int end)
{
    const struct tld_description *  tld(nullptr);
    int                             i(0), r(0);
    uint32_t                        l(0);
    const char *                    name(nullptr);

    /* now test all from the arrays */
    for(i = start; i < end; ++i)
    {
        tld = tld_file_description(g_tld_file, i);
        name = tld_file_string(g_tld_file, tld->f_tld, &l);
        if(verbose)
        {
            printf("{%d..%d} i = %d, [%.*s]\n", start, end, i, l, name);
        }
        r = search(start, end, name, l);
        if(r != i)
        {
            fprintf(stderr, "error: test_search_array() failed with \"%.*s\", expected %d and got %d [4]\n",
                    l, name, i, r);
            ++err_count;
        }
        if(tld->f_start_offset != USHRT_MAX)
        {
            test_search_array(tld->f_start_offset, tld->f_end_offset);
        }
    }
}

void test_search_all()
{
    test_search_array(
              g_tld_file->f_header->f_tld_start_offset
            , g_tld_file->f_header->f_tld_end_offset);
}


int main(int argc, char *argv[])
{
    fprintf(stderr, "testing internal tld version %s\n", tld_version());

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
    test_compare();
    test_search();
    test_search_all();

    if(err_count)
    {
        fprintf(stderr, "%d error%s occured.\n",
                    err_count, err_count != 1 ? "s" : "");
    }
    exit(err_count ? 1 : 0);
}

} // extern "C"

// vim: ts=4 sw=4 et
