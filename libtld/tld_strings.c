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
 * \brief Implementation of some functions converting numbers to strings.
 *
 * This file includes functions converting numbers to strings.
 */

// self
//
#include "libtld/tld.h"


// C lib
//
#include    <string.h>


/** \brief Transform the status to a string.
 *
 * The status returned in a tld_info can be converted to a string using
 * this function. This is useful to print out an error message.
 *
 * \param[in] status  The status to convert to a string.
 *
 * \return A string representing the input tld_status.
 */
const char * tld_status_to_string(enum tld_status status)
{
    switch(status)
    {
    case TLD_STATUS_VALID:
        return "valid";

    case TLD_STATUS_PROPOSED:
        return "proposed";

    case TLD_STATUS_DEPRECATED:
        return "deprecated";

    case TLD_STATUS_UNUSED:
        return "unused";

    case TLD_STATUS_RESERVED:
        return "reserved";

    case TLD_STATUS_INFRASTRUCTURE:
        return "infrastructure";

    case TLD_STATUS_EXAMPLE:
        return "example";

    case TLD_STATUS_UNDEFINED:
        return "undefined";

    case TLD_STATUS_EXCEPTION:
        return "exception";

    }

    return "unknown";
}


/** \brief This is for backward compatibility.
 *
 * Many times, a simple category is not useful because one TLD may actually
 * be part of multiple groups (i.e. a groups, a country, a language, an
 * entrepreneurial TLD can very well exist!)
 *
 * The idea is to be backward compatible for anyone who was using the old
 * category value. This function will convert the specified \p word in a
 * category. The word is expected to be a non null terminated string,
 * hence the parameter \p n to specify its length.
 *
 * \param[in] word  The word to convert.
 * \param[in] n  The exact number of characters in the word.
 *
 * \return The corresponding TLD_CATEGORY_... or TLD_CATEGORY_UNDEFINED if
 * the word could not be converted.
 */
enum tld_category tld_word_to_category(const char *word, int n)
{
    char buf[32];

    if(word != NULL
    && (size_t) n < sizeof(buf))
    {
        // force all lowercase
        //
        for(int idx = 0; idx < n; ++idx)
        {
            if(word[idx] >= 'A' && word[idx] <= 'Z')
            {
                buf[idx] = word[idx] | 0x20;
            }
            else
            {
                buf[idx] = word[idx];
            }
        }
        buf[n] = '\0';

        switch(buf[0])
        {
        case 'b':
            if(strcmp(buf, "brand") == 0)
            {
                return TLD_CATEGORY_BRAND;
            }
            break;

        case 'c':
            if(strcmp(buf, "country") == 0)
            {
                return TLD_CATEGORY_COUNTRY;
            }
            break;

        case 'e':
            if(strcmp(buf, "entrepreneurial") == 0)
            {
                return TLD_CATEGORY_ENTREPRENEURIAL;
            }
            break;

        case 'i':
            if(strcmp(buf, "international") == 0)
            {
                return TLD_CATEGORY_INTERNATIONAL;
            }
            break;

        case 'g':
            if(strcmp(buf, "group") == 0)
            {
                return TLD_CATEGORY_GROUP;
            }
            break;

        case 'l':
            if(strcmp(buf, "language") == 0)
            {
                return TLD_CATEGORY_LANGUAGE;
            }
            if(strcmp(buf, "location") == 0)
            {
                return TLD_CATEGORY_LOCATION;
            }
            break;

        case 'p':
            if(strcmp(buf, "professionals") == 0)
            {
                return TLD_CATEGORY_PROFESSIONALS;
            }
            break;

        case 'r':
            if(strcmp(buf, "region") == 0)
            {
                return TLD_CATEGORY_REGION;
            }
            break;

        case 't':
            if(strcmp(buf, "technical") == 0)
            {
                return TLD_CATEGORY_TECHNICAL;
            }
            break;

        }
    }

    return TLD_CATEGORY_UNDEFINED;
}


/* vim: ts=4 sw=4 et
 */
