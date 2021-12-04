/* TLD library -- Test the TLD library by including the tld.c file.
 * Copyright (c) 2011-2021  Made to Order Software Corp.  All Rights Reserved
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

#include <libtld/tld.c>
#include <libtld/tld_data.c>
#include <libtld/tld_domain_to_lowercase.c>

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
		{ 8842, 10466, "uk", 2, 10348 },

		/* get each offset of the .uk 2nd level domain */
		{ 8595, 8625, "ac", 2,							8595 },
		{ 8595, 8625, "barsy", 5,						8596 },
		{ 8595, 8625, "bl", 2,							8597 },
		{ 8595, 8625, "british-library", 15,			8598 },
		{ 8595, 8625, "co", 2,							8599 },
		{ 8595, 8625, "conn", 4,						8600 },
		{ 8595, 8625, "copro", 5,						8601 },
		{ 8595, 8625, "gov", 3,							8602 },
		{ 8595, 8625, "govt", 4,						8603 },
		{ 8595, 8625, "hosp", 4,						8604 },
		{ 8595, 8625, "icnet", 5,						8605 },
		{ 8595, 8625, "jet", 3,							8606 },
		{ 8595, 8625, "lea", 3,							8607 },
		{ 8595, 8625, "ltd", 3,							8608 },
		{ 8595, 8625, "me", 2,							8609 },
		{ 8595, 8625, "mil", 3,							8610 },
		{ 8595, 8625, "mod", 3,							8611 },
		{ 8595, 8625, "national-library-scotland", 25,	8612 },
		{ 8595, 8625, "nel", 3,							8613 },
		{ 8595, 8625, "net", 3,							8614 },
		{ 8595, 8625, "nhs", 3,							8615 },
		{ 8595, 8625, "nic", 3,							8616 },
		{ 8595, 8625, "nls", 3,							8617 },
		{ 8595, 8625, "org", 3,							8618 },
		{ 8595, 8625, "orgn", 4,						8619 },
		{ 8595, 8625, "parliament", 10,					8620 },
		{ 8595, 8625, "plc", 3,							8621 },
		{ 8595, 8625, "police", 6,						8622 },
		{ 8595, 8625, "pymnt", 5,						8623 },
		{ 8595, 8625, "sch", 3,							8624 },

		/* test with a few invalid TLDs for .uk */
		{ 8595, 8625, "com", 3, -1 },
		{ 8595, 8625, "aca", 3, -1 },
		{ 8595, 8625, "aac", 3, -1 },
		{ 8595, 8625, "ca", 2, -1 },
		{ 8595, 8625, "cn", 2, -1 },
		{ 8595, 8625, "cp", 2, -1 },
		{ 8595, 8625, "cz", 2, -1 },
		{ 8595, 8625, "school", 2, -1 },

		/* get the .vu offset */
		{ 8842, 10466, "vu", 2, 10395 },

		/* get the 2nd level .vu offsets */
		{ 8755, 8764, "blog", 4, 8755 },
		{ 8755, 8764, "cn", 2,   8756 },
		{ 8755, 8764, "com", 3,  8757 },
		{ 8755, 8764, "dev", 3,  8758 },
		{ 8755, 8764, "edu", 3,  8759 },
		{ 8755, 8764, "gov", 3,  8760 },
		{ 8755, 8764, "me", 2,   8761 },
		{ 8755, 8764, "net", 3,  8762 },
		{ 8755, 8764, "org", 3,  8763 },

		/* test with a few .vu 2nd level domains that do not exist */
		{ 8755, 8764, "nom", 3, -1 },
		{ 8755, 8764, "sch", 3, -1 },
		{ 8755, 8764, "zero", 4, -1 },

		/* verify ordering of mari, mari-el, and marine (from .ru) */
		{ 7928, 8077, "mari",    4, 7994 },
		{ 7928, 8077, "mari-el", 7, 7995 },
		{ 7928, 8077, "marine",  6, 7996 },
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
