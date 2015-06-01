#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE(game, 1.00)
AC_CONFIG_SRCDIR([cgi/mMainMoneyResult.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC


# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([string.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday memmove])

AC_CONFIG_FILES(Makefile \
				adm/Makefile \
				server/Makefile \
				common/Makefile \
				cron/Makefile \
				cgi/Makefile)
AC_OUTPUT