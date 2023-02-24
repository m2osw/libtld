/* TLD library -- TLD, domain name, and sub-domain extraction
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

/** \file
 * \brief Implementation of the TLD file data handling.
 *
 * This file handles the loading of the TLDs from an RIFF file.
 */

// self
//
#include    "libtld/tld_file.h"
#include    "libtld/tld.h"
#include    "libtld/tld_data.h"


// C++ lib
//
#include    <fstream>
#include    <iostream>
#include    <sstream>


// C lib
//
#include    <limits.h>
#include    <string.h>



tld_file_error tld_file_load_stream(tld_file ** file, std::istream & in)
{
    tld_magic magic;
    in.read(reinterpret_cast<char *>(&magic), sizeof(magic));
    if(!in
    || in.gcount() != sizeof(magic))
    {
        return TLD_FILE_ERROR_CANNOT_READ_FILE;
    }

    if(magic.f_riff != TLD_MAGIC
    || magic.f_type != TLD_TLDS)
    {
        return TLD_FILE_ERROR_UNRECOGNIZED_FILE;
    }
    if(magic.f_size < sizeof(tld_header) + 4
    || magic.f_size > 1024 * 1024)
    {
        return TLD_FILE_ERROR_INVALID_FILE_SIZE;
    }
    uint32_t size(magic.f_size - sizeof(uint32_t));

    // we already read the type so we can skip that one in the following
    // memory buffer & read
    //
    *file = reinterpret_cast<tld_file *>(malloc(sizeof(tld_file) + size));
    if(*file == nullptr)
    {
        return TLD_FILE_ERROR_OUT_OF_MEMORY;
    }

    class auto_free
    {
    public:
        auto_free(tld_file ** ptr)
            : f_ptr(ptr)
        {
        }

        auto_free(auto_free const &) = delete;

        ~auto_free()
        {
            if(f_ptr != nullptr
            && *f_ptr != nullptr)
            {
                free(*f_ptr);
                *f_ptr = nullptr;
            }
        }

        auto_free & operator = (auto_free const &) = delete;

        void keep()
        {
            f_ptr = nullptr;
        }

    private:
        tld_file ** f_ptr = nullptr;
    };
    auto_free safe_ptr(file);

    memset(*file, 0, sizeof(tld_file));

    tld_hunk * hunk(reinterpret_cast<tld_hunk *>(*file + 1));

    in.read(reinterpret_cast<char *>(hunk), size);
    if(!in
    || in.gcount() != size) // this doesn't fail if the file is larger...
    {
        return TLD_FILE_ERROR_CANNOT_READ_FILE;
    }

    for(;;)
    {
        if(size == 0)
        {
            break;
        }

        if(sizeof(tld_hunk) > size)
        {
            return TLD_FILE_ERROR_INVALID_HUNK_SIZE;
        }
        size -= sizeof(tld_hunk);

        if(hunk->f_size > size)
        {
            return TLD_FILE_ERROR_INVALID_HUNK_SIZE;
        }
        size -= hunk->f_size;

        switch(hunk->f_name)
        {
        case TLD_HEADER:
            if(sizeof(tld_header) != hunk->f_size)
            {
                return TLD_FILE_ERROR_INVALID_STRUCTURE_SIZE;
            }
            if((*file)->f_header != nullptr)
            {
                return TLD_FILE_ERROR_HUNK_FOUND_TWICE;
            }
            (*file)->f_header = reinterpret_cast<tld_header *>(hunk + 1);
            if((*file)->f_header->f_version_major != TLD_FILE_VERSION_MAJOR
            || (*file)->f_header->f_version_minor != TLD_FILE_VERSION_MINOR)
            {
                return TLD_FILE_ERROR_UNSUPPORTED_VERSION;
            }
            break;

        case TLD_DESCRIPTIONS:
            (*file)->f_descriptions_count = hunk->f_size / sizeof(tld_description);
            if((*file)->f_descriptions_count * sizeof(tld_description) != hunk->f_size)
            {
                return TLD_FILE_ERROR_INVALID_ARRAY_SIZE;
            }
            if((*file)->f_descriptions != nullptr)
            {
                return TLD_FILE_ERROR_HUNK_FOUND_TWICE;
            }
            (*file)->f_descriptions = reinterpret_cast<tld_description *>(hunk + 1);
            break;

        case TLD_TAGS:
            // the tags ar ea bit peculiar in that the compression happens
            // by uin32_t and not by tld_tags so the number of tags cannot
            // be inferred by the hunk size
            //
            (*file)->f_tags_size = hunk->f_size / sizeof(uint32_t);
            if((*file)->f_tags_size * sizeof(uint32_t) != hunk->f_size)
            {
                return TLD_FILE_ERROR_INVALID_ARRAY_SIZE;
            }
            if((*file)->f_tags != nullptr)
            {
                return TLD_FILE_ERROR_HUNK_FOUND_TWICE;
            }
            (*file)->f_tags = reinterpret_cast<uint32_t *>(hunk + 1);
            break;

        case TLD_STRING_OFFSETS:
            if((*file)->f_strings_count == 0)
            {
                (*file)->f_strings_count = hunk->f_size / sizeof(tld_string_offset);
                if((*file)->f_strings_count == 0)
                {
                    return TLD_FILE_ERROR_INVALID_ARRAY_SIZE;
                }
            }
            if((*file)->f_strings_count * sizeof(tld_string_offset) != hunk->f_size)
            {
                return TLD_FILE_ERROR_INVALID_ARRAY_SIZE;
            }
            if((*file)->f_string_offsets != nullptr)
            {
                return TLD_FILE_ERROR_HUNK_FOUND_TWICE;
            }
            (*file)->f_string_offsets = reinterpret_cast<tld_string_offset *>(hunk + 1);
            break;

        case TLD_STRING_LENGTHS:
            if((*file)->f_strings_count == 0)
            {
                (*file)->f_strings_count = hunk->f_size / sizeof(tld_string_length);
                if((*file)->f_strings_count == 0)
                {
                    return TLD_FILE_ERROR_INVALID_ARRAY_SIZE;
                }
            }
            if((*file)->f_strings_count * sizeof(tld_string_length) != hunk->f_size)
            {
                return TLD_FILE_ERROR_INVALID_ARRAY_SIZE;
            }
            if((*file)->f_string_lengths != nullptr)
            {
                return TLD_FILE_ERROR_HUNK_FOUND_TWICE;
            }
            (*file)->f_string_lengths = reinterpret_cast<tld_string_length *>(hunk + 1);
            break;

        case TLD_STRINGS:
            if(hunk->f_size == 0)
            {
                return TLD_FILE_ERROR_INVALID_ARRAY_SIZE;
            }
            if((*file)->f_strings != nullptr)
            {
                return TLD_FILE_ERROR_HUNK_FOUND_TWICE;
            }
            (*file)->f_strings = reinterpret_cast<char *>(hunk + 1);
            (*file)->f_strings_end = reinterpret_cast<char *>(hunk + 1 + hunk->f_size);
            break;

        default:
            // just skip unrecognized hunks
            break;

        }

        hunk = reinterpret_cast<tld_hunk *>(reinterpret_cast<char *>(hunk + 1) + hunk->f_size);
    }

    // verify we got all the required tables
    //
    if((*file)->f_header == nullptr
    || (*file)->f_descriptions == nullptr
    || (*file)->f_tags == nullptr
    || (*file)->f_string_offsets == nullptr
    || (*file)->f_string_lengths == nullptr
    || (*file)->f_strings == nullptr)
    {
        return TLD_FILE_ERROR_MISSING_HUNK;
    }

    // it worked, do no lose the allocated pointer
    //
    safe_ptr.keep();

    return TLD_FILE_ERROR_NONE;
}


#ifdef __cplusplus
extern "C" {
#endif


enum tld_file_error tld_file_load(char const * filename, tld_file ** file)
{
    if(file == nullptr
    || filename == nullptr)
    {
        return TLD_FILE_ERROR_INVALID_POINTER;
    }
    if(*file != nullptr)
    {
        return TLD_FILE_ERROR_POINTER_PRESENT;
    }

    std::ifstream in;
    in.open(filename);
    if(!in.is_open())
    {
        return TLD_FILE_ERROR_CANNOT_OPEN_FILE;
    }

    return tld_file_load_stream(file, in);
}


const char *tld_file_errstr(tld_file_error err)
{
    switch(err)
    {
    case TLD_FILE_ERROR_NONE:
        return "No error";

    case TLD_FILE_ERROR_INVALID_POINTER:
        return "Invalid pointer";

    case TLD_FILE_ERROR_POINTER_PRESENT:
        return "Pointer present when it should ne nullptr";

    case TLD_FILE_ERROR_CANNOT_OPEN_FILE:
        return "Cannot open file";

    case TLD_FILE_ERROR_CANNOT_READ_FILE:
        return "I/O error reading file";

    case TLD_FILE_ERROR_UNRECOGNIZED_FILE:
        return "Unrecognized input file";

    case TLD_FILE_ERROR_INVALID_FILE_SIZE:
        return "Invalid file size";

    case TLD_FILE_ERROR_OUT_OF_MEMORY:
        return "Out of memory";

    case TLD_FILE_ERROR_INVALID_HUNK_SIZE:
        return "Invalid hunk size";

    case TLD_FILE_ERROR_INVALID_STRUCTURE_SIZE:
        return "Invalid structure size";

    case TLD_FILE_ERROR_INVALID_ARRAY_SIZE:
        return "Invalid array size";

    case TLD_FILE_ERROR_UNSUPPORTED_VERSION:
        return "Unsupported version";

    case TLD_FILE_ERROR_MISSING_HUNK:
        return "Missing hunk";

    case TLD_FILE_ERROR_HUNK_FOUND_TWICE:
        return "Found the same hunk twice";

    //default: -- handled below, without a default, we know whether we missed
    //            some new TLD_FILE_ERROR_... in our cases above.
    }

    return "Unknown tld_file error number";
}


const tld_description * tld_file_description(tld_file const * file, uint32_t id)
{
    if(id >= file->f_descriptions_count)
    {
        return nullptr;
    }
    return file->f_descriptions + id;
}


const tld_tag * tld_file_tag(tld_file const * file, uint32_t id)
{
    if(id + 1 >= file->f_tags_size)
    {
        return nullptr;
    }
    return reinterpret_cast<tld_tag *>(file->f_tags + id);
}


const char * tld_file_string(tld_file const * file, uint32_t id, uint32_t * length)
{
    if(length == nullptr)
    {
        errno = EINVAL;
        return nullptr;
    }
    *length = 0;

    --id;
    if(id >= file->f_strings_count)
    {
        errno = EINVAL;
        return nullptr;
    }
    char * s(file->f_strings + file->f_string_offsets[id].f_string_offset);
    uint32_t l(file->f_string_lengths[id].f_string_length);
    char * e(s + l);
    if(s > file->f_strings_end
    || e > file->f_strings_end)
    {
        // assuming the file is valid, this should not happen
        //
        errno = EINVAL;
        return nullptr;
    }
    *length = l;
    return s;
}


/** \brief Transform a tld_file to a JSON string.
 *
 * This function transforms a tld_file in a JSON string which gets returned.
 * If something goes wrong, then the function may return a nullptr instead.
 *
 * The returned strings must be freed by you with the `free()` function.
 *
 * \param[in] file  The tld_file to transform to a JSON.
 *
 * \return A string with the tld_file JSON or nullptr on error.
 */
char *tld_file_to_json(tld_file const * file)
{
    if(file == nullptr
    || file->f_header == nullptr
    || file->f_descriptions == nullptr
    || file->f_tags == nullptr
    || file->f_string_offsets == nullptr
    || file->f_string_lengths == nullptr
    || file->f_strings == nullptr)
    {
        return nullptr;
    }

    std::stringstream out;

    out << "{\n";
    out << "\"version\":\"" << static_cast<int>(file->f_header->f_version_major)
                     << '.' << static_cast<int>(file->f_header->f_version_minor) << "\",\n";
    out << "\"created-on\":" << file->f_header->f_created_on << ",\n";
    out << "\"max-level\":" << static_cast<int>(file->f_header->f_tld_max_level) << ",\n";
    out << "\"tld-start-offset\":" << static_cast<int>(file->f_header->f_tld_start_offset) << ",\n";
    out << "\"tld-end-offset\":" << static_cast<int>(file->f_header->f_tld_end_offset) << ",\n";
    out << "\"descriptions\":[\n";
    for(uint32_t idx(0); idx < file->f_descriptions_count; ++idx)
    {
        tld_description const * d(tld_file_description(file, idx));

        out << (idx == 0 ? "" : ",\n");

        {
            uint32_t length(0);
            char const * tld(tld_file_string(file, d->f_tld, &length));
            out << "{\"tld\":\"" << std::string(tld, length) << "\"";
        }

        out << ",\"status\":\"" << tld_status_to_string(static_cast<tld_status>(d->f_status)) << "\"";

        if(d->f_exception_apply_to != USHRT_MAX)
        {
            tld_description const * apply_to(tld_file_description(file, d->f_exception_apply_to));
            uint32_t length(0);
            char const * to_tld(tld_file_string(file, apply_to->f_tld, &length));
            out << ",\"apply-to\":\"" << std::string(to_tld, length) << "\"";
        }

        if(d->f_start_offset != USHRT_MAX)
        {
            out << ",\"start-offset\":" << d->f_start_offset;
            out << ",\"end-offset\":" << d->f_end_offset;
        }

        for(uint32_t tidx(0); tidx < d->f_tags_count; ++tidx)
        {
            const tld_tag * tag(tld_file_tag(file, d->f_tags + tidx * 2));
            {
                uint32_t length(0);
                char const * tag_name(tld_file_string(file, tag->f_tag_name, &length));
                out << ",\"" << std::string(tag_name, length)
                    << "\":\"";
            }
            {
                uint32_t length(0);
                char const * tag_value(tld_file_string(file, tag->f_tag_value, &length));
                out << std::string(tag_value, length)
                    << "\"";
            }
        }

        out << "}";
    }
    out << "]}\n";

    return strdup(out.str().c_str());
}


void tld_file_free(tld_file ** file)
{
    if(file != nullptr
    && *file != nullptr)
    {
        free(*file);
        *file = nullptr;
    }
}


#ifdef __cplusplus
}
#endif

// vim: ts=4 sw=4 et
