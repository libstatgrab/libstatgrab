#!/bin/sh

# $Id$

if [ ! -f config.guess ]; then
	wget -O config.guess "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD"
fi

if [ ! -f config.sub ]; then
	wget -O config.sub "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD"
fi

autoreconf -i
