#!/bin/sh

# $Id$

aclocal
autoheader
autoconf
libtoolize -c
automake -a -c
