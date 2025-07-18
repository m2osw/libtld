/* TLD library -- test the TLD interface against the Public Suffix List
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
 * \brief Test the domain names against the public_suffix_list.dat file.
 *
 * Mozilla maintains a file named public_suffix_list.dat which includes
 * all the domain names that are currently supported by the various
 * companies managing them, including \em private names (such as the
 * .omg.lol domain name).
 */



#include    "libtld/tld.h"

// C++
//
#include    <algorithm>
#include    <iostream>
#include    <map>
#include    <string>
#include    <vector>


// C
//
#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>



int g_err_count = 0;
int g_verbose = 0;

/*
 * This test calls the tld() function with all the TLDs as defined
 * by Mozilla to determine whether we are up to date.
 *
 * extern enum tld_result tld(const char *uri, struct tld_info *info);
 */

/* special cases which we handle differently */
std::map<std::string, std::string> g_special_cases = {
    {
        "*.bd",
        "ac.bd,com.bd,co.bd,edu.bd,gov.bd,info.bd,mil.bd,net.bd,org.bd"
    },
    {
        "*.er",
        "com.er,edu.er,gov.er,net.er,org.er"
    },
    {
        "*.ck",
        "co.ck,org.ck,edu.ck,gov.ck,net.ck,gen.ck,biz.ck,info.ck"
    },
    {
        "*.fk",
        "co.fk,org.fk,gov.fk,ac.fk,nom.fk,net.fk"
    },
    {
        "*.jm",
        "com.jm,net.jm,org.jm,edu.jm,gov.jm,mil.jm"
    },
    {
        "*.kh",
        "per.kh,com.kh,edu.kh,gov.kh,mil.kh,net.kh,org.kh"
    },
    {
        "*.mm",
        "net.mm,com.mm,edu.mm,gov.mm,mil.mm,org.mm"
    },
    {
        "*.np",
        "com.np,edu.np,gov.np,mil.np,net.np,org.np"
    },
    {
        "*.pg",
        "com.pg,net.pg,ac.pg,gov.pg,mil.pg,org.pg"
    },
};



struct tld_t
{
    std::string     f_name = std::string();
    int             f_line = 0;
};
typedef std::vector<tld_t> tld_vector_t;
tld_vector_t g_tlds;


char to_hex(int v)
{
    if(v >= 10)
    {
        return v - 10 + 'a';
    }

    return v + '0';
}


/** \brief Encode a URL.
 *
 * This function transforms the characters in a valid URI string.
 */
std::string tld_encode(const std::string& tld, int& level)
{
    std::string result;
    level = 0;

    int max(tld.length());
    const char *p = tld.data();
    for(int l = 0; l < max; ++l)
    {
        char c(p[l]);
        if(static_cast<unsigned char>(c) < 0x20)
        {
            fflush(stdout);
            fprintf(stderr, "error: controls characters (^%c) are not allowed in TLDs (%s).\n", c, p);
            exit(1);
        }
        if((c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || (c >= '0' && c <= '9')
        || c == '.' || c == '-')
        {
            // these are accepted as is; note that we already checked the
            // validty of the data w
            if(c == '.')
            {
                ++level;
            }
            result += c;
        }
        else
        {
            // add/remove as appropriate
            //
            if(c == '/' || c == ':' || c == '&')
            {
                fflush(stdout);
                fprintf(stderr, "error: character (^%c) is not allowed in TLDs.\n", c);
                exit(1);
            }
            result += '%';
            int byte(c & 255);
            if(byte < 16)
            {
                result += '0';
                result += to_hex(byte);
            }
            else
            {
                result += to_hex(byte >> 4);
                result += to_hex(byte & 15);
            }
        }
    }
    // at this time the maximum level we declared is 4 but there are cases
    // where countries defined 5 levels (which is definitively crazy!)
    // there is also one Amazon server using 7 levels
    if(level < 0 || level > 7)
    {
        fflush(stdout);
        fprintf(stderr, "error: level out of range (%d) in \"%s\"; if larger than the maximum limit, you may want to increase the limit.\n", level, p);
        exit(1);
    }

    return result;
}


/*
 * The function reads the public_suffix_list.dat file in memory.
 *
 * We call exit(1) if we find an error while reading the data.
 */
void test_load()
{
    FILE *f = fopen("public_suffix_list.dat", "r");
    if(f == nullptr)
    {
        f = fopen("tests/public_suffix_list.dat", "r");
        if(f == nullptr)
        {
            fflush(stdout);
            fprintf(stderr, "error: could not open the \"public_suffix_list.dat\" file; did you start the test in the source directory?\n");
            exit(1);
        }
    }
    char buf[256];
    buf[sizeof(buf) - 1] = '\0';
    int line(0);
    while(fgets(buf, sizeof(buf) - 1, f) != NULL)
    {
        ++line;
        int const l = strlen(buf);
        if(l == sizeof(buf) - 1)
        {
            // the fgets() failed in this case so forget it
            fflush(stdout);
            fprintf(stderr, "public_suffix_list.dat:%d:error: line too long.\n", line);
            ++g_err_count;
        }
        else
        {
            char * start(buf);
            while(isspace(*start))
            {
                ++start;
            }
            char * end(buf + l);
            while(end > start && isspace(end[-1]))
            {
                --end;
            }
            std::string s(start, end);
            if(s.length() == 1)
            {
                // all TLDs are at least 2 characters
                fflush(stdout);
                fprintf(stderr, "public_suffix_list.dat:%d:error: a TLD must be at least two characters.\n", line);
                ++g_err_count;
            }
            else if(s.length() > 1 && s[0] != '/' && s[1] != '/')
            {
                // this is not a comment and not an empty line, that's a TLD
                //
                auto const it(g_special_cases.find(s));
                if(it != g_special_cases.cend())
                {
                    std::string const replacement(it->second);
                    std::string name;
                    for(auto c : replacement)
                    {
                        if(c == ',')
                        {
                            tld_t t;
                            t.f_name = name;
                            t.f_line = line;
                            g_tlds.push_back(t);
                            name.clear();
                        }
                        else
                        {
                            name += c;
                        }
                    }

                    if(!name.empty())
                    {
                        tld_t t;
                        t.f_name = name;
                        t.f_line = line;
                        g_tlds.push_back(t);
                    }
                }
                else
                {
                    tld_t t;
                    t.f_name = s;
                    t.f_line = line;
                    g_tlds.push_back(t);
//printf("found [%s]\n", s.c_str());
                }
            }
        }
    }
    fclose(f);
    if(g_verbose)
    {
        printf("Found %d TLDs in public_suffix_list.dat.\n", static_cast<int>(g_tlds.size()));
    }
}


/*
 * This test checks out URIs that end with an invalid TLD. This is
 * expected to return an error every single time.
 */
void test_tlds()
{
    for(tld_vector_t::const_iterator it(g_tlds.begin()); it != g_tlds.end(); ++it)
    {
        tld_info info;

        // note: it is possible for the input to have an asterisk (*) anywhere
        //       in the name, although at this time it only appears at the
        //       start and we just handle it as a special case here
        //
        if(it->f_name.at(0) == '*'
        && it->f_name.at(1) == '.')
        {
            // as is (well, without the '*'), a '*.tld' must return INVALID
            // and status UNUSED
            //
            std::string base_tld(it->f_name.substr(2));
            if(base_tld.find('.') == std::string::npos)
            {
                // at least one '.', however for one such as '*.example.com'
                // we just want the 'example.com' part, no extra '.',
                // otherwise the test itself would fail.
                //
                base_tld = "." + base_tld;
            }
            tld_result r = tld(base_tld.c_str(), &info);
            if(r != TLD_RESULT_INVALID)
            {
                // we're good if invalid since that's what we expect in this
                // case (i.e. the "*" must be satisfied)
                //
                fflush(stdout);
                fprintf(stderr, "error: tld(\"%s\", &info) for \"%s\" expected %d, got %d instead.\n",
                            base_tld.c_str(),
                            it->f_name.c_str(),
                            TLD_RESULT_INVALID,
                            r);
                ++g_err_count;
            }

            // then try with a sub-name, in most cases it is invalid
            // although it can be success (it depends on whether the
            // '*' has a few specific cases or none at all)
            //
            std::string url("we-want-to-test-just-one-domain-name");
            url += it->f_name.substr(1);
            r = tld(url.c_str(), &info);
            if(r != TLD_RESULT_SUCCESS)
            {
                // this time, it had to succeed
                //
                fflush(stdout);
                fprintf(stderr,
                        "error: tld(\"%s\", &info) returned %d when 3rd or 4th level name is \"%s\" in public_suffix_list.dat and we provided that name.\n",
                        url.c_str(), r, it->f_name.c_str());
                ++g_err_count;
            }
        }
        else if(it->f_name.at(0) == '!')
        {
            std::string url;//("we-want-to-test-just-one-domain-name.");
            url += it->f_name.substr(1);
            tld_result r = tld(url.c_str(), &info);
            if(r != TLD_RESULT_SUCCESS)
            {
                // if it worked then we have a problem
                fflush(stdout);
                fprintf(stderr, "error: exception for tld(\"%s\", &info) = %d failed with an exception that should have been accepted.\n",
                        url.c_str(), r);
                ++g_err_count;
            }
        }
        else
        {
            std::string url("www.this-is-a-long-domain-name-that-should-not-make-it-in-a-tld.");
            url += it->f_name;
            int level;
            std::string uri(tld_encode(url, level));
            tld_result r = tld(uri.c_str(), &info);
            if(r == TLD_RESULT_SUCCESS || r == TLD_RESULT_INVALID)
            {
                // it succeeded, but is it the right length?
                std::string encoded_uri(tld_encode(it->f_name, level));
                if(strlen(info.f_tld) != static_cast<size_t>(encoded_uri.size() + 1))
                {
                    fflush(stdout);
                    fprintf(stderr, "error:%d: tld(\"%s\", &info) length mismatch (\"%s\", %d/%d).\n",
                            it->f_line,
                            uri.c_str(),
                            info.f_tld,
                            static_cast<int>(strlen(info.f_tld)),
                            static_cast<int>((encoded_uri.size() + 1)));
// s3-website.ap-northeast-2.amazonaws.com
std::string s(it->f_name);
fflush(stdout);
fprintf(stderr, "%d> %s [%s] {%s} -> %d ",
        r,
        it->f_name.c_str(),
        uri.c_str(),
        info.f_tld,
        static_cast<int>(s.length()));
// TODO: s is UTF-8 so we'd have to convert to char32_t if we want to do that
//for(int i(0); i < s.length(); ++i) {
//fprintf(stderr, "&#x%04X;", s.at(i).unicode());
//}
fprintf(stderr, "\n");
                    ++g_err_count;
                }
            }
            else
            {
                fflush(stdout);
                //fprintf(stderr, "error: tld(\"%s\", &info) failed.\n", it->f_name.c_str());
std::string s(it->f_name);
printf("error:%d: tld(\"%s\", &info) failed with %d [%s] -> %d ",
        it->f_line,
        it->f_name.c_str(),
        r,
        uri.c_str(),
        static_cast<int>(s.length()));
// TODO: s is UTF-8 so we'd have to convert to char32_t if we want to do that
//for(int i(0); i < s.length(); ++i) {
//printf("&#x%04X;", s.at(i).unicode());
//}
printf("\n");
                ++g_err_count;
            }
        }
    }
}


void test_tlds_flip()
{
    // now we want to compare the other way around, in other words, we
    // want to test with the domain names we have and see whether we
    // still have definitions that were removed from the public list
    // (i.e. entries that should be marked deprecated)
    //
    struct tld_enumeration_state state = {};
    struct tld_info info = {};
    for(int count(0);; ++count)
    {
        tld_result const r(tld_next_tld(&state, &info));
        switch(r)
        {
        case TLD_RESULT_NOT_FOUND:
            // test successful, we found the end
            //
            //std::cerr << "--- found " << count << " items.\n";
            return;

        case TLD_RESULT_NULL:
            ++g_err_count;
            fflush(stdout);
            fprintf(stderr, "error: tld_next_tld() received a TLD_RESULT_NULL which is an internal error.\n");
            return;

        case TLD_RESULT_NO_TLD:
            ++g_err_count;
            fflush(stdout);
            fprintf(stderr, "error: tld_next_tld() received a TLD_RESULT_NO_TLD which means the number of levels is larger than what the state structure supports.\n");
            return;

        case TLD_RESULT_BAD_URI:
            ++g_err_count;
            fflush(stdout);
            fprintf(stderr, "error: tld_next_tld() received a TLD_RESULT_BAD_URI which is an internal error (index, offset, or length overflow).\n");
            return;

        case TLD_RESULT_INVALID:
            if(g_verbose || (info.f_status != TLD_STATUS_DEPRECATED
                          && info.f_status != TLD_STATUS_UNUSED
                          && info.f_status != TLD_STATUS_RESERVED
                          && info.f_status != TLD_STATUS_PROPOSED
                          && info.f_status != TLD_STATUS_INFRASTRUCTURE
                          && info.f_status != TLD_STATUS_EXCEPTION))      // here exception means that this is not a TLD but a website exception
            {
                printf("--- INVALID: %d. [%s] with status: %s (%d)\n",
                        info.f_tld_index, info.f_tld + info.f_offset,
                        tld_status_to_string(info.f_status), info.f_status);
            }
            break;

        case TLD_RESULT_SUCCESS:
            {
                auto it(std::find_if(
                      g_tlds.begin()
                    , g_tlds.end()
                    , [info](auto const & tld)
                    {
                        return tld.f_name == info.f_tld + info.f_offset + 1;
                    }));
                if(it == g_tlds.end())
                {
                    ++g_err_count;
                    fflush(stdout);
                    fprintf(stderr, "error: tld_next_tld() found \"%s\" (index: %d, status: %s/%d) which was not found in the public_suffix_list.dat file.\n",
                                    info.f_tld + info.f_offset, info.f_tld_index,
                                    tld_status_to_string(info.f_status), info.f_status);
                }
            }
            break;

        }
    }
}




int main(int argc, char *argv[])
{
    printf("testing tld names version %s\n", tld_version());

    if(argc > 1)
    {
        if(strcmp(argv[1], "-v") == 0)
        {
            g_verbose = 1;
        }
    }

    /* call all the tests, one by one
     * failures are "recorded" in the g_err_count global variable
     * and the process stops with an error message and exit(1)
     * if g_err_count is not zero.
     */
    test_load();

    if(g_err_count == 0)
    {
        test_tlds();
    }
    if(g_err_count == 0)
    {
        test_tlds_flip();
    }

    if(g_err_count || g_verbose)
    {
        fflush(stdout);
        fprintf(stderr, "%d error%s occured.\n",
                    g_err_count, g_err_count != 1 ? "s" : "");
    }
    exit(g_err_count ? 1 : 0);
}

/* vim: ts=4 sw=4 et
 */
