# libstatgrab

[![pipeline status](https://gitlab.com/libstatgrab/libstatgrab/badges/master/pipeline.svg)](https://gitlab.com/libstatgrab/libstatgrab/commits/master)
[![coverage report](https://gitlab.com/libstatgrab/libstatgrab/badges/master/coverage.svg)](https://gitlab.com/libstatgrab/libstatgrab/commits/master)

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

Our CI builds a tarball every time a commit is pushed to master, and for every pull request. This allows you to grab the [latest tarball for master](https://gitlab.com/libstatgrab/libstatgrab/builds/artifacts/master/browse?job=distfile) which gives you an archive that's built identically to a release.

Or, you can find all the builds on the [pipelines](https://gitlab.com/libstatgrab/libstatgrab/pipelines) page - look for the distfile job at the start of each pipeline for the tarball. This allows you to test out the code on a particular branch or pull request.

## Reporting bugs or issues

As mentioned on the website, the best place to report issues is here on GitHub. You can check existing issues and open new ones on the [issues page](https://github.com/libstatgrab/libstatgrab/issues), and you can open pull requests with any changes you'd like us to consider. Issues with pull requests attached that pass the CI are likely to get merged more promptly.
