#!/usr/bin/env python

from distutils.core import setup, Extension

setup(	name = "statgrab",
	version = "0.8.1",
	description = "Python bindings for libstatgrab",
	author = "i-scream",
	author_email = "dev@i-scream.org",
	url = "http://www.i-scream.org/libstatgrab",
	license = "GNU GPL v2 or later",
	ext_modules=[Extension(
		"statgrab",
		["statgrab.c"],
		libraries = ["statgrab", "kvm", "devstat"],
		include_dirs = ["/usr/local/include"],
		library_dirs = ["/usr/local/lib"],
	)],
)
