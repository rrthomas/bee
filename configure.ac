# configure.ac for Bee
# Process this file with autoconf to produce a configure script
#
# (c) Reuben Thomas 2011-2022
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AC_INIT(Bee, 1.0, rrt@sc3d.org)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Check for programs
AC_PROG_CC_C99
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

# Check for header files
AC_HEADER_STDC
gl_INIT

# Check features
AC_C_BIGENDIAN
AC_CHECK_SIZEOF([intptr_t]) # word_bytes
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
        src/private.h
        src/include/bee/bee.h
        tests/Makefile
])
AC_OUTPUT
