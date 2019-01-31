SUBDIRS = .

ACLOCAL_AMFLAGS = -I m4

#	Binaries
bin_PROGRAMS = \
	wacs wacsc wacs-http wacs-snmp

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

AM_CXXFLAGS = 

nobase_dist_include_HEADERS = \
	platform.h log.h \
	wacs-config.h wacs.h wacs-c.h wacs-http-config.h \
	wacsc-config.h wacsc.h utilstring.h http-server.h \
	dblog.h hostapd-log-entry.h send-log-entry.h lmdbwriter.h NanoMessage.h errorcodes.h \
	daemonize.h \
	snmpagent-wacs.h snmp-params.h utilsnmp.h \
	wacs-snmp-config.h \
	third_party/get_rss/get_rss.h \
	if-ip-address.h \
	$(ARGT3HEADERS)

common_src = $(ARGT3SOURCES)

wacs_lib_source = \
	NanoMessage.cpp errorcodes.cpp \
	hostapd-log-entry.cpp send-log-entry.cpp \
	utilstring.cpp

#
#	libwpn.a
#
libwacs_a_SOURCES = $(wacs_lib_source) wacs-c.cpp
libwacs_a_CPPFLAGS = $(commoncppflags)


#
#	libwpn.la C++ shared library
#
libwacs_la_SOURCES = $(wacs_lib_source) wacs-c.cpp
libwacs_la_CPPFLAGS = $(commoncppflags)
libwacs_la_LDFLAGS = -version-info 0:1:0

#
#	wacs
#
wacs_SOURCES = \
	wacs-config.cpp wacs.cpp \
	lmdbwriter.cpp NanoMessage.cpp errorcodes.cpp \
	dblog.cpp hostapd-log-entry.cpp \
	daemonize.cpp utilstring.cpp \
	snmpagent-wacs.cpp snmp-params.cpp utilsnmp.cpp \
	if-ip-address.cpp \
	third_party/get_rss/get_rss.c \
	$(common_src)
wacs_LDADD = $(commonlibs) -lmdbx -lnanomsg \
	-lnetsnmp -lnetsnmpagent -lpthread
wacs_CPPFLAGS = $(commoncppflags) -DUSE_SNMP

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
#	wacs-snmp
#
wacs_snmp_SOURCES= \
	wacs-snmp-config.cpp wacs-snmp.cpp \
	$(common_src)
wacs_snmp_LDADD = $(commonlibs) -lnetsnmp -lnetsnmpagent
wacs_snmp_CPPFLAGS = $(commoncppflags)

#
#	wacs-http
#
wacs_http_SOURCES= \
	wacs-http.cpp wacs-http-config.cpp \
	http-server.cpp \
	dblog.cpp hostapd-log-entry.cpp \
	utilstring.cpp daemonize.cpp \
	$(common_src)
wacs_http_LDADD = $(commonlibs) -lmicrohttpd -lmdbx
wacs_http_CPPFLAGS = $(commoncppflags)

#
#	Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
	autogen.sh \
	snmpd.conf \
	mib/COMMANDUS-SMI-MIB  mib/WACS-COMMANDUS-MIB \
	LICENSE COPYING CODE_OF_CONDUCT.md CONTRIBUTING.md HISTORY README.md \
	third_party/argtable3/README.argtable3.md \
	autogen.sh
