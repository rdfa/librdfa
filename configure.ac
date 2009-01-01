# Configure script for Digital Bazaar librdfa library
# Usage: Run ./configure once 
# 
# This file is part of librdfa.
#
# librdfa is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# librdfa is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with librdfa. If not, see <http://www.gnu.org/licenses/>.
# 
# Author: Manu Sporny

AC_INIT(librdfa, 0.15, support@digitalbazaar.com)
AC_CONFIG_AUX_DIR(setup)

# Setup standard build environment variables
RDFADIR=`pwd`
RELEASE=`date +%Y%m%d`

# Check for particular build options
# Check for particular build options
AC_ARG_ENABLE([linux],
   AS_HELP_STRING([--enable-linux], [compile for linux [[yes]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_LINUX=yes ;;
         no) BUILD_FOR_LINUX=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-linux) ;;
      esac
   ], [BUILD_FOR_LINUX=yes])  dnl Default value

AC_ARG_ENABLE([windows],
   AS_HELP_STRING([--enable-windows], [compile for windows [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_WINDOWS=yes ;;
         no) BUILD_FOR_WINDOWS=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-windows) ;;
      esac
   ], [BUILD_FOR_WINDOWS=no])  dnl Default value

AC_ARG_ENABLE([macos],
   AS_HELP_STRING([--enable-macos], [compile for MacOS [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_MACOS=yes ;;
         no) BUILD_FOR_MACOS=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-macos) ;;
      esac
   ], [BUILD_FOR_MACOS=no])  dnl Default value

AC_ARG_ENABLE([python],
               AC_HELP_STRING([--enable-python],
                              [compile extensions for Python (default is NO)]),
              [ac_enable_python_build=yes], [ac_enable_python_build=no])

AC_ARG_ENABLE([docs],
               AC_HELP_STRING([--enable-docs],
                              [build documentation (default is NO)]),
              [ac_enable_docs_build=yes], [ac_enable_docs_build=no])

echo -e "\n--------- Configuring librdfa Build Environment -----------"

# Check the system type
AC_CANONICAL_BUILD()
AC_CANONICAL_HOST()
AC_CANONICAL_SYSTEM()
OS="$build_os"

# Checking for standard build tools
#AC_LANG(C++)
AC_PROG_INSTALL()

# Conditional setup for Linux build
if test "$BUILD_FOR_LINUX" = "yes" ; then
   AC_PROG_CC()
   AC_CHECK_PROG(AR, ar, ar, not found)
   AC_CHECK_PROG(AS, as, as, not found)
fi

# Conditional setup for Windows build
if test "$BUILD_FOR_WINDOWS" = "yes" ; then
   AC_CHECK_PROGS(CC, i586-mingw32msvc-gcc gcc, not found)
   AC_CHECK_PROGS(AR, i586-mingw32msvc-ar ar, not found)
   AC_CHECK_PROGS(AS, i586-mingw32msvc-as as, not found)
   AC_CHECK_PROGS(DLLWRAP, i586-mingw32msvc-dllwrap dllwrap, not found)
   OS="mingw32"
fi

# Conditional setup for MacOS build
if test "$BUILD_FOR_MACOS" = "yes" ; then
   AC_PROG_CC()
   AC_CHECK_PROG(AR, ar, ar, not found)
   AC_CHECK_PROG(AS, as, as, not found)
fi

CC="$CC"

# Check to see if we need to build the Python modules
if test "$ac_enable_python_build" = "yes" ; then
   AC_CHECK_FILE(/usr/include/python2.4/Python.h, [], AC_MSG_ERROR("no"))
   AC_CHECK_FILE(/usr/include/python2.5/Python.h, [], AC_MSG_ERROR("no"))

   BUILD_FOR_PYTHON="yes"
fi

# Check doc build tools
if test "$ac_enable_docs_build" = "yes"; then
   AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, yes, no)
   AC_SUBST(HAVE_DOXYGEN)
fi

# Perform compilation environment tests
#AC_CHECK_HEADERS(iostream)

# Make sure the proper libraries exist
AC_CHECK_LIB(expat, main, [], AC_MSG_ERROR(could not find expat library), -L/usr/lib -L/sw/lib)

# Set the OS so that we can do certain build modifications if needed


# Make all the proper substitutions
AC_SUBST(RDFADIR)

# Make all the proper substitutions
VERSION="$PACKAGE_VERSION"
AC_SUBST(VERSION)
AC_SUBST(RELEASE)

AC_SUBST(BUILD_FOR_LINUX)
AC_SUBST(BUILD_FOR_WINDOWS)
AC_SUBST(BUILD_FOR_PYTHON)

AC_SUBST(CC)
AC_SUBST(AR)
AC_SUBST(AS)
AC_SUBST(DLLWRAP)

AC_SUBST(OS)

# Generating files
AC_OUTPUT(Makefile \
          setup/Makefile.base \
          c/Makefile \
          python/Makefile \
          docs/Makefile \
          tests/Makefile \
          installers/debian/changelog)

# Dump the build configuration for the developer
echo -e "\n--------- librdfa Build Environment -----------"
echo "Product        : $PACKAGE_STRING-$RELEASE"
echo "Build target   : $build_cpu-$build_vendor-$build_os"
echo "C compiler     : $CC"
echo "Archiver       : $AR"
echo "Assembler      : $AS"
if test "$DLLWRAP" != "not found"; then
   echo "DLL wrapper    : $DLLWRAP"
fi

if test "$BUILD_FOR_PYTHON" = "yes"; then
   echo "Python modules : enabled"
else
   echo "Python modules : disabled (use --enable-python to enable)"
fi
if test "$ac_enable_docs_build" = "yes"; then
   echo "Documentation  : enabled"
else
   echo "Documentation  : disabled (use --enable-docs to enable)"
fi
