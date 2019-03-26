SUBDIRS = .

if OS_MACH
LMDB=lmdb
OS_CPPFLAGS=-DUSE_LMDB -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/libpq/include
else
OS_CPPFLAGS=
LMDB=mdbx
endif

# Uncomment for debug
if DEBUG
CFLAGS=-g -O0
CXXFLAGS=-g -O0
else
endif

AM_CXXFLAGS = -std=c++11 
ACLOCAL_AMFLAGS = -I m4

#	Binaries
bin_PROGRAMS = \
	wacs wacs-pg \
	wacsc wacs-http wacs-snmp

#	Libraries
lib_LIBRARIES = \
	libwacs.a
lib_LTLIBRARIES = \
	libwacs.la

commoncppflags = -O2 -I/usr/local/include -Ithird_party -Ithird_party/ecec \
	$(OS_CPPFLAGS)

commonlibs = -L.

#
#	Exclude generated sources from distribution
#
nodist_wacs_SOURCES =
BUILT_SOURCES =
CLEANFILES =

ARGT3HEADERS = third_party/argtable3/argtable3.h
ARGT3SOURCES = third_party/argtable3/argtable3.c

nobase_dist_include_HEADERS = \
	platform.h log.h \
	wacs-config.h wacs.h wacs-c.h wacs-http-config.h \
	wacsc-config.h wacsc.h utilstring.h http-server.h \
	dblog.h hostapd-log-entry.h send-log-entry.h lmdbwriter.h NanoMessage.h errorcodes.h \
	daemonize.h \
	snmpagent-wacs.h snmp-params.h utilsnmp.h \
	wacs-snmp-config.h \
	third_party/get_rss/get_rss.h \
	third_party/wpn/utilvapid.h \
	third_party/nlohmann/json.hpp \
	if-ip-address.h histogrammacspertime.h \
	third_party/ecec/ece/keys.h third_party/ecec/ece/trailer.h \
	pglog.h pgwriter.h wacs-pg-config.h wacs-pg.h \
	$(ARGT3HEADERS)

common_src = $(ARGT3SOURCES)

wacs_lib_source = \
	NanoMessage.cpp errorcodes.cpp \
	hostapd-log-entry.cpp send-log-entry.cpp \
	utilstring.cpp wacs-c.cpp

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
wacs_SOURCES = \
	wacs-config.cpp wacs.cpp \
	lmdbwriter.cpp NanoMessage.cpp errorcodes.cpp \
	dblog.cpp hostapd-log-entry.cpp \
	daemonize.cpp utilstring.cpp \
	snmpagent-wacs.cpp snmp-params.cpp utilsnmp.cpp \
	if-ip-address.cpp \
	third_party/get_rss/get_rss.c \
	third_party/wpn/utilvapid.cpp \
	third_party/ecec/base64url.c third_party/ecec/decrypt.c third_party/ecec/encrypt.c third_party/ecec/keys.c third_party/ecec/params.c third_party/ecec/trailer.c \
	$(common_src)
wacs_LDADD = $(commonlibs) -l$(LMDB) -lnanomsg \
	-lnetsnmp -lnetsnmpagent -lpthread -lcurl -lssl
wacs_CPPFLAGS = $(commoncppflags) -DUSE_SNMP

#
#	wacs-pg
#
wacs_pg_SOURCES = \
	wacs-pg-config.cpp wacs-pg.cpp pglog.cpp \
	pgwriter.cpp NanoMessage.cpp errorcodes.cpp \
	hostapd-log-entry.cpp \
	daemonize.cpp utilstring.cpp \
	snmpagent-wacs.cpp snmp-params.cpp utilsnmp.cpp \
	if-ip-address.cpp \
	third_party/get_rss/get_rss.c \
	third_party/wpn/utilvapid.cpp \
	third_party/ecec/base64url.c third_party/ecec/decrypt.c third_party/ecec/encrypt.c third_party/ecec/keys.c third_party/ecec/params.c third_party/ecec/trailer.c \
	$(common_src)
wacs_pg_LDADD = $(commonlibs) -lnanomsg \
	-lnetsnmp -lnetsnmpagent -lpthread -lcurl -lssl -lpq
wacs_pg_CPPFLAGS = $(commoncppflags) -DUSE_SNMP

#
#	wacsc
#
wacsc_SOURCES = \
	wacsc-config.cpp wacsc.cpp \
	NanoMessage.cpp errorcodes.cpp \
	dblog.cpp send-log-entry.cpp hostapd-log-entry.cpp \
	utilstring.cpp \
	histogrammacspertime.cpp \
	$(common_src)
wacsc_LDADD = $(commonlibs) -l$(LMDB) -lnanomsg
wacsc_CPPFLAGS = $(commoncppflags)

#
#	wacs-snmp
#
wacs_snmp_SOURCES = \
	wacs-snmp-config.cpp wacs-snmp.cpp \
	$(common_src)
wacs_snmp_LDADD = $(commonlibs) -lnetsnmp -lnetsnmpagent
wacs_snmp_CPPFLAGS = $(commoncppflags)

#
#	wacs-http
#
wacs_http_SOURCES = \
	wacs-http.cpp wacs-http-config.cpp \
	http-server.cpp \
	dblog.cpp hostapd-log-entry.cpp \
	utilstring.cpp daemonize.cpp \
	histogrammacspertime.cpp \
	$(common_src)
wacs_http_LDADD = $(commonlibs) -lmicrohttpd -l$(LMDB)
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
	autogen.sh \
	notification.sample.json
