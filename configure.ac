# Configure script for Digital Bazaar librdfa library
# Usage: Run ./configure once 
#
# Author: Manu Sporny

AC_INIT(librdfa, 1.0, support@digitalbazaar.com)
AC_CONFIG_AUX_DIR(setup)

# Setup standard build environment variables
RDFADIR=`pwd`

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

echo -e "\n--------- Configuring librdfa Build Environment -----------"

# Check the system type
AC_CANONICAL_BUILD()

# Checking for standard build tools
#AC_LANG(C++)
AC_PROG_INSTALL()

# Check to see if we need to build for linux
if test "$ac_enable_linux_build" = "yes" ; then
   AC_PROG_CC()
   AC_CHECK_PROG(LINUX_AR, ar, ar, not found)
   LINUX_CC="$CC"
   BUILD_FOR_LINUX="yes"
fi

# Check to see if we need to build for windows
if test "$ac_enable_windows_build" = "yes" ; then
   AC_CHECK_PROG(WINDOWS_CC, i586-mingw32msvc-gcc, 
                 i586-mingw32msvc-gcc, not found)
   AC_CHECK_PROG(WINDOWS_AR, i586-mingw32msvc-ar, 
                 i586-mingw32msvc-ar, not found)
   AC_CHECK_PROG(WINDOWS_DLLWRAP, i586-mingw32msvc-dllwrap,
                 i586-mingw32msvc-dllwrap, not found)
   AC_CHECK_PROG(WINDOWS_AS, i586-mingw32msvc-as,
                 i586-mingw32msvc-as, not found)
   # If the mingw32 compiler wasn't found, use the system default
   if test "$WINDOWS_CC" = "not found" ; then
      AC_PROG_CC()
      WINDOWS_CC="$CC"
   fi

   BUILD_FOR_WINDOWS="yes"
fi

# Check doc build tools
if test "$ac_enable_docs_build" = "yes"; then
   AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, yes, no)
   AC_SUBST(HAVE_DOXYGEN)
fi

# Perform compilation environment tests
#AC_CHECK_HEADERS(iostream)

# Make sure the proper libraries exist
AC_CHECK_LIB(expat, main, [], AC_MSG_ERROR(could not find expat library))


# Make all the proper substitutions
AC_SUBST(RDFADIR)

# Make all the proper substitutions
AC_SUBST(BUILD_FOR_LINUX)
AC_SUBST(BUILD_FOR_WINDOWS)

AC_SUBST(LINUX_CC)
AC_SUBST(WINDOWS_CC)

AC_SUBST(LINUX_AR)
AC_SUBST(WINDOWS_AR)

AC_SUBST(WINDOWS_AS)
AC_SUBST(WINDOWS_DLLWRAP)

AC_SUBST(DBCOREDIR)
AC_SUBST(SIGNTOOL)

# Generating files
AC_OUTPUT(Makefile \
          c/Makefile \
          tests/Makefile)

# Dump the build configuration for the developer
echo -e "\n--------- Bitmunk Build Environment -----------"
echo "Product           : $PACKAGE_STRING"
echo "Build system      : $build_cpu-$build_vendor-$build_os"
if test "$BUILD_FOR_LINUX" = "yes"; then
   echo "Linux C++ compiler: $LINUX_CC"
   echo "Linux C++ archiver: $LINUX_AR"
fi
if test "$BUILD_FOR_WINDOWS" = "yes"; then
   echo "Win32 C++ compiler: $WINDOWS_CC"
   echo "Win32 C++ archiver: $WINDOWS_AR"
   echo "Win32 assembler   : $WINDOWS_AS"
   echo "Win32 DLL wrapper : $WINDOWS_DLLWRAP"
fi
if test "$ac_enable_docs_build" = "yes"; then
   echo "Documentation     : enabled"
else
   echo "Documentation     : disabled (use --enable-docs to enable)"
fi
