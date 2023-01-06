# configure.ac for Bee
# Process this file with autoconf to produce a configure script
#
# (c) Reuben Thomas 2011-2023
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AC_PREREQ([2.71])

# Make inter-release version strings look like, e.g., v6.9-219-g58ddd, which
# indicates that it is built from the 219th delta (in _some_ repository)
# following the v6.9 tag, and that 58ddd is a prefix of the commit SHA1.
# The explicit URL can be removed when autoconf > 2.69 is released.
AC_INIT([Bee],
        [m4_esyscmd(build-aux/git-version-gen .tarball-version)],
        [rrt@sc3d.org],
        [bee],
        [https://github.com/rrthomas/bee/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

dnl Release number rules:
dnl This is derived from "Versioning" chapter of info libtool documentation.
dnl Format is MAJOR:MINOR:MICRO
dnl     4a) Increment major when removing or changing interfaces.
dnl     4a) 5) Increment minor when adding interfaces.
dnl     6) Set minor to zero when removing or changing interfaces.
dnl     3) Increment micro when interfaces not changed at all,
dnl               only bug fixes or internal changes made.
dnl     4b) Set micro to zero when adding, removing or changing interfaces.

dnl First extract pieces from the version number string
LIBBEE_MAJOR_VERSION=`echo $VERSION | awk -F. '{print $1}'`
LIBBEE_MINOR_VERSION=`echo $VERSION | awk -F. '{print $2}'`
LIBBEE_MICRO_VERSION=`echo $VERSION | awk -F. '{print $3}'`
LIBBEE_VERSION=$LIBBEE_MAJOR_VERSION.$LIBBEE_MINOR_VERSION.$LIBBEE_MICRO_VERSION$LIBBEE_MICRO_VERSION_SUFFIX
LIBBEE_VERSION_NUMBER=`expr $LIBBEE_MAJOR_VERSION \* 1000000 + $LIBBEE_MINOR_VERSION \* 1000 + $LIBBEE_MICRO_VERSION`

LIBBEE_SONUM=$LIBBEE_MAJOR_VERSION
AGE=`echo $LIBBEE_MINOR_VERSION | tr -d a-z`
REVISION=`echo $LIBBEE_MICRO_VERSION | awk -F- '{print $1}'`
CURRENT=`expr $LIBBEE_SONUM + $AGE`

dnl Version info for libraries = CURRENT:REVISION:AGE
VERSION_INFO=$CURRENT:$REVISION:$AGE

AC_SUBST(VERSION_INFO)
AC_SUBST(LIBBEE_MAJOR_VERSION)
AC_SUBST(LIBBEE_MINOR_VERSION)
AC_SUBST(LIBBEE_MICRO_VERSION)

# Check for programs
AC_PROG_CC
gl_EARLY
AM_PROG_AR
LT_INIT

# help2man
# Set a value even if not found, so that an invocation via build-aux/missing works
AC_PATH_PROG([HELP2MAN], [help2man], [help2man])

# Optimization
AX_CC_MAXOPT
AX_GCC_BUILTIN(__builtin_expect)
if test "$ax_cv_have___builtin_expect" = yes; then
  HAVE___BUILTIN_EXPECT=1
else
  HAVE___BUILTIN_EXPECT=0
fi
AC_SUBST([HAVE___BUILTIN_EXPECT])

# beetle-mijit
AC_ARG_WITH([mijit],
  [AS_HELP_STRING([--with-mijit], [use mijit-bee JIT compiler])],
  [case $withval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $withval for mijit option]) ;;
   esac
   with_mijit=$withval],
  [with_mijit=no]
)
if test "$with_mijit" = yes; then
  AC_PATH_PROG(CARGO, cargo)
  if test -z "$ac_cv_path_CARGO"; then
    AC_MSG_ERROR(cargo not found)
  fi
  AC_DEFINE(HAVE_MIJIT, 1, [Whether we are using Mijit.])
fi
AM_CONDITIONAL([HAVE_MIJIT], [test "$with_mijit" = yes])

# Extra warnings with GCC
AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--disable-gcc-warnings],
                  [turn off lots of GCC warnings])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=yes]
)
if test "$gl_gcc_warnings" = yes; then
  # Set up the list of undesired warnings.
  nw=
  nw="$nw -Wsystem-headers"   # Don’t let system headers trigger warnings

  gl_MANYWARN_ALL_GCC([warnings])

  # Enable all GCC warnings not in this list.
  gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
  for w in $warnings; do
    gl_WARN_ADD([$w])
  done

  # When compiling with GCC, prefer -isystem to -I when including system
  # include files, to avoid generating useless diagnostics for the files.
  ISYSTEM='-isystem '
else
  ISYSTEM='-I'
fi
AC_SUBST([ISYSTEM])

# Initialize gnulib
gl_INIT

# Windows-specific checks
AC_MSG_CHECKING([for native Win32])
native_win32=no
case "$host" in
  *-*-mingw*)
    native_win32=yes
    ;;
esac
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, test "x$native_win32" = "xyes")

# Check features
AC_C_BIGENDIAN
AC_CHECK_SIZEOF([intptr_t]) # word_bytes
AC_CHECK_SIZEOF([off_t])
SIZEOF_INTPTR_T=$ac_cv_sizeof_intptr_t
AS_IF([test "$SIZEOF_INTPTR_T" != 4 -a "$SIZEOF_INTPTR_T" != 8],
   AC_MSG_ERROR([sizeof(intptr_t) must be 4 or 8!]))
AC_SUBST([SIZEOF_INTPTR_T])
AX_C_ARITHMETIC_RSHIFT

# Package suffix for side-by-side installation of multiple builds
AC_ARG_ENABLE([package-suffix],
  [AS_HELP_STRING([--enable-package-suffix],
                  [add suffix denoting the word size, e.g. "-32", to binaries and man pages, enabling side-by-side installation of multiple builds.])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for package-suffix option]) ;;
   esac
   gl_package_suffix=$enableval],
  [gl_package_suffix=no]
)
if test "$gl_package_suffix" = yes; then
  PACKAGE_SUFFIX=-$(( $ac_cv_sizeof_intptr_t * 8 ))
fi
AC_SUBST([PACKAGE_SUFFIX])

# Generate output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
        lib/Makefile
        src/Makefile
        src/libbee.rc
        tests/Makefile
])
AC_OUTPUT
