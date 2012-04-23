#!/bin/sh

# $Id$

aclocal -I m4 && \
autoheader && \
autoconf && \
libtoolize -c && \
gettextize && \
automake -a -c && \
