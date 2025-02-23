AM_CPPFLAGS = -I$(top_srcdir)/tdecore -I$(top_srcdir) $(all_includes)

check_PROGRAMS = example # autoexample
EXTRA_PROGRAMS = autoexample

example_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
example_LDADD = $(LIB_TDECORE)
example_SOURCES = example.cpp exampleprefs_base.cpp

autoexample_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
autoexample_LDADD = $(LIB_TDECORE) $(LIB_TDEUI)
autoexample_SOURCES = exampleprefs_base.cpp general_base.ui myoptions_base.ui \
  autoexample.cpp  

example.o exampleprefs_base.o: exampleprefs_base.h
# avoid running the below command in parallel
exampleprefs_base.cpp: exampleprefs_base.h
exampleprefs_base.cpp exampleprefs_base.h: $(srcdir)/example.kcfg ../tdeconfig_compiler $(srcdir)/exampleprefs_base.kcfgc
	../tdeconfig_compiler $(srcdir)/example.kcfg $(srcdir)/exampleprefs_base.kcfgc

METASOURCES = AUTO

CLEANFILES = exampleprefs_base.h exampleprefs_base.cpp

## The example's messages should not go into tdelibs.pot
messages: rc.cpp
	true
