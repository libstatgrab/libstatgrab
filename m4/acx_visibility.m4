dnl find flags to hide symbols from being exported in a shared library
dnl
dnl Motivation: prevent developers to use internal API
dnl
dnl Jens Rehsack <sno@NetBSD.org>

dnl ACX_CHECK_VISIBILITY([action-if-found], [action-if-not-found])
dnl
AC_DEFUN([ACX_CHECK_VISIBILITY],
[
    AC_REQUIRE([AC_PROG_CC])dnl
    AC_REQUIRE([AC_PROG_LD])dnl

    AC_CACHE_CHECK([how to control symbol visibility], [acx_cv_visibility],
    [
	ac_save_[]_AC_LANG_ABBREV[]_werror_flag="$ac_[]_AC_LANG_ABBREV[]_werror_flag"
	AC_LANG_WERROR()
	AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([
int __attribute__ ((visibility("default"))) foo() { return 1; }
int __attribute__ ((visibility("hidden"))) bar() { return 1; }
int __attribute__ ((visibility("private"))) baz() { return 1; }
	])],
	[
	    dnl GNU C 4 style is supported
	    acx_cv_visibility="gcc4"
	],
	[
	    AC_COMPILE_IFELSE(
	    [AC_LANG_SOURCE([
int __attribute__ ((dllexport)) foo() { return 1; }
	    ])],
	    [
		dnl GNU C 3 style
		acx_cv_visibility="gcc3"
	    ],
	    [
		AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([
int __hidden foo() { return 1; }
		])],
		[
		    dnl SUN Pro style
		    acx_cv_visibility="sunpro"
		],
		[
		    AC_COMPILE_IFELSE(
		    [AC_LANG_SOURCE([
int __declspec(dllexport) foo() { return 1; }
		    ])],
		    [
			dnl Microsoft style
			acx_cv_visibility="ms"
		    ],
		    [
			dnl probably add support for e.g. .def files (AIX, Windows)
			acx_cv_visibility="unknown"
		    ])
		])
	    ])
	])
	ac_[]_AC_LANG_ABBREV[]_werror_flag="$ac_save_[]_AC_LANG_ABBREV[]_werror_flag"
    ])

    AS_CASE([$acx_cv_visibility],
        [gcc4], [
            acx_visibility_export='__attribute__((visibility("default")))'
            acx_visibility_import='__attribute__((visibility("default")))'
            acx_visibility_private='__attribute__((visibility("hidden")))'
        ],
        [gcc3], [
            acx_visibility_export='__attribute__((dllexport))'
            acx_visibility_import='__attribute__((dllimport))'
            acx_visibility_private=''
        ],
        [sunpro], [
            acx_visibility_export='__global'
            acx_visibility_import='__global'
            acx_visibility_private='__hidden'
        ],
        [ms], [
            acx_visibility_export='__declspec(dllexport)'
            acx_visibility_import='__declspec(dllimport)'
            acx_visibility_private=''
        ], [
            acx_visibility_export=''
            acx_visibility_import=''
            acx_visibility_private=''
        ]
    )

    AS_IF([test "x$acx_cv_visibility" != "xunknown"],[m4_ifnblank([$1],[$1],[:])],[m4_ifnblank([$2],[$2],[:])])
])
