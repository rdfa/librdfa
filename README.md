librdfa - The Fastest RDFa Processor on the Internet
====================================================

librdfa is a SAX-based [RDFa] processor written in C for XML and HTML 
family languages. It currently supports XML+RDFa, XHTML+RDFa, SVG+RDFa,
HTML4+RDFa and HTML5+RDFa for both RDFa 1.0 and RDFa 1.1.

Building and Running librdfa
----------------------------

Change into the librdfa source directory and run the following:

    ./autogen.sh
    ./configure
    make
    ./tests/speed 

If everything goes well, the speed test should show you being able to
process roughly 5k triples/sec.

Optionally, you can build the Python bindings with the following:

    ./configure --enable-python
    make

Development
-----------

pkg-config support is provided to setup cflags and libs. The .pc file is
installed along with development files. Add the librdfa source dir to
the `PKG_CONFIG_PATH` environment var to use the uninstalled library.

Building a simple program using librdfa pkg-config support:

    cc `pkg-config --libs --cflags librdfa` -o test test.c

Installation
------------

Use configure options to setup install directories.  For more information:

    ./configure --help

To install:

    make install

[RDFa]: http://rdfa.info/
