# libstatgrab

[![CI](https://github.com/libstatgrab/libstatgrab/actions/workflows/ci.yml/badge.svg)](https://github.com/libstatgrab/libstatgrab/actions/workflows/ci.yml)
[![CodeQL](https://github.com/libstatgrab/libstatgrab/actions/workflows/codeql.yml/badge.svg)](https://github.com/libstatgrab/libstatgrab/actions/workflows/codeql.yml)

https://libstatgrab.org/

## libstatgrab releases

The easiest place to start is by grabbing the latest release. These are all available on the [releases page](https://github.com/libstatgrab/libstatgrab/releases). Also, see the [libstatgrab website](https://libstatgrab.org/#packages-of-libstatgrab) for operating system packages that you can install.

However, if you're here, you might be after the latest code, in which case you'll need one of the options below.

## Building from source

Starting from a Git checkout there are a few things that need to be done before the normal build procedure. These require autoconf, automake, and libtool to be installed. If you want the manual pages you'll also need docbook2X.

Then run the following;

    ./autogen.sh

After this the normal build procedure in the [README](README) file can be followed.

## Git master tarball

Our CI builds a release-style tarball every time a commit is pushed to master. You can download the [latest master tarball](https://github.com/libstatgrab/libstatgrab/releases/download/master-snapshot/libstatgrab-master.tar.gz), along with its [SHA-256 checksum](https://github.com/libstatgrab/libstatgrab/releases/download/master-snapshot/libstatgrab-master.tar.gz.sha256). This gives you an archive built in the same way as a release, and is useful for testing the latest code without having to generate the distribution tarball yourself.

## Reporting bugs or issues

As mentioned on the website, the best place to report issues is here on GitHub. You can check existing issues and open new ones on the [issues page](https://github.com/libstatgrab/libstatgrab/issues), and you can open pull requests with any changes you'd like us to consider. Issues with pull requests attached that pass the CI are likely to get merged more promptly.
