/* TLD tools -- TLD, domain name, and sub-domain extraction
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

// libtld lib
//
#include    <libtld/tld_compiler.h>
#include    <libtld/tld_file.h>


// C++ lib
//
#include    <fstream>
#include    <iostream>
#include    <sstream>


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
    void            set_c_file(std::string const & c);
    void            set_verify(bool verify);
    void            set_output_json(bool verify);
    void            set_include_offsets(bool include_offsets);
    void            set_verbose(bool verbose);

    void            run();

private:
    void            verify_output(tld_compiler & c);

    int             f_errcnt = 0;
    std::string     f_input_path = std::string();
    std::string     f_output = std::string();
    std::string     f_c_file = std::string();
    bool            f_verify = false;
    bool            f_output_json = false;
    bool            f_include_offsets = false;
    bool            f_verbose = false;
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


void compiler::set_c_file(std::string const & c_file)
{
    f_c_file = c_file;
}


void compiler::set_verify(bool verify)
{
    f_verify = verify;
}


void compiler::set_output_json(bool output_json)
{
    f_output_json = output_json;
}


void compiler::set_include_offsets(bool include_offsets)
{
    f_include_offsets = include_offsets;
}


void compiler::set_verbose(bool verbose)
{
    f_verbose = verbose;
}


void compiler::run()
{
    if(f_errcnt != 0)
    {
        // command line found errors, return immediately
        return;
    }

    if(f_input_path.empty())
    {
        ++f_errcnt;
        std::cerr << "error: an input path is required.\n";
        return;
    }

    if(f_output.empty())
    {
        ++f_errcnt;
        std::cerr << "error: an output filename is required.\n";
        return;
    }

    std::cout << "Compiling TLDs from \"" << f_input_path << "\"..." << std::endl;

    tld_compiler c;
    c.set_input_folder(f_input_path);
    c.set_output(f_output);
    c.set_c_file(f_c_file);
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

    std::cout << "Number of strings:        " << c.get_string_manager().size()         << "\n";
    std::cout << "Longest string:           " << c.get_string_manager().max_length()   << "\n";
    std::cout << "Total string length:      " << c.get_string_manager().total_length() << "\n";
    std::cout << "Included strings:         " << c.get_string_manager().included_count() << " (saved length: " << c.get_string_manager().included_length() << ")\n";
    std::cout << "Mergeable strings:        " << c.get_string_manager().merged_count() << " (saved length: " << c.get_string_manager().merged_length() << ")\n";
    std::cout << "Compressed string length: " << c.get_string_manager().compressed_length() << std::endl;
    // TODO: add info about tags

    if(f_output_json)
    {
        std::string filename;
        std::string::size_type const dot(f_output.rfind('.'));
        if(dot != std::string::npos
        && dot > 0
        && f_output[dot - 1] != '/')
        {
            filename = f_output.substr(0, dot) + ".json";
        }
        else
        {
            filename = f_output + ".json";
        }
        std::ofstream out;
        out.open(filename);
        if(out.is_open())
        {
            c.output_to_json(out, f_include_offsets);
        }
        else
        {
            ++f_errcnt;
            std::cerr
                << "error: could not open JSON output file: \""
                << filename
                << "\".\n";
            return;
        }
    }

    if(f_verify)
    {
        verify_output(c);
    }
}


void compiler::verify_output(tld_compiler & c)
{
    tld_file * file(nullptr);
    auto_free_tld_file auto_free(&file);
    tld_file_error err(tld_file_load(f_output.c_str(), &file));
    if(err != TLD_FILE_ERROR_NONE)
    {
        ++f_errcnt;
        std::cerr << "error: could not load output file \""
            << f_output
            << "\" -- err: "
            << tld_file_errstr(err)
            << " ("
            << static_cast<int>(err)
            << ").\n";
        return;
    }

    // generate a JSON from what was just loaded
    // and it has to match the compiler's JSON
    //
    char * json(tld_file_to_json(file));
    if(json == nullptr)
    {
        ++f_errcnt;
        std::cerr << "error: conversion of file to JSON failed.\n";
        return;
    }
    auto_free_string auto_delete(json);

    // save the verification JSON to a file if we also saved the
    // JSON of the compiler to a file
    //
    if(f_output_json)
    {
        std::string filename;
        std::string::size_type const dot(f_output.rfind('.'));
        if(dot != std::string::npos
        && dot > 0
        && f_output[dot - 1] != '/')
        {
            filename = f_output.substr(0, dot) + "-verify.json";
        }
        else
        {
            filename = f_output + "-verify.json";
        }
        std::ofstream out;
        out.open(filename);
        if(out.is_open())
        {
            out.write(json, strlen(json));
        }
        else
        {
            ++f_errcnt;
            std::cerr
                << "error: could not open JSON output file: \""
                << filename
                << "\".\n";
            return;
        }
    }

    std::stringstream compiler_json;
    c.output_to_json(compiler_json, false);

    if(compiler_json.str() != json)
    {
        ++f_errcnt;
        std::cerr
            << "error: compiler & verification JSON differ."
            << (f_output_json
                ? " Check the two .json output files to see the differences."
                : " Try using the --output-json command line option to get the .json files to find the differences.")
            << "\n";
        return;
    }
}




void usage(char * argv0)
{
    std::string progname(argv0);
    std::string::size_type pos(progname.rfind('/'));
    if(pos != std::string::npos)
    {
        progname = progname.substr(pos + 1);
    }
    std::cout << progname << " v" << LIBTLD_VERSION << "\n";
    std::cout << "Usage: " << progname << " [--opts] [<output>]\n";
    std::cout << "Where --opts is one or more of the following:\n";
    std::cout << "    --help | -h             prints out this help screen and exit\n";
    std::cout << "    --c-file                path and filename to the \"tld_data.c\" file\n";
    std::cout << "    --include-offsets       print offset in comment in .json file\n";
    std::cout << "    --output-json           also save to a .json file\n";
    std::cout << "    --source | -s <folder>  define the source (input) folder\n";
    std::cout << "    --verify                verify loading results and compare against sources\n";
    std::cout << "    --verbose               print out more information about what is happening\n";
    std::cout << "    --version | -V          print out the version and exit\n";
    std::cout << "\n";
    std::cout << "The default source is \"/usr/share/libtld/tlds\".\n";
    std::cout << "The default output is \"/var/lib/libtld/tlds.tld\".\n";
    std::cout << progname << " will not output a C-file or JSON by default.\n";
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
            else if(strcmp(argv[i], "-V") == 0
                 || strcmp(argv[i], "--version") == 0)
            {
                std::cout << LIBTLD_VERSION << std::endl;
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
            else if(strcmp(argv[i], "--verify") == 0)
            {
                tldc.set_verify(true);
            }
            else if(strcmp(argv[i], "--c-file") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    tldc.error()
                        << "error: argument missing for --output-c-file.\n";
                }
                else
                {
                    tldc.set_c_file(argv[i]);
                }
            }
            else if(strcmp(argv[i], "--output-json") == 0)
            {
                tldc.set_output_json(true);
            }
            else if(strcmp(argv[i], "--include-offsets") == 0)
            {
                tldc.set_include_offsets(true);
            }
            else if(strcmp(argv[i], "--verbose") == 0)
            {
                tldc.set_verbose(true);
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
