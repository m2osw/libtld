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
 * \brief Implementation of some functions converting numbers to strings.
 *
 * This file includes functions converting numbers to strings.
 */

#include "libtld/tld.h"


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




/* vim: ts=4 sw=4 et
 */
