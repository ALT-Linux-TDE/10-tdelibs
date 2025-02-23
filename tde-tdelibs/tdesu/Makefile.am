## Makefile.am for libtdesu

INCLUDES = -I$(top_srcdir)/tdeio/ $(all_includes)

lib_LTLIBRARIES = libtdesu.la
libtdesu_la_SOURCES = client.cpp process.cpp kcookie.cpp su.cpp ssh.cpp stub.cpp tdesu_pty.cpp
libtdesu_la_LDFLAGS = -version-info 6:0:2 -no-undefined $(all_libraries) 
libtdesu_la_LIBADD = $(LIB_TDECORE) $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la
libtdesu_la_NMCHECK = $(srcdir)/libtdesu.nmcheck
libtdesu_la_NMCHECKWEAK = $(srcdir)/libtdesu_weak.nmcheck $(top_srcdir)/tdecore/libtdecore_weak.nmcheck \
	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/tdecore/libtqt-mt_weak.nmcheck

tdesudir = $(includedir)/tdesu
tdesu_HEADERS = defaults.h client.h process.h tdesu_pty.h kcookie.h su.h ssh.h stub.h

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(tdesudir)
	-rm -f $(DESTDIR)$(tdesudir)/pty.h
	ln -s tdesu_pty.h $(DESTDIR)$(tdesudir)/pty.h

uninstall-local:
	-rm -f $(DESTDIR)$(tdesudir)/pty.h

bin_PROGRAMS = tdesu_stub
tdesu_stub_SOURCES = tdesu_stub.c
tdesu_stub_LDFLAGS = $(all_libraries)
tdesu_stub_LDADD =

include $(top_srcdir)/admin/Doxyfile.am

