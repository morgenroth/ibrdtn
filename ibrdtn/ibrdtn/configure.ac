dnl --------------------------------
dnl Package name and version number
dnl --------------------------------

# include version information
m4_include([version.m4])

dnl --------------------------------
dnl Initialization macros.
dnl --------------------------------

AC_INIT([ibrdtn], PKG_FULL_VERSION, morgenro@ibr.cs.tu-bs.de)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(ibrdtn/ibrdtn.h)
AC_CONFIG_HEADER(ibrdtn/config.h)
LT_INIT
AM_PATH_CPPUNIT(1.9.6)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl --------------------------------
dnl library version number
dnl --------------------------------

#API version
GENERIC_API_VERSION=PKG_MAJOR_VERSION.PKG_MINOR_VERSION

#shared library versioning
GENERIC_LIBRARY_VERSION=PKG_LIB_VERSION
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed

# define the library name
GENERIC_LIBRARY_NAME=$PACKAGE_NAME
GENERIC_VERSION=$PACKAGE_VERSION
GENERIC_RELEASE=PKG_MAJOR_VERSION.PKG_MINOR_VERSION

AC_SUBST(GENERIC_API_VERSION)
AC_SUBST(GENERIC_LIBRARY_VERSION)
AC_SUBST(GENERIC_LIBRARY_NAME)
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CXX
AC_PROG_CC

AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AM_SANITY_CHECK

AC_LANG_CPLUSPLUS

# ANDROID configuration
# We generate a mostly static configuration for Android as PKG_CHECK_MODULES obviously does not work with the Android NDK
AC_ARG_ENABLE([android],
	AS_HELP_STRING([--enable-android], [Enable compile switches for Android]))

AS_IF([test "x$enable_android" = "xyes"], [
	AC_MSG_WARN([Compiling for Android! Overriding other parameters!])

	# Check for presence of androgenizer
	AC_CHECK_PROG(ANDROGENIZER, androgenizer, androgenizer)
	AS_IF([test -z "$ANDROGENIZER"], [
		AC_MSG_ERROR(Unable to find androgenizer in your PATH.)
	])

	# Headers are defined in Application.mk, as it is impossible to link from here
	
	# Basic
	AC_DEFINE(ANDROID, [1], [Compiling for Android!])

	ibrcommon_LIBS="-librcommon"
	AC_SUBST(ibrcommon_LIBS)

	# OpenSSL
	#AC_DEFINE(WITH_BUNDLE_SECURITY, [1], [])
	#AC_DEFINE(HAVE_OPENSSL, [1], [])
	#OPENSSL_LIBS="-lssl -lcrypto"
	#AC_SUBST(OPENSSL_LIBS)
	#with_dtnsec="yes"
	with_dtnsec="no"

	# Compression
	AC_DEFINE(WITH_COMPRESSION, [1], [])
	AC_DEFINE(HAVE_ZLIB, [1], [])
	ZLIB_LIBS="-lz"
	AC_SUBST(ZLIB_LIBS)
	with_compression="yes"
],[
#ANDROID else begin
	AC_CHECK_HEADERS([netinet/in.h])
	AC_TYPE_SSIZE_T

	# Checks for libraries.
	AC_CHECK_LIB([rt], [clock_gettime])

	# check for ibrcommon library
	LOCAL_IBRCOMMON_VERSION=0.8
	LOCAL_CHECK_IBRCOMMON
	AC_SUBST(ibrcommon_CFLAGS)
	AC_SUBST(ibrcommon_LIBS)

	# Checks for header files.
	AC_CHECK_HEADERS([inttypes.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

	# Checks for typedefs, structures, and compiler characteristics.
	AC_HEADER_STDBOOL
	AC_C_INLINE
	AC_TYPE_INT32_T
	AC_TYPE_SIZE_T
	AC_TYPE_UINT16_T
	AC_TYPE_UINT32_T
	AC_TYPE_UINT8_T

	# Checks for library functions.
	AC_FUNC_MALLOC
	AC_FUNC_MKTIME
	AC_CHECK_FUNCS([gettimeofday sqrt])

	dnl -----------------------------------------------
	dnl optional parameter: DTN security support
	dnl -----------------------------------------------

	AC_ARG_WITH(dtnsec,
		AS_HELP_STRING([--with-dtnsec], [build with DTNSEC support
			@<:@default=no@:>@]),
	[
		AS_IF([test "x$with_dtnsec" = "xyes"], [
			PKG_CHECK_MODULES(OPENSSL, openssl, [
				AC_DEFINE(WITH_BUNDLE_SECURITY, [1], ["bundle security support enabled"])
				AC_DEFINE(HAVE_OPENSSL, [1], ["openssl library is available"])
				AC_SUBST(OPENSSL_CFLAGS)
				AC_SUBST(OPENSSL_LIBS)
				REQUIRES_LIBS="$REQUIRES_LIBS openssl"
				
				dnl -----------------------------------------------
				dnl check for glib support
				dnl -----------------------------------------------
				PKG_CHECK_MODULES([GLIB], [glib-2.0], [
					AC_SUBST(GLIB_CFLAGS)
					AC_SUBST(GLIB_LIBS)
					AC_DEFINE(HAVE_GLIB, [1], ["glib library is available"])
					REQUIRES_LIBS="$REQUIRES_LIBS glib-2.0"
				], [
					# alternatively endian header is required
					AC_CHECK_HEADERS([endian.h], [
						AC_MSG_NOTICE([use endian.h instead of glib.])
					], [
						AC_MSG_ERROR([glib or endian.h is necessary to include bundle security protocol!])
					])
				])
			], [
				with_dtnsec="no"
				AC_MSG_WARN([DTNSEC enabled, but no openssl libraries are found. Security extensions are disabled.])
			])
		], [
			with_dtnsec="no"
		])
	],[
		with_dtnsec="no"
	])

	dnl -----------------------------------------------
	dnl optional parameter: compression support
	dnl -----------------------------------------------

	AC_ARG_WITH(compression,
		AS_HELP_STRING([--with-compression], [build with compression support
			@<:@default=no@:>@]),
	[
		AS_IF([test "x$with_compression" = "xyes"], [
			PKG_CHECK_MODULES(ZLIB, zlib, [
				AC_DEFINE(WITH_COMPRESSION, [1], ["bundle compression support enabled"])
				AC_DEFINE(HAVE_ZLIB, [1], ["zlib library is available"])
				AC_SUBST(ZLIB_CFLAGS)
				AC_SUBST(ZLIB_LIBS)
				REQUIRES_LIBS="$REQUIRES_LIBS zlib"
			] , [
				AC_CHECK_LIB([z], [gzread],[
					AC_DEFINE(WITH_COMPRESSION, [1], ["bundle compression support enabled"])
					AC_DEFINE(HAVE_ZLIB, [1], ["zlib library is available"])
					ZLIB_CFLAGS=""
					ZLIB_LIBS="-lz"
					AC_SUBST(ZLIB_CFLAGS)
					AC_SUBST(ZLIB_LIBS)
					
					ZLIB_LIBS_EXTRA="-lz"
					AC_SUBST(ZLIB_LIBS_EXTRA)
				] , [
					with_compression="no"
					AC_MSG_ERROR([compression enabled, but zlib libraries are not found.])
				])
			])
		], [
			with_compression="no"
		])
	],[
		with_compression="no"
	])

#ANDROID else end
])

# Variables for Makefile.am
AM_CONDITIONAL(DTNSEC, test x$with_dtnsec = xyes)
AM_CONDITIONAL(COMPRESSION, test x$with_compression = xyes)
AM_CONDITIONAL(ANDROID, test x$enable_android = xyes)

AC_SUBST(REQUIRES_LIBS)

dnl -----------------------------------------------
dnl Generates Makefiles, configuration files and scripts
dnl -----------------------------------------------

AC_CONFIG_FILES([Makefile \
	ibrdtn.pc \
	ibrdtn/Makefile \
	ibrdtn/api/Makefile \
	ibrdtn/data/Makefile \
	ibrdtn/streams/Makefile \
	ibrdtn/utils/Makefile \
	ibrdtn/security/Makefile \
	tests/Makefile])
	
AC_OUTPUT
