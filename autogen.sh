#!/bin/sh

# $Id$

git log --stat --name-only --date=short  --format='%n*%ad  %an  <%ae>%n%n%w(76,4,4)%s%n%n%b%n#Hash: %H%n#Files affected:'  | sed  -e 's/^\([^ \*]\)/    \1/'  -e 's/^    \#/  /'  -e 's/     Hash/    Hash/'  -e 's/^\*//'  | tail -n +2 > ChangeLog

autoreconf -i
