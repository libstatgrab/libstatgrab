dnl find flags to hide symbols from being exported in a shared library
dnl
dnl Motivation: prevent developers to use internal API
dnl
dnl Jens Rehsack <sno@NetBSD.org>

dnl AX_CHECK_VISIBILITY([action-if-found], [action-if-not-found])
dnl
AC_DEFUN([AX_CHECK_VISIBILITY],
[
    AC_REQUIRE([AC_PROG_CC])dnl
    AC_REQUIRE([AC_PROG_LD])dnl

    AC_CACHE_CHECK([how to control symbol visibility], [ax_cv_visibility],
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
	    ax_cv_visibility="gcc4"
	],
	[
	    AC_COMPILE_IFELSE(
	    [AC_LANG_SOURCE([
int __attribute__ ((dllexport)) foo() { return 1; }
	    ])],
	    [
		dnl GNU C 3 style
		ax_cv_visibility="gcc3"
	    ],
	    [
		AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([
int __hidden foo() { return 1; }
		])],
		[
		    dnl SUN Pro style
		    ax_cv_visibility="sunpro"
		],
		[
		    AC_COMPILE_IFELSE(
		    [AC_LANG_SOURCE([
int __declspec(dllexport) foo() { return 1; }
		    ])],
		    [
			dnl Microsoft style
			ax_cv_visibility="ms"
		    ],
		    [
			dnl probably add support for e.g. .def files (AIX, Windows)
			ax_cv_visibility="unknown"
		    ])
		])
	    ])
	])
	ac_[]_AC_LANG_ABBREV[]_werror_flag="$ac_save_[]_AC_LANG_ABBREV[]_werror_flag"
    ])

    AS_CASE([$ax_cv_visibility],
        [gcc4], [
            ax_visibility_export='__attribute__((visibility("default")))'
            ax_visibility_import='__attribute__((visibility("default")))'
            ax_visibility_private='__attribute__((visibility("hidden")))'
        ],
        [gcc3], [
            ax_visibility_export='__attribute__((dllexport))'
            ax_visibility_import='__attribute__((dllimport))'
            ax_visibility_private=''
        ],
        [sunpro], [
            ax_visibility_export='__global'
            ax_visibility_import='__global'
            ax_visibility_private='__hidden'
        ],
        [ms], [
            ax_visibility_export='__declspec(dllexport)'
            ax_visibility_import='__declspec(dllimport)'
            ax_visibility_private=''
        ], [
            ax_visibility_export=''
            ax_visibility_import=''
            ax_visibility_private=''
        ]
    )

    AS_IF([test "x$ax_cv_visibility" != "xunknown"],[m4_ifnblank([$1],[$1],[:])],[m4_ifnblank([$2],[$2],[:])])
])
