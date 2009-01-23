librdfa - The Fastest RDFa Parser on the Internet
-------------------------------------------------

librdfa is a C-based RDFa parser for XML family languages. 
It currently supports XHTML+RDFa 1.0.

Building and running librdfa
----------------------------

Change into the librdfa directory (the same directory that contains
this README.txt file).

autoconf
./configure
make
export LD_LIBRARY_PATH=`pwd`/libs
./tests/dist/speed 

If everything goes well, the speed test should show you being able to
process roughly 10k triples/sec.
