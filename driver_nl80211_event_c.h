/**
 * In the hostapd source file
 * git/hostap/src/drivers/driver_nl80211_event.c
 * add in the function mlme_event_mgmt(), after line 655 code:
 * 
 * #include "driver_nl80211_event_c.h"
 * 
 * Then add include and library path to the wacs, add -lwacs and make it
 * See http://quadfinity.blogspot.com/2014/09/compile-latest-hostapd-v2.3-on-Raspberry-Pi-or-ODROID.html
 * 
 */
#include "wacs-c.h"

sendLogEntryC("tcp://127.0.0.1:55555", 1, ssi_signal, mgmt->sa, 1, 0);
