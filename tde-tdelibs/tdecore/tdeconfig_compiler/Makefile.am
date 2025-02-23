SUBDIRS = example tests

AM_CPPFLAGS = -I$(top_srcdir)/tdecore -I$(top_srcdir) $(all_includes)

bin_PROGRAMS = tdeconfig_compiler

tdeconfig_compiler_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdeconfig_compiler_LDADD = $(LIB_TDECORE)
tdeconfig_compiler_SOURCES = tdeconfig_compiler.cpp

TESTFILES = test1.kcfg test2.kcfg test3.kcfg test4.kcfg test_dpointer.kcfg

check-local:
	for i in $(TESTFILES); \
	  do xmllint --noout --schema $(srcdir)/kcfg.xsd $(srcdir)/tests/$$i; \
	  perl $(top_srcdir)/tdecore/tdeconfig_compiler/checkkcfg.pl \
          $(top_srcdir)/tdecore/tdeconfig_compiler/tests/$$i; done
	  
