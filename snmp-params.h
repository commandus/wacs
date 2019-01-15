#ifndef SNMP_PARAMS_H
#define SNMP_PARAMS_H
#include <string>
#include <inttypes.h>
#include "hostapd-log-entry.h"

class MonitoringParams 
{
public:
	MonitoringParams();
	std::string databasefilename;
	std::string ip;
	uint32_t port;			// 32 for MIB is better
    uint32_t starttime;		// time_t can be 8 bit so do not!
    int32_t requestsperhour;
	std::string lastmac;
	int32_t lastssisignal;
	int32_t totalmac;
	int32_t errorcount;
	
	void clear();
	size_t getDatabaseFileSize();
	std::string getIP();
	void put
	(
		const void *value
	);
};

#endif
