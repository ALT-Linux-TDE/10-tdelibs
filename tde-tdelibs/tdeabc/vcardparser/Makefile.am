INCLUDES = -I$(top_builddir)/tdeabc -I$(top_srcdir)/tdeabc $(all_includes)

noinst_LTLIBRARIES = libvcards.la
libvcards_la_SOURCES = vcard.cpp vcardline.cpp vcardparser.cpp

vcardsincludedir = $(includedir)/tdeabc
vcardsinclude_HEADERS = vcard.h vcardline.h vcardparser.h

check_PROGRAMS = testread testwrite testread2

testread_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testread_LDADD = libvcards.la $(top_builddir)/tdeabc/libtdeabc.la
testread_SOURCES = testread.cpp

testread2_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testread2_LDADD = libvcards.la $(top_builddir)/tdeabc/libtdeabc.la
testread2_SOURCES = testread2.cpp testutils.cpp

testwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testwrite_LDADD = libvcards.la $(top_builddir)/tdeabc/libtdeabc.la
testwrite_SOURCES = testwrite.cpp

TESTFILES = vcard1.vcf vcard2.vcf vcard3.vcf vcard4.vcf vcard6.vcf vcard7.vcf

check-local: testread
	rm -f FAILED; \
	for i in $(TESTFILES); \
	  do perl $(top_srcdir)/tdeabc/vcardparser/checkvcard.pl \
	    $(top_srcdir)/tdeabc/vcardparser/tests/$$i; \
	done; \
	[ ! -e FAILED ]
