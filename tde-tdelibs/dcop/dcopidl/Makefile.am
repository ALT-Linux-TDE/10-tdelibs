
INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries)
AM_CPPFLAGS = -DYYDEBUG

####### Files

bin_PROGRAMS = dcopidl

dcopidl_SOURCES = main.cpp yacc.cpp scanner.cpp
noinst_HEADERS = yacc.cpp.h

dcopidl_LDADD = $(LIB_TQT)
dcopidl_LDFLAGS = $(KDE_RPATH) $(AM_LDFLAGS)

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cpp scanner.ll  ;\
	bison -t -d -v -o yacc.cpp yacc.yy && mv yacc.hh yacc.cpp.h

check-local: 
	$(srcdir)/run_test.sh $(srcdir)

EXTRA_DIST = dcopidl_test.h scanner.ll yacc.yy

MAINTAINERCLEANFILES = yacc.cpp.output
