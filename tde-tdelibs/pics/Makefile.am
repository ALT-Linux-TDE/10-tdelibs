SUBDIRS = . hicolor crystalsvg emoticons

INCLUDES = $(LIBART_CFLAGS) -I$(top_srcdir)/tdecore/svgicons -I$(top_srcdir)/tdefx $(all_includes)

if include_SVGICONS
KSVGTOPNG=ksvgtopng
endif

bin_PROGRAMS = $(KSVGTOPNG)

ksvgtopng_SOURCES = ksvgtopng.cpp
ksvgtopng_LDADD =  $(LIB_TDECORE) $(top_builddir)/tdecore/svgicons/libkdesvgicons.la
ksvgtopng_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor

install-data-local:
	-rm -f $(DESTDIR)$(kde_icondir)/default.tde
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)
	$(LN_S) crystalsvg $(DESTDIR)$(kde_icondir)/default.tde
