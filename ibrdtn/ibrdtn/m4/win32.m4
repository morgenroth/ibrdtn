# MINGW_AC_BUILD
# --------------------------
# Check if the runtime platform is a native Win32 host.
#
AC_DEFUN([MINGW_AC_BUILD],
[
	AC_COMPILE_IFELSE([
		AC_LANG_SOURCE([[
#ifndef _WIN32
 choke me
#endif
		]])
	], [
		enable_win32="yes"
		$1
	], [
		enable_win32="no"
		$2
	])
])# MINGW_AC_BUILD
