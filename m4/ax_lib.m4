dnl Find a library and update compiler flags
dnl
dnl Motivation: lib-link.m4/AC_LIB_HAVE_LINKFLAGS doesn't work with
dnl             prefixed path's on AIX/HPUX
dnl
dnl Jens Rehsack <sno@NetBSD.org>

dnl AX_CHECK_WITH_LIB(name,[default])
dnl Declares configure options
dnl   --with-${name}
dnl   --without-${name}
dnl   --with-lib${name}-prefix
dnl   --without-lib${name}-prefix
dnl If neither --with-${name} nor --without-${name} is specified for configure
dnl   run, the default (default: check) is taken

AC_DEFUN([AX_CHECK_WITH_LIB],
[
  dnl AC_REQUIRE(AC_PROG_SED)dnl
  define([Name],[patsubst(translit([$1],[./+-], [____]),[_*$])])
  define([NAME],[patsubst(translit([$1],[abcdefghijklmnopqrstuvwxyz./+-],
                                        [ABCDEFGHIJKLMNOPQRSTUVWXYZ____]),[_*$])])
  AC_CACHE_CHECK([if lib[]$1 is wanted], [ax_cv_with_lib[]Name], [
    ifelse([], [$2], [ax_with_lib[]Name[]_default="check"], [ax_with_lib[]Name[]_default="$2"])
    AC_ARG_WITH(Name, [
  --with-[]Name[]                 will check for $1
  --without-[]Name[]              will not check for $1],
      [ax_with_lib[]Name="${withval}"],
      [ax_with_lib[]Name="${ax_with_lib[]Name[]_default}"]
    )
    ax_cv_with_lib[]Name="${ax_with_lib[]Name}"])

  AC_CACHE_CHECK([if lib[]$1 wants a prefix], [ax_cv_with_lib[]Name[]_prefix], [
    AS_IF(
      [test "x${ax_with_lib[]Name[]}" != "xno"],
      [
        AC_ARG_WITH([lib[]Name[]-prefix],
[  --with-lib[]Name[]-prefix[[=DIR]] search for $1 in DIR/include and DIR/lib
  --without-lib[]Name[]-prefix    search for $1 in DIR/include and DIR/lib],
	  [
	    ax_with_lib[]Name[]_prefix="`echo ${withval} | ${SED} -e 's|/*$||'`"
          ],
          [ax_with_lib[]Name[]_prefix=""]
        )
      ],
      [ax_with_lib[]Name[]_prefix=""]
    )
    AS_IF([test "x${ax_with_lib[]Name[]_prefix}" != "x"], [ax_cv_with_lib[]Name[]_prefix="${ax_with_lib[]Name[]_prefix}"], [ax_cv_with_lib[]Name[]_prefix="no"])
  ])
])

dnl AX_CHECK_LIB_COMPILE(name, dependencies, function, [includes])
dnl tries to compile given test for a library
dnl Note in difference to AC_SEARCH_LIBS this macro allowes approval of
dnl API, too.
dnl
dnl if succeeds, found_[]name is set

AC_DEFUN([AX_CHECK_LIB_COMPILE],
[
  define([Name],[patsubst(translit([$1],[./+-], [____]),[_*$])])
  define([testcode], ifelse(x"regexp([$3], [[^a-zA-Z_]])", x"-1", [void *p = &$3], [$3]))

  AX_APPEND_TO_VAR([LIBS], ${Name[]_LIBS})

  m4_ifnblank([$2], [AC_MSG_CHECKING([if lib[]$1[] can be linked with $2])], [AC_MSG_CHECKING([if lib[]$1[] can be linked alone])])
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([$4], [testcode])],
    [AC_MSG_RESULT([yes])
found_[]Name[]="yes"],
    [AC_MSG_RESULT([no])]
  )
  LIBS="${ax_safe_libs}"
])

dnl _AX_DEFAULT_LINKEXT_REQUIREMENTS
AC_DEFUN([_AX_DEFAULT_LINKEXT_REQUIREMENTS],
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
AS_CASE(["$host"],
         [*-[[Hh]][[Pp]][[Uu]][[Xx]]*], [ax_linkext_default_shared=".sl";
	      ax_linkext_default_archive=".a";
	      ax_linkext_default_objext="$OBJEXT"],
	 [*-[[Dd]][[Aa]][[Rr]][[Ww]][[Ii]][[Nn]]*], [ax_linkext_default_shared=".dylib";
	      ax_linkext_default_archive=".a";
	      ax_linkext_default_objext="$OBJEXT"],
	     [ax_linkext_default_shared=".so";
	      ax_linkext_default_archive=".a";
	      ax_linkext_default_objext="$OBJEXT"])
ax_default_linkexts="$ax_linkext_default_shared:$ax_linkext_default_archive:$ax_linkext_default_objext"
])dnl

AC_DEFUN([_AX_SEARCH_PATH],
[${ax_with_lib[]$1[]_prefix} ${acl_final_exec_prefix} ${acl_final_prefix} ${PRODUKTDIR} /usr/pkg /opt/freeware /usr/local /usr])

dnl AX_CHECK_LIB_FLAGS(name, dependencies, function, [includes],
dnl                    [pkg-config-module], [action-if-found],
dnl                    [action-if-not-found])
dnl searches for lib[]name and the libraries corresponding to explicit and
dnl implicit dependencies, together with the specified include files and
dnl the ability to compile and link the specified function.
dnl If found, action-if-found is run, action-if-not-found otherwise.
dnl Action if found is to set and AC_SUBST HAVE_LIB${NAME}, LIBS_${NAME},
dnl   and INC_${NAME} and run action-if-found when specified
dnl Action if not found is to bail out with error when --with-${name} is
dnl   specified as configure flag or LIB${NAME} is mandatory
dnl   (ax_cv_with_lib${name} != "no" - see AX_CHECK_WITH_LIB) after
dnl   ran action-if-not-found when specified
dnl
dnl TODO support lib32/lib64 (libstem)
dnl TODO support AC_SEARCH_LIBS like multiple libs in addition
dnl TODO support other languages than C/C++
dnl TODO support re-run without unset ax_cv_found_lib[]Name for other
dnl      includes

AC_DEFUN([AX_CHECK_LIB_FLAGS],
[
  dnl AC_REQUIRE([AC_LIB_RPATH])
  dnl AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  AC_REQUIRE([AC_PROG_GREP])
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])
  AC_REQUIRE([_AX_DEFAULT_LINKEXT_REQUIREMENTS])

  define([libtitle], m4_car($1))
  define([libfiles], [m4_ifnblank(m4_cdr($1),m4_cdr($1),libtitle)])

  AX_CHECK_WITH_LIB(libtitle,[check])

  define([Name],[patsubst(translit(libtitle,[./+-], [____]),[_*$])])
  define([NAME],[patsubst(translit(libtitle,[abcdefghijklmnopqrstuvwxyz./+-],
                                            [ABCDEFGHIJKLMNOPQRSTUVWXYZ____]),[_*$])])

  define([header_file], [regexp([$4], [.*include[ 	]+["<]\([^">]+\)[">].*], [\1])])

  define([search_path], [_AX_SEARCH_PATH(Name)])

  AS_IF(
    [test "x${ax_with_lib[]Name[]}" != "xno"],
    [
      AS_VAR_SET_IF([ax_cv_found_lib[]Name], , [
        AS_IF([test m4_ifnblank(header_file, ["x${Name[]_CFLAGS}" = "x" -a]) "x${Name[]_LIBS}" = "x"], [
          m4_ifnblank(header_file, [ax_safe_cppflags="${CPPFLAGS}"])
          ax_safe_libs="${LIBS}"

          m4_ifnblank([$5], [
            ax_safe_pkg_config_path="${PKG_CONFIG_PATH}"
            PKG_CONFIG_PATH=""
            ax_pkg_config_path_sep=""
            for dir in search_path;
            do
              AS_IF([test -d "${dir}/lib/pkgconfig" -a -z "`AS_ECHO $PKG_CONFIG_PATH | $GREP ${dir}/lib/pkgconfig`"], [
                PKG_CONFIG_PATH="${PKG_CONFIG_PATH}${ax_pkg_config_path_sep}${dir}/lib/pkgconfig"
                ax_pkg_config_path_sep=$PATH_SEPARATOR
              ])
            done
            AS_IF([test -n "${ax_safe_pkg_config_path}"], [PKG_CONFIG_PATH="${PKG_CONFIG_PATH}${ax_pkg_config_path_sep}${ax_safe_pkg_config_path}"])
            export PKG_CONFIG_PATH
            PKG_CHECK_MODULES(Name, [$5], [
              m4_ifnblank(header_file, [AX_APPEND_TO_VAR([CPPFLAGS], ${Name[]_CFLAGS})])
              AX_CHECK_LIB_COMPILE(libtitle, [flags from pkg-config], [$3], [$4])
              ax_save_Name[]_LIBS="${Name[]_LIBS}"
              m4_ifnblank([$2], [
                m4_foreach([libdep], [$2], [
                  AS_IF([test "x${found_[]Name[]}" != "xyes"], [
                    Name[]_LIBS="${ax_save_Name[]_LIBS} m4_normalize(m4_foreach_w([lib], libdep, [ -l[]lib]))"
                    AX_CHECK_LIB_COMPILE(libtitle, [flags from pkg-config and "m4_normalize(m4_foreach_w([lib], libdep, [ -l[]lib]))"], [$3], [$4])
                  ])
                ])
              ])
            ], [
              found_[]Name[]_lib=
            ])
            PKG_CONFIG_PATH="${ax_safe_pkg_config_path}"
          ])

          ax_search_lib_path_checked=":"
          AS_IF([test "x_$found_[]Name" != "x_yes"], [
            for dir in search_path;
            do
              AS_IF([test -d "${dir}" -a -z "`AS_ECHO $ax_search_lib_path_checked | $GREP :${dir}:`"], [
                ax_search_lib_path_checked="${ax_search_lib_path_checked}${dir}:"
                AC_MSG_CHECKING([if lib[]libtitle[] is in $dir])

                m4_ifnblank(header_file, [
                  found_[]Name[]_include=
                  if test -f "$dir/include/[]header_file[]"; then
                    found_[]Name[]_include="yes";
                  fi
                ])
                m4_foreach_w([libnm], libfiles, [
                  define([libfnm],[patsubst(translit(libnm,[./+-], [____]),[_*$])])
                  found_[]libfnm[]_lib=
                  if test -f "$dir/lib/lib[]libnm[]$ax_linkext_default_shared"; then
                    found_[]libfnm[]_lib="yes";
                  elif test -f "$dir/lib/lib[]libnm[]$ax_linkext_default_archive"; then
                    found_[]libfnm[]_lib="yes";
                  elif test -f "$dir/lib/[]libnm[]$ax_linkext_default_objext"; then
                    found_[]libfnm[]_lib="yes";
                  fi
                ])
                AS_IF(
                  [test m4_ifnblank(header_file, ["x${found_[]Name[]_include}" != "x"], [true]) m4_foreach_w([libnm], libfiles, [define([libfnm],[patsubst(translit(libnm,[./+-], [____]),[_*$])]) -a "x${found_[]libfnm[]_lib}" != "x"])],
                  [
                    AC_MSG_RESULT([yes])
                    m4_ifnblank(header_file, [
                      Name[]_CFLAGS="-I$dir/include"
                      AX_APPEND_TO_VAR([CPPFLAGS], ${Name[]_CFLAGS})
                    ])
                    Name[]_LIBS="-L$dir/lib m4_normalize(m4_foreach_w([lib], libfiles, [ -l[]lib]))"
                    AX_CHECK_LIB_COMPILE(libtitle, [], [$3], [$4])
                    AS_IF([test "x${found_[]Name[]}" = "xyes"], [break])
                    m4_ifnblank([$2], [
                        m4_foreach([libdep], [$2], [
                          Name[]_LIBS="-L$dir/lib m4_normalize(m4_foreach_w([lib], libfiles libdep, [ -l[]lib]))"
                          AX_CHECK_LIB_COMPILE(libtitle, m4_normalize(m4_foreach_w([lib], libdep, [ -l[]lib])), [$3], [$4])
                          AS_IF([test "x${found_[]Name[]}" = "xyes"], [break])
                        ])
                    ])
                  ],
                  [AC_MSG_RESULT([no])])

                AS_IF([test "x${found_[]Name[]}" = "xyes"], [break])
                m4_ifnblank(header_file, [CPPFLAGS="${ax_safe_cppflags}"])
              ])
            done
          ])
          m4_ifnblank(header_file, [CPPFLAGS="${ax_safe_cppflags}"])
          LIBS="${ax_safe_libs}"
          ax_cv_found_lib[]Name="${found_[]Name}"
        ], [
          found_[]Name[]="yes"
          ax_cv_found_lib[]Name="${found_[]Name}"
        ])
      ])
      AS_IF(
        [test "x_$found_[]Name" != "x_yes"],
        [
          $7
          AS_IF([test "x${ax_with_lib[]Name[]}" = "xyes"],[AC_MSG_ERROR([Cannot find suitable lib[]libtitle library])])
        ],
        [
          AS_IF([test -n "${Name[]_CFLAGS}"], [
            INC_[]NAME=""
            INC_[]NAME[]_SEP=""
            for incpm in ${Name[]_CFLAGS}
            do
              INC_[]NAME="${INC_[]NAME}${INC_[]NAME[]_SEP}${incpm}"
              INC_[]NAME[]_SEP=" "
            done
            dnl INC_[]NAME="${Name[]_CFLAGS}"
            AC_SUBST(INC_[]NAME)
          ])
          LIBS_[]NAME=""
          LIBS_[]NAME[]_SEP=""
          for libpm in ${Name[]_LIBS}
          do
            LIBS_[]NAME="${LIBS_[]NAME}${LIBS_[]NAME[]_SEP}${libpm}"
            LIBS_[]NAME[]_SEP=" "
          done
          AC_SUBST(LIBS_[]NAME)
          HAVE_LIB[]NAME=yes
          AC_SUBST(HAVE_LIB[]NAME)
          $6
        ])
    ],
    [
      $7
      HAVE_LIB[]NAME=
    ]
  )
])
