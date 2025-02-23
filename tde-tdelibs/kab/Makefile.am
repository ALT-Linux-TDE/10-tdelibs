INCLUDES= $(all_includes)

noinst_LTLIBRARIES = libkab.la
KABVERSION=	4.0.0
KABPATCH=	0

libkab_la_SOURCES	= \
	kabapi.cpp \
	addressbook.cpp \
	qconfigDB.cpp 

libkab_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined
libkab_la_LIBADD = ../tdeui/libtdeui.la

libkab_la_METASOURCES = AUTO
