# WACS

[Project](https://docs.google.com/document/d/1Xjgj_nK7Dp-szmNC2FTRJLNodoFXepTgCzgeHNKiscw/edit?usp=sharing)

## hostapd config

```
sudo vi /etc/hostapd.conf:

interface=wlx90f6520fe13a
#bridge=br0
driver=nl80211
logger_stdout=-1
logger_stdout_level=2

#AP name
ssid=mi
hw_mode=g
channel=6
auth_algs=3
max_num_sta=5
wpa=2

#Password
wpa_passphrase=xxxxxxxx
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP CCMP
rsn_pairwise=CCMP

#Hide ssid
#ignore_broadcast_ssid=1

sudo killall wpa_supplicant

sudo hostapd -d /etc/hostapd.conf >>hostapd.2.log
```

## hostapd log 

grep MAC address

```
grep RX hostapd.log | cut -d ' ' -f 4 | cut -d '=' -f 2 | sort |  uniq
40:f3:08:4a:6a:05
72:56:8e:e5:42:b0
```

## hostapd build

[How to build](http://quadfinity.blogspot.com/2014/09/compile-latest-hostapd-v2.3-on-Raspberry-Pi-or-ODROID.html)

```
cd hostap/hostapd
vi Makefile
	CFLAGS +=-I/home/andrei/src/wacs
	LIBS += -L/home/andrei/src/wacs/.libs -lwacs -lmdbx

cp defconfig .config
sed -i 's/^#CONFIG_DRIVER_NL80211=y/CONFIG_DRIVER_NL80211=y/g' .config

# enable 802.11n and 802.11ac
sed -i 's/^#CONFIG_IEEE80211N=y/CONFIG_IEEE80211N=y/g' .config
sed -i 's/^#CONFIG_IEEE80211AC=y/CONFIG_IEEE80211AC=y/g' .config

# enable automatic channel selection
sed -i 's/^#CONFIG_ACS=y/CONFIG_ACS=y/g' .config

make && sudo make install
```


Note CONFIG_TLS=gnutls with openssl can not find openssl?

## hostapd changes

In the git/hostap/src/drivers/driver_nl80211_event.c

after line 650: 

```
mlme_event_mgmt() -> wpa_printf()
```

add line:

```
#include "driver_nl80211_event_c.h"
```

In the file git/hostap/hostapd/Makefile

after line 27 add include and lib path, add libked libraries:

```
CFLAGS +=-I/home/andrei/src/wacs
LIBS += -L/home/andrei/src/wacs/.libs -lwacs -lmdbx
```

Then

make
 
## Database tables

### log


|  Attribute  |    Bytes    |   Key/value |    Remarks  |
|-------------|-------------|-------------|-------------|
|  tag        |      1      |   Key       | 'L'         |
|  sa         |      6      |   Key       | MAC addr    |
|  dt         |      4      |   Key       | seconds     |
|  device_id  |      2      |   Value     | 0..65535    |
|  ssi_signal |      2      |   Value     | dB          |


### last_probe


|  Attribute  |    Bytes    |   Key/value |    Remarks  |
|-------------|-------------|-------------|-------------|
|  tag        |      1      |   Key       | 'L'         |
|  sa         |      6      |   Key       | MAC addr    |
|  dt         |      4      |   Value     | seconds     |

## Architecture

```
        +---------------+
        |  hostapd fork |
        +---------------+
			        |
			+---------------+
			|    IPC call   |
			+---------------+
			     		|
			+---------------+
			| wacs listener |
			+---------------+
	      |             | 
	+---------------+  +---------------+
	| LMDB database |  |       ?       |
	+---------------+  +---------------+
```

## Dependencies

### OpenSSL

```
yum install wget tar make sudo perl which
cd tools
./install-openssl-1.1.0.sh
```

### microhttpd

```
sudo yum install libmicrohttpd-devel
```

### SNMP

[libsnmp-dev](http://www.net-snmp.org/)

version 5.7.3 

```
sudo apt install libsnmp-dev
```

or full

```
sudo apt install libsnmp-dev snmp snmpd snmptrapd 
```

### libmdbx (LMDB clone)

[libmdbx](https://github.com/leo-yuriev/libmdbx)

OpenLDAP Public License

```
git clone https://github.com/leo-yuriev/libmdbx.git
cd libmdbx
make
$ sudo make install
```

#### Issues

```
./wacs: error while loading shared libraries: libmdbx.so: cannot open shared object file: No such file or directory
```

### nanomsg IPC

(nanomsg)[http://nanomsg.org]

MIT/X11 license

```
git clone https://github.com/nanomsg/nanomsg.git
cd nanomsg
mkdir build
cd build
cmake ..
cmake --build .
ctest .
sudo cmake --build . --target install
sudo ldconfig
```

## Test


```
./wacs -vvv 
./wacsc test -vvv -d 1 -b "-54" -a "11:22:33:44:55:66" -n 5
```

```
./wacsc log -vvv -d 1 -a "11:22:33:44:55:66"
```

## SNMP settings

### Check 

```
./wacs-snmp  -vvv
DB	IP	Port	Started	Elapsed	Requests/hour	MAC	SSI signal	Total MAC	DB size, kB	Mem.peak, kB	Mem.current, kB
/home/andrei/src/wacs	127.0.0.1, 192.168.43.207, ::1, fe80::2efd:8bd8:f425:ce75	0	2019-01-15 20:59:26	0:22	0		0	0	-1	7212	7120	0
```

#### Agent is not installed

Warning: Failed to connect to the agentx master agent ([NIL]): 

(SO)[https://stackoverflow.com/questions/13193471/warning-failed-to-connect-to-the-agentx-master-agent-nil]

SNMP's udp:161 is priveleged port

```
sudo apt install -o DPkg::options::=--force-confmiss --reinstall snmp snmpd snmptrapd
sudo apt install smitools
sudo cp /etc/snmp/snmpd.conf /etc/snmp/snmpd.conf.bak
sudo cp snmpd.conf /etc/snmp
sudo service snmpd restart
sudo ./wacs --snmp
```

#### Cannot find module (SNMPv2-MIB)

```
sudo apt install snmp-mibs-downloader snmptrapd
sudo download-mibs
sudo sed -i "s/^\(mibs *:\).*/#\1/" /etc/snmp/snmp.conf
```

```
mkdir -vp ~/.snmp/mibs
sudo mkdir -p /root/.snmp/mib
cp mib/* ~/.snmp/mibs
sudo cp mib/* /root/.snmp/mib

sudo /etc/init.d/snmpd stop

snmptranslate -On -m +WACS-COMMANDUS-MIB -IR wacs
.1.3.6.1.4.1.46821.2

smilint -l3  -s -p ./mib/*

sudo /etc/init.d/snmpd start
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::totalmac.0
WACS-COMMANDUS-MIB::totalmac.0 = INTEGER: 0
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::databasefilename.0
WACS-COMMANDUS-MIB::databasefilename.0 = STRING: /home/andrei/src/wacs
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::starttime.0
WACS-COMMANDUS-MIB::starttime.0 = INTEGER: 1547553566
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::memorypeak.0
WACS-COMMANDUS-MIB::memorypeak.0 = INTEGER: 7212
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::memorycurrent.0
WACS-COMMANDUS-MIB::memorypeak.0 = INTEGER: 7120
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::memorycurrent.0
WACS-COMMANDUS-MIB::memorycurrent.0 = INTEGER: 29784
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::lastmac.0
WACS-COMMANDUS-MIB::lastmac.0 = STRING:
snmpget -v2c -c private 127.0.0.1 WACS-COMMANDUS-MIB::ip.0
WACS-COMMANDUS-MIB::ip.0 = STRING: 127.0.0.1, 192.168.43.207, ::1, fe80::2efd:8bd8:f425:ce75
```

#### ERROR: You don't have the SNMP perl module installed.

#### Warning: no access control information configured.

```
Warning: no access control information configured.
  (Config search path: /etc/snmp:/usr/share/snmp:/usr/lib/x86_64-linux-gnu/snmp:/home/andrei/.snmp)
  It's unlikely this agent can serve any useful purpose in this state.
  Run "snmpconf -g basic_setup" to help you configure the wacssvc.conf file for this agent.
```  
```
snmpconf -g basic_setup
```

#### Error opening specified endpoint "udp:161"

161 привелигированный порт, запустить от рута.

```
sudo ./wacs --snmp 1
```

## docker

### Initial setup
```
docker run -itv /home/andrei/src:/home/andrei/src centos:7.3.1611 bash
yum install centos-release-scl devtoolset-7-gcc-c++  wget tar make sudo perl git cmake libmicrohttpd-devel autoconf automake net-snmp-devel
scl enable devtoolset-7 bash

cd /home/andrei/src/wacs
```

### Build

```
docker run -itv /home/andrei/src:/home/andrei/src centos:7.3.1611 bash
cd /home/andrei/src/wacs
```

### закоммитить образ 

```
docker ps -a
docker commit stoic_ramanujan
docker images
docker tag c30cb68a6443 centos:nova

```
