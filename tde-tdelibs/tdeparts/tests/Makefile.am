
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/libltdl -I$(top_srcdir)/tdefile -I$(top_srcdir)/tdeio $(all_includes)
AM_LDFLAGS = $(all_libraries)
LDADD = $(LIB_TDEPARTS)

check_PROGRAMS = tdepartstest normalktmtest ghostviewtest

tdepartstest_SOURCES = example.cpp parts.cpp
tdepartstest_LDADD = libnotepad.la $(LIB_TDEPARTS)
normalktmtest_SOURCES = normalktm.cpp parts.cpp
normalktmtest_LDADD = libnotepad.la $(LIB_TDEPARTS)
ghostviewtest_SOURCES = ghostview.cpp
ghostviewtest_LDADD = $(LIB_TDEPARTS)

# These are not really libraries, but modules dynamically opened.
# So they should be installed in kde_module_dir, which is usually $prefix/lib/trinity
kde_module_LTLIBRARIES = libspellcheckplugin.la libnotepadpart.la

libspellcheckplugin_la_SOURCES = plugin_spellcheck.cpp
libspellcheckplugin_la_LIBADD = $(LIB_TDEPARTS) 
libspellcheckplugin_la_LDFLAGS = -module $(KDE_PLUGIN)

libnotepadpart_la_SOURCES = notepadpart.cpp
libnotepadpart_la_LIBADD = libnotepad.la $(LIB_TDEPARTS)
libnotepadpart_la_LDFLAGS = -module $(KDE_PLUGIN)

noinst_LTLIBRARIES = libnotepad.la
libnotepad_la_SOURCES = notepad.cpp

rcdir = $(kde_datadir)/tdepartstest
rc_DATA = tdepartstest_shell.rc

testpartdir = $(kde_datadir)/tdepartstestpart
testpart_DATA = tdepartstest_part1.rc

notepaddir = $(kde_datadir)/notepadpart
notepad_DATA = notepadpart.rc

ghostviewtestdir = $(kde_datadir)/ghostviewtest
ghostviewtest_DATA = ghostviewtest_shell.rc

pluginsdir = $(kde_datadir)/notepadpart/kpartplugins
plugins_DATA = plugin_foobar.rc plugin_spellcheck.rc

METASOURCES = AUTO

