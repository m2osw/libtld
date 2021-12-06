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

/** \file
 * \brief Implementation of the TLD parser library.
 *
 * This file includes all the functions available in the C library
 * of libtld that pertain to the parsing of URIs and extraction of
 * TLDs.
 */

// self
//
#include    "libtld/tld_compiler.h"


// C++ lib
//
#include    <fstream>
#include    <iostream>


// C lib
//
#include    <dirent.h>
#include    <string.h>
#include    <sys/stat.h>



bool tld_definition::add_tld(std::string const & tld)
{
    if((f_set & SET_TLD) != 0)
    {
        return false;
    }
    // f_set |= SET_TLD; -- reset_set_flags() sets this one

    f_tld.push_back(tld);
    f_name.clear();

    return true;
}


tld_definition::tld_t const & tld_definition::get_tld() const
{
    return f_tld;
}


std::string tld_definition::get_name() const
{
    if(f_name.empty())
    {
        for(auto const & segment : f_tld)
        {
            f_name += '.';
            f_name += segment;
        }
    }

    return f_name;
}


bool tld_definition::set_status(tld_status status)
{
    if((f_set & SET_STATUS) != 0)
    {
        return false;
    }
    f_set |= SET_STATUS;

    f_status = status;

    return true;
}


tld_status tld_definition::get_status() const
{
    return f_status;
}


bool tld_definition::set_category(tld_category category)
{
    if((f_set & SET_CATEGORY) != 0)
    {
        return false;
    }
    f_set |= SET_CATEGORY;

    f_category = category;

    return true;
}


tld_category tld_definition::get_category() const
{
    return f_category;
}


bool tld_definition::set_country(std::string const & country)
{
    if((f_set & SET_COUNTRY) != 0)
    {
        return false;
    }
    f_set |= SET_COUNTRY;

    // TODO: verify country name
    f_country = country;
    f_category = TLD_CATEGORY_COUNTRY;

    return true;
}


std::string const & tld_definition::get_country() const
{
    return f_country;
}


bool tld_definition::set_region(tld_region region)
{
    if((f_set & SET_REGION) != 0)
    {
        return false;
    }
    f_set |= SET_REGION;

    f_region = region;

    return true;
}


tld_region tld_definition::get_region() const
{
    return f_region;
}


bool tld_definition::set_nic(std::string const & nic)
{
    if((f_set & SET_NIC) != 0)
    {
        return false;
    }
    f_set |= SET_NIC;

    f_nic = nic;

    return true;
}


std::string const & tld_definition::get_nic() const
{
    return f_nic;
}


bool tld_definition::set_description(std::string const & description)
{
    if((f_set & SET_DESCRIPTION) != 0)
    {
        return false;
    }
    f_set |= SET_DESCRIPTION;

    f_description = description;

    return true;
}


std::string const & tld_definition::get_description() const
{
    return f_description;
}


bool tld_definition::set_note(std::string const & note)
{
    if((f_set & SET_NOTE) != 0)
    {
        return false;
    }
    f_set |= SET_NOTE;

    f_note = note;

    return true;
}


std::string const & tld_definition::get_note() const
{
    return f_note;
}


bool tld_definition::set_apply_to(std::string const & apply_to)
{
    if((f_set & SET_APPLY_TO) != 0)
    {
        return false;
    }
    f_set |= SET_APPLY_TO;

    f_apply_to = apply_to;

    return true;
}


std::string const & tld_definition::get_apply_to() const
{
    return f_apply_to;
}


void tld_definition::reset_set_flags()
{
    f_set = SET_TLD;
}


void tld_definition::set_named_parameter(
      std::string const & name
    , std::string const & value
    , std::string & errmsg)
{
    if(!name.empty())
    {
        switch(name[0])
        {
        case 'a':
            if(name == "apply_to")
            {
                if(!set_apply_to(value))
                {
                    errmsg = "\"apply_to\" defined a second time (\"" + value + "\").";
                }
                return;
            }
            break;

        case 'c':
            if(name == "category")
            {
                if(!value.empty())
                {
                    tld_category category(TLD_CATEGORY_UNDEFINED);
                    switch(value[0])
                    {
                    case 'b':
                        if(value == "brand")
                        {
                            category = TLD_CATEGORY_BRAND;
                        }
                        break;

                    case 'c':
                        if(value == "country")
                        {
                            category = TLD_CATEGORY_LOCATION;
                            if(!set_region(TLD_REGION_COUNTRY))
                            {
                                errmsg = "\"category\" / \"region\" defined a twice (\"" + value + "\").";
                                return;
                            }
                        }
                        break;

                    case 'e':
                        if(value == "entrepreneurial")
                        {
                            category = TLD_CATEGORY_ENTREPRENEURIAL;
                        }
                        break;

                    case 'g':
                        if(value == "group")
                        {
                            category = TLD_CATEGORY_GROUP;
                        }
                        break;

                    case 'i':
                        if(value == "international")
                        {
                            category = TLD_CATEGORY_INTERNATIONAL;
                        }
                        break;

                    case 'l':
                        if(value == "language")
                        {
                            category = TLD_CATEGORY_LANGUAGE;
                        }
                        else if(value == "location")
                        {
                            category = TLD_CATEGORY_LOCATION;
                        }
                        break;

                    case 'p':
                        if(value == "professionals")
                        {
                            category = TLD_CATEGORY_PROFESSIONALS;
                        }
                        break;

                    case 'r':
                        if(value == "region")
                        {
                            category = TLD_CATEGORY_REGION;
                        }
                        break;

                    case 't':
                        if(value == "technical")
                        {
                            category = TLD_CATEGORY_TECHNICAL;
                        }
                        break;

                    }
                    if(category != TLD_CATEGORY_UNDEFINED)
                    {
                        if(!set_category(category))
                        {
                            errmsg = "\"category\" of \""
                                   + get_name()
                                   + "\" defined a second time (\""
                                   + value
                                   + "\").";
                        }
                        return;
                    }
                }
                errmsg = "unknown \"category\": \"" + value + "\".";
                return;
            }
            if(name == "country")
            {
                if(!set_country(value))
                {
                    errmsg = "\"country\" defined a second time (\"" + value + "\").";
                }
                return;
            }
            break;

        case 'd':
            if(name == "description")
            {
                if(!set_description(value))
                {
                    errmsg = "\"description\" defined a second time (\"" + value + "\").";
                }
                return;
            }
            break;

        case 'n':
            if(name == "nic")
            {
                if(!set_nic(value))
                {
                    errmsg = "\"nic\" defined a second time (\"" + value + "\").";
                }
                return;
            }
            if(name == "note")
            {
                if(!set_note(value))
                {
                    errmsg = "\"note\" defined a second time (\"" + value + "\").";
                }
                return;
            }
            break;

        case 'r':
            if(name == "region")
            {
                if(!value.empty())
                {
                    tld_region region(TLD_REGION_UNDEFINED);
                    switch(value[0])
                    {
                    case 'a':
                        if(value == "area")
                        {
                            region = TLD_REGION_AREA;
                        }
                        break;

                    case 'c':
                        if(value == "city")
                        {
                            region = TLD_REGION_CITY;
                        }
                        else if(value == "country")
                        {
                            region = TLD_REGION_COUNTRY;
                        }
                        else if(value == "county")
                        {
                            region = TLD_REGION_COUNTY;
                        }
                        break;

                    case 'p':
                        if(value == "province")
                        {
                            region = TLD_REGION_PROVINCE;
                        }
                        break;

                    case 's':
                        if(value == "state")
                        {
                            region = TLD_REGION_STATE;
                        }
                        break;

                    case 'u':
                        if(value == "union")
                        {
                            region = TLD_REGION_UNION;
                        }
                        break;

                    }
                    if(region != TLD_REGION_UNDEFINED)
                    {
                        if(!set_region(region))
                        {
                            errmsg = "\"region\" defined a second time (\"" + value + "\").";
                        }
                        return;
                    }
                }
                errmsg = "unknown \"region\": \"" + value + "\".";
                return;
            }
            break;

        case 's':
            if(name == "status")
            {
                if(!value.empty())
                {
                    tld_status status(TLD_STATUS_UNDEFINED);
                    switch(value[0])
                    {
                    case 'd':
                        if(value == "deprecated")
                        {
                            status = TLD_STATUS_DEPRECATED;
                        }
                        break;

                    case 'e':
                        if(value == "example")
                        {
                            status = TLD_STATUS_EXAMPLE;
                        }
                        else if(value == "exception")
                        {
                            status = TLD_STATUS_EXCEPTION;
                        }
                        break;

                    case 'i':
                        if(value == "infrastructure")
                        {
                            status = TLD_STATUS_INFRASTRUCTURE;
                        }
                        break;

                    case 'p':
                        if(value == "proposed")
                        {
                            status = TLD_STATUS_PROPOSED;
                        }
                        break;

                    case 'r':
                        if(value == "reserved")
                        {
                            status = TLD_STATUS_RESERVED;
                        }
                        break;

                    case 'v':
                        if(value == "valid")
                        {
                            status = TLD_STATUS_VALID;
                        }
                        break;

                    case 'u':
                        if(value == "unused")
                        {
                            status = TLD_STATUS_UNUSED;
                        }
                        break;

                    }
                    if(status != TLD_STATUS_UNDEFINED)
                    {
                        if(!set_status(status))
                        {
                            errmsg = "\"status\" defined a second time (\"" + value + "\").";
                        }
                        return;
                    }
                }
                errmsg = "unknown \"status\": \"" + value + "\".";
                return;
            }
            break;

        }
    }

    errmsg = "unknown variable name \"" + name + "\".";
}









tld_compiler::token::token(
          std::string const & filename
        , int line
        , token_t tok
        , std::string const & value)
    : f_filename(filename)
    , f_line(line)
    , f_token(tok)
    , f_value(value)
{
}


std::string const & tld_compiler::token::get_filename() const
{
    return f_filename;
}


int tld_compiler::token::get_line() const
{
    return f_line;
}


tld_compiler::token_t tld_compiler::token::get_token() const
{
    return f_token;
}


std::string const & tld_compiler::token::get_value() const
{
    return f_value;
}









void tld_compiler::set_input_folder(std::string const & path)
{
    f_input_folder = path;
}


std::string const & tld_compiler::get_input_folder() const
{
    return f_input_folder;
}


void tld_compiler::set_output(std::string const & output)
{
    f_output = output;
}


std::string const & tld_compiler::get_output() const
{
    return f_output;
}


bool tld_compiler::compile()
{
    find_files(f_input_folder);
    if(get_errno() != 0)
    {
        return false;
    }

    process_input_files();
    if(get_errno() != 0)
    {
        return false;
    }

    return true;
}


int tld_compiler::get_errno() const
{
    return f_errno;
}


std::string const & tld_compiler::get_errmsg() const
{
    return f_errmsg;
}


int tld_compiler::get_line() const
{
    return f_line;
}


std::string const & tld_compiler::get_filename() const
{
    return f_filename;
}


void tld_compiler::find_files(std::string const & path)
{
    DIR * d = opendir(path.c_str());
    if(d == nullptr)
    {
        f_errno = errno;
        f_errmsg = "could not open directory \"" + path + "\".";
        return;
    }
    // TODO: add `d` to a smart pointer

    for(;;)
    {
        struct dirent *e(readdir(d));
        if(e == nullptr)
        {
            break;
        }

        std::string name(e->d_name);
        switch(e->d_type )
        {
        case DT_DIR:
            if(strcmp(e->d_name, ".") != 0
            && strcmp(e->d_name, "..") != 0)
            {
                find_files(path + '/' + name);
                if(get_errno() != 0)
                {
                    break;
                }
            }
            break;

        case DT_REG:
        case DT_LNK:
            if(name.length() > 4
            && strcmp(name.c_str() + name.length() - 4, ".ini") == 0)
            {
                // collect .ini files
                //
                f_input_files.push_back(path + '/' + name);
            }
            break;

        default:
            // ignore other file types
            break;

        }
    }

    closedir(d);
}


void tld_compiler::process_input_files()
{
    for(auto const & filename : f_input_files)
    {
        process_file(filename);
        if(get_errno() != 0)
        {
            return;
        }
    }
}


void tld_compiler::process_file(std::string const & filename)
{
    f_globals.clear();
    f_current_tld.clear();

    struct stat s;
    int r(stat(filename.c_str(), &s));
    if(r != 0)
    {
        f_errno = errno;
        f_errmsg = "could not get statistics about \"" + filename + "\".";
        return;
    }
    f_data.resize(s.st_size);

    {
        std::fstream in(filename);
        in.read(reinterpret_cast<char *>(f_data.data()), f_data.size());
        if(static_cast<size_t>(in.tellg()) != f_data.size())
        {
            f_errno = errno;
            f_errmsg = "could not read file \"" + filename + "\" in full.";
            return;
        }
    }

    f_pos = 0;
    f_line = 1;
    f_filename = filename;
    for(;;)
    {
        read_line();
        if(get_errno() != 0)
        {
            return;
        }
        if(f_tokens.empty())
        {
            continue;
        }
        if(f_tokens.size() == 1
        && f_tokens[0].get_token() == TOKEN_EOF)
        {
            break;
        }
        parse_line();
    }
}


void tld_compiler::read_line()
{
    f_tokens.clear();

    for(;;)
    {
        char32_t c(getc());
        switch(c)
        {
        case CHAR_ERR:
            return;

        case CHAR_EOF:
            if(f_tokens.empty())
            {
                f_tokens.emplace_back(
                      f_filename
                    , f_line
                    , TOKEN_EOF
                    , std::string());
            }
            return;

        case '\r':
            c = getc();
            if(c == CHAR_ERR)
            {
                return;
            }
            if(c != '\n')
            {
                ungetc(c);
            }
            ++f_line;
            return;

        case '\n':
            ++f_line;
            return;

        case ';':
            // "end of line" delimiter
            // additional values etc. can appear after a semicolon
            return;

        case '=':
            f_tokens.emplace_back(
                  f_filename
                , f_line
                , TOKEN_EQUAL
                , "=");
            break;

        case '.':
            f_tokens.emplace_back(
                  f_filename
                , f_line
                , TOKEN_DOT
                , ".");
            break;

        case '*':
            f_tokens.emplace_back(
                  f_filename
                , f_line
                , TOKEN_WILD_CARD
                , "*");
            break;

        case '?':
            f_tokens.emplace_back(
                  f_filename
                , f_line
                , TOKEN_EXCEPTION
                , "?");
            break;

        case '[':
            f_tokens.emplace_back(
                  f_filename
                , f_line
                , TOKEN_OPEN_SQUARE_BRACKET
                , "[");
            break;

        case ']':
            f_tokens.emplace_back(
                  f_filename
                , f_line
                , TOKEN_CLOSE_SQUARE_BRACKET
                , "]");
            break;

        case '#':
            for(;;)
            {
                c = getc();
                switch(c)
                {
                case CHAR_ERR:
                case CHAR_EOF:
                    return;

                case L'\r':
                    c = getc();
                    if(c != L'\n')
                    {
                        ungetc(c);
                    }
                    ++f_line;
                    return;

                case L'\n':
                    ++f_line;
                    return;

                }
            }
            break;

        case '"':
        case '\'':
            {
                int start_line(f_line);
                char32_t quote(c);

                std::string value;
                for(;;)
                {
                    c = getc();
                    if(c == CHAR_ERR)
                    {
                        return;
                    }
                    if(c == CHAR_EOF)
                    {
                        f_errno = EINVAL;
                        f_errmsg = "missing closing quote (";
                        f_errmsg += static_cast<char>(quote);
                        f_errmsg += ") for string.";
                        return;
                    }
                    if(c == quote)
                    {
                        break;
                    }
                    if(c == '\\')
                    {
                        c = getc();
                        if(c == CHAR_ERR)
                        {
                            return;
                        }
                        switch(c)
                        {
                        case CHAR_EOF:
                            f_errno = EINVAL;
                            f_errmsg = "missing closing quote (";
                            f_errmsg += static_cast<char>(quote);
                            f_errmsg += ") for string.";
                            return;

                        case '"':
                        case '\'':
                        case '\\':
                            break;

                        default:
                            // for anything else, keep the backslash as is
                            value += '\\';
                            break;

                        }
                    }
                    append_wc(value, c);
                }

                f_tokens.emplace_back(
                      f_filename
                    , start_line
                    , TOKEN_STRING
                    , value);
            }
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                std::string value;
                value += static_cast<char>(c);

                for(;;)
                {
                    c = getc();
                    if(c == CHAR_ERR)
                    {
                        return;
                    }
                    if(c < '0' || c > '9')
                    {
                        break;
                    }
                    value += static_cast<char>(c);
                }
                ungetc(c);

                f_tokens.emplace_back(
                      f_filename
                    , f_line
                    , TOKEN_NUMBER
                    , value);
            }
            break;

        default:
            if(is_space(c))
            {
                // ignore spaces
                break;
            }

            if((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || c == '_')
            {
                // identifier
                //
                std::string value;
                value += static_cast<char>(c);
                for(;;)
                {
                    c = getc();
                    if(c == CHAR_ERR)
                    {
                        return;
                    }
                    if((c < 'A' || c > 'Z')
                    && (c < 'a' || c > 'z')
                    && (c < '0' || c > '9')
                    && c != '_')
                    {
                        break;
                    }
                    value += static_cast<char>(c);
                }
                if(!is_space(c))
                {
                    ungetc(c);
                }

                f_tokens.emplace_back(
                      f_filename
                    , f_line
                    , TOKEN_IDENTIFIER
                    , value);
                break;
            }

            // invalid character (mainly controls)
            //
            if(c < 0x20                  // controls
            || (c >= 0x7F && c <= 0x9F)) // delete & graphic controls
            {
                f_errno = EINVAL;
                f_errmsg = "unexpected character found '";
                if(c < 0x20)
                {
                    f_errmsg += '^';
                    f_errmsg += static_cast<char>(c + '@');
                }
                else if(c == 0x7F)
                {
                    f_errmsg += "<DEL>";
                }
                else
                {
                    f_errmsg += '@';
                    f_errmsg += static_cast<char>(c - '@');
                }
                f_errmsg += "'.";
                return;
            }

            {
                // anything else represents a "word"
                //
                std::string value;
                value += static_cast<char>(c);
                for(;;)
                {
                    c = getc();
                    if(c == CHAR_ERR)
                    {
                        return;
                    }
                    if(c == CHAR_EOF
                    || is_space(c))
                    {
                        break;
                    }
                    if(c == '['
                    || c == '='
                    || c == ']')
                    {
                        ungetc(c);
                        break;
                    }
                    append_wc(value, c);
                }

                f_tokens.emplace_back(
                      f_filename
                    , f_line
                    , TOKEN_WORD
                    , value);
            }
            break;

        }
    }
}



bool tld_compiler::is_space(char32_t wc) const
{
    if(wc == '\r'
    || wc == '\n')
    {
        return false;
    }

    return iswspace(wc);
}


char32_t tld_compiler::getc()
{
    if(f_ungetc_pos > 0)
    {
        --f_ungetc_pos;
        return f_ungetc[f_ungetc_pos];
    }

    if(f_pos >= f_data.size())
    {
        return CHAR_EOF;
    }

    int c(f_data[f_pos]);
    ++f_pos;

    if(c < 0x80)
    {
        return static_cast<char32_t>(c);
    }

    char32_t wc(L'\0');
    int cnt(0);
    if(c >= 0xF0)
    {
        if(c >= 0xF8)
        {
            return CHAR_ERR;
        }
        wc = c & 0x07;
        cnt = 3;
    }
    else if(c >= 0xE0)
    {
        wc = c & 0x0F;
        cnt = 2;
    }
    else if(c >= 0xC0)
    {
        wc = c & 0x1F;
        cnt = 1;
    }
    else
    {
        return CHAR_ERR;
    }

    for(; cnt > 0; --cnt)
    {
        c = f_data[f_pos];
        if(c == '\0')
        {
            return CHAR_ERR;
        }
        if(c < 0x80 || c > 0xBF)
        {
            return CHAR_ERR;
        }
        wc = (wc << 6) | (c & 0x3F);
    }

    return wc;
}


void tld_compiler::ungetc(char32_t c)
{
    if(c == CHAR_EOF
    || c == CHAR_ERR)
    {
        return;
    }

    if(f_ungetc_pos >= std::size(f_ungetc))
    {
        throw std::logic_error("f_ungetc buffer is full");
    }

    f_ungetc[f_ungetc_pos] = c;
    ++f_ungetc_pos;
}


void tld_compiler::append_wc(std::string & value, char32_t wc)
{
    if(wc < 0x80)
    {
        value += static_cast<char>(wc);
    }
    else if(wc < 0x800)
    {
        value += static_cast<char>(((wc >> 6) & 0x1F) | 0xC0);
        value += static_cast<char>(((wc >> 0) & 0x3F) | 0x80);
    }
    else if(wc < 0x10000)
    {
        if(wc >= 0xD800 && wc <= 0xDFFF)
        {
            // surrogates
            throw std::logic_error("trying to encode a surrogate character");
        }

        value += static_cast<char>(((wc >> 12) & 0x0F) | 0xE0);
        value += static_cast<char>(((wc >>  6) & 0x3F) | 0x80);
        value += static_cast<char>(((wc >>  0) & 0x3F) | 0x80);
    }
    else if(wc < 0x110000)
    {
        value += static_cast<char>(((wc >> 18) & 0x07) | 0xF0);
        value += static_cast<char>(((wc >> 12) & 0x3F) | 0x80);
        value += static_cast<char>(((wc >>  6) & 0x3F) | 0x80);
        value += static_cast<char>(((wc >>  0) & 0x3F) | 0x80);
    }
    else if(wc != CHAR_EOF)
    {
        throw std::logic_error("trying to encode an invalid Unicode character");
    }
}


void tld_compiler::parse_line()
{
    switch(f_tokens[0].get_token())
    {
    case TOKEN_OPEN_SQUARE_BRACKET:
        // defining a new TLD
        //
        parse_tld();
        break;

    case TOKEN_IDENTIFIER:
        parse_variable();
        break;

    default:
        f_errno = EINVAL;
        f_errmsg = "invalid line, not recognized as a TLD definition nor a variable definition";
//print_tokens();
        break;

    }
}


void tld_compiler::parse_variable()
{
    if(f_tokens.size() < 2
    || f_tokens[1].get_token() != TOKEN_EQUAL)
    {
        f_errno = EINVAL;
        f_errmsg = "a variable name must be followed by an equal sign";
        return;
    }

    std::string value;
    if(f_tokens.size() > 3UL)
    {
        // we do not allow mixing words & strings in the value, so make
        // sure that if we have more than 3 tokens, none at index
        // 2+ are strings
        //
        for(std::size_t idx(2); idx < f_tokens.size(); ++idx)
        {
            if(f_tokens[idx].get_token() == TOKEN_STRING)
            {
                f_errno = EINVAL;
                f_errmsg = "a variable value cannot mix words and a string";
                return;
            }
            if(idx != 2)
            {   
                value += ' ';
            }
            value = f_tokens[idx].get_value();
        }
    }
    else if(f_tokens.size() == 3)
    {
        value = f_tokens[2].get_value();
    }

    std::string const & name(f_tokens[0].get_value());

    if(f_current_tld.empty())
    {
        if(f_globals.find(name) != f_globals.end())
        {
            f_errno = EINVAL;
            f_errmsg = "\"" + name + "\" global variable defined more than once.";
            return;
        }

        // name != "apply_to" -- I don't think that would be useful as a global
        if(name != "category"
        && name != "country"
        && name != "description"
        && name != "nic"
        && name != "note"
        && name != "status")
        {
            f_errno = EINVAL;
            f_errmsg = "variable with name \"" + name + "\" is not supported.";
            return;
        }

        f_globals[name] = value;
    }
    else
    {
        f_definitions[f_current_tld].set_named_parameter(name, value, f_errmsg);
        if(!f_errmsg.empty())
        {
            f_errno = EINVAL;
            return;
        }
    }
}


void tld_compiler::parse_tld()
{
    std::size_t const max(f_tokens.size() - 1);
    if(max < 2
    || f_tokens[max].get_token() != TOKEN_CLOSE_SQUARE_BRACKET)
    {
        f_errno = EINVAL;
        f_errmsg = "a TLD must end with a closing square bracket (]) and not be empty";
//print_tokens();
        return;
    }

    std::size_t idx(1);

    bool is_exception(false);
    if(f_tokens[idx].get_token() == TOKEN_EXCEPTION)
    {
        is_exception = true;
        ++idx;

        if(idx >= max)
        {
            f_errno = EINVAL;
            f_errmsg = "a TLD cannot just be an exception (?), a name is required";
            return;
        }
    }

    // the very first dot is optional now
    //
    if(f_tokens[idx].get_token() == TOKEN_DOT)
    {
        ++idx;

        if(idx >= max)
        {
            f_errno = EINVAL;
            f_errmsg = "a TLD cannot just be a dot (?), a name is required";
            return;
        }
    }

    tld_definition tld;

    // a TLD always starts with a dot, but we do not force the user to enter it
    //
    // TODO: keep the name separated (since we already cut it at the dots)
    //
    for(;;)
    {
        switch(f_tokens[idx].get_token())
        {
        case TOKEN_DOT:
            f_errno = EINVAL;
            f_errmsg = "a TLD cannot include two dots (.) in a raw.";
            return;

        case TOKEN_WILD_CARD:
            if(!tld.add_tld("*"))
            {
                f_errno = EINVAL;
                f_errmsg = "the TLD cannot be edited anymore (\"*\").";
                return;
            }
            ++idx;
            break;

        case TOKEN_IDENTIFIER:
        case TOKEN_WORD:
        case TOKEN_NUMBER:
            {
                std::string segment(f_tokens[idx].get_value());
                bool found_dot(false);
                ++idx;
                while(idx < max && !found_dot)
                {
                    switch(f_tokens[idx].get_token())
                    {
                    case TOKEN_IDENTIFIER:
                    case TOKEN_WORD:
                    case TOKEN_NUMBER:
                        segment += f_tokens[idx].get_value();
                        ++idx;
                        break;

                    case TOKEN_DOT:
                        found_dot = true;
                        break;

                    default:
                        f_errno = EINVAL;
                        f_errmsg = "unexpected token in a TLD (strings and special characters are not allowed).";
                        return;

                    }
                }
                if(!tld.add_tld(segment))
                {
                    f_errno = EINVAL;
                    f_errmsg = "the TLD cannot be edited anymore (\"" + segment + "\").";
                    return;
                }
            }
            break;

        default:
            f_errno = EINVAL;
            f_errmsg = "unexpected token in a TLD (strings and special characters are not allowed.)";
            return;

        }

        if(idx >= max)
        {
            break;
        }

        if(f_tokens[idx].get_token() != TOKEN_DOT)
        {
            f_errno = EINVAL;
            f_errmsg = "expected a dot (.) between TLD names";
            return;
        }
        ++idx;

        if(idx >= max)
        {
            // allow ending names with a period (optional)
            //
            break;
        }
    }

    f_current_tld = tld.get_name();
    f_definitions[f_current_tld] = tld;

    // add the globals to this definition
    //
    for(auto const & g : f_globals)
    {
        f_definitions[f_current_tld].set_named_parameter(g.first, g.second, f_errmsg);
        if(!f_errmsg.empty())
        {
            // this should not happen since the globals are defined in a map
            //
            f_errno = EINVAL;
            return;
        }
    }

    f_definitions[f_current_tld].reset_set_flags();
}


void tld_compiler::print_tokens()
{
    for(auto const & t : f_tokens)
    {
        std::cerr
            << t.get_filename()
            << ":"
            << t.get_line()
            << ": "
            << static_cast<int>(t.get_token())
            << " = \""
            << t.get_value()
            << "\"\n";

        //std::string const &     get_filename() const;
        //int                     get_line() const;
        //token_t                 get_token() const;
        //std::string const &     get_value() const;
    }
}



// vim: ts=4 sw=4 et
