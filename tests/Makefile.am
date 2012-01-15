# Makefile for libstatgrab/docs
# http://www.i-scream.org/libstatgrab/
# $Id$

SUBDIRS = testlib $(tests_dirs)

tests_dirs = single_threaded $(threaded_tests_dirs)
if ENABLE_THREADS
threaded_tests_dirs = multi_threaded
endif

if TEST_SCRIPTS
test:
	$(PERL5) -MApp::Prove -e 'my $$app = App::Prove->new(); $$app->process_args(@ARGV); exit( $$app->run ? 0 : 1 );' $(tests_dirs)
endif
