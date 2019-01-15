#include "snmp-params.h"
#include <fstream>
#include "if-ip-address.h"

MonitoringParams::MonitoringParams()
: databasefilename(""), ip(""),
	port(0), starttime(time(NULL)), requestsperhour(0), lastmac(""),
	lastssisignal(0), totalmac(0), errorcount(0)
{
}

void MonitoringParams::clear()
{
	databasefilename = "";
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
    std::ifstream in(databasefilename, std::ifstream::ate | std::ifstream::binary);
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
