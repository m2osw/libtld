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
#ifndef LIB_TLD_FILE_H
#define LIB_TLD_FILE_H
/** \file
 * \brief Declaration of the TLD file structures.
 *
 * The older version of the libtld had a pre-compiled structure within the
 * library. The main problem with that designed is that you can't quickly
 * update the list of domain name TLDs. You have to recompile the whole
 * library, generate a new package, install that new package.
 *
 * The pros are of course that you only have to deal with one file (.so
 * or .a or similar file for your OS).
 *
 * Yet, the number of TLDs has been growing and changing a lot back and
 * forth in the last 10 years or so and having a compressed external file
 * will make it a lot faster and a lot easier to update the available TLDs.
 *
 * The structures found here descript that external file. The basic format
 * is IFF (like a WAVE file, sizes and other numbers will be in the endian
 * of your computer, so watch out on that part.)
 *
 * At time of writing, I have three hunks:
 *
 * * Header -- a header with a few parameters such as the maximum TLD level
 * * Descriptions -- the array of descriptions
 * * Strings -- one super-string; the descriptions include an offset and a
 *   size for each one of those strings
 */

// C lib
//
#include    <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define TLD_FILE_VERSION_MAJOR      1
#define TLD_FILE_VERSION_MINOR      0

#define TLD_HUNK(a, b, c, d)    ((uint32_t)((a)|((b)<<8)|((c)<<16)|((d)<<24)))

#define TLD_MAGIC           TLD_HUNK('R','I','F','F')
#define TLD_TLDS            TLD_HUNK('T','L','D','S')
#define TLD_HEADER          TLD_HUNK('H','E','A','D')
#define TLD_DESCRIPTIONS    TLD_HUNK('D','E','S','C')
#define TLD_TAGS            TLD_HUNK('T','A','G','S')
#define TLD_STRING_OFFSETS  TLD_HUNK('S','O','F','F')
#define TLD_STRING_LENGTHS  TLD_HUNK('S','L','E','N')
#define TLD_STRINGS         TLD_HUNK('S','T','R','S')




struct tld_magic
{
    uint32_t        f_riff;     // 'RIFF' (Reversed IFF)
    uint32_t        f_size;     // total size of this file - 8
    uint32_t        f_type;     // 'TLDS'
};


struct tld_hunk
{
    uint32_t        f_name;
    uint32_t        f_size;
};


struct tld_header
{
    // WARNING: do not change the version position
    //          anything else may change based on that information
    //
    uint8_t                 f_version_major;    // 1.0
    uint8_t                 f_version_minor;
    uint8_t                 f_pad0;
    uint8_t                 f_tld_max_level;

    uint16_t                f_tld_start_offset;
    uint16_t                f_tld_end_offset;

    int64_t                 f_created_on;
};


struct tld_description
{
    uint8_t                 f_status;
    uint8_t                 f_exception_level;
    uint16_t                f_exception_apply_to;   // index of tld_description this exception applies to

    uint16_t                f_start_offset;         // next level or -1 (65535)
    uint16_t                f_end_offset;

    uint16_t                f_tld;                  // string ID

    uint16_t                f_tags;                 // offset in tld_tag table
    uint16_t                f_tags_count;
};


struct tld_tag
{
    uint32_t                f_tag_name;     // string ID
    uint32_t                f_tag_value;    // string ID
};


struct tld_string_offset
{
    uint32_t                f_string_offset;    // offset in STRS
};


struct tld_string_length
{
    uint16_t                f_string_length;    // corresponding length
};


struct tld_file
{
    tld_header *            f_header;
    uint32_t                f_descriptions_count;
    tld_description *       f_descriptions;
    uint32_t                f_tags_size;        // WARNING: this is the number of uint32_t, not tld_tag
    uint32_t *              f_tags;             // these are tld_tags which may be merged at any level (a tag id may be odd)
    uint32_t                f_strings_count;
    tld_string_offset *     f_string_offsets;
    tld_string_length *     f_string_lengths;
    char *                  f_strings;
    char *                  f_strings_end;
};


enum tld_file_error
{
    TLD_FILE_ERROR_NONE = 0,
    TLD_FILE_ERROR_INVALID_POINTER,
    TLD_FILE_ERROR_POINTER_PRESENT,
    TLD_FILE_ERROR_CANNOT_OPEN_FILE,
    TLD_FILE_ERROR_CANNOT_READ_FILE,
    TLD_FILE_ERROR_UNRECOGNIZED_FILE,
    TLD_FILE_ERROR_INVALID_FILE_SIZE,
    TLD_FILE_ERROR_OUT_OF_MEMORY,
    TLD_FILE_ERROR_INVALID_HUNK_SIZE,
    TLD_FILE_ERROR_INVALID_STRUCTURE_SIZE,
    TLD_FILE_ERROR_INVALID_ARRAY_SIZE,
    TLD_FILE_ERROR_UNSUPPORTED_VERSION,
    TLD_FILE_ERROR_MISSING_HUNK,
};


tld_file_error      tld_file_load(const char * filename, tld_file ** file);
char const *        tld_file_errstr(tld_file_error err);
tld_description *   tld_file_description(tld_file const * file, uint32_t id);
tld_tag *           tld_file_tag(tld_file const * file, uint32_t id);
char *              tld_file_string(tld_file const * file, uint32_t id, uint32_t * length);
char *              tld_file_to_json(tld_file const * file);
void                tld_file_free(tld_file ** file);


#ifdef __cplusplus
}
#endif

#endif
//#ifndef LIB_TLD_FILE_H
// vim: ts=4 sw=4 et