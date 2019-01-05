SUBDIRS = .

ACLOCAL_AMFLAGS = -I m4

#	Binaries
bin_PROGRAMS = \
	wacs wacsc

#	Libraries
lib_LIBRARIES = \
	libwacs.a
lib_LTLIBRARIES = \
	libwacs.la

commoncppflags = -O2 -I/usr/local/include -Ithird_party

commonlibs = -L.

#
#	Exclude generated sources from distribution
#
nodist_wacs_SOURCES =
BUILT_SOURCES =
CLEANFILES =

ARGT3HEADERS = third_party/argtable3/argtable3.h
ARGT3SOURCES = third_party/argtable3/argtable3.c

AM_CXXFLAGS = -std=c++11 

nobase_dist_include_HEADERS = \
	platform.h \
	wacs-config.h wacs.h \
	wacsc-config.h wacsc.h utilstring.h \
	dblog.h hostapd-log-entry.h send-log-entry.h lmdbwriter.h NanoMessage.h errorcodes.h \
	daemonize.h \
	$(ARGT3HEADERS)

common_src = $(ARGT3SOURCES)

wacs_lib_source = \
	lmdbwriter.cpp NanoMessage.cpp errorcodes.cpp \
	dblog.cpp hostapd-log-entry.cpp send-log-entry.cpp \
	utilstring.cpp

#
#	libwpn.a
#
libwacs_a_SOURCES = $(wacs_lib_source)
libwacs_a_CPPFLAGS = $(commoncppflags)


#
#	libwpn.la C++ shared library
#
libwacs_la_SOURCES = $(wacs_lib_source)
libwacs_la_CPPFLAGS = $(commoncppflags)
libwacs_la_LDFLAGS = -version-info 0:1:0

#
#	wacs
#
wacs_SOURCES= \
	wacs-config.cpp wacs.cpp \
	lmdbwriter.cpp NanoMessage.cpp errorcodes.cpp \
	dblog.cpp hostapd-log-entry.cpp \
	daemonize.cpp \
	$(common_src)
wacs_LDADD = $(commonlibs) -lmdbx -lnanomsg
wacs_CPPFLAGS = $(commoncppflags)

#
#	wacsc
#
wacsc_SOURCES= \
	wacsc-config.cpp wacsc.cpp \
	NanoMessage.cpp errorcodes.cpp \
	dblog.cpp send-log-entry.cpp hostapd-log-entry.cpp \
	utilstring.cpp \
	$(common_src)
wacsc_LDADD = $(commonlibs) -lmdbx -lnanomsg
wacsc_CPPFLAGS = $(commoncppflags)


#
#	Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
	LICENSE COPYING CODE_OF_CONDUCT.md CONTRIBUTING.md HISTORY README.md \
	third_party/argtable3/README.argtable3.md
	autogen.sh
