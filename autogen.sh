#!/bin/sh

# $Id$

if [ ! -f config.guess ]; then
	wget -O config.guess "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD"
fi

if [ ! -f config.sub ]; then
	wget -O config.sub "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD"
fi

git log --stat --name-only --date=short  --format='%n*%ad  %an  <%ae>%n%n%w(76,4,4)%s%n%n%b%n#Hash: %H%n#Files affected:'  | sed  -e 's/^\([^ \*]\)/    \1/'  -e 's/^    \#/  /'  -e 's/     Hash/    Hash/'  -e 's/^\*//'  | tail -n +2 > ChangeLog

autoreconf -i
