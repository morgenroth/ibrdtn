AC_DEFUN([LOCAL_CHECK_IBRCOMMON], [
ibrcommon=notfound

if test -e "$(pwd)/../configure.in"; then
	# I think we are a subproject. Search for ibrcommon in ../
	if test -d "$(pwd)/../ibrcommon"; then
		# allow the pattern PKG_CONFIG_PATH
		m4_pattern_allow([^PKG_CONFIG_PATH$])
		
		# export the relative path of ibrcommon
		export PKG_CONFIG_PATH="$(pwd)/../ibrcommon"
		
		# check for the svn version of ibrcommon
		if pkg-config --atleast-version=$LOCAL_IBRCOMMON_VERSION ibrcommon; then
			# substitute pkg-config paths
			pkgpaths="--define-variable=includedir=$(pwd)/../ibrcommon --define-variable=libdir=$(pwd)/../ibrcommon/ibrcommon/.libs"
		
			# read LIBS options for ibrcommon
			ibrcommon_LIBS="$(pkg-config $pkgpaths --libs ibrcommon)"
			
			# read CFLAGS options for ibrcommon
			ibrcommon_CFLAGS="$(pkg-config $pkgpaths --cflags ibrcommon) -I$(pwd)/../ibrcommon"
			
			# some output
			echo "using relative ibrcommon library in $(pwd)/../ibrcommon"
			echo " with CFLAGS: $ibrcommon_CFLAGS"
			echo " with LIBS: $ibrcommon_LIBS"
			
			# set ibrcommon as available
			ibrcommon=yes
		fi 
	fi
fi

if test "x$ibrcommon" = "xnotfound"; then
	# check for ibrcommon library
	PKG_CHECK_MODULES([ibrcommon], [ibrcommon >= $LOCAL_IBRCOMMON_VERSION], [
		echo "using ibrcommon with CFLAGS: $ibrcommon_CFLAGS"
		echo "using ibrcommon with LIBS: $ibrcommon_LIBS"
	], [
		echo "ibrcommon library not found!"
		exit 1
	])
fi
])
