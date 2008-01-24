# Configure script for Digital Bazaar Bitmunk product line
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_INIT(bitmunk, 3.0, support@digitalbazaar.com)
AC_CONFIG_AUX_DIR(setup)

# Setup standard build environment variables
DBCOREDIR=`pwd`/../dbcore
BITMUNKDIR=`pwd`

# Check for particular build options
AC_ARG_ENABLE([linux],
               AC_HELP_STRING([--enable-linux],
                              [compile for linux (default is YES)]),
              [ac_enable_linux_build=yes], [ac_enable_linux_build=yes])

AC_ARG_ENABLE([windows],
               AC_HELP_STRING([--enable-windows],
                              [compile for windows (default is NO)]),
              [ac_enable_windows_build=yes], [ac_enable_windows_build=no])

AC_ARG_ENABLE([docs],
               AC_HELP_STRING([--enable-docs],
                              [build documentation (default is NO)]),
              [ac_enable_docs_build=yes], [ac_enable_docs_build=no])

echo -e "\n--------- Configuring Bitmunk Build Environment -----------"

# Check the system type
AC_CANONICAL_BUILD()

# Checking for standard build tools
#AC_LANG(C++)
AC_PROG_INSTALL()

# Check to see if we need to build for linux
if test "$ac_enable_linux_build" = "yes" ; then
   AC_PROG_CXX()
   AC_CHECK_PROG(LINUX_AR, ar, ar, not found)
   LINUX_CXX="$CXX"
   BUILD_FOR_LINUX="yes"
fi

# Check to see if we need to build for windows
if test "$ac_enable_windows_build" = "yes" ; then
   AC_CHECK_PROG(WINDOWS_CXX, i586-mingw32msvc-g++, 
                 i586-mingw32msvc-g++, not found)
   AC_CHECK_PROG(WINDOWS_AR, i586-mingw32msvc-ar, 
                 i586-mingw32msvc-ar, not found)
   AC_CHECK_PROG(WINDOWS_DLLWRAP, i586-mingw32msvc-dllwrap,
                 i586-mingw32msvc-dllwrap, not found)
   AC_CHECK_PROG(WINDOWS_AS, i586-mingw32msvc-as,
                 i586-mingw32msvc-as, not found)
   # If the mingw32 compiler wasn't found, use the system default
   if test "$WINDOWS_CXX" = "not found" ; then
      AC_PROG_CXX()
      WINDOWS_CXX="$CXX"
   fi

   BUILD_FOR_WINDOWS="yes"
fi

# Check doc build tools
if test "$ac_enable_docs_build" = "yes"; then
   AC_CHECK_PROG(HAVE_RST2HTML, rst2html, yes, no)
   AC_SUBST(HAVE_RST2HTML)
fi

# Check for the Netscape signtool (used for signing Firefox XPI files)
AC_CHECK_PROG(SIGNTOOL, signtool,
              /usr/local/nss-3.9/bin/signtool, not found,
              /usr/local/nss-3.9/bin/)

# Perform compilation environment tests
#AC_CHECK_HEADERS(iostream)

# Make sure the proper libraries exist
#AC_CHECK_LIB(pthread, main, [], AC_MSG_ERROR(could not find pthread library))
#AC_CHECK_LIB(crypto, main, [], AC_MSG_ERROR(could not find crypto library))
#AC_CHECK_LIB(ssl, main, [], AC_MSG_ERROR(could not find ssl library))
#AC_CHECK_LIB(expat, main, [], AC_MSG_ERROR(could not find expat library))
#AC_CHECK_LIB(sqlite3, main, [], AC_MSG_ERROR(could not find sqlite3 library))
#AC_CHECK_LIB(mysqlclient, main, [], AC_MSG_ERROR(could not find mysqlclient library))


# Make all the proper substitutions
AC_SUBST(DBCOREDIR)
AC_SUBST(BITMUNKDIR)

# Make all the proper substitutions
AC_SUBST(BUILD_FOR_LINUX)
AC_SUBST(BUILD_FOR_WINDOWS)

AC_SUBST(LINUX_CXX)
AC_SUBST(WINDOWS_CXX)

AC_SUBST(LINUX_AR)
AC_SUBST(WINDOWS_AR)

AC_SUBST(WINDOWS_AS)
AC_SUBST(WINDOWS_DLLWRAP)

AC_SUBST(DBCOREDIR)
AC_SUBST(SIGNTOOL)

# Generating files
AC_OUTPUT(Makefile \
          setup/Makefile.base \
          setup/docs.doxygen \
          docs/Makefile \
          rt/cpp/Makefile \
          common/cpp/Makefile \
          database/cpp/Makefile \
          data/cpp/Makefile \
          protocol/cpp/Makefile \
          mail/cpp/Makefile \
          bfp/cpp/Makefile \
          bfp/v1_1/cpp/Makefile \
          purchase/cpp/Makefile \
          sell/cpp/Makefile \
          customcatalog/cpp/Makefile \
          sva/cpp/Makefile \
          uva/cpp/Makefile \
          webui/cpp/Makefile \
          apps/cpp/bitmunk/Makefile \
          apps/cpp/bitmunk/bitmunk.h \
          apps/cpp/firefox/Makefile \
          apps/cpp/firefox/components/Makefile \
          test/cpp/Makefile \
          test/cpp/bitmunk-unit-tests.h)

# Dump the build configuration for the developer
echo -e "\n--------- Bitmunk Build Environment -----------"
echo "System            : $PACKAGE_STRING"
echo "Build system      : $build_cpu-$build_vendor-$build_os"
if test "$BUILD_FOR_LINUX" = "yes"; then
   echo "Linux C++ compiler: $LINUX_CXX"
   echo "Linux C++ archiver: $LINUX_AR"
fi
if test "$BUILD_FOR_WINDOWS" = "yes"; then
   echo "Win32 C++ compiler: $WINDOWS_CXX"
   echo "Win32 C++ archiver: $WINDOWS_AR"
   echo "Win32 assembler   : $WINDOWS_AS"
   echo "Win32 DLL wrapper : $WINDOWS_DLLWRAP"
fi
if test "$ac_enable_docs_build" = "yes"; then
   echo "Documentation     : enabled"
else
   echo "Documentation     : disabled (use --enable-docs to enable)"
fi
if test "$SIGNTOOL" = "not found"; then
   echo "Firefox XPIs      : unsigned (signtool not found)"
else
   echo "Firefox XPIs      : signed"
fi
