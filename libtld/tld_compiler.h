/* TLD library -- TLD, domain name, and sub-domain extraction
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

/** \file
 * \brief Implementation of the TLD parser library.
 *
 * This file includes all the functions available in the C library
 * of libtld that pertain to the parsing of URIs and extraction of
 * TLDs.
 */

// self
//
#include "libtld/tld.h"

#ifdef __cplusplus

// C++ lib
//
#include    <iostream>
#include    <list>
#include    <map>
#include    <memory>
#include    <set>
#include    <vector>

// C lib
//
#include    <limits.h>


typedef uint32_t                            string_id_t;
typedef std::map<string_id_t, string_id_t>  tags_t;
typedef uint32_t                            tag_id_t;

constexpr string_id_t       STRING_ID_NULL = 0;

class tld_string
{
public:
    typedef std::shared_ptr<tld_string>             pointer_t;
    typedef std::map<std::string, pointer_t>        map_by_string_t;
    typedef std::map<string_id_t, pointer_t>        map_by_id_t;

                            tld_string(string_id_t id, std::string const & s);

    string_id_t             get_id() const;
    std::string const &     get_string() const;
    std::string::size_type  length() const;
    void                    set_found_in(string_id_t id);
    string_id_t             get_found_in() const;

private:
    string_id_t             f_id = STRING_ID_NULL;
    std::string             f_string = std::string();
    string_id_t             f_found_in = STRING_ID_NULL;
};


class tld_string_manager
{
public:
    string_id_t                 add_string(std::string const & s);
    string_id_t                 find_string(std::string const & s);
    std::string                 get_string(string_id_t id) const;
    string_id_t                 get_next_string_id() const;
    std::size_t                 size() const;
    std::size_t                 max_length() const;
    std::size_t                 total_length() const;
    std::string const &         compressed_strings() const;
    std::size_t                 compressed_length() const;
    void                        merge_strings();
    std::size_t                 included_count() const;
    std::size_t                 included_length() const;
    std::size_t                 merged_count() const;
    std::size_t                 merged_length() const;
    std::size_t                 get_string_offset(std::string const & s) const;
    std::size_t                 get_string_offset(string_id_t id) const;

private:
    typedef std::set<string_id_t>   set_id_t;

    std::string::size_type      end_start_match(
                                      std::string const & s1
                                    , std::string const & s2);
    bool                        merge_two_strings();

    string_id_t                 f_next_id = STRING_ID_NULL;
    tld_string::map_by_string_t f_strings_by_string = tld_string::map_by_string_t();
    tld_string::map_by_id_t     f_strings_by_id = tld_string::map_by_id_t();
    set_id_t                    f_strings_reviewed = set_id_t();
    std::size_t                 f_max_length = 0;
    std::size_t                 f_total_length = 0;
    std::size_t                 f_included_count = 0;
    std::size_t                 f_included_length = 0;
    std::size_t                 f_merged_count = 0;
    std::size_t                 f_merged_length = 0;
    std::string                 f_merged_strings = std::string();
};


class tld_tag_manager
{
public:
    typedef std::vector<string_id_t>    tags_table_t;

    void                        add(tags_t const & tags);
    void                        merge();
    tags_table_t const &        merged_tags() const;
    std::size_t                 merged_size() const;
    std::size_t                 get_tag_offset(tags_t const & tags) const;

private:
    typedef std::vector<tags_table_t>   tags_vector_t;

    tags_table_t                tags_to_table(tags_t const & tags) const;
    std::size_t                 end_start_match(
                                      tags_table_t const & s1
                                    , tags_table_t const & s2);

    tags_vector_t               f_tags = tags_vector_t();
    tags_table_t                f_merged_tags = tags_table_t();
};


class tld_definition
{
public:
    typedef std::shared_ptr<tld_definition>         pointer_t;
    typedef std::vector<string_id_t>                segments_t;
    typedef std::map<std::string, pointer_t>        map_t;

    static constexpr std::uint32_t      SET_TLD =         0x0001;
    static constexpr std::uint32_t      SET_STATUS =      0x0002;
    static constexpr std::uint32_t      SET_APPLY_TO =    0x0080;

                            tld_definition(tld_definition const &) = default;
                            tld_definition(tld_string_manager & strings);

    tld_definition &        operator = (tld_definition const &);

    bool                    add_segment(std::string const & segment, std::string & errmsg);
    segments_t const &      get_segments() const;
    std::string             get_name() const;
    std::string             get_inverted_name() const;
    std::string             get_parent_name() const;
    std::string             get_parent_inverted_name() const;

    void                    set_index(int idx);
    int                     get_index() const;

    bool                    set_status(tld_status status);
    tld_status              get_status() const;

    bool                    set_apply_to(std::string const & apply_to);
    std::string             get_apply_to() const;

    void                    add_tag(
                                  std::string const & tag_name
                                , std::string const & value
                                , std::string & errmsg);
    tags_t const &          get_tags() const;

    void                    reset_set_flags();
    void                    set_named_parameter(
                                  std::string const & name
                                , std::string const & value
                                , std::string & errmsg);

    void                    set_start_offset(uint16_t start);
    void                    set_end_offset(uint16_t end);
    uint16_t                get_start_offset() const;
    uint16_t                get_end_offset() const;

private:
    tld_string_manager &    f_strings;

    int                     f_set = 0;
    segments_t              f_tld = segments_t();
    int                     f_index = 0;
    tld_status              f_status = TLD_STATUS_VALID;
    std::string             f_apply_to = std::string();

    tags_t                  f_tags = tags_t();

    uint16_t                f_start_offset = USHRT_MAX;
    uint16_t                f_end_offset = USHRT_MAX;
};


class tld_compiler
{
public:
    void                    set_input_folder(std::string const & path);
    std::string const &     get_input_folder() const;
    void                    set_output(std::string const & filename);
    std::string const &     get_output() const;
    void                    set_c_file(std::string const & filename);
    std::string const &     get_c_file() const;
    bool                    compile();
    int                     get_errno() const;
    std::string const &     get_errmsg() const;
    int                     get_line() const;
    std::string const &     get_filename() const;
    tld_string_manager &    get_string_manager();
    void                    output_to_json(std::ostream & out, bool verbose) const;

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
    void                    define_default_category();
    void                    find_max_level();
    void                    compress_tags();
    uint16_t                find_definition(std::string name) const;
    void                    output_tlds(std::ostream & out);
    void                    save_to_file(std::string const & buffer);
    void                    output_header(std::ostream & out);
    void                    save_to_c_file(std::string const & buffer);

    std::string             f_input_folder = "/usr/share/libtld/tlds";
    std::string             f_output = "/var/lib/libtld/tlds.tld";
    std::string             f_c_file = std::string();
    int                     f_errno = 0;
    std::string             f_errmsg = std::string();
    paths_t                 f_input_files = paths_t();
    values_t                f_global_variables = values_t();
    values_t                f_global_tags = values_t();
    std::string             f_current_tld = std::string();
    tld_definition::map_t   f_definitions = tld_definition::map_t();
    token::vector_t         f_tokens = token::vector_t();
    data_t                  f_data = data_t();
    std::string::size_type  f_pos = 0;
    int                     f_line = 1;
    std::string             f_filename = std::string();
    char32_t                f_ungetc[1] = {};
    std::string::size_type  f_ungetc_pos = 0;
    tld_string_manager      f_strings = tld_string_manager();
    string_id_t             f_strings_count = 0;
    tld_tag_manager         f_tags = tld_tag_manager();
    time_t                  f_created_on = time(nullptr);
    uint8_t                 f_tld_max_level = 0;
    uint16_t                f_tld_start_offset = USHRT_MAX;
    uint16_t                f_tld_end_offset = USHRT_MAX;
};
#endif
/*#ifdef __cplusplus*/

/* vim: ts=4 sw=4 et
 */
