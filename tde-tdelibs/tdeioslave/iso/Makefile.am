kde_module_LTLIBRARIES = tdeio_iso.la


INCLUDES = $(all_includes)


#LDFLAGS =      

tdeio_iso_la_METASOURCES=AUTO

tdeio_iso_la_SOURCES = kisodirectory.cpp kisofile.cpp qfilehack.cpp kiso.cpp iso.cpp
tdeio_iso_la_LIBADD = libisofs/libisofs.la $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEIO)

tdeio_iso_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)


SUBDIRS = libisofs

noinst_HEADERS = iso.h kiso.h qfilehack.h kisofile.h kisodirectory.h
EXTRA_DIST = iso.protocol isoservice.desktop tdeio_iso.desktop

install-data-local:
	$(mkinstalldirs)  $(DESTDIR)$(kde_servicesdir)/
	$(INSTALL_DATA) $(srcdir)/iso.protocol  $(DESTDIR)$(kde_servicesdir)/iso.protocol
	$(mkinstalldirs)  $(DESTDIR)$(kde_datadir)/konqueror/servicemenus/
	$(INSTALL_DATA) $(srcdir)/isoservice.desktop  $(DESTDIR)$(kde_datadir)/konqueror/servicemenus/isoservice.desktop
	$(mkinstalldirs)  $(DESTDIR)$(kde_confdir)/
	$(INSTALL_DATA) $(srcdir)/tdeio_isorc $(DESTDIR)$(kde_confdir)/tdeio_isorc
	$(mkinstalldirs)  $(DESTDIR)$(kde_appsdir)/
	$(INSTALL_DATA) $(srcdir)/tdeio_iso.desktop  $(DESTDIR)$(kde_appsdir)/tdeio_iso.desktop

uninstall-local:
	-rm -f  $(DESTDIR)$(kde_servicesdir)/iso.protocol
	-rm -f  $(DESTDIR)$(kde_datadir)/konqueror/servicemenus/isoservice.desktop
	-rm -f  $(DESTDIR)$(kde_confdir)/tdeio_isorc
	-rm -f  $(DESTDIR)$(kde_appsdir)/tdeio_iso.desktop

# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).


# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in tdelibs
messages:
	LIST=`find . -name \*.h -o -name \*.cpp -o -name \*.c`; \
	if test -n "$$LIST"; then \
		 $(XGETTEXT) -C -ki18n -x $(kde_includes)/tde.pot $$LIST -o ../po/iso.pot; \
	fi

