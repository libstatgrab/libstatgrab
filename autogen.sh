#!/bin/sh

git --no-pager log \
	--no-show-signature --no-color --no-expand-tabs \
	--name-only --date=short \
	--format='%n*%cd  %an  <%ae>%n%n%w(0,8,8)%B%w(0,0,0)%nHash: %H%nFiles affected:' \
	| sed "
		1d
		/^\*[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}  / {
		s/^\*//
		b
		}
		s/^        //
		/./s/^/$(printf '\t')/
	" > ChangeLog

autoreconf -fiv
