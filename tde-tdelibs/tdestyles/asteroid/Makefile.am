# Yaaay poorly-documented hackish magical software!
# (re: autoconf and automake and friends,
#      not my own software, of course) -clee

AM_CPPFLAGS = -DQT_PLUGIN
METASOURCES = AUTO

INCLUDES = $(all_includes)

noinst_HEADERS = asteroid.h
kde_style_LTLIBRARIES = asteroid.la

asteroid_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
asteroid_la_LIBADD = -ltdefx -ltdeutils
asteroid_la_SOURCES = asteroid.cpp

themercdir = $(kde_datadir)/tdestyle/themes
themerc_DATA = asteroid.themerc
