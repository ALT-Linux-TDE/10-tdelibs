AM_CPPFLAGS = -DKDE_NO_COMPAT -I$(top_srcdir)/tdespell2 -I$(top_srcdir)/tdespell2/ui $(all_includes)
METASOURCES = AUTO


check_PROGRAMS = test test_filter backgroundtest \
	test_dialog test_highlighter test_configdialog

test_SOURCES = test.cpp
test_LDADD = ../ui/libtdespell2.la $(LIB_TDECORE)

test_filter_SOURCES = test_filter.cpp
test_filter_LDADD = ../ui/libtdespell2.la $(LIB_TDECORE)

backgroundtest_SOURCES = backgroundtest.cpp
backgroundtest_LDADD = ../ui/libtdespell2.la $(LIB_TDECORE)

test_dialog_SOURCES = test_dialog.cpp
test_dialog_LDADD = ../ui/libtdespell2.la $(LIB_TDECORE)

test_highlighter_SOURCES = test_highlighter.cpp
test_highlighter_LDADD = ../ui/libtdespell2_noinst.la ../libtdespell2base.la $(LIB_TDECORE) $(LIB_TDEPARTS) ../../tdeutils/libtdeutils.la

test_configdialog_SOURCES = test_configdialog.cpp
test_configdialog_LDADD = ../ui/libtdespell2_noinst.la ../libtdespell2base.la $(LIB_TDECORE) $(LIB_TDEPARTS) ../../tdeutils/libtdeutils.la
