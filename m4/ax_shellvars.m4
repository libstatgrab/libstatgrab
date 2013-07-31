dnl AX_VAR_CONTAINS_IF_ELSE(VAR, ELEMENT, IF-CONTAINED, IF-NOT-CONTAINED)
dnl runs IF-CONTAINED when $VAR containes ELEMENT, IF-NOT-CONTAINED otherwise
dnl
dnl Works only for vars like CPPFLAGS, not for LIB* variables because that
dnl sometimes contains two or three consecutive elements that belong together.
AC_DEFUN([AX_VAR_CONTAINS_IF_ELSE], [
  ax_var_contains_found=
  for x in $[$1]; do
    AS_IF([test "X$x" = "X$2"], [
      ax_var_contains_found=yes
      break])
  done
  AS_IF([test -n "$ax_var_contains_found"], $3, $4)
])

dnl AX_VAR_CONTAINS_ANY_IF_ELSE(VAR, LIST, IF-CONTAINED, IF-NOT-CONTAINED)
dnl runs IF-CONTAINED when $VAR containes at least one element of LIST,
dnl IF-NOT-CONTAINED otherwise
dnl Works only for vars like CPPFLAGS, not for LIB* variables because that
dnl sometimes contains two or three consecutive elements that belong together.
AC_DEFUN([AX_VAR_CONTAINS_ANY_IF_ELSE], [
  ax_var_contains_any_found=
  for element in [$2]; do
    AX_VAR_CONTAINS_IF_ELSE([$1], [$element], [
      ax_var_contains_any_found=yes
      break])
  done
  AS_IF([test -n "$ax_var_contains_any_found"], $3, $4)
])

dnl AX_APPEND_TO_VAR(VAR, CONTENTS) appends the elements of CONTENTS to VAR,
dnl unless already present in VAR.
dnl Works only for vars like CPPFLAGS, not for LIB* variables because that
dnl sometimes contains two or three consecutive elements that belong together.
AC_DEFUN([AX_APPEND_TO_VAR],
[
  for element in [$2]; do
    AX_VAR_CONTAINS_IF_ELSE([$1], [$element], , [ [$1]="${[$1]}${[$1]:+ }$element" ])
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
