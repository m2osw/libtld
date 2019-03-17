/* TLD library -- Test the TLD library by including the tld.c file.
 * Copyright (c) 2011-2019  Made to Order Software Corp.  All Rights Reserved
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
		{ 8117, 9731, "uk", 2, 9613 },

		/* get each offset of the .uk 2nd level domain */
		{ 7895, 7921, "ac", 2,							7895 },
		{ 7895, 7921, "barsy", 5,						7896 },
		{ 7895, 7921, "bl", 2,							7897 },
		{ 7895, 7921, "british-library", 15,			7898 },
		{ 7895, 7921, "co", 2,							7899 },
		{ 7895, 7921, "gov", 3,							7900 },
		{ 7895, 7921, "govt", 4,						7901 },
		{ 7895, 7921, "icnet", 5,						7902 },
		{ 7895, 7921, "jet", 3,							7903 },
		{ 7895, 7921, "lea", 3,							7904 },
		{ 7895, 7921, "ltd", 3,							7905 },
		{ 7895, 7921, "me", 2,							7906 },
		{ 7895, 7921, "mil", 3,							7907 },
		{ 7895, 7921, "mod", 3,							7908 },
		{ 7895, 7921, "national-library-scotland", 25,	7909 },
		{ 7895, 7921, "nel", 3,							7910 },
		{ 7895, 7921, "net", 3,							7911 },
		{ 7895, 7921, "nhs", 3,							7912 },
		{ 7895, 7921, "nic", 3,							7913 },
		{ 7895, 7921, "nls", 3,							7914 },
		{ 7895, 7921, "org", 3,							7915 },
		{ 7895, 7921, "orgn", 4,						7916 },
		{ 7895, 7921, "parliament", 10,					7917 },
		{ 7895, 7921, "plc", 3,							7918 },
		{ 7895, 7921, "police", 6,						7919 },
		{ 7895, 7921, "sch", 3,							7920 },

		/* test with a few invalid TLDs for .uk */
		{ 7895, 7921, "com", 3, -1 },
		{ 7895, 7921, "aca", 3, -1 },
		{ 7895, 7921, "aac", 3, -1 },
		{ 7895, 7921, "ca", 2, -1 },
		{ 7895, 7921, "cn", 2, -1 },
		{ 7895, 7921, "cp", 2, -1 },
		{ 7895, 7921, "cz", 2, -1 },

		/* get the .vu offset */
		{ 8117, 9731, "vu", 2, 9660 },

		/* get the 2nd level .vu offsets */
		{ 8044, 8049, "com", 3, 8044 },
		{ 8044, 8049, "edu", 3, 8045 },
		{ 8044, 8049, "gov", 3, 8046 },
		{ 8044, 8049, "net", 3, 8047 },
		{ 8044, 8049, "org", 3, 8048 },

		/* test with a few .vu 2nd level domains that do not exist */
		{ 8044, 8049, "nom", 3, -1 },
		{ 8044, 8049, "sch", 3, -1 },

		/* verify ordering of mari, mari-el, and marine (from .ru) */
		{ 7297, 7437, "mari",    4, 7361 },
		{ 7297, 7437, "mari-el", 7, 7362 },
		{ 7297, 7437, "marine",  6, 7363 },
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
