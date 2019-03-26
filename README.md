# WACS

## History

2019-02-13 wacsc: commands remove, log-

2019-02-11 wacsc: option --sa removed

[Project](https://docs.google.com/document/d/1Xjgj_nK7Dp-szmNC2FTRJLNodoFXepTgCzgeHNKiscw/edit?usp=sharing)

## INSTALL
```
./autogen.sh
./configure CFLAGS='-I/WHICHOPENNSSLDIR/openssl/include' LDFLAGS='-L/WHICHOPENNSSLDIR/openssl'
make
```


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

#Ошибка SIOCSIFFLAGS: Operation not possible due to RF-kill”?
sudo rfkill unblock wifi; sudo rfkill unblock all

sudo hostapd -d /etc/hostapd.conf >>hostapd.2.log
```

## Patch updated

При изменении библиотеки не забыть обновить либу

```
sudo cp .libs/libwacs.so.0.0.1 /usr/local/lib
```

## Tests

### Checking opened file descriptor count

```
sudo ls /proc/20661/fd | wc -l
```

15-20 is ok

## HTTP/1.1 access

Start HTTP server:

```
nohup ./wacs-http -p 55550 &
```

Start HTTPS server:

```
./wacs-http --crt wacs-http.pem --key wacs-http.key
nohup ./wacs-http --crt wacs-http.pem --key wacs-http.key &
```

### Generate self-signed SSL ceriticates

```
cd tools
./gen-cert.sh
```
produces wacs-http.pem wacs-http.key files.

### Chrome import self-signed certificates

List

```
certutil -d sql:$HOME/.pki/nssdb -L
```

Add server

```
certutil -d sql:$HOME/.pki/nssdb -A -t "P,," -n "221.commandus.com:55550" -i wacs-http.pem
```

Delete server

```
certutil -d sql:$HOME/.pki/nssdb -D -n 221.commandus.com:55550
```

Add CA

```
certutil -d sql:\$HOME/.pki/nssdb -A -t \"CT,C,C\" -n "ca.commandus.com" -i ca.crt
```

### Get list of all MAC addresses:

```
wget http://127.0.0.1:55550/last
[
{"sa":"00:0c:e7:35:f8:07","dt":1548920395},
{"sa":"00:12:36:1f:ef:de","dt":1548918644},
{"sa":"00:26:08:fa:58:47","dt":1548921258},
...
```

Maximum record/page size is 1000.

Paginate:

```
wget http://127.0.0.1:55550/last?o=0&c=10
[
{"sa":"00:0c:e7:35:f8:07","dt":1548920395},
{"sa":"00:12:36:1f:ef:de","dt":1548918644},
{"sa":"00:26:08:fa:58:47","dt":1548921258},
...
```

Filter MAC addresses:

```
wget http://127.0.0.1:55550/last?a=00:0c
[
{"sa":"00:0c:e7:35:f8:07","dt":1548920395}
]
```


Get log for specified MAC address

```
wget http://84.237.104.128:55550/log?sa=ec:35:86:2f:6e:a8
[
{"sa":"ec:35:86:2f:6e:a8","dt":1548917292,"device_id":1,"ssi_signal":-74},
{"sa":"ec:35:86:2f:6e:a8","dt":1548917299,"device_id":1,"ssi_signal":-74},
...
```

Get log for specific MAC address between start and finish times

```
wget http://84.237.104.128:55550/log?sa=ec:35:86:2f:6e:a8&s=0&f=1548917300
[
{"sa":"ec:35:86:2f:6e:a8","dt":1548917292,"device_id":1,"ssi_signal":-74},
{"sa":"ec:35:86:2f:6e:a8","dt":1548917299,"device_id":1,"ssi_signal":-74},
...
```

Paginate:

```
wget http://84.237.104.128:55550/log?sa=ec:35:86:2f:6e:a8&s=0&f=1548917300&o=0&c=10
[
{"sa":"ec:35:86:2f:6e:a8","dt":1548917292,"device_id":1,"ssi_signal":-74},
{"sa":"ec:35:86:2f:6e:a8","dt":1548917299,"device_id":1,"ssi_signal":-74},
...
```

Count log:

```
wget http://84.237.104.128:55550/log-count?sa=ec:35:86:2f:6e:a8&s=0&f=1548917300
[744 ]
```

Count last:

```
wget http://84.237.104.128:55550/last-count?sa=ec:35:86:2f:6e:a8&s=0&f=1548917300
[1744 ]
```

Valid filter options:

- sa
- start
- finish

Histogram:

```
wget http://localhost:55550/macs-per-time?step=60

[{"t": 1548917280,"c": 44},
{"t": 1548917340,"c": 40},
...
```

## hostapd log 

grep MAC address

```
grep RX hostapd.log | cut -d ' ' -f 4 | cut -d '=' -f 2 | sort |  uniq
40:f3:08:4a:6a:05
72:56:8e:e5:42:b0
```

## hostapd build

[hostapd](http://w1.fi/hostapd/)

### Dependencies

- libgnutls-dev libgcrypt20-dev (tested) or
- OpenSSL > 1.0.0 (not working)

```
sudo apt install libgnutls-dev libgcrypt20-dev
```

Mac OS X

Update bison, check gcrypt:

```
brew tap homebrew/dupes && brew install bison
brew install libgcrypt 
```

Install [libnl](https://www.infradead.org/~tgr/libnl/files/libnl-3.2.25.tar.gz)


[How to build](http://quadfinity.blogspot.com/2014/09/compile-latest-hostapd-v2.3-on-Raspberry-Pi-or-ODROID.html)

```
git clone git://w1.fi/srv/git/hostap.git
git clone http://w1.fi/hostap.git
wget http://w1.fi/releases/hostapd-2.7.tar.gz
cd hostap/hostapd
vi Makefile
#Add lines below line 28:
CFLAGS +=-I/home/andrei/src/wacs
LIBS += -L/home/andrei/src/wacs/.libs -lwacs -lnanomsg
cp defconfig .config
```

Edit .config:

```
CONFIG_TLS=gnutls
sed -i 's/^#CONFIG_DRIVER_NL80211=y/CONFIG_DRIVER_NL80211=y/g' .config
#enable 802.11n and 802.11ac
sed -i 's/^#CONFIG_IEEE80211N=y/CONFIG_IEEE80211N=y/g' .config
sed -i 's/^#CONFIG_IEEE80211AC=y/CONFIG_IEEE80211AC=y/g' .config

#enable 802.11n and 802.11ac
sed -i 's/^#CONFIG_IEEE80211N=y/CONFIG_IEEE80211N=y/g' .config
sed -i 's/^#CONFIG_IEEE80211AC=y/CONFIG_IEEE80211AC=y/g' .config

#enable automatic channel selection
sed -i 's/^#CONFIG_ACS=y/CONFIG_ACS=y/g' .config

```

Build:

```
make && sudo make install
```

Note CONFIG_TLS=gnutls with openssl can not find openssl?

### Check dependencies

In case of error:

fatal error: netlink/genl/genl.h: No such file or directory

```
apt-file search /netlink/genl/genl.h
sudo apt install libnl-3-dev
```

In case of error:

cannot find -lnl-genl-3

```
apt-file search libnl-genl-3.a
libnl-genl-3-dev: /lib/x86_64-linux-gnu/libnl-genl-3.a
sudo apt install libnl-genl-3-dev
```

## hostapd changes

In the git/hostap/src/drivers/driver_nl80211_event.c, function mlme_event_mgmt()

after line 650: 

```
wpa_printf(MSG_DEBUG,
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

# TODO

Now sendLogEntryC() open socket to the collector (wacs), send to opened socket, close socket. 

Replace to (wacs-c.h)

- openSocketLogC() - open socket
- closeSocketLogC() - closesocket
- sendLogEntrySocketC()- send to socket
 
 
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
			     		|count 
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

Mac OS X

```
export CPPFLAGS="-I/usr/local/opt/openssl@1.1/include"
export LDFLAGS="-L/usr/local/opt/openssl@1.1/lib"
./configure
...
```

### microhttpd

CentOS
```
sudo yum install libmicrohttpd-devel
```

Mac OS X
```
brew install libmicrohttpd
```

### libpq

Mac OS X

```
brew install libpq
export LDFLAGS="-L/usr/local/opt/libpq/lib"
export CPPFLAGS="-I/usr/local/opt/libpq/include"
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

Mac OS X

```
brew install libsnmp snmp snmpd snmptrapd
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

Mac OS X

```
brew tap discoteq/discoteq
brew install flock
```

fatal error: 'malloc.h' file not found

vi src/osal.h

#if defined(__MACH__)
#include <stdlib.h>
#else 
#include <malloc.h>

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

## Collector

Start collector (foregroud):

```
./wacs -vvv 
Socket binded successfully to tcp://127.0.0.1:55555
Received bytes: 10
Received bytes: 10, MAC: ec:35:86:2f:6e:a8, device_id: 1, ssi_signal: -80
...

```

Start collector (backgroud):

Демон:

```
sudo ./wacs -d
ps -ef | grep wacs
root     22856  2011  0 15:51 ?        00:00:00 ./wacs -d
```

или

```
nohup ./wacs &
```
### See log

```
./wacsc log
00:ec:0a:ed:5c:55	2019-01-31T15:48:49+09	1	-76
00:ec:0a:ed:5c:55	2019-01-31T15:48:50+09	1	-76
0c:ee:e6:c6:e7:70	2019-01-31T15:48:18+09	1	-78
12:98:6b:22:93:aa	2019-01-31T15:49:51+09	1	-74
12:98:6b:22:93:aa	2019-01-31T15:49:52+09	1	
...
```

### Filter by MAC address

Provide first bytes of MAC address:

```
./wacsc log 00:ec
00:ec:0a:ed:5c:55	2019-01-31T15:48:49+09	1	-76
00:ec:0a:ed:5c:55	2019-01-31T15:48:50+09	1	-76
```

### Paginate

```
./wacsc log -o 0 -c 10
00:ec:0a:ed:5c:55	2019-01-31T15:48:49+09	1	-76
00:ec:0a:ed:5c:55	2019-01-31T15:48:50+09	1	-76
0c:ee:e6:c6:e7:70	2019-01-31T15:48:18+09	1	-78
12:98:6b:22:93:aa	2019-01-31T15:49:51+09	1	-74
12:98:6b:22:93:aa	2019-01-31T15:49:52+09	1	
...
```

### Count log and probes

```
./wacsc probe-count
744
./wacsc log-count
3185
```

Valid filter options:

- sa
- start
- finish

### Histogram

```
./wacsc macs-per-time --start 2019-01-31T00:00:00 --step 60
2019-01-31T15:48:00+09	44
2019-01-31T15:49:00+09	40
2019-01-31T15:50:00+09	40
2019-01-31T15:51:00+09	62
2019-01-31T15:52:00+09	81
...
```

### Simulate activity

```
./wacsc test -vvv -d 1 -b "-54" -n 5 "11:22:33:44:55:66"
./wacsc test 11:22:33:44:55:66 66:55:44:33:22:11 11:11:11:11:11:11 22:22:22:22:22:22 33:33:33:33:33:33 44:44:44:44:44:44 55:55:55:55:55:55 66:66:66:66:66:66 -d 2 -b -34 -n 100
./wacsc log -vvv -d 1 "11:22:33:44:55:66"
```

### Dump

Записать лог

```
./wacsc log > foo.bar
```

Загрузить лог из файла

```
./wacsc log-read < foo.bar
3185 records added.
```

Можно использовать опцию -l, --log-file для указания имени файла лога.

Формат строки лога:

Строки разделяются символом LF.

Поля разделяются символом табуляции. Допускается использование пробелов.

| № п/п |  Название поля    | Формат                                      | Пример                  | Примечание                                |
|-------|-------------------|---------------------------------------------|-------------------------|-------------------------------------------|
|   1   |  MAC              | 6 шестнадцатиричных чисел, разделенных ':'  | f6:72:6b:85:b9:21       |                                           |
|   2   |  Дата             | YYYY-MM-DDThh:mm:ss местного времени        | 2019-01-31T16:09:55+09  | или десятичное число Unix epoch seconds   |
|   3   |  Номер устройства | десятичное целое число 0..255               | 1                       | 0 лучше не использлвать в качестве номера |
|   4   |  Уровень сигнала  | десятичное целое число -32768..32767        | -80                     | dBm, практический диапазон -80..80        |
|   6   |  fc               | десятичное целое число 0..255               | 0x40                    | fc                                        |


Если в дате указан часовой пояс, его значение не используется. Применяется часовой пояс из текущей локали.

[dBm](https://en.wikipedia.org/wiki/DBm)

Пример

```
f6:72:6b:85:b9:21	2019-01-31T16:09:55+09	1	-74
```

### Erase database

Erase all:

```
./wacsc remove

```

Valid filter options:

- sa
- start
- finish

## Notification

List

```
./wacsc notification
```

Write to file:

```
./wacsc notification -l a.lst
```

## Put notification on MAC

```
./wacsc notification-put 11:22:33:44:55:66 < notification.sample.json
```

or

```
./wacsc notification-put 11:22:33:44:55:66 -l notification.sample.json
```

## Checking map size

```
mdbx_stat -e  .
Environment Info
...
Current mapsize: 33554432 bytes, 8192 pages
```

## Error codes

COMMAND							-1
PARSE_COMMAND					-2
WRONG_PARAM						-3

LMDB_TXN_BEGIN					-10
LMDB_TXN_COMMIT					-11
LMDB_OPEN						-12
LMDB_CLOSE						-13
LMDB_PUT						-14
LMDB_PUT_PROBE					-15
LMDB_GET						-16

NN_SOCKET						-20
NN_CONNECT	 					-21
NN_BIND		 					-22
NN_SUBSCRIBE					-23
NN_SHUTDOWN 					-24
NN_RECV							-25
NN_SEND							-26
NN_SET_SOCKET_OPTION			-27
NN_FREE_MSG						-28
NN_ACCEPT						-29

STOP							-40
NO_CONFIG						-41
NO_MEMORY						-42
NOT_IMPLEMENTED					-43

SOCKET_SEND						-50
GET_ADDRINFO                    -51
SOCKET_CREATE					-52
SOCKET_SET_OPTIONS				-53
SOCKET_BIND						-54
SOCKET_CONNECT                  -55
SOCKET_LISTEN					-56
SOCKET_READ						-57
SOCKET_WRITE					-58

FORK							-60
EXEC							-61
CONFIG							-62
WSA_ERROR						-63

NO_NOBILE_SUBSCRIBERS			-70

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
