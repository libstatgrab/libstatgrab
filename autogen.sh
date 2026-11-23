#!/bin/sh

git --no-pager log \
	--no-show-signature --no-color --no-expand-tabs \
	--name-only --date=short \
	--format='%n*%cd  %an  <%ae>%n%n%w(0,8,8)%B%w(0,0,0)%nHash: %H%nFiles affected:' \
	| sed \
		-e '1d' \
		-e '/^\*[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}  / { s/^\*//; b; }' \
		-e 's/^        //' \
		-e "/./s/^/$(printf '\t')/" \
	> ChangeLog

autoreconf -fiv
