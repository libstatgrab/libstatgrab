# Makefile for libstatgrab
# http://www.i-scream.org.uk/
# $Id$

AUTOMAKE_OPTIONS = gnu

SUBDIRS = src docs

ChangeLog:
	cvs2cl --stdout --no-wrap -r -t -w -S -g "-d :pserver:anonymous@cvs.i-scream.org.uk:/cvs/i-scream" -g "-q" > ChangeLog
