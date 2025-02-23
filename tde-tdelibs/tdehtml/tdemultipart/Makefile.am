INCLUDES = -I$(top_srcdir)/tdehtml -I$(top_srcdir)/tdeio/httpfilter -I$(top_srcdir)/tdeutils $(all_includes)

# These are not really libraries, but modules dynamically opened.
# So they should be installed in kde_module_dir, which is usually $kde_prefix/lib/trinity
kde_module_LTLIBRARIES = libtdemultipart.la

libtdemultipart_la_SOURCES = tdemultipart.cpp
libtdemultipart_la_LIBADD = $(LIB_TDEPARTS) $(top_builddir)/tdeio/httpfilter/libhttpfilter.la $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEUI) $(LIB_TDEFILE) $(LIBZ)
libtdemultipart_la_DEPENDENCIES = $(LIB_TDEPARTS)
libtdemultipart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

# Automatically generate moc files
METASOURCES = AUTO

# Install the .desktop file into the kde_services directory
kde_services_DATA = tdemultipart.desktop

