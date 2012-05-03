#!/bin/sh
#
# This script is used to perform a number of quality assurance tests on
# the source tree.

cd ..
test -d _build && chmod -R u+wx _build

if [ -z "$CPPFLAGS" ]
then
	# This is needed to get strdup(), snprintf(), strtok_r() ...
	#
	export CPPFLAGS="-D_POSIX_C_SOURCE=200809"
fi

( ####

set -ex

rm -rf _build
mkdir  _build
cd     _build

../configure --enable-python --enable-strict "$@"

make

make install DESTDIR=$PWD/_dest

make distcheck

) || exit ####

echo "QA build passed."

# EOF
