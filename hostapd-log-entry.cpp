#include <netinet/in.h>
#include <sstream>
#include <iomanip>
#include <stdio.h>
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
	const void *value
)
{
	std::stringstream ss;
	uint8_t *p = (uint8_t *) value;
	ss << std::setfill('0') << std::hex 
		<< std::setw(2) << (int) p[0] << ":"
		<< std::setw(2) << (int) p[1] << ":"
		<< std::setw(2) << (int) p[2] << ":"
		<< std::setw(2) << (int) p[3] << ":"
		<< std::setw(2) << (int) p[4] << ":"
		<< std::setw(2) << (int) p[5];
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

/**
 * string to MAC address
 */
bool strtomacaddress
(
	void *retval,
	const std::string &value
)
{
	unsigned int v[6];
	int c = sscanf(value.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
		&v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);
	uint8_t *r = (uint8_t *) retval;
	r[0] = v[0];
	r[1] = v[1];
	r[2] = v[2];
	r[3] = v[3];
	r[4] = v[4];
	r[5] = v[5];
}
