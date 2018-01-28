/* TLD library -- Test the TLD library by including the tld.c file.
 * Copyright (c) 2011-2018  Made to Order Software Corp.  All Rights Reserved
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

#include "tld.c"
#include "tld_data.c"
#include "tld_domain_to_lowercase.c"

#include <stdlib.h>
#include <string.h>

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
		r = cmp(d[i].a, d[i].b, d[i].n);
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
		r = cmp(d[i].a, d[i].b, d[i].n);
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
		int				f_start;
		int				f_end;
		const char *	f_tld;
		int				f_length;
		int				f_result;
	};
	struct search_info d[] = {
		/*
		 * This table is very annoying since each time the data changes
		 * it gets out of sync. On the other hand that's the best way
		 * to make sure our tests work like in the real world.
		 */

		/* get the .uk offset */
		{ 7929, 9537, "uk", 2, 9419 },

		/* get each offset of the .uk 2nd level domain */
		{ 7712, 7737, "ac", 2,							7712 },
		{ 7712, 7737, "bl", 2,							7713 },
		{ 7712, 7737, "british-library", 15,			7714 },
		{ 7712, 7737, "co", 2,							7715 },
		{ 7712, 7737, "gov", 3,							7716 },
		{ 7712, 7737, "govt", 4,						7717 },
		{ 7712, 7737, "icnet", 5,						7718 },
		{ 7712, 7737, "jet", 3,							7719 },
		{ 7712, 7737, "lea", 3,							7720 },
		{ 7712, 7737, "ltd", 3,							7721 },
		{ 7712, 7737, "me", 2,							7722 },
		{ 7712, 7737, "mil", 3,							7723 },
		{ 7712, 7737, "mod", 3,							7724 },
		{ 7712, 7737, "national-library-scotland", 25,	7725 },
		{ 7712, 7737, "nel", 3,							7726 },
		{ 7712, 7737, "net", 3,							7727 },
		{ 7712, 7737, "nhs", 3,							7728 },
		{ 7712, 7737, "nic", 3,							7729 },
		{ 7712, 7737, "nls", 3,							7730 },
		{ 7712, 7737, "org", 3,							7731 },
		{ 7712, 7737, "orgn", 4,						7732 },
		{ 7712, 7737, "parliament", 10,					7733 },
		{ 7712, 7737, "plc", 3,							7734 },
		{ 7712, 7737, "police", 6,						7735 },
		{ 7712, 7737, "sch", 3,							7736 },

		/* test with a few invalid TLDs for .uk */
		{ 7712, 7737, "com", 3, -1 },
		{ 7712, 7737, "aca", 3, -1 },
		{ 7712, 7737, "aac", 3, -1 },
		{ 7712, 7737, "ca", 2, -1 },
		{ 7712, 7737, "cn", 2, -1 },
		{ 7712, 7737, "cp", 2, -1 },
		{ 7712, 7737, "cz", 2, -1 },

		/* get the .vu offset */
		{ 7929, 9537, "vu", 2, 9466 },

		/* get the 2nd level .vu offsets */
		{ 7859, 7864, "com", 3, 7859 },
		{ 7859, 7864, "edu", 3, 7860 },
		{ 7859, 7864, "gov", 3, 7861 },
		{ 7859, 7864, "net", 3, 7862 },
		{ 7859, 7864, "org", 3, 7863 },

		/* test with a few .vu 2nd level domains that do not exist */
		{ 7859, 7864, "nom", 3, -1 },
		{ 7859, 7864, "sch", 3, -1 },

		/* verify ordering of mari, mari-el, and marine (from .ru) */
		{ 7126, 7264, "mari",    4, 7190 },
		{ 7126, 7264, "mari-el", 7, 7191 },
		{ 7126, 7264, "marine",  6, 7192 },
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
	int		i, r;

	/* now test all from the arrays */
	for(i = start; i < end; ++i)
	{
		if(verbose)
		{
			printf("{%d..%d} i = %d, [%s]\n", start, end, i, tld_descriptions[i].f_tld);
		}
		r = search(start, end, tld_descriptions[i].f_tld, strlen(tld_descriptions[i].f_tld));
		if(r != i)
		{
			fprintf(stderr, "error: test_search_array() failed with \"%s\", expected %d and got %d [4]\n",
					tld_descriptions[i].f_tld, i, r);
			++err_count;
		}
		if(tld_descriptions[i].f_start_offset != USHRT_MAX)
		{
			test_search_array(tld_descriptions[i].f_start_offset,
							  tld_descriptions[i].f_end_offset);
		}
	}
}

void test_search_all()
{
	test_search_array(tld_start_offset, tld_end_offset);
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

/* vim: ts=4 sw=4
 */
