dnl --------------------------------
dnl Initialization macros.
dnl --------------------------------


AC_INIT(ibrdtn-tools, m4_normalize(m4_include([version.inc])), morgenro@ibr.cs.tu-bs.de)
AC_CONFIG_SRCDIR(src/dtnping.cpp)
AM_INIT_AUTOMAKE
LT_INIT
AM_PATH_CPPUNIT(1.9.6)

AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CXX
AC_PROG_CC

AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG_CPLUSPLUS

# Checks for libraries.
AC_CHECK_LIB([rt], [clock_gettime], [])
AC_CHECK_LIB([m], [pow], [])

# check for ibrdtn library
LOCAL_IBRDTN_VERSION=0.8
LOCAL_CHECK_IBRDTN
AC_SUBST(ibrdtn_CFLAGS)
AC_SUBST(ibrdtn_LIBS)

# Checks for header files required by dtntunnel
AC_CHECK_HEADERS([arpa/inet.h fcntl.h sys/ioctl.h sys/socket.h])
AC_CHECK_HEADER([linux/if_tun.h], [
		has_tun="yes"
])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AM_CONDITIONAL([TUNNEL], [test x$has_tun = xyes])

dnl -----------------------------------------------
dnl Generates Makefiles, configuration files and scripts
dnl -----------------------------------------------

AC_CONFIG_FILES([Makefile \
                 doc/Makefile \
                 man/Makefile \
                 src/Makefile])
	
AC_OUTPUT
