dnl AX_APPEND_TO_VAR(VAR, CONTENTS) appends the elements of CONTENTS to VAR,
dnl unless already present in VAR.
dnl Works only for vars like CPPFLAGS, not for LIB* variables because that
dnl sometimes contains two or three consecutive elements that belong together.
AC_DEFUN([AX_APPEND_TO_VAR],
[
  for element in [$2]; do
    haveit=
    for x in $[$1]; do
      AS_IF([test "X$x" = "X$element"], [
        haveit=yes
        break
      ])
    done
    AS_IF([test -z "$haveit"], [ [$1]="${[$1]}${[$1]:+ }$element" ])
  done
])

dnl AX_REMOVE_FROM_VAR(VAR, CONTENTS) removes the elements of CONTENTS from VAR.
dnl Works only for vars like CPPFLAGS, not for LIB* variables because that
dnl sometimes contains two or three consecutive elements that belong together.
AC_DEFUN([AX_REMOVE_FROM_VAR],
[
  for element in [$2]; do
    [ax_save_$1]="${[$1]}"
    for x in $[ax_save_$1]; do
      AS_IF([test "X$x" != "X$element"], [[$1]="${[$1]}${[$1]:+ }$element"])
    done
  done
])
