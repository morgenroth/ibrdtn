# ANDROID_AC_BUILD
# --------------------------
# Check if the runtime platform is Android.
#
AC_DEFUN([ANDROID_AC_BUILD],
[
	AC_ARG_ENABLE([android], AS_HELP_STRING([--enable-android], [Enable compile switches for Android]))
	AS_IF([test "x$enable_android" = "xyes"], [$1], [$2])
])
# ANDROID_AC_BUILD
