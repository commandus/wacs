#include "snmp-params.h"
#include <fstream>
#include "if-ip-address.h"

#define DEF_FILE_NAME "mdbx.dat"

MonitoringParams::MonitoringParams()
: databasefilename(DEF_FILE_NAME), ip(""),
	port(0), starttime(time(NULL)), requestsperhour(0), lastmac(""),
	lastssisignal(0), totalmac(0), errorcount(0)
{
}

void MonitoringParams::clear()
{
	databasefilename = DEF_FILE_NAME;
	ip = "";
	port = 0;
	starttime = time(NULL);
	requestsperhour = 0;
	lastmac = "";
	lastssisignal = 0;
	totalmac = 0;
	errorcount = 0;
}

size_t MonitoringParams::getDatabaseFileSize()
{
    std::ifstream in(databasefilename.c_str(), std::ifstream::ate | std::ifstream::binary);
	std::ifstream::pos_type r = in.tellg();
    return r; 
}

std::string MonitoringParams::getIP()
{
	ip = ifIPAddressString();
	return ip;
}

void MonitoringParams::put
(
	const void *value
)
{
	if (value)
	{
		lastmac = mactostr(((LogEntry *)value)->sa);
		lastssisignal = ((LogEntry *)value)->ssi_signal;
	}
}
