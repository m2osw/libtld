/* TLD library -- TLD, domain name, and sub-domain extraction
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

// self
//
#include <libtld/tld_compiler.h>


// C++ lib
//
#include    <iostream>


// C lib
//
#include    <string.h>


class compiler
{
public:

    std::ostream &  error();
    int             exit_code() const;
    void            set_input_path(std::string const & path);
    void            set_output(std::string const & output);

    void            run();

private:
    int             f_errcnt = 0;
    std::string     f_input_path = std::string();
    std::string     f_output = std::string();
};


std::ostream & compiler::error()
{
    ++f_errcnt;
    return std::cerr;
}


int compiler::exit_code() const
{
    return f_errcnt == 0 ? 0 : 1;
}


void compiler::set_input_path(std::string const & path)
{
    f_input_path = path;
}


void compiler::set_output(std::string const & output)
{
    f_output = output;
}


void compiler::run()
{
    if(f_errcnt != 0)
    {
        // command line found errors, return immediately
        return;
    }

    tld_compiler c;
    c.set_input_folder(f_input_path);
    c.set_output(f_output);
    if(!c.compile())
    {
        ++f_errcnt;
        std::cerr
            << "error:"
            << c.get_filename()
            << ":"
            << c.get_line()
            << ": "
            << c.get_errmsg()
            << " (errno: "
            << c.get_errno()
            << ", "
            << strerror(c.get_errno())
            << ")\n";
        return;
    }
}



void usage(char * progname)
{
    std::cout << "Usage: " << progname << " [--opts] [<output>]\n";
    std::cout << "Where --opts is one or more of the following:\n";
    std::cout << "    --help | -h             prints out this help screen\n";
    std::cout << "    --source | -s <folder>  define the source (input) folder\n";
}


int main(int argc, char * argv[])
{
    compiler tldc;

    for(int i(1); i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "--help") == 0)
            {
                usage(argv[0]);
                return 1;
            }
            else if(strcmp(argv[i], "-s") == 0
                 || strcmp(argv[i], "--source") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    tldc.error()
                        << "error: argument missing for --source.\n";
                }
                else
                {
                    tldc.set_input_path(argv[i]);
                }
            }
            else
            {
                tldc.error()
                    << "error: unknown command line option \""
                    << argv[i]
                    << "\".\n";
            }
        }
        else
        {
            tldc.set_output(argv[i]);
        }
    }

    tldc.run();

    return tldc.exit_code();
}

// vim: ts=4 sw=4 et
