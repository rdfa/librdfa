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

AC_PREREQ([2.67])
AC_INIT([LibRDFa], [1.1], [support@digitalbazaar.com], [librdfa])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([c/rdfa.c])
AC_CONFIG_AUX_DIR([setup])
AC_CONFIG_MACRO_DIR([setup])

AM_INIT_AUTOMAKE([1.11.1 foreign])
AM_MAINTAINER_MODE

LIBRDFA_SO_VERSION="`echo $VERSION | tr . :`"
AC_SUBST([LIBRDFA_SO_VERSION])

AC_ARG_ENABLE([libxml2],
              [AS_HELP_STRING([--enable-libxml2],
                              [enable libxml2 as the default parser [default=yes]])],
              [enable_parser_libxml2=$enableval], [enable_parser_libxml2=yes])

AC_ARG_ENABLE([raptor],
              [AS_HELP_STRING([--enable-raptor],
                              [enable raptor as the default parser [default=no]])],
              [enable_parser_raptor=$enableval], [enable_parser_raptor=no])

AC_ARG_ENABLE([python],
              [AS_HELP_STRING([--enable-python],
                              [compile extensions for Python [default=no]])],
              [enable_python_build=$enableval], [enable_python_build=no])

AC_ARG_ENABLE([docs],
              [AS_HELP_STRING([--enable-docs],
                              [build documentation [default=no]])],
              [enable_docs_build=$enableval], [enable_docs_build=no])

AC_ARG_ENABLE([strict],
              [AS_HELP_STRING([--enable-strict],
                              [enable extra-strict flags for GCC [default=no]])],
              [enable_strict_build=$enableval], [enable_strict_build=no])

# Checking for standard build tools
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT
PKG_PROG_PKG_CONFIG([0.23])

AC_CANONICAL_HOST
mingw=no
case "$host" in
	*-*-mingw*) mingw32=yes ;;
esac
AM_CONDITIONAL([MINGW32], [test "$mingw32" = "yes"])

# Check to see if we need to build the Python modules
have_swig=no
if test "_$enable_python_build" = "_yes" ; then
   AX_PYTHON_DEVEL([>= '2.5'])
   AX_PKG_SWIG([2.0.0], [have_swig=yes])
   if test "$have_swig" = "yes"
   then
      AX_SWIG_ENABLE_CXX
      AX_SWIG_PYTHON([yes])
   fi
fi
AM_CONDITIONAL([BUILD_PYTHON], [test "$have_swig" = "yes"])

# Check default parser
if test "_$enable_parser_libxml2" = "_yes"; then
   PARSER="libxml2"
   dnl AM_PATH_XML2([2.6.26], , [AC_MSG_ERROR([Cannot find libxml2])])
   PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.6.26])
elif test "_$enable_parser_raptor" = "_yes"; then
   PARSER="raptor"
   PKG_CHECK_MODULES([RAPTOR2], [raptor2 >= 2.0.6])
else
   AC_MSG_ERROR([no parser has been enabled])
fi

AM_CONDITIONAL([PARSER_LIBXML2], [test "$PARSER" = "libxml2"])
AM_CONDITIONAL([PARSER_RAPTOR],  [test "$PARSER" = "raptor"])

# Check doc build tools
if test "_$enable_docs_build" = "_yes"; then
   AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen], [false])
fi
AM_CONDITIONAL([BUILD_DOCS], [test "_$enable_docs_build" = "_yes" -a "$DOXYGEN" != "false"])

# Perform compilation environment tests
#AC_CHECK_HEADERS(iostream)

# Check functions
AC_CHECK_FUNCS([strtok_r])

AM_CONDITIONAL([NEED_STRTOK_R], [test "$ac_cv_func_strtok_r" = "no"])

# Check extra-strict compiler flags for GCC
if test "_$enable_strict_build" = "_yes"; then
   if test "$GCC" != "yes"; then
      AC_MSG_ERROR([strict build is only supported for GCC])
   fi
   CFLAGS="-ansi -pedantic -fno-common -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wdeclaration-after-statement -Wdisabled-optimization -Wformat=2 -Winit-self -Winvalid-pch -Wlogical-op -Wmissing-declarations -Wnested-externs -Wold-style-definition -Wpointer-arith -Wstrict-prototypes -Wundef -Werror -Wno-error=cast-qual -Wno-error=overlength-strings -Wno-error=unused-parameter -Wno-error=unused-result -Wno-error=unused-variable $CFLAGS"
fi

# add newlines to internal output file list
backslash_n='\n'
CONFIGURE_GENERATED_FILES=`echo $ac_config_files | tr ' ' "$backslash_n"`
AC_SUBST([CONFIGURE_GENERATED_FILES])

# Generating files
AC_CONFIG_FILES([
   .gitignore
   Makefile
   c/Makefile
   python/Makefile
   docs/Makefile
   tests/Makefile
   installers/debian/changelog
   librdfa.pc
   librdfa-uninstalled.pc
])

AC_OUTPUT

# Dump the build configuration for the developer
cat <<EOF
--------- librdfa Build Environment -----------
Product        : $PACKAGE_STRING
Build host     : $build_cpu-$build_vendor-$build_os
C compiler     : $CC
Archiver       : $AR
Default parser : $PARSER
EOF

if test "_$enable_python_build" = "_yes"; then
   echo "Python modules : enabled"
else
   echo "Python modules : disabled (use --enable-python to enable)"
fi
if test "_$enable_docs_build" = "_yes"; then
   echo "Documentation  : enabled"
else
   echo "Documentation  : disabled (use --enable-docs to enable)"
fi
