#include <netinet/in.h>
#include <sstream>
#include <iomanip>
#include "hostapd-log-entry.h"

/**
 * Network to host conversion
 */
void ntohLogEntry
(
	LogEntry *value
)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	value->device_id = ntohs(value->device_id);
	value->ssi_signal = ntohs(value->ssi_signal);
#endif	
}

/**
 * Host to network conversion
 */
void htonLogEntry
(
	LogEntry *value
)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	value->device_id = htons(value->device_id);
	value->ssi_signal = htons(value->ssi_signal);
#endif	
}

/**
 * MAC address to string
 */
std::string mactostr
(
	void *value
)
{
	std::stringstream ss;
	char *p = (char *) value;
	ss << std::hex << std::setw(2) << std::setfill('0') 
		<< (int) p[0] << ":"
		<< (int) p[1] << ":"
		<< (int) p[2] << ":"
		<< (int) p[3] << ":"
		<< (int) p[4] << ":"
		<< (int) p[5];
	return ss.str();
}

/**
 * MAC address to string
 */
std::string mactostr
(
	LogEntry *value
)
{
	return mactostr(value->sa);
}
