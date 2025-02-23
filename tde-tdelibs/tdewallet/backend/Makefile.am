
INCLUDES = -I$(srcdir)/../client $(all_includes)

lib_LTLIBRARIES = libtdewalletbackend.la

libtdewalletbackend_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0 -no-undefined
libtdewalletbackend_la_LIBADD = $(LIB_TQT) ../../tdecore/libtdecore.la
libtdewalletbackend_la_SOURCES = blockcipher.cpp		\
			       blowfish.cpp		\
			       cbc.cpp			\
			       sha1.cpp			\
			       tdewalletentry.cpp		\
			       tdewalletbackend.cpp


libtdewalletbackend_la_METASOURCES = AUTO

noinst_HEADERS = blowfishtables.h	\
		 cbc.h			\
		 sha1.h			\
		 blockcipher.h		\
		 tdewalletentry.h		\
		 tdewalletbackend.h	\
		 blowfish.h

