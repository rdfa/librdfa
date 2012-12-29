librdfa - The Fastest RDFa Processor on the Internet
====================================================

librdfa is a SAX-based [RDFa] processor written in C for XML and HTML 
family languages. It currently supports XML+RDFa, XHTML+RDFa, SVG+RDFa,
HTML4+RDFa and HTML5+RDFa for both RDFa 1.0 and RDFa 1.1.

Building and Running librdfa
----------------------------

Make sure that you have the following software installed on your system
before attempting to build librdfa:

    autoconf
    automake
    gcc
    make
    pkg-config
    libtool
    libxml2-dev

Make sure you have the following packages installed if you're attempting
to build the Python RDFa libraries:

    g++
    python-dev 
    swig

Make sure you have the following packages installed if you are attempting
to build the Debian/Ubuntu packages:

    dpkg-buildpackage
    dpkg-dev
    python-all-dev 
    dh-buildinfo 
    debhelper

Change into the librdfa source directory and run the following:

    ./autogen.sh
    ./configure
    make
    ./tests/speed 

If everything goes well, the speed test should be able to crank through
documents at 32 MB/sec and generate roughly 6.5k triples/sec.

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
