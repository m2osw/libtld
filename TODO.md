
* Make the coverage 100% since I added a lot of code for v2.x it's not complete.
* Make sure the superstring is checked on both ends, I think I test only one.
* Further enhance the TLD definitions.
* Review the README.md and other docs to make them up to date to v2.x.
* Clean up the copyright notices.
* Verify that it still works with the latest version of PHP.
* Add a Debian package that installs the PHP extension (so auto-build that too).
* Look at adding a Node.js extension.
* Convert everything to C++ and offer an interface to C programmers.
* Look at how to handle the special cases in `tests/tld_test_tld_names.cpp`
  (i.e some definitions are just `*.er` in the public list).
* Add man pages for the tld extractor (extract-tld).
* Fixed the extract-tld tool errors on 65535 offsets (try with .arpa).


