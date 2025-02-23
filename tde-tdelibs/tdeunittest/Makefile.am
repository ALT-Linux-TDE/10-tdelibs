INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libtdeunittest.la
libtdeunittest_la_SOURCES = runner.cpp tester.cpp
libtdeunittest_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0:0
libtdeunittest_la_LIBADD = $(LIB_TDECORE) $(LIB_TQT)

libtdeunittestinclude_HEADERS = runner.h tester.h module.h
libtdeunittestincludedir = $(includedir)/tdeunittest

bin_PROGRAMS = tdeunittestmodrunner
tdeunittestmodrunner_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdeunittestmodrunner_LDADD = libtdeunittest.la $(LIB_TDECORE)
tdeunittestmodrunner_SOURCES = modrunner.cpp

# The check_ target makes sure we don't install the modules,
# $(KDE_CHECK_PLUGIN) assures a shared library is created.
check_LTLIBRARIES = tdeunittest_samplemodule.la
tdeunittest_samplemodule_la_SOURCES = samplemodule.cpp
tdeunittest_samplemodule_la_LIBADD = libtdeunittest.la
tdeunittest_samplemodule_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

check-local:
	./tdeunittestmodrunner

include ../admin/Doxyfile.am
