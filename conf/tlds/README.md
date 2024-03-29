
# External TLD Definitions

The libtld reads a binary file from disk to get the list of TLDs.
On Unix, the path defaults to `/var/lib/libtld/tlds.tbl`. On MS-Windows,
it can be whatever you want.

This binary file is offered as is in the project. Each time that the tables
change, the project recompiles the file and has it for installation. That
makes it easier to deal with the file on OSes other than Unix.

The library also added a command line tool to allow you to recompile the
TLD files at any time. On Unix systems, the .ini TLD files are found under
`/usr/share/libtld/tlds/...`.

# The .ini Files

The new format is .ini which we parse as follow:

## Tokenization

We recognize a few tokens as described below. A .ini file is read one line
at a time. Then the characters within that line are tokenized and parsed.

We do not support line continuation, however, a string can include newline
and carriage return characters.

    "This
    is
    a
    valid
    string"

So when reading _one line_ the reader actually pays attention to strings.

The parsing views any character with code 160 (0xA0) and over as a Unicode
character which are accepted anywhere. Control characters (0 to 31 and 127
to 159) are viewed as erroneous unless they represent a blank
(`\n|\r\n|\r|\t|\f|\v`).

### Semi-colon (`;`)

The semi-colon is viewed as an end of line delimiter. It itself is not added
to the list of tokens found on a line. It breaks the line just like a newline
does, except that gives you the ability to enter multiple values on one line.

Using this capability is not recommended. It is there because it is supported
in regular .ini files.

### Brackets (`[` and `]`)

The brackets are used to define the name of a TLD. It is then followed
by a list of variables which further describe the TLD.

### Identifier (`[a-zA-Z_][a-zA-Z0-9_]*`)

An identifier is used for variable names. It can also appear as the value
of a variable.

### Word

A word is a character which is not currently recognize as the introducer
of another token. It can be followed by any other characters up to the next
blank.

### Equal (`=`)

The equal sign is used between a variable name and its value.

### Dot (`.`)

The dot is used to separate various TLD names.

**Note:** Obviously, this means the periods have nothing to do with a
possible hierarchy since these are _just_ domain name segment separators.
Similarly, the backslash is not a separator.

### Domain Name (Any Unicode character other than `.`, `*`, `[`, and `]`)

When defining a TLD name, any Unicode character is considered valid
except for the few characters used to separate TLD names, the wild
card, and the brackets which delimit the TLD name.

### Exception (`?`)

Although some TLD names are forbidden (such as `.ar` second level domains:
`my-own-domain.ar`), some countries have exceptions for governmental agencies.
These can be defined with the Exception character:

    [?.congresodelalengua3.ar]

The `reason` variable can also be set to "exception":

    [.congresodelalengua3.ar]
    reason=exception

### Wild Card (`*`)

The TLD name can include a wild card. This is denoted by an asterisk.

### Integer (`[0-9]+`)

The value of a variable can be an integer.

**Note:** We do not support floating points because that can cause problems
while parsing segments of a TLD.

### String (`"([^"]*|\\["\\])"` or `'([^']*|\\['\\])'`)

The value of a variable can be written inside a string. This is useful if you
have spaces, tabs, newlines, or some special character that you want to keep
in the string.

A string can include a quotation mark if escaped by a backslash. The backslash
can also be escaped by itself (see the Backslash section for more information).

Strings can be written between double or single quotes.

**Note:** a variable value can be written without quotes, in that case, the
          blanks between each token are collapsed to one space and the value
          cannot include newlines.

### Backslash (`\\.` or `\\xXX` or `\\uXXXX` or `\\UXXXXXX`)

We accept backslashed characters anywhere in the document.

The syntax accomodates special characters and Unicode characters of 2, 4, or
6 digits. You can start the number with zeroes if necessary.

The following are the supported backslash characters:

* `\\` -- insert a backslash
* `\"` -- insert a double quote
* `\'` -- insert a single quote
* '\0` -- Null character
* '\a` -- Bell/Alert/Audible
* '\b` -- Backspace, Bell character for some applications
* '\t` -- Tab character
* '\f` -- Form feed
* '\r` -- Carriage return
* '\n` -- Line feed
* '\v` -- Vertical Tab
* '\;` -- Semicolon
* '\#` -- Number sign
* '\=` -- Equals sign
* '\:` -- Colon
* `\xXX` -- insert the character defined by hexadecimal number XX
* `\uXXXX` -- insert the character defined by hexadecimal number XXXX
* `\UXXXXXX` -- insert the character defined by hexadecimal number XXXXXX


### Space (`[:blank:]+`)

Unprotected spaces (space, tab, etc.) that are not found in a String are
collapsed in one single space.

### End of Line (`\n|\r\n|\r`)

The end of the line is returned because a value ends at the end of the line.

The end of the line is mostly ignored otherwise (other than for counting
lines).

### Comment (`#.*`)

A comment is introduced by a `#` character and ends at the end of the line.
These are returned as an "End of Line" token instead of the comment.

Note that the `#` character is not viewed as the comment starter if found
within a word or a string.

## Parser

The grammar as defined as follow:

    start: global_variables tld_list

    global_variables: variable_list

    tld_list: tld
            | tld_list
            | EOL

    tld: '[' tld_exception dotted_tld composed_tld_name optional_dot ']' EOL variable_list

    tld_exception: <empty>
                 | EXCEPTION

    dotted_tld: DOT dotted_tld_list
              | dotted_tld_list
              | <empty>

    dotted_tld_list: dotted_tld_name
                   | dotted_tld_list dotted_tld_name

    dotted_tld_name: composed_tld_name DOT
                   | WILD_CARD DOT

    composed_tld_name: tld_name
                     | tld_name composed_tld_name

    tld_name: WORD
            | IDENTIFIER
            | NUMBER

    optional_dot: DOT
                | <empty>

    variable_list: variable EOL
                 | variable_list variable EOL
                 | EOL

    variable: IDENTIFIER EQUAL value

    value: item_list
         | STRING
         | <empty>

    item_list: item
             | item_list item

    item: IDENTIFIER
        | NUMBER
        | WORD
        | DOT
        | EXCEPTION
        | WILD_CARD
        | EQUAL

A true .ini file defines the `item` (the value of a variable) as being
anything and it trims spaces at the start and end only.

## Definitions

The parser generates definitions for each TLD name.

The global variables are used to pre-define variables of a TLD. These can
be overwritten once by a variable defined within the TLD name declaration.

Variables cannot be defined more than once. This is an error.

A TLD name cannot be defined more than once.

The following are the currently supported variables.

### `status=valid|exception|proposed|deprecated|unused|reserved|infrastructure|example`

The status of TLD name definition. We currently support:

* Valid (default if defined)
* Proposed -- not yet accepted (.mail)
* Deprecated -- was in use at some point, not usable anymore (.freight.aero)
* Unused -- the domain is assigned, it was never used (.ye)
* Infrastructure -- used by backends (.arpa)
* Example -- a domain which represents an example (test.ru)

The `exception` status can be indicated with the `?` special character
right after the opening bracket (`[?`) used to define the TLD name.

By default this parameter is set to `undefined`. The compiler will
automatically fix the value based on other values if still `undefined` once
ready to generate the output.

Pretty much all statuses other than `valid` mean that the TLD should be
viewed as undefined. For the most part, the other values are kept to track
changes over time. However, in some cases, they are required. For example,
the `.ye` TLD must be used with a second level name (i.e.
`<your-name>.com.ye`). That means the `.ye` entry must be marked as `unused`.

### `category=international|professionals|language|group|region|technical|country|entrepreneurial`

The `category` of the TLD name.

### `country=...`

The name of the country that owns this TLD name extension.

When this parameter is defined, the category must be `country`. If the
category is not defined, then `country` is automatically assumed.

This variable is required when a TLD category is marked as `country`.

### `nic=...`

This parameter defines the URL to the website managing this TLD name
when available.

# History

In the first version, I created one large XML file with all the TLD names.
That file is difficult to maintain and to read the XML I used the Qt library.
This added an unwelcome dependency (it's hard to deal with the Qt library on
various systems that do not support it by default).

The new version separates each TLD definition in a separate file. This is
much easier to maintain.

The format was also changed from XML to .ini files.

# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/libtld/issues).


vim: ts=4 sw=4 et

_This file is part of the [snapcpp project](https://snapwebsites.org/)._
