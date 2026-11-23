# Makefile for libstatgrab
# https://libstatgrab.org/

AUTOMAKE_OPTIONS = gnu
ACLOCAL_AMFLAGS = -I m4

DOCS_SUBDIR = docs

if TESTS
TESTS_SUBDIR = tests
endif

SUBDIRS = src examples $(DOCS_SUBDIR) $(TESTS_SUBDIR)

test:
if TEST_SCRIPTS
	cd tests && $(MAKE) test
else
	@echo Not built
endif

check: test

EXTRA_DIST = libstatgrab.pc.in statgrab.properties.in saidar.properties.in

MAINTAINERCLEANFILES = ChangeLog

if MAINTAINER_MODE
ChangeLog:
	git --no-pager log \
		--no-show-signature --no-color --no-expand-tabs \
		--name-only --date=short \
		--format='%n*%cd  %an  <%ae>%n%n%w(0,8,8)%B%w(0,0,0)%nHash: %H%nFiles affected:' \
		| $(SED) \
			-e '1d' \
			-e '/^\*[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}  / { s/^\*//; b; }' \
			-e 's/^        //' \
			-e "/./s/^/$$(printf '\t')/" \
		> ChangeLog
endif

dist_doc_DATA = NEWS PLATFORMS README AUTHORS COPYING COPYING.LGPL

if WITH_LIBLOG4CPLUS
sysconf_DATA = statgrab.properties saidar.properties

prop_edit=	-e 's|@localstatedir[@]|$(localstatedir)|g'
prop_edit_cmd=	$(SED) $(prop_edit)

statgrab.properties: Makefile $(srcdir)/statgrab.properties.in
	rm -f $@ $@.tmp
	srcdir=''; \
	  test -f ./$@.in || srcdir=$(srcdir)/; \
	  $(prop_edit_cmd) $${srcdir}$@.in >$@.tmp
	chmod +r $@.tmp
	chmod a-wx $@.tmp
	mv $@.tmp $@

saidar.properties: Makefile $(srcdir)/saidar.properties.in
	rm -f $@ $@.tmp
	srcdir=''; \
	  test -f ./$@.in || srcdir=$(srcdir)/; \
	  $(prop_edit_cmd) $${srcdir}$@.in >$@.tmp
	chmod +r $@.tmp
	chmod a-wx $@.tmp
	mv $@.tmp $@
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libstatgrab.pc
