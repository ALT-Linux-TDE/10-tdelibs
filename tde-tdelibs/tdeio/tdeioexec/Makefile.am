AM_CPPFLAGS = $(all_includes)

bin_PROGRAMS = tdeioexec

tdeioexec_SOURCES = main.cpp
tdeioexec_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdeioexec_LDADD   = $(LIB_TDEIO)

noinst_HEADERS = main.h
METASOURCES = AUTO

messages:
	$(XGETTEXT) $(tdeioexec_SOURCES) -o $(podir)/tdeioexec.pot

install-exec-local:
	@rm -f $(DESTDIR)$(bindir)/kfmexec
	@$(LN_S) tdeioexec $(DESTDIR)$(bindir)/kfmexec

include $(top_srcdir)/admin/Doxyfile.am
