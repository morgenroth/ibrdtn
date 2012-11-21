AC_DEFUN([LOCAL_CHECK_IBRCOMMON],
[
	ibrcommon=notfound

	AC_ARG_WITH([ibrcommon],
		AS_HELP_STRING([--with-ibrcommon=PATH], [set the ibrcommon source path]), [ibrcommon_path=${withval}]
	)
	
	AS_IF([test -z "${ibrcommon_path}" -a -e "$(pwd)/../configure.in" -a -d "$(pwd)/../ibrcommon"], [
		# set relative path
		ibrcommon_path="$(pwd)/../ibrcommon"
	])
	
	AS_IF([test -n "${ibrcommon_path}"], [
		# allow the pattern PKG_CONFIG_PATH
		m4_pattern_allow([^PKG_CONFIG_PATH$])
		
		# export the path of ibrcommon
		export PKG_CONFIG_PATH="${ibrcommon_path}"
		
		# check for the svn version of ibrcommon
		if pkg-config --atleast-version=$LOCAL_IBRCOMMON_VERSION ibrcommon; then
			# read LIBS options for ibrcommon
			ibrcommon_LIBS="-L${ibrcommon_path}/ibrcommon/.libs $(pkg-config --libs ibrcommon)"
			
			# read CFLAGS options for ibrcommon
			ibrcommon_CFLAGS="-I${ibrcommon_path} $(pkg-config --cflags ibrcommon)"
			
			# some output
			AC_MSG_NOTICE([using ibrcommon library in ${ibrcommon_path}])
			AC_MSG_NOTICE([  with CFLAGS: $ibrcommon_CFLAGS])
			AC_MSG_NOTICE([  with LIBS: $ibrcommon_LIBS])
			
			# set ibrcommon as available
			ibrcommon=yes
		fi
	])
	
	AS_IF([test "x${ibrcommon}" = "xnotfound"], [
		# check for ibrcommon library
		PKG_CHECK_MODULES([ibrcommon], [ibrcommon >= $LOCAL_IBRCOMMON_VERSION], [
			AC_MSG_NOTICE([using ibrcommon])
			AC_MSG_NOTICE([  with CFLAGS: $ibrcommon_CFLAGS])
			AC_MSG_NOTICE([  with LIBS: $ibrcommon_LIBS])
		], [
			AC_MSG_ERROR([ibrcommon library not found!])
		])
	])
])
