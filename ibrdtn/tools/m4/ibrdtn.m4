AC_DEFUN([LOCAL_CHECK_IBRDTN],
[
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
		if pkg-config --atleast-version=$LOCAL_IBRDTN_VERSION ibrdtn; then
			# read LIBS options for ibrdtn
			ibrdtn_LIBS="-L${ibrdtn_path}/ibrdtn/.libs ${ibrdtn_LIBS} ${ibrcommon_LIBS} $(pkg-config --libs ibrdtn)"
			
			# read CFLAGS options for ibrdtn
			ibrdtn_CFLAGS="-I${ibrdtn_path} ${ibrcommon_CFLAGS} $(pkg-config --cflags ibrdtn)"
			
			# some output
			AC_MSG_NOTICE([using ibrdtn library in ${ibrdtn_path}])
			AC_MSG_NOTICE([  with CFLAGS: $ibrdtn_CFLAGS])
			AC_MSG_NOTICE([  with LIBS: $ibrdtn_LIBS])
			
			# set ibrdtn as available
			ibrdtn=yes
		fi
	])

	AS_IF([test "x$ibrdtn" = "xnotfound"], [
		# check for ibrdtn library
		PKG_CHECK_MODULES([ibrdtn], [ibrdtn >= $LOCAL_IBRDTN_VERSION], [
			AC_MSG_NOTICE([using ibrdtn])
			AC_MSG_NOTICE([  with CFLAGS: $ibrdtn_CFLAGS])
			AC_MSG_NOTICE([  with LIBS: $ibrdtn_LIBS])
		], [
			AC_MSG_ERROR([ibrdtn library not found!])
		])
	])
])
