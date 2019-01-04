# WACS

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

### libmdbx (LMDB clone)

OpenLDAP Public License

```
git clone git@github.com:leo-yuriev/libmdbx.git
cd libmdbx
make
$ sudo make install
```

### nanomsg IPC

(nanomsg)[http://nanomsg.org]

MIT/X11 license

```
git clone git@github.com:nanomsg/nanomsg.git
```

## Test


```
./wacs -vvv 
./wacsc test -vvv -d 1 -b "-54" -a "11:22:33:44:55:66" -n 10
```

```
./wacsc log -vvv -d 1 -a "11:22:33:44:55:66"
```
