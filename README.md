
# Introduction

`libtld` is an easy to use C/C++/PHP library that gives you information
about domain names (TLD) found in a URI. The result can be used to know
whether someone entered a valid URI without having to hit the Internet.
It is also important in the event you wanted to check an SSL certificate
(i.e. the exact TLD + one name must be checked in that case.)


# Official source of TLDs

ICANN has a [list of all the top-level domain
names](https://www.icann.org/resources/pages/tlds-2012-02-25-en).

Note that some of the entries may or may not be working. They say that they
maintain this list.

We also use the [list from
Mozilla](https://publicsuffix.org/list/public_suffix_list.dat) which
is likely up to date because many companies make use of it and send
them requests to make sure it gets as close as possible to reality.
Note that this list also includes sub-domain resellers (i.e. some companies
are built around selling a sub-domain of a domain they purchased.)




# Notes:

* Version 1.5.9 has been updated to the latest version of the world TLDs.
  It also includes a fix to the email code so spaces are forbidden.

* Version 1.5.0 offers a function to fold the characters of a domain
  to lowercase; including support for %XX encoding and UTF-8.

* Version 1.4.17 and following version includes even more new
  global TLD entries.
  
* "Intermediate" versions are "lost" to our nightly build server.

* Version 1.4.6 includes many (most) of the new allowed global TLD entries.
  (INA expects about 960 new TLDs within the next few years.)

* Version 1.4.3, 1.4.4, and 1.4.5 are missing because these were used to
  finish up the Debian installer. Although one version included a memory
  leak but I did not then find the time to upload a new version here.

* The .deb versions for Ubuntu can be downloaded from our `snapcpp` PPA
  on launchpad (this is not being updated anymore...):

      https://code.launchpad.net/~snapcpp/+archive/ppa/+packages

* The PHP extension was not downloaded in the previous version, so I
  won't upload a new version here. It did not change anyway.


# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/libtld/issues).


_This file is part of the [snapcpp project](https://snapwebsites.org/)._
