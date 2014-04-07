AC_DEFUN([AC_CHECK_IBRDTN],
[
	AC_MSG_CHECKING([checking for ibrdtn >= $1])
	ibrdtn=notfound
	
	AC_ARG_WITH([ibrdtn],
		AS_HELP_STRING([--with-ibrdtn=PATH], [set the ibrdtn source path]), [
			ibrdtn_path=${withval}
		]
	)
	
	# check if we are in the dev environment
	AS_IF([test -z "${ibrdtn_path}" -a -e "$(pwd)/../ibrdtn/ibrdtn.pc"], [
		# set relative path
		ibrdtn_path="$(pwd)/../ibrdtn"
	])
	
	AS_IF([test -n "${ibrdtn_path}"], [
		# allow the pattern PKG_CONFIG_PATH
		m4_pattern_allow([^PKG_CONFIG_PATH$])
		
		# export the relative path of ibrdtn
		export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${ibrdtn_path}"
		
		# check for the svn version of ibrdtn
		if pkg-config --atleast-version=$1 ibrdtn; then
			# read LIBS options for ibrdtn
			ibrdtn_LIBS="-L${ibrdtn_path}/ibrdtn/.libs ${ibrdtn_LIBS} ${ibrcommon_LIBS} $(pkg-config --libs ibrdtn)"
			
			# read CFLAGS options for ibrdtn
			ibrdtn_CFLAGS="-I${ibrdtn_path} ${ibrcommon_CFLAGS} $(pkg-config --cflags ibrdtn)"
			
			# set ibrdtn as available
			ibrdtn=yes
			AC_MSG_RESULT([yes])
		fi
	])

	AS_IF([test "x$ibrdtn" = "xnotfound"], [
		# check for ibrdtn library
		PKG_CHECK_MODULES([ibrdtn], [ibrdtn >= $1], [
			AC_MSG_RESULT([yes])
		], [
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([ibrdtn library not found!])
		])
	])
	
	AC_SUBST(ibrdtn_CFLAGS)
	AC_SUBST(ibrdtn_LIBS)
])

AC_DEFUN([AC_CHECK_IBRDTN_BSP],
[
	AC_MSG_CHECKING([checking whether ibrdtn has BSP extensions])
	old_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$ibrdtn_CFLAGS"
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#include <ibrdtn/ibrdtn.h>
		]], [[
			#ifdef IBRDTN_SUPPORT_BSP
			// BSP is supported
			#else
			# BSP not supported
			#endif
		]])
	], [
		AC_MSG_RESULT([yes])
		$1
	], [
		AC_MSG_RESULT([no])
		$2
	])
	CPPFLAGS="$old_CPPFLAGS"
])

AC_DEFUN([AC_CHECK_IBRDTN_COMPRESSION],
[
	AC_MSG_CHECKING([checking whether ibrcommon has compression extensions])
	old_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$ibrdtn_CFLAGS"
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#include <ibrdtn/ibrdtn.h>
		]], [[
			#ifdef IBRDTN_SUPPORT_COMPRESSION
			// COMPRESSION is supported
			#else
			# COMPRESSION not supported
			#endif
		]])
	], [
		AC_MSG_RESULT([yes])
		$1
	], [
		AC_MSG_RESULT([no])
		$2
	])
	CPPFLAGS="$old_CPPFLAGS"
])
