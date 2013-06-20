# Makefile for libstatgrab/examples
# http://www.i-scream.org/libstatgrab/
# $Id$

noinst_PROGRAMS = $(test_bin) $(test_script)

EXTRA_DIST = run_tests.synopsis.in libstatgrab-test.properties

test_bin = full_stats diff_stats

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)\
	      -I$(top_srcdir)/src -I$(top_builddir)/src\
	      -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab \
	      -I$(top_srcdir)/tests/testlib \
	       @CLIBFLAGS@

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la $(top_builddir)/tests/testlib/libtestlib.a @LINKFLAGS@

if TEST_SCRIPTS
full_stats_edit=	$(PERL5) $(srcdir)/../testlib/mk_run_tests.pl \
		-d srcdir="$(srcdir)" \
		-d test-bin="full_stats" \
		-d test-name=full-stats \
		-d test-dir="$(subdir)" \
		-f synopsis=$(srcdir)/run_tests.synopsis.in

diff_stats_edit=	$(PERL5) $(srcdir)/../testlib/mk_run_tests.pl \
		-d srcdir="$(srcdir)" \
		-d test-bin="diff_stats" \
		-d test-name=diff-stats \
		-d test-dir="$(subdir)" \
		-f synopsis=$(srcdir)/run_tests.synopsis.in

test_script = full_stats.t diff_stats.t

full_stats_t_SOURCES = $(srcdir)/../testlib/run_tests.t.in run_tests.synopsis.in

full_stats.t: Makefile $(srcdir)/../testlib/mk_run_tests.pl $(srcdir)/../testlib/run_tests.t.in run_tests.synopsis.in
	rm -f $@ $@.tmp
	srcdir=''; \
	  test -f ../testlib/$@.in || srcdir=$(srcdir)/; \
	  $(full_stats_edit) -i $${srcdir}../testlib/run_tests.t.in >$@.tmp
	chmod +rx $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@

diff_stats_t_SOURCES = $(srcdir)/../testlib/run_tests.t.in run_tests.synopsis.in

diff_stats.t: Makefile $(srcdir)/../testlib/mk_run_tests.pl $(srcdir)/../testlib/run_tests.t.in run_tests.synopsis.in
	rm -f $@ $@.tmp
	srcdir=''; \
	  test -f ../testlib/$@.in || srcdir=$(srcdir)/; \
	  $(diff_stats_edit) -i $${srcdir}../testlib/run_tests.t.in >$@.tmp
	chmod +rx $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@

test: $(noinst_PROGRAMS)
	$(PERL5) -MApp::Prove -e 'my $$app = App::Prove->new(); $$app->process_args(@ARGV); exit( $$app->run ? 0 : 1 );' .
endif
