INCLUDES = -I$(top_srcdir)/tdeabc -I$(top_builddir)/tdeabc $(all_includes)

kde_module_LTLIBRARIES = tdeabcformat_binary.la

tdeabcformat_binary_la_SOURCES = binaryformat.cpp
tdeabcformat_binary_la_LDFLAGS =  -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                                -no-undefined
tdeabcformat_binary_la_LIBADD  = $(LIB_TDEABC) $(LIB_TQT) $(LIB_TDECORE)
tdeabcformat_binary_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h

# these are the headers for your project
noinst_HEADERS = binaryformat.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabcformat_binary.pot

linkdir = $(kde_datadir)/tdeabc/formats
link_DATA = binary.desktop
EXTRA_DIST = $(link_DATA)
