#!/usr/bin/env python

from distutils.core import setup, Extension
from commands import getstatusoutput
from sys import exit

cflags = getstatusoutput("pkg-config --cflags libstatgrab")
libs = getstatusoutput("pkg-config --libs libstatgrab")

if cflags[0] != 0:
	exit("Failed to get cflags: " + cflags[1])

if libs[0] != 0:
	exit("Failed to get libs: " + libs[1])

setup(	name = "statgrab",
	version = "0.8.2",
	description = "Python bindings for libstatgrab",
	author = "i-scream",
	author_email = "dev@i-scream.org",
	url = "http://www.i-scream.org/libstatgrab",
	license = "GNU GPL v2 or later",
	ext_modules=[Extension(
		"statgrab",
		["statgrab.c"],
		extra_compile_args = cflags[1].split(),
		extra_link_args = libs[1].split(),
	)],
)
