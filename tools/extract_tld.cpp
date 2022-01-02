/* TLD tools -- TLD, domain name, and sub-domain extraction
 * Copyright (c) 2011-2022  Made to Order Software Corp.  All Rights Reserved
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

// libtld lib
//
#include    <libtld/tld_file.h>
#include    <libtld/tld.h>


// C++ lib
//
#include    <fstream>
#include    <iostream>
#include    <sstream>


// C lib
//
#include    <string.h>
#include    <limits.h>



int g_errcnt;
tld_file * g_tld_file = nullptr;

void load_tld_file(char * filename)
{
    tld_file_free(&g_tld_file);
    tld_file_error err(tld_file_load(filename, &g_tld_file));
    if(err != TLD_FILE_ERROR_NONE)
    {
        ++g_errcnt;
        std::cerr
            << "error: could not load TLD file \""
            << filename
            << "\".\n";
        return;
    }
}


bool has_tld_file()
{
    if(g_tld_file == nullptr)
    {
        ++g_errcnt;
        std::cerr
            << "error: not loaded TLD file, try --input before other options.\n";
        return false;
    }

    return true;
}


void print_string(int index, bool newline = true)
{
    if(!has_tld_file())
    {
        return;
    }

    uint32_t l;
    const char *str = tld_file_string(g_tld_file, index, &l);
    std::string s(str, l);

    std::cout << "tld_string[" << index << "] = \"" << s << "\".";
    if(newline)
    {
        std::cout << "\n";
    }
}

void print_tag(int index, bool one_line = false)
{
    if(!has_tld_file())
    {
        return;
    }

    const tld_tag *tag(tld_file_tag(g_tld_file, index));
    if(tag == nullptr)
    {
        ++g_errcnt;
        std::cerr
            << "error: tag["
            << index
            << "] not found (index too large?).\n";
        return;
    }

    std::cout << "tag[" << index << "].f_name = ";
    print_string(tag->f_tag_name, !one_line);
    if(one_line)
    {
        std::cout << "  .f_value = ";
        print_string(tag->f_tag_value);
    }
    else
    {
        std::cout << "tag[" << index << "].f_value = ";
        print_string(tag->f_tag_value);
    }
}

void print_tld(int index)
{
    if(!has_tld_file())
    {
        return;
    }

    const tld_description *tld(tld_file_description(g_tld_file, index));
    if(tld == nullptr)
    {
        ++g_errcnt;
        std::cerr
            << "error: tld["
            << index
            << "] not found (index too large?).\n";
        return;
    }

    std::cout << "tld[" << index << "].f_status = "
        << static_cast<int>(tld->f_status)
        << " ("
        << tld_status_to_string(static_cast<tld_status>(tld->f_status))
        << ")\n";

    std::cout << "tld[" << index << "].f_exception_level = "
        << static_cast<int>(tld->f_exception_level)
        << "\n";

    if(tld->f_exception_apply_to != SHRT_MAX)
    {
        std::cout << "tld[" << index << "].f_exception_apply_to =\n\n";
        print_tld(tld->f_exception_apply_to);
        std::cout << "\n";
    }

    std::cout << "tld[" << index << "].f_start_offset = "
        << tld->f_start_offset
        << "\n";

    std::cout << "tld[" << index << "].f_end_offset = "
        << tld->f_end_offset
        << "\n";

    std::cout << "tld[" << index << "].f_tld = ";
    print_string(tld->f_tld);

    for(uint16_t idx(0); idx < tld->f_tags_count; ++idx)
    {
        std::cout << "tld[" << index << "].f_tags[" << idx << "] = ";
        print_tag(tld->f_tags + idx * 2, true);
    }
}


int main(int argc, char * argv[])
{
    for(int i(1); i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "--help") == 0)
            {
                std::cout << "Usage: extract-tld <opts>\n"
                    "where <opts> is one or more of:\n"
                    "  --help | -h           print this help screen\n"
                    "  --tag | -T <index>    print tag at that index out of all the tags\n"
                    "  --tld | -t <offset>   retrieve that specific TLD\n"
                    "  --string | -s <index> print string at that index\n"
                    "  --version | -V        print out the version and exit\n"
                    "  --input <filename>    use <filename> as the TLD file to read from\n"
                    ;
                return 1;
            }
            else if(strcmp(argv[i], "-V") == 0
                 || strcmp(argv[i], "--version") == 0)
            {
                std::cout << LIBTLD_VERSION << std::endl;
                return 1;
            }
            else if(strcmp(argv[i], "-s") == 0
                 || strcmp(argv[i], "--string") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    ++g_errcnt;
                    std::cerr
                        << "error: argument missing for --string.\n";
                }
                else
                {
                    print_string(atoi(argv[i]));
                }
            }
            else if(strcmp(argv[i], "-T") == 0
                 || strcmp(argv[i], "--tag") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    ++g_errcnt;
                    std::cerr
                        << "error: argument missing for --tag.\n";
                }
                else
                {
                    print_tag(atoi(argv[i]));
                }
            }
            else if(strcmp(argv[i], "-t") == 0
                 || strcmp(argv[i], "--tld") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    ++g_errcnt;
                    std::cerr
                        << "error: argument missing for --tld.\n";
                }
                else
                {
                    print_tld(atoi(argv[i]));
                }
            }
            else if(strcmp(argv[i], "--input") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    ++g_errcnt;
                    std::cerr
                        << "error: argument missing for --input.\n";
                }
                else
                {
                    load_tld_file(argv[i]);
                }
            }
            else
            {
                ++g_errcnt;
                std::cerr
                    << "error: unknown command line option \""
                    << argv[i]
                    << "\".\n";
            }
        }
        else
        {
            ++g_errcnt;
            std::cerr
                << "error: unknown command line option \""
                << argv[i]
                << "\".\n";
        }
    }

    return g_errcnt > 0 ? 1 : 0;
}

// vim: ts=4 sw=4 et
