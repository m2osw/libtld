/* TLD library -- Test the TLD library by including the tld.c file.
 * Copyright (c) 2011-2025  Made to Order Software Corp.  All Rights Reserved
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
        { 10339, 11965, "uk", 2, 11847 },

        /* get each offset of the .uk 2nd level domain */
        { 10013, 10043, "ac", 2,                         10013 },
        { 10013, 10043, "barsy", 5,                      10014 },
        { 10013, 10043, "co", 2,                         10015 },
        { 10013, 10043, "conn", 4,                       10016 },
        { 10013, 10043, "copro", 5,                      10017 },
        { 10013, 10043, "gov", 3,                        10018 },
        { 10013, 10043, "govt", 4,                       10019 },
        { 10013, 10043, "hosp", 4,                       10020 },
        { 10013, 10043, "icnet", 5,                      10021 },
        { 10013, 10043, "independent-commission", 22,    10022 },
        { 10013, 10043, "independent-inquest", 19,       10023 },
        { 10013, 10043, "independent-inquiry", 19,       10024 },
        { 10013, 10043, "independent-panel", 17,         10025 },
        { 10013, 10043, "independent-review", 18,        10026 },
        { 10013, 10043, "lea", 3,                        10027 },
        { 10013, 10043, "ltd", 3,                        10028 },
        { 10013, 10043, "me", 2,                         10029 },
        { 10013, 10043, "mil", 3,                        10030 },
        { 10013, 10043, "net", 3,                        10031 },
        { 10013, 10043, "nhs", 3,                        10032 },
        { 10013, 10043, "nimsite", 7,                    10033 },
        { 10013, 10043, "oraclegovcloudapps", 18,        10034 },
        { 10013, 10043, "org", 3,                        10035 },
        { 10013, 10043, "orgn", 4,                       10036 },
        { 10013, 10043, "plc", 3,                        10037 },
        { 10013, 10043, "police", 6,                     10038 },
        { 10013, 10043, "public-inquiry", 14,            10039 },
        { 10013, 10043, "pymnt", 5,                      10040 },
        { 10013, 10043, "royal-commission", 16,          10041 },
        { 10013, 10043, "sch", 3,                        10042 },

        /* test with a few invalid TLDs for .uk */
        { 10022, 10052, "com", 3, -1 },
        { 10022, 10052, "aca", 3, -1 },
        { 10022, 10052, "aac", 3, -1 },
        { 10022, 10052, "bl", 2, -1 },
        { 10022, 10052, "british-library", 15, -1 },
        { 10022, 10052, "ca", 2, -1 },
        { 10022, 10052, "cn", 2, -1 },
        { 10022, 10052, "cp", 2, -1 },
        { 10022, 10052, "cz", 2, -1 },
        { 10022, 10052, "jet", 3, -1 },
        { 10022, 10052, "mod", 3, -1 },
        { 10022, 10052, "national-library-scotland", 25, -1 },
        { 10022, 10052, "nel", 3, -1 },
        { 10022, 10052, "nic", 3, -1 },
        { 10022, 10052, "nls", 3, -1 },
        { 10022, 10052, "parliament", 10, -1 },
        { 10022, 10052, "school", 2, -1 },

        /* get the .vu offset */
        { 10339, 11965, "vu", 2, 11894 },

        /* get the 2nd level .vu offsets */
        { 10249, 10257, "blog", 4, 10249 },
        { 10249, 10257, "cn", 2,   10250 },
        { 10249, 10257, "com", 3,  10251 },
        { 10249, 10257, "dev", 3,  10252 },
        { 10249, 10257, "edu", 3,  10253 },
        { 10249, 10257, "me", 2,   10254 },
        { 10249, 10257, "net", 3,  10255 },
        { 10249, 10257, "org", 3,  10256 },

        /* test with a few .vu 2nd level domains that do not exist */
        { 10249, 10257, "gov", 3, -1 },
        { 10249, 10257, "nom", 3, -1 },
        { 10249, 10257, "sch", 3, -1 },
        { 10249, 10257, "zero", 4, -1 },

        /* verify ordering of mari, mari-el, and marine (from .ru) */
        { 9282, 9432, "mari",    4, 9349 },
        { 9282, 9432, "mari-el", 7, 9350 },
        { 9282, 9432, "marine",  6, 9351 },
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
