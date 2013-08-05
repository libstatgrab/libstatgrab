dnl @synopsis AX_WIN32([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl @summary figure out whether the target is native WIN32 or not

AC_DEFUN([AX_WIN32], [
AC_LANG_SAVE
AC_LANG_C
    AC_CACHE_CHECK(
        [whether compiling for native Win32],
        ax_cv_native_win32,
        [AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
                [[]],
                [[
                    #ifndef _WIN32
                    #error "Not Win32"
                    #endif
                ]]
            )],
            [ax_cv_native_win32=yes],
            [ax_cv_native_win32=no]
        )
    ])

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
AS_IF([test x"$ax_cv_native_win32" = xyes],[$1],[$2])
AC_LANG_RESTORE
])dnl AX_WIN32
