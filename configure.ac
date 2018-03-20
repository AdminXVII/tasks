AC_INIT([tasks], [0.1], [dev.xlheureux@gmail.com])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror dist-xz])
AC_PROG_CC
AC_CONFIG_FILES([Makefile src/Makefile])

AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADER_STDBOOL
AC_FUNC_MALLOC
AC_TYPE_SSIZE_T
AC_TYPE_SIZE_T

PKG_PROG_PKG_CONFIG
AC_ARG_WITH([systemduserunitdir],
    [AS_HELP_STRING([--with-systemduserunitdir=DIR], [Directory for systemd service files])],,
    [with_systemduserunitdir=auto])
AS_IF([test "x$with_systemduserunitdir" = "xyes" -o "x$with_systemduserunitdir" = "xauto"], [
    def_systemduserunitdir=$($PKG_CONFIG --variable=systemduserunitdir systemd)

    AS_IF([test "x$def_systemduserunitdir" = "x"],
  [AS_IF([test "x$with_systemduserunitdir" = "xyes"],
   [AC_MSG_ERROR([systemd support requested but pkg-config unable to query systemd package])])
   with_systemduserunitdir=no],
  [with_systemduserunitdir="$def_systemduserunitdir"])])
AS_IF([test "x$with_systemduserunitdir" != "xno"],
     [AC_SUBST([systemduserunitdir], [$with_systemduserunitdir])])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemduserunitdir" != "xno"])
AC_OUTPUT
