# Make sure $(all_includes) remains last!
INCLUDES = -I$(top_builddir)/tdeabc -I$(top_srcdir)/tdeabc -I$(top_srcdir)/kab \
           -I$(srcdir)/../vcardparser/ -I$(srcdir)/../vcard/include \
           -I$(srcdir)/../vcard/include/generated \
           -I$(srcdir)/../vcardparser $(all_includes)
LDADD = ../libtdeabc.la

METASOURCES = AUTO

check_PROGRAMS = testlock testldapclient

testlock_LDFLAGS = $(all_libraries)
testlock_SOURCES = testlock.cpp

testldapclient_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testldapclient_SOURCES = testldapclient.cpp

EXTRA_PROGRAMS = testkabc testkabcdlg testdistlist bigread bigwrite testdb \
  testaddressee testaddresseelist testaddressfmt tdeabcargl testaddresslineedit

testtdeabc_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testtdeabc_SOURCES = testkabc.cpp

testaddressee_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testaddressee_SOURCES = testaddressee.cpp

testaddresseelist_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testaddresseelist_SOURCES = testaddresseelist.cpp

testaddressfmt_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testaddressfmt_SOURCES = testaddressfmt.cpp

testkabcdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testkabcdlg_SOURCES = testkabcdlg.cpp

testdistlist_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testdistlist_SOURCES = testdistlist.cpp

testaddresslineedit_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testaddresslineedit_SOURCES = testaddresslineedit.cpp

bigread_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
bigread_LDADD = ../libtdeabc.la $(top_builddir)/tdeabc/plugins/file/libtdeabc_file.la
bigread_SOURCES = bigread.cpp

bigwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
bigwrite_LDADD = ../libtdeabc.la $(top_builddir)/tdeabc/plugins/file/libtdeabc_file.la
bigwrite_SOURCES = bigwrite.cpp

testdb_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testdb_SOURCES = testdb.cpp

kabcargl_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
kabcargl_SOURCES = kabcargl.cpp

