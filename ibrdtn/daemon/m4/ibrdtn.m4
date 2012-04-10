AC_DEFUN([LOCAL_CHECK_IBRDTN], [
ibrdtn=notfound

if test -e "$(pwd)/../configure.in"; then
	# I think we are a subproject. Search for ibrdtn in ../
	if test -d "$(pwd)/../ibrdtn"; then
		# allow the pattern PKG_CONFIG_PATH
		m4_pattern_allow([^PKG_CONFIG_PATH$])
		
		# export the relative path of ibrdtn
		export PKG_CONFIG_PATH="$(pwd)/../ibrcommon:$(pwd)/../ibrdtn"
		
		# check for the svn version of ibrdtn
		if pkg-config --atleast-version=$LOCAL_IBRDTN_VERSION ibrdtn; then
			# substitute pkg-config paths
			pkgpaths="--define-variable=includedir=$(pwd)/../ibrdtn --define-variable=libdir=$(pwd)/../ibrdtn/ibrdtn/.libs"
		
			# read LIBS options for ibrdtn
			ibrdtn_LIBS="$(pkg-config $pkgpaths --libs ibrdtn)"
			
			# read CFLAGS options for ibrdtn
			ibrdtn_CFLAGS="$(pkg-config $pkgpaths --cflags ibrdtn) -I$(pwd)/../ibrdtn"
			
			# some output
			echo "using relative ibrdtn library in $(pwd)/../ibrdtn"
			echo " with CFLAGS: $ibrdtn_CFLAGS"
			echo " with LIBS: $ibrdtn_LIBS"
			
			# set ibrdtn as available
			ibrdtn=yes
		fi 
	fi
fi

if test "x$ibrdtn" = "xnotfound"; then
	# check for ibrdtn library
	PKG_CHECK_MODULES([ibrdtn], [ibrdtn >= $LOCAL_IBRDTN_VERSION], [
		echo "using ibrdtn with CFLAGS: $ibrdtn_CFLAGS"
		echo "using ibrdtn with LIBS: $ibrdtn_LIBS"
	], [
		echo "ibrdtn library not found!"
		exit 1
	])
fi
])
