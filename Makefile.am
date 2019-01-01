SUBDIRS = .

ACLOCAL_AMFLAGS = -I m4

#	Binaries
bin_PROGRAMS = \
	wacs

commoncppflags = -O2 -I/usr/local/include -Ithird_party
commoncppflags += -I/usr/local/opt/openssl@1.1/include

commonlibs = -L. -L/usr/local/opt/openssl@1.1/lib

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
	lmdbwriter.h NanoMessage.h errorcodes.h \
	daemonize.h \
	$(ARGT3HEADERS)

common_src = $(ARGT3SOURCES)

#
#	wacs
#
wacs_SOURCES= \
	wacs-config.cpp wacs.cpp \
	lmdbwriter.cpp NanoMessage.cpp errorcodes.cpp \
	daemonize.cpp \
	$(common_src)

wacs_LDADD = $(commonlibs) -lmdbx -lnanomsg
wacs_CPPFLAGS = $(commoncppflags)

#
#	Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
	LICENSE COPYING CODE_OF_CONDUCT.md CONTRIBUTING.md HISTORY README.md \
	third_party/argtable3/README.argtable3.md
	autogen.sh
