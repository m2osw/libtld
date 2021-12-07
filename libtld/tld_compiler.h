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

#include "libtld/tld.h"

#ifdef __cplusplus
#include <list>
#include <map>
#include <vector>
class tld_definition
{
public:
    typedef std::vector<std::string>                tld_t;
    typedef std::map<std::string, tld_definition>   map_t;

    static constexpr std::uint32_t      SET_TLD =         0x0001;
    static constexpr std::uint32_t      SET_STATUS =      0x0002;
    static constexpr std::uint32_t      SET_CATEGORY =    0x0004;
    static constexpr std::uint32_t      SET_COUNTRY =     0x0008;
    static constexpr std::uint32_t      SET_NIC =         0x0010;
    static constexpr std::uint32_t      SET_DESCRIPTION = 0x0020;
    static constexpr std::uint32_t      SET_NOTE =        0x0040;
    static constexpr std::uint32_t      SET_APPLY_TO =    0x0080;
    static constexpr std::uint32_t      SET_REGION =      0x0100;

    bool                    add_tld(std::string const & tld, std::string & errmsg);
    tld_t const &           get_tld() const;
    std::string             get_name() const;
    bool                    set_status(tld_status status);
    tld_status              get_status() const;
    bool                    set_category(tld_category category);
    tld_category            get_category() const;
    bool                    set_country(std::string const & country);
    std::string const &     get_country() const;
    bool                    set_region(tld_region region);
    tld_region              get_region() const;
    bool                    set_nic(std::string const & nic);
    std::string const &     get_nic() const;
    bool                    set_description(std::string const & description);
    std::string const &     get_description() const;
    bool                    set_note(std::string const & note);
    std::string const &     get_note() const;
    bool                    set_apply_to(std::string const & apply_to);
    std::string const &     get_apply_to() const;
    void                    reset_set_flags();
    void                    set_named_parameter(
                                  std::string const & name
                                , std::string const & value
                                , std::string & errmsg);

private:
    int                     f_set = 0;
    tld_t                   f_tld = tld_t();
    mutable std::string     f_name = std::string();
    tld_status              f_status = TLD_STATUS_VALID;
    tld_category            f_category = TLD_CATEGORY_UNDEFINED;
    std::string             f_country = std::string();
    tld_region              f_region = TLD_REGION_UNDEFINED;
    std::string             f_nic = std::string();
    std::string             f_description = std::string();
    std::string             f_note = std::string();
    std::string             f_apply_to = std::string();
};
class tld_compiler
{
public:
    void                    set_input_folder(std::string const & path);
    std::string const &     get_input_folder() const;
    void                    set_output(std::string const & filename);
    std::string const &     get_output() const;
    bool                    compile();
    int                     get_errno() const;
    std::string const &     get_errmsg() const;
    int                     get_line() const;
    std::string const &     get_filename() const;

private:
    typedef std::vector<std::string>                paths_t;
    typedef std::vector<std::uint8_t>               data_t;
    typedef std::map<std::string, std::string>      values_t;

    static constexpr char32_t const        CHAR_ERR = static_cast<char32_t>(-2);
    static constexpr char32_t const        CHAR_EOF = static_cast<char32_t>(-1);

    enum token_t
    {
        TOKEN_EOF,
        TOKEN_STRING,
        TOKEN_IDENTIFIER,
        TOKEN_WORD,
        TOKEN_NUMBER,
        TOKEN_EQUAL,
        TOKEN_DOT,
        TOKEN_WILD_CARD,
        TOKEN_EXCEPTION,
        TOKEN_OPEN_SQUARE_BRACKET,
        TOKEN_CLOSE_SQUARE_BRACKET,
    };
    class token
    {
    public:
        typedef std::vector<token>      vector_t;

                                token(std::string const & filename
                                    , int line
                                    , token_t token
                                    , std::string const & value);

        std::string const &     get_filename() const;
        int                     get_line() const;
        token_t                 get_token() const;
        std::string const &     get_value() const;

    private:
        std::string const       f_filename;
        int const               f_line = 0;
        token_t const           f_token = TOKEN_EOF;
        std::string const       f_value = std::string();
    };

    void                    find_files(std::string const & path);
    void                    process_input_files();
    void                    process_file(std::string const & filename);
    bool                    get_backslash(char32_t & c);
    void                    read_line();
    bool                    is_space(char32_t wc) const;
    char32_t                getc();
    void                    ungetc(char32_t c);
    bool                    append_wc(std::string & value, char32_t wc);
    void                    parse_line();
    void                    parse_variable();
    void                    parse_tld();
    void                    print_tokens();

    std::string             f_input_folder = "/usr/share/libtld/tlds";
    std::string             f_output = "/var/lib/libtld/tlds.tld";
    int                     f_errno = 0;
    std::string             f_errmsg = std::string();
    paths_t                 f_input_files = paths_t();
    values_t                f_globals = values_t();
    std::string             f_current_tld = std::string();
    tld_definition::map_t   f_definitions = tld_definition::map_t();
    token::vector_t         f_tokens = token::vector_t();
    data_t                  f_data = data_t();
    std::string::size_type  f_pos = 0;
    int                     f_line = 1;
    std::string             f_filename = std::string();
    char32_t                f_ungetc[1] = {};
    std::string::size_type  f_ungetc_pos = 0;
};
#endif
/*#ifdef __cplusplus*/

/* vim: ts=4 sw=4 et
 */
