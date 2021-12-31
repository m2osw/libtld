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
#include    "libtld/tld_file.h"


// C++ lib
//
#include    <algorithm>
#include    <fstream>
#include    <iostream>
#include    <sstream>
#include    <iomanip>


// C lib
//
#include    <dirent.h>
#include    <string.h>
#include    <sys/stat.h>





tld_string::tld_string(string_id_t id, std::string const & s)
    : f_id(id)
    , f_string(s)
{
}


string_id_t tld_string::get_id() const
{
    return f_id;
}


std::string const & tld_string::get_string() const
{
    return f_string;
}


std::string::size_type tld_string::length() const
{
    return f_string.length();
}


void tld_string::set_found_in(string_id_t id)
{
    f_found_in = id;
}                   


string_id_t tld_string::get_found_in() const
{
    return f_found_in;
}










string_id_t tld_string_manager::add_string(std::string const & s)
{
    string_id_t id(find_string(s));

    if(id == STRING_ID_NULL)
    {
        id = ++f_next_id;
        tld_string::pointer_t str(std::make_shared<tld_string>(id, s));
        f_strings_by_string[s] = str;
        f_strings_by_id[id] = str;

        f_total_length += s.length();
        if(s.length() > f_max_length)
        {
            f_max_length = s.length();
        }
    }

    return id;
}


string_id_t tld_string_manager::find_string(std::string const & s)
{
    auto it(f_strings_by_string.find(s));
    if(it == f_strings_by_string.end())
    {
        return STRING_ID_NULL;
    }

    return it->second->get_id();
}


std::string tld_string_manager::get_string(string_id_t id) const
{
    auto it(f_strings_by_id.find(id));
    if(it == f_strings_by_id.end())
    {
        return std::string();
    }
    return it->second->get_string();
}


string_id_t tld_string_manager::get_next_string_id() const
{
    return f_next_id;
}


std::size_t tld_string_manager::size() const
{
    return f_strings_by_id.size();
}


std::size_t tld_string_manager::max_length() const
{
    return f_max_length;
}


std::size_t tld_string_manager::total_length() const
{
    return f_total_length;
}


std::string const & tld_string_manager::compressed_strings() const
{
    return f_merged_strings;
}


std::size_t tld_string_manager::compressed_length() const
{
    return f_merged_strings.length();
}


std::string::size_type tld_string_manager::end_start_match(std::string const & s1, std::string const & s2)
{
    char const *c1(s1.c_str() + s1.length());
    char const *c2(s2.c_str());
    for(std::string::size_type max(std::min(s1.length(), s2.length()) - 1);
        max > 0;
        --max)
    {
        if(strncmp(c1 - max, c2, max) == 0)
        {
            return max;
        }
    }
    return 0;
}


void tld_string_manager::merge_strings()
{
    // we want to save all the strings as P-strings (a.k.a. "Pascal" strings)
    // with the size of the string inside our table; as a result, this means
    // all our strings can be merged in one superstring (i.e. no '\0' at all)
    //
    // (i.e. the implementation of the tld library makes use of a length in
    // various places, so having the length pre-computed allows us to avoid
    // an strlen() call each time we need it)

    // first we check for strings fully included in another string; those
    // do not need any special handling so we eliminate them first
    //
//std::cout << "info: find included strings" << std::endl;
    for(auto & s1 : f_strings_by_id)
    {
        for(auto & s2 : f_strings_by_id)
        {
            if(s1.first != s2.first
            && s2.second->get_found_in() == STRING_ID_NULL
            && s1.second->length() > s2.second->length()
            && s1.second->get_string().find(s2.second->get_string()) != std::string::npos)
            {
                s2.second->set_found_in(s1.first);
                ++f_included_count;
                f_included_length += s2.second->length();
                break;
            }
        }
    }

    // at this time I implemented a simplified superstring implementation;
    // I just look for the longest merge between two strings and use that
    // then move on to the next string; it's probably 50% correct already
    //
    // note: at the time I tested this one, I saved just under 2Kb so I
    // don't want to sweat it too much either, that said, with all the
    // compression, we save 2/3rd of the space (at the moment, a little
    // under 50Kb final instead of over 150Kb without any compression)
    //
//std::cout << "info: find mergeable strings" << std::endl;
    while(merge_two_strings()); // TODO: This is dead slow...

    // now we have all the strings merged (or not if not possible)
    // create one big resulting string of the result
    //
//std::cout << "info: generate final super-string" << std::endl;
    for(auto s : f_strings_by_id)
    {
        if(s.second->get_found_in() == STRING_ID_NULL)
        {
            f_merged_strings += s.second->get_string();
        }
    }
//std::cout << "final super-string: ["
//    << f_merged_strings
//    << "] length="
//    << f_merged_strings.length()
//    << std::endl;
}


bool tld_string_manager::merge_two_strings()
{
    string_id_t id1(STRING_ID_NULL);
    string_id_t id2(STRING_ID_NULL);
    std::string::size_type best(0);
    for(auto & s1 : f_strings_by_id)
    {
        if(s1.second->get_found_in() == STRING_ID_NULL
        && f_strings_reviewed.find(s1.first) == f_strings_reviewed.end())
        {
            for(auto & s2 : f_strings_by_id)
            {
                if(s1.first != s2.first
                && s2.second->get_found_in() == STRING_ID_NULL)
                {
                    std::string const & str1(s1.second->get_string());
                    std::string const & str2(s2.second->get_string());
                    std::string::size_type const d(end_start_match(str1, str2));

                    if(d > best)
                    {
                        best = d;
                        id1 = s1.first;
                        id2 = s2.first;
                    }
                }
            }
            f_strings_reviewed.insert(s1.first);
        }
    }

    if(best > 0)
    {
        std::string const & str1(f_strings_by_id[id1]->get_string());
        std::string const & str2(f_strings_by_id[id2]->get_string());

        std::string const merged(str1 + str2.substr(best));
#if 0
std::cout << "\n"
<< "Found " << best
<< ": [" << str1
<< "] vs [" << str2
<< "] -> [" << merged
<< "]" << std::endl;
#endif

        string_id_t merged_id(add_string(merged));

        f_strings_by_id[id1]->set_found_in(merged_id);
        f_strings_by_id[id2]->set_found_in(merged_id);

        ++f_merged_count;
        f_merged_length += best;
        return true;
    }

    // no merge happened
    //
    return false;
}


std::size_t tld_string_manager::included_count() const
{
    return f_included_count;
}


std::size_t tld_string_manager::included_length() const
{
    return f_included_length;
}


std::size_t tld_string_manager::merged_count() const
{
    return f_merged_count;
}


std::size_t tld_string_manager::merged_length() const
{
    return f_merged_length;
}


std::size_t tld_string_manager::get_string_offset(std::string const & s) const
{
    return f_merged_strings.find(s);
}


std::size_t tld_string_manager::get_string_offset(string_id_t id) const
{
    auto it(f_strings_by_id.find(id));
    if(it == f_strings_by_id.end())
    {
        return std::string::npos;
    }

    return get_string_offset(it->second->get_string());
}








void tld_tag_manager::add(tags_t const & tags)
{
    // transform the tags in an array as we will save in the output
    //
    tags_table_t const table(tags_to_table(tags));

    // if another description has the exact same tags, do not duplicate
    //
    for(auto const & it : f_tags)
    {
        if(it == table)
        {
            return;
        }
    }

    // save the result in the vector if not found
    //
    f_tags.push_back(table);
}


void tld_tag_manager::merge()
{
    std::set<int> processed_tags;
    std::set<int> processed_intermediates;
    std::set<int> unhandled_tags;
    std::set<int> unhandled_intermediates;
    tags_vector_t intermediate_tags;

    for(auto t1(f_tags.begin()); t1 != f_tags.end(); ++t1)
    {
        processed_tags.insert(std::distance(f_tags.begin(), t1));

        auto best_match(f_tags.end());
        auto best_intermediate_match(intermediate_tags.end());
        std::size_t best(0);
        std::size_t best_swapped(0);

        // check against other unmerged tags
        //
        for(auto t2(f_tags.begin()); t2 != f_tags.end(); ++t2)
        {
            if(processed_tags.find(std::distance(f_tags.begin(), t2)) != processed_tags.end())
            {
                // this was already used up, ignore
                continue;
            }

            std::size_t const d1(end_start_match(*t1, *t2));
            std::size_t const d2(end_start_match(*t2, *t1));
            if(d2 > d1)
            {
                if(d2 > best_swapped)
                {
                    best_swapped = d2;
                    best_match = t2;
                }
            }
            else
            {
                if(d1 > best)
                {
                    best = d1;
                    best_match = t2;
                }
            }
        }

        // check against already merged tags
        //
        for(auto ti(intermediate_tags.begin()); ti != intermediate_tags.end(); ++ti)
        {
            if(processed_intermediates.find(std::distance(intermediate_tags.begin(), ti)) != processed_intermediates.end())
            {
                // TBD: I may just want to remove those used up intermediates
                //      and I think I don't need this test at all
                continue;
            }

            std::size_t const d1(end_start_match(*t1, *ti));
            std::size_t const d2(end_start_match(*ti, *t1));
            if(d2 > d1
            && d2 > best_swapped)
            {
                best_swapped = d2;
                best_intermediate_match = ti;
            }
            else if(d1 > best)
            {
                best = d1;
                best_intermediate_match = ti;
            }
        }

        if(best_intermediate_match != intermediate_tags.end())
        {
            if(best_swapped > best)
            {
                tags_table_t merged(*best_intermediate_match);
                merged.insert(
                          merged.end()
                        , t1->begin() + best_swapped
                        , t1->end());
                intermediate_tags.push_back(merged);
            }
            else
            {
                tags_table_t merged(*t1);
                merged.insert(
                          merged.end()
                        , best_intermediate_match->begin() + best
                        , best_intermediate_match->end());
                intermediate_tags.push_back(merged);
            }

            processed_intermediates.insert(std::distance(intermediate_tags.begin(), best_intermediate_match));
        }
        else if(best_match != f_tags.end())
        {
            // we found a best match meaning that we can merged t1 & t2 a bit
            //
            if(best_swapped > best)
            {
                tags_table_t merged(*best_match);
                merged.insert(
                          merged.end()
                        , t1->begin() + best_swapped
                        , t1->end());
                intermediate_tags.push_back(merged);
            }
            else
            {
                tags_table_t merged(*t1);
                merged.insert(
                          merged.end()
                        , best_match->begin() + best
                        , best_match->end());
                intermediate_tags.push_back(merged);
            }

            processed_tags.insert(std::distance(f_tags.begin(), best_match));
        }
        else
        {
            // no merging possible, keep item as is for final
            //
            unhandled_tags.insert(std::distance(f_tags.begin(), t1));
        }
    }

    // repeat with the intermediate (which is unlikely to generate much
    // more merging, but we never know...)
    //
    bool repeat(false);
    do
    {
        repeat = false;

        for(std::size_t i1(0); i1 < intermediate_tags.size(); ++i1)
        {
            if(processed_intermediates.find(i1) != processed_intermediates.end())
            {
                continue;
            }

            processed_intermediates.insert(i1);

            std::size_t best_intermediate_match(static_cast<std::size_t>(-1));
            std::size_t best(0);
            std::size_t best_swapped(0);

            // check against other unmerged tags
            //
            for(std::size_t i2(0); i2 < intermediate_tags.size(); ++i2)
            {
                if(processed_intermediates.find(i2) != processed_intermediates.end())
                {
                    // this was already used up, ignore
                    continue;
                }

                std::size_t const d1(end_start_match(intermediate_tags[i1], intermediate_tags[i2]));
                std::size_t const d2(end_start_match(intermediate_tags[i2], intermediate_tags[i1]));
                if(d2 > d1
                && d2 > best_swapped)
                {
                    best_swapped = d2;
                    best_intermediate_match = i2;
                }
                else if(d1 > best)
                {
                    best = d1;
                    best_intermediate_match = i2;
                }
            }

            if(best_intermediate_match != static_cast<std::size_t>(-1))
            {
                repeat = true;

                if(best_swapped > best)
                {
                    tags_table_t merged(intermediate_tags[best_intermediate_match]);
                    merged.insert(
                              merged.end()
                            , intermediate_tags[i1].begin() + best_swapped
                            , intermediate_tags[i1].end());
                    intermediate_tags.push_back(merged);
                }
                else
                {
                    tags_table_t merged(intermediate_tags[i1]);
                    merged.insert(
                              merged.end()
                            , intermediate_tags[best_intermediate_match].begin() + best
                            , intermediate_tags[best_intermediate_match].end());
                    intermediate_tags.push_back(merged);
                }
            }
            else
            {
                // no merging possible, keep item as is for now
                //
                unhandled_intermediates.insert(i1);
            }
        }
    }
    while(repeat);

    // once done merging, we end up with a set of tables which we can
    // merge all together and any tag table can then be found in this
    // final super-table
    //
    for(auto const & idx : unhandled_tags)
    {
        f_merged_tags.insert(
                  f_merged_tags.end()
                , f_tags[idx].begin()
                , f_tags[idx].end());
    }

    for(auto const & idx : unhandled_intermediates)
    {
        f_merged_tags.insert(
                  f_merged_tags.end()
                , intermediate_tags[idx].begin()
                , intermediate_tags[idx].end());
    }
}


tld_tag_manager::tags_table_t const & tld_tag_manager::merged_tags() const
{
    return f_merged_tags;
}


std::size_t tld_tag_manager::merged_size() const
{
    return f_merged_tags.size();
}


std::size_t tld_tag_manager::get_tag_offset(tags_t const & tags) const
{
    tags_table_t const table(tags_to_table(tags));
    auto it(std::search(
            f_merged_tags.begin(), f_merged_tags.end(),
            table.begin(), table.end()));
    if(it == f_merged_tags.end())
    {
        throw std::logic_error("tags not found in the list of merged tags.");
    }

    return std::distance(f_merged_tags.begin(), it);
}


tld_tag_manager::tags_table_t tld_tag_manager::tags_to_table(tags_t const & tags) const
{
    tld_tag_manager::tags_table_t table;
    for(auto const & t : tags)
    {
        table.push_back(t.first);
        table.push_back(t.second);
    }
    return table;
}


std::size_t tld_tag_manager::end_start_match(tags_table_t const & tag1, tags_table_t const & tag2)
{
    for(std::string::size_type max(std::min(tag1.size(), tag2.size()) - 1);
        max > 0;
        --max)
    {
        if(std::equal(tag1.end() - max, tag1.end(), tag2.begin()))
        {
            return max;
        }
    }

    return 0;
}















tld_definition::tld_definition(tld_string_manager & strings)
    : f_strings(strings)
{
}


bool tld_definition::add_segment(
          std::string const & segment
        , std::string & errmsg)
{
    if((f_set & SET_TLD) != 0)
    {
        errmsg = "the TLD cannot be edited anymore (cannot add \""
               + segment
               + "\" to \""
               + get_name()
               + "\").";
        return false;
    }
    // f_set |= SET_TLD; -- reset_set_flags() sets this one

    if(segment.empty())
    {
        errmsg = "a TLD segment cannot be an empty string.";
        return false;
    }

    if(segment.front() == '-'
    || segment.back() == '-')
    {
        errmsg = "a TLD segment (\""
               + segment
               + "\") cannot start or end with a dash ('-').";
        return false;
    }

    for(auto const & c : segment)
    {
        switch(c)
        {
        case '-':
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
            break;

        default:
            if((c < 'a' || c > 'z')
            && (c < 'A' || c > 'Z')
            && static_cast<unsigned char>(c) < 0x80)
            {
                errmsg = "this TLD segment: \""
                       + segment
                       + "\" includes unsupported character: '"
                       + c
                       + "'.";
                return false;
            }
            break;

        }
    }

    f_tld.push_back(f_strings.add_string(segment));

    return true;
}


tld_definition::segments_t const & tld_definition::get_segments() const
{
    return f_tld;
}


/** \brief The domain name with periods separating each segment.
 *
 * This function rebuilds the full domain name. The idea is to have a way
 * to write error messages about various errors including the domain name.
 *
 * \return The name of the domain with each segment separated by periods.
 */
std::string tld_definition::get_name() const
{
    std::string name;

    for(auto const & segment : f_tld)
    {
        std::string const s(f_strings.get_string(segment));
        if(s.empty())
        {
            throw std::logic_error("a segment string is not defined");
        }
        name += '.';
        name += s;
    }

    return name;
}


/** \brief Get the full TLD as a reversed domain name.
 *
 * This function re-assembles the domain segments in a full name in reverse
 * order. This is used to properly sort sub-domain names (still part of the
 * TLD) by their parent domain name.
 *
 * We use '!' as the separate instead of the '.' because some domain names
 * have a dash in their as the order still needs to be correct and '.' > '-'
 * when we need the opposite, but '!' < '-'.
 *
 * \return The concatenated domain name in reverse order with '!' as separators.
 */
std::string tld_definition::get_inverted_name() const
{
    std::string name;

    for(auto it(f_tld.rbegin()); it != f_tld.rend(); ++it)
    {
        std::string const s(f_strings.get_string(*it));
        if(s.empty())
        {
            throw std::logic_error("a segment string is not defined");
        }
        name += '!';
        name += s;
    }

    return name;
}


std::string tld_definition::get_parent_name() const
{
    std::string name;

    bool skip_first(true);
    for(auto const & segment : f_tld)
    {
        std::string const s(f_strings.get_string(segment));
        if(s.empty())
        {
            throw std::logic_error("a segment string is not defined");
        }
        if(skip_first)
        {
            skip_first = false;
        }
        else
        {
            name += '.';
            name += s;
        }
    }

    return name;
}


std::string tld_definition::get_parent_inverted_name() const
{
    std::string name;

    for(std::size_t idx(f_tld.size() - 1); idx > 0; --idx)
    {
        std::string const s(f_strings.get_string(f_tld[idx]));
        if(s.empty())
        {
            throw std::logic_error("a segment string is not defined");
        }
        name += '!';
        name += s;
    }

    return name;
}


void tld_definition::set_index(int idx)
{
    f_index = idx;
}


int tld_definition::get_index() const
{
    return f_index;
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


bool tld_definition::set_apply_to(std::string const & apply_to)
{
    if((f_set & SET_APPLY_TO) != 0)
    {
        return false;
    }
    f_set |= SET_APPLY_TO;

    if(!apply_to.empty())
    {
        if(apply_to[0] == '.')
        {
            // remove the introductory period if present
            //
            f_apply_to = apply_to.substr(1);
            return true;
        }
    }
    f_apply_to = apply_to;

    return true;
}


std::string tld_definition::get_apply_to() const
{
    return f_apply_to;
}


void tld_definition::add_tag(
      std::string const & tag_name
    , std::string const & value
    , std::string & errmsg)
{
    if(tag_name.empty())
    {
        errmsg = "tag name cannot be empty.";
        return;
    }

    f_tags[f_strings.add_string(tag_name)] = f_strings.add_string(value);
}


tags_t const & tld_definition::get_tags() const
{
    return f_tags;
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


void tld_definition::set_start_offset(uint16_t start)
{
    if(f_start_offset == USHRT_MAX)
    {
        f_start_offset = start;
    }
}


void tld_definition::set_end_offset(uint16_t end)
{
    f_end_offset = end;
}


uint16_t tld_definition::get_start_offset() const
{
    return f_start_offset;
}


uint16_t tld_definition::get_end_offset() const
{
    return f_end_offset;
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


tld_string_manager & tld_compiler::get_string_manager()
{
    return f_strings;
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


void tld_compiler::set_c_file(std::string const & filename)
{
    f_c_file = filename;
}


std::string const & tld_compiler::get_c_file() const
{
    return f_c_file;
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

    define_default_category();
    if(get_errno() != 0)
    {
        return false;
    }

    // the merge feature is going to add merged strings to the table which
    // are not going to be found in the description tables, so here we want
    // to save the total number of strings prior to the merge process
    //
    f_strings_count = static_cast<string_id_t>(f_strings.size());

    f_strings.merge_strings();

    compress_tags();

    find_max_level();

    std::stringstream out;
    output_tlds(out);
    if(get_errno() != 0)
    {
        return false;
    }

    save_to_file(out.str());    // save to tlds.tld (RIFF/TLDS format)
    if(get_errno() != 0)
    {
        return false;
    }

    save_to_c_file(out.str());
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
    f_global_variables.clear();
    f_global_tags.clear();
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


bool tld_compiler::get_backslash(char32_t & c)
{
    c = getc();
    if(c == CHAR_ERR)
    {
        return false;
    }

    int count(0);
    switch(c)
    {
    case CHAR_EOF:
        c = '\\';
        return true;

    case '\\':
    case '\'':
    case '"':
    case ';':
    case '#':
    case '=':
    case ':':
        return true;

    // TODO: support octal
    //
    case '0': // null
        c = 0x0;
        return true;

    case 'a': // bell
        c = 0x07;
        return true;

    case 'b': // backspace
        c = 0x08;
        return true;

    case 't': // tab
        c = 0x09;
        return true;

    case 'f': // form feed
        c = 0x0C;
        return true;

    case 'r': // carriage return
        c = 0x0D;
        return true;

    case 'n': // line feed
        c = 0x0A;
        return true;

    case 'x':
    case 'X':
        count = 2;
        break;

    case 'u':
        count = 4;
        break;

    case 'U':
        count = 6; // in C/C++ this is 8
        break;

    }

    c = 0;
    for(int i(0); i < count; ++i)
    {
        char32_t d(getc());
        if(d == CHAR_ERR)
        {
            f_errno = EINVAL;
            f_errmsg = "unexpected error while reading escape Unicode character.";
            return false;
        }
        if(d == CHAR_EOF)
        {
            break;
        }
        c <<= 4;
        if(d >= 'a' && d <= 'f')
        {
            c |= d - 'a' + 10;
        }
        else if(d >= 'A' && d <= 'F')
        {
            c |= d - 'A' + 10;
        }
        else if(d >= '0' && d <= '9')
        {
            c |= d - '0';
        }
        else
        {
            if(i == 0)
            {
                f_errno = EINVAL;
                f_errmsg = "a Unicode character must include at least one hexdecimal digit.";
                return false;
            }

            // premature end is okay by us
            //
            c >>= 4;        // cancel the shift
            ungetc(d);
            break;
        }
    }

    return true;
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
                        if(!get_backslash(c))
                        {
                            return;
                        }
                    }
                    if(!append_wc(value, c))
                    {
                        return;
                    }
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
                    && c != '_'
                    && c != '/')
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
                for(;;)
                {
                    if(c == '\\')
                    {
                        if(!get_backslash(c))
                        {
                            return;
                        }
                    }
                    if(!append_wc(value, c))
                    {
                        return;
                    }

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
                    if(c == '.'
                    || c == '['
                    || c == '='
                    || c == ']')
                    {
                        ungetc(c);
                        break;
                    }
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


bool tld_compiler::append_wc(std::string & value, char32_t wc)
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
            // you can't directly use a surrogate
            //
            // TODO: convert to hex number instead of base 10
            //
            f_errno = EINVAL;
            f_errmsg = "trying to encode a surrogate Unicode code \""
                     + std::to_string(static_cast<std::uint32_t>(wc))
                     + "\" (base 10).";
            return false;
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
        // TODO: convert to hex number instead of base 10
        //
        f_errno = EINVAL;
        f_errmsg = "trying to encode invalid Unicode character \""
                 + std::to_string(static_cast<std::uint32_t>(wc))
                 + "\" (base 10).";
        return false;
    }

    return true;
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

    std::string const & name(f_tokens[0].get_value());
    std::string::size_type const pos(name.find('/'));
    bool const is_tag(pos != std::string::npos);
    if(is_tag)
    {
        if(name.substr(0, pos) != "tag")
        {
            f_errno = EINVAL;
            f_errmsg = "variable name \""
                + name
                + "\" does not start with \"tag/...\".";
            return;
        }
        std::string::size_type const more(name.find('/', pos + 1));
        if(more != std::string::npos)
        {
            f_errno = EINVAL;
            f_errmsg = "variable name \""
                + name
                + "\" cannot include more than one slash (/).";
            return;
        }
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

    if(is_tag)
    {
        std::string const tag_name(name.substr(pos + 1));
        if(f_current_tld.empty())
        {
            f_global_tags[tag_name] = value;
        }
        else
        {
            f_definitions[f_current_tld]->add_tag(tag_name, value, f_errmsg);
            if(!f_errmsg.empty())
            {
                f_errno = EINVAL;
                return;
            }
        }
    }
    else
    {
        if(f_current_tld.empty())
        {
            if(f_global_variables.find(name) != f_global_variables.end())
            {
                f_errno = EINVAL;
                f_errmsg = "\"" + name + "\" global variable defined more than once.";
                return;
            }

            // name != "apply_to" -- I don't think that would be useful as a global
            if(pos != std::string::npos     // any tag
            && name != "status")
            {
                f_errno = EINVAL;
                f_errmsg = "variable with name \"" + name + "\" is not supported. Missing \"tag/\"?";
                return;
            }

            f_global_variables[name] = value;
        }
        else
        {
            f_definitions[f_current_tld]->set_named_parameter(name, value, f_errmsg);
            if(!f_errmsg.empty())
            {
                f_errno = EINVAL;
                return;
            }
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

    tld_definition::pointer_t tld(std::make_shared<tld_definition>(f_strings));

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
            if(!tld->add_segment("*", f_errmsg))
            {
                f_errno = EINVAL;
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
                if(!tld->add_segment(segment, f_errmsg))
                {
                    f_errno = EINVAL;
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

    // use the '!' (0x21) for sorting, because '.' (0x2E) is after '-' (0x2D)
    // and there is no '!' allowed in domain names (so far)
    //
    f_current_tld = tld->get_inverted_name();
    f_definitions[f_current_tld] = tld;

    // add the globals to this definition
    //
    for(auto const & g : f_global_variables)
    {
        f_definitions[f_current_tld]->set_named_parameter(g.first, g.second, f_errmsg);
        if(!f_errmsg.empty())
        {
            // this should not happen since the globals are defined in a map
            //
            f_errno = EINVAL;
            return;
        }
    }

    for(auto const & g : f_global_tags)
    {
        f_definitions[f_current_tld]->add_tag(g.first, g.second, f_errmsg);
        if(!f_errmsg.empty())
        {
            // this should not happen since the globals are defined in a map
            //
            f_errno = EINVAL;
            return;
        }
    }

    f_definitions[f_current_tld]->reset_set_flags();
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


void tld_compiler::define_default_category()
{
    string_id_t const category_id(f_strings.add_string("category"));
    string_id_t const country_id(f_strings.add_string("country"));

    for(auto const & d : f_definitions)
    {
        tags_t const & tags(d.second->get_tags());
        auto it(tags.find(category_id));
        if(it == tags.end())
        {
            // there is no category yet, let's determine that now
            //
            if(tags.find(country_id) != tags.end())
            {
                d.second->add_tag("category", "country", f_errmsg);
                if(!f_errmsg.empty())
                {
                    f_errno = EINVAL;
                    return;
                }
            }
            else
            {
                f_errmsg = "domain \""
                    + d.second->get_name()
                    + "\" has no category and we had no way to determine a default category.";
                f_errno = EINVAL;
                return;
            }
        }
    }
}


void tld_compiler::compress_tags()
{
    for(auto const & d : f_definitions)
    {
        f_tags.add(d.second->get_tags());
    }

    f_tags.merge();
}


uint16_t tld_compiler::find_definition(std::string name) const
{
    if(!name.empty())
    {
        if(name[0] != '.')
        {
            name = '.' + name;
        }
        for(auto const & it : f_definitions)
        {
            if(it.second->get_name() == name)
            {
                return it.second->get_index();
            }
        }
    }

    return USHRT_MAX;
}


/** \brief Determine the longest TLD in terms of levels.
 * 
 * This function searches all the definitions checking for the longest
 * number of segments (which is the number of periods in the TLD including
 * the starting period, so in ".com", we have a level of 1).
 */
void tld_compiler::find_max_level()
{
    f_tld_max_level = 0;

    auto it(std::max_element(
              f_definitions.begin()
            , f_definitions.end()
            , [](auto const & a, auto const & b)
              {
                  return a.second->get_segments().size()
                                        < b.second->get_segments().size();
              }));
    if(it == f_definitions.end())
    {
        f_errno = EINVAL;
        f_errmsg = "error: could not find a definition with a larger level.";
        return;
    }

    f_tld_max_level = it->second->get_segments().size();
}


void tld_compiler::output_tlds(std::ostream & out)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    tld_header header =
    {
        .f_version_major = 1,
        .f_version_minor = 0,
        .f_pad0 = 0,
        .f_tld_max_level = f_tld_max_level,
        .f_tld_start_offset = USHRT_MAX,
        .f_tld_end_offset = USHRT_MAX,
        .f_created_on = f_created_on,
    };
#pragma GCC diagnostic pop

    // define the "offsets" (indices) of all the items
    //
    // the index will be used for the `apply_to` below to properly
    // determine the exception
    //
    int i(0);
    for(uint8_t level(f_tld_max_level); level > 0; --level)
    {
        for(auto const & d : f_definitions)
        {
            if(d.second->get_segments().size() == level)
            {
                d.second->set_index(i);
                ++i;
            }
        }
    }

    // now we create the TLD table with the largest levels first,
    // as we do so we save the index of the start and stop
    // points of each level in the previous level (hence the
    // need for a level 0 entry)
    //
    // we create the table in memory; we need the level 0 offsets in
    // the header before we can start saving the results in the output
    // file...
    //
    std::vector<tld_description> descriptions;
    i = 0;
    for(uint8_t level(header.f_tld_max_level); level > 0; --level)
    {
        for(auto const & d : f_definitions)
        {
            if(d.second->get_segments().size() == level)
            {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
                tld_description description =
                {
                    // make sure it's set to exception if we have an "apply to"
                    // (probably not required since we can check whether we do
                    // have an apply to)
                    //
                    .f_status = static_cast<uint8_t>(d.second->get_apply_to().empty()
                                    ? d.second->get_status()
                                    : TLD_STATUS_EXCEPTION),
                    .f_exception_level = level,
                    .f_exception_apply_to = find_definition(d.second->get_apply_to()),
                    .f_start_offset = d.second->get_start_offset(),
                    .f_end_offset = d.second->get_end_offset(),
                    .f_tld = static_cast<uint16_t>(d.second->get_segments()[0]),
                    .f_tags = static_cast<uint16_t>(f_tags.get_tag_offset(d.second->get_tags())),
                    .f_tags_count = static_cast<uint16_t>(d.second->get_tags().size()),
                };
#pragma GCC diagnostic pop

                std::string const parent_name(d.second->get_parent_inverted_name());
                if(parent_name.empty())
                {
                    if(f_tld_start_offset == USHRT_MAX)
                    {
                        f_tld_start_offset = i;
                    }
                    f_tld_end_offset = i + 1;
                }
                else
                {
                    auto it(f_definitions.find(parent_name));
                    if(it == f_definitions.end())
                    {
                        f_errno = EINVAL;
                        f_errmsg = "parent domain \""
                            + parent_name
                            + "\" not found.";
                        return;
                    }
                    it->second->set_start_offset(i);
                    it->second->set_end_offset(i + 1);
                }

                descriptions.push_back(description);

                ++i;
            }
        }
    }

    header.f_tld_start_offset = f_tld_start_offset;
    header.f_tld_end_offset = f_tld_end_offset;

    tld_hunk header_hunk;
    header_hunk.f_name = TLD_HEADER;
    header_hunk.f_size = sizeof(tld_header);

    tld_hunk descriptions_hunk;
    descriptions_hunk.f_name = TLD_DESCRIPTIONS;
    descriptions_hunk.f_size = sizeof(tld_description) * f_definitions.size();

    tld_hunk tags_hunk;
    tags_hunk.f_name = TLD_TAGS;
    tags_hunk.f_size = f_tags.merged_tags().size() * sizeof(uint32_t); // NOT sizeof(tld_tags) because the merged vector is not one to one equivalent

    tld_hunk string_offsets_hunk;
    string_offsets_hunk.f_name = TLD_STRING_OFFSETS;
    string_offsets_hunk.f_size = static_cast<std::size_t>(f_strings_count) * sizeof(tld_string_offset);

    tld_hunk string_lengths_hunk;
    string_lengths_hunk.f_name = TLD_STRING_LENGTHS;
    string_lengths_hunk.f_size = static_cast<std::size_t>(f_strings_count) * sizeof(tld_string_length);

    tld_hunk strings_hunk;
    strings_hunk.f_name = TLD_STRINGS;
    strings_hunk.f_size = f_strings.compressed_length();

    tld_magic magic;
    magic.f_riff = TLD_MAGIC;
    magic.f_size = sizeof(magic.f_type)
        + sizeof(tld_hunk) + header_hunk.f_size
        + sizeof(tld_hunk) + descriptions_hunk.f_size
        + sizeof(tld_hunk) + tags_hunk.f_size
        + sizeof(tld_hunk) + string_offsets_hunk.f_size
        + sizeof(tld_hunk) + string_lengths_hunk.f_size
        + sizeof(tld_hunk) + strings_hunk.f_size;
    magic.f_type = TLD_TLDS;

    out.write(reinterpret_cast<char const *>(&magic), sizeof(magic));

    // header
    //
    out.write(reinterpret_cast<char const *>(&header_hunk), sizeof(header_hunk));
    out.write(reinterpret_cast<char const *>(&header), sizeof(header));

    // descriptions
    //
    out.write(reinterpret_cast<char const *>(&descriptions_hunk), sizeof(descriptions_hunk));
    out.write(reinterpret_cast<char const *>(descriptions.data()), descriptions.size() * sizeof(tld_description));

    // tags
    //
    out.write(reinterpret_cast<char const *>(&tags_hunk), sizeof(tags_hunk));
    out.write(reinterpret_cast<char const *>(f_tags.merged_tags().data()), tags_hunk.f_size);

    // strings: offsets
    //
    out.write(reinterpret_cast<char const *>(&string_offsets_hunk), sizeof(string_offsets_hunk));
    for(string_id_t idx(1); idx <= f_strings_count; ++idx)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        tld_string_offset offset =
        {
            .f_string_offset = static_cast<uint32_t>(f_strings.get_string_offset(idx)),
        };
#pragma GCC diagnostic pop
        out.write(reinterpret_cast<char const *>(&offset), sizeof(offset));
    }

    // strings: lengths
    //
    out.write(reinterpret_cast<char const *>(&string_lengths_hunk), sizeof(string_lengths_hunk));
    for(string_id_t idx(1); idx <= f_strings_count; ++idx)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        tld_string_length length =
        {
            .f_string_length = static_cast<uint16_t>(f_strings.get_string(idx).length()),
        };
#pragma GCC diagnostic pop
        out.write(reinterpret_cast<char const *>(&length), sizeof(length));
    }

    // strings: actual strings
    //
    out.write(reinterpret_cast<char const *>(&strings_hunk), sizeof(strings_hunk));
    out.write(f_strings.compressed_strings().c_str(), strings_hunk.f_size);
}


void tld_compiler::save_to_file(std::string const & buffer)
{
    std::ofstream out;
    out.open(f_output);
    if(!out)
    {
        f_errno = errno;
        f_errmsg = "error: could not open output file \""
                 + f_output
                 + "\", errno: "
                 + std::to_string(f_errno)
                 + ", "
                 + strerror(f_errno)
                 + ".";
        return;
    }

    out.write(buffer.c_str(), buffer.length());
}


void tld_compiler::output_header(std::ostream & out)
{
    time_t const now(time(nullptr));
    struct tm t;
    localtime_r(&now, &t);
    char year[16];
    strftime(year, sizeof(year), "%Y", &t);

    std::string basename;
    std::string::size_type const pos(f_c_file.rfind('/'));
    if(pos == std::string::npos)
    {
        basename = f_c_file;
    }
    else
    {
        basename = f_c_file.substr(pos + 1);
    }

    out << "/* *** AUTO-GENERATED *** DO NOT EDIT ***\n"
           " *\n"
           " * This list of TLDs was auto-generated using the tldc compiler.\n"
           " * Fix the tld_compiler.cpp or the .ini files used as input instead\n"
           " * of this file.\n"
           " *\n"
           " * Copyright (c) 2011-" << year << "  Made to Order Software Corp.  All Rights Reserved.\n"
           " *\n"
           " * Permission is hereby granted, free of charge, to any person obtaining a\n"
           " * copy of this software and associated documentation files (the\n"
           " * \"Software\"), to deal in the Software without restriction, including\n"
           " * without limitation the rights to use, copy, modify, merge, publish,\n"
           " * distribute, sublicense, and/or sell copies of the Software, and to\n"
           " * permit persons to whom the Software is furnished to do so, subject to\n"
           " * the following conditions:\n"
           " *\n"
           " * The above copyright notice and this permission notice shall be included\n"
           " * in all copies or substantial portions of the Software.\n"
           " *\n"
           " * THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
           " * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
           " * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
           " * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
           " * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n"
           " * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n"
           " * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
           " */\n"
           "\n"
           "/** \\file\n"
           " * \\brief GENERATED FILE -- the " << basename << " file is generated -- DO NOT EDIT\n"
           " *\n"
           " * This file is generated using the tldc tool and the conf/tlds/... files.\n"
           " * It is strongly advised that you do not edit this file directly except to\n"
           " * test before editing the source of the tldc tool and tld_compiler.cpp file.\n"
           " *\n"
           " * The file includes information about all the TLDs as defined in the\n"
           " * .ini files. It is used by the tld() function to determine whether\n"
           " * a string with a domain name matches a valid TLD. It includes all the\n"
           " * currently assigned TLDs (all countries plus international or common TLDs.)\n"
           " *\n"
           " * In this new implementation, the C version to compile is actually the\n"
           " * RIFF/TLDS binary. We load it with the tld_file_load() function as if it\n"
           " * were on disk. This way we have exactly the same code to load the\n"
           " * compiled-in and the TLDs from files.\n"
           " */\n"
           "#include <stdint.h>\n";
}


void tld_compiler::save_to_c_file(std::string const & buffer)
{
    // user requested that file?
    //
    if(f_c_file.empty())
    {
        return;
    }

    std::ofstream out;
    out.open(f_c_file);
    if(!out)
    {
        f_errno = errno;
        f_errmsg = "error: could not open C-file output file \""
                 + f_output
                 + "\", errno: "
                 + std::to_string(f_errno)
                 + ", "
                 + strerror(f_errno)
                 + ".";
        return;
    }

    output_header(out);

    out << "uint8_t const tld_static_tlds[] = {\n"
        << std::hex
        << std::setfill('0');

    for(std::uint32_t idx(0); idx + 16 < buffer.length(); idx += 16)
    {
        out << "   ";
        for(std::uint32_t o(0); o < 16; ++o)
        {
            out << " 0x"
                << std::setw(2)
                << static_cast<int>(static_cast<uint8_t>(buffer[idx + o]))
                << ",";
        }
        out << "\n";
    }
    std::uint32_t const leftover(buffer.length() % 16);
    std::uint32_t const offset(buffer.length() - leftover);
    if(leftover > 0)
    {
        out << "   ";
        for(std::uint32_t o(0); o < leftover; ++o)
        {
            out << " 0x"
                << std::setw(2)
                << static_cast<int>(static_cast<uint8_t>(buffer[offset + o]))
                << ",";
        }
        out << "\n";
    }
    out << "};\n";
}


void tld_compiler::output_to_json(std::ostream & out) const
{
    out << "{\n";
    out << "\"version\":\"" << TLD_FILE_VERSION_MAJOR
                     << '.' << TLD_FILE_VERSION_MINOR << "\",\n";
    out << "\"created-on\":" << f_created_on << ",\n";
    out << "\"max-level\":" << static_cast<int>(f_tld_max_level) << ",\n";
    out << "\"tld-start-offset\":" << f_tld_start_offset << ",\n";
    out << "\"tld-end-offset\":" << f_tld_end_offset << ",\n";
    out << "\"descriptions\":[\n";
    for(std::size_t idx(0); idx < f_definitions.size(); ++idx)
    {
        auto it(std::find_if(
              f_definitions.begin()
            , f_definitions.end()
            , [idx](auto const & d)
                {
                    return d.second->get_index() == static_cast<int>(idx);
                }));
        if(it == f_definitions.end())
        {
            std::cerr << "error: could not find definition at index "
                << idx
                << "\n";
            return;
        }
        //out << "\"index\":\"" << it->second->get_index() << "\"";

        out << (idx == 0 ? "" : ",\n");

        //out << "/* " << it->second->get_name() << " */ ";

        out << "{\"tld\":\"" << f_strings.get_string(it->second->get_segments()[0]) << "\"";

        out << ",\"status\":\"" << tld_status_to_string(it->second->get_status()) << "\"";

        if(!it->second->get_apply_to().empty())
        {
            out << ",\"apply-to\":\"" << it->second->get_apply_to() << "\"";
        }

        if(it->second->get_start_offset() != USHRT_MAX)
        {
            out << ",\"start-offset\":" << it->second->get_start_offset();
            out << ",\"end-offset\":" << it->second->get_end_offset();
        }

        for(auto const & t : it->second->get_tags())
        {
            out << ",\"" << f_strings.get_string(t.first)
                << "\":\"" << f_strings.get_string(t.second)
                << "\"";
        }

        out << "}";
    }
    out << "]}\n";
}


// vim: ts=4 sw=4 et
