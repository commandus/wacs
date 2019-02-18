/**
 * 
 */

#ifndef HOSTAPD_LOG_ENTRY_H
#define HOSTAPD_LOG_ENTRY_H 1
#include <string>
#include <stdint.h>

#ifdef _MSC_VER
#define ALIGN	__declspec(align(1))
#define PACKED	
#pragma pack(push,1)
#else
#define ALIGN
#define PACKED	__attribute__((aligned(1), packed))
#endif

typedef ALIGN struct 
{
	uint16_t device_id;	// 0..255 AP identifier
	int16_t ssi_signal;	// dB
	uint8_t sa[6];		// MAC address
} PACKED LogEntry;

/**
 * Network to host conversion
 */
void ntohLogEntry(LogEntry *value);

/**
 * Host to network conversion
 */
void htonLogEntry(LogEntry *value);

/**
 * MAC address to string
 */
std::string mactostr
(
	const void *value
);

/**
 * MAC address to string
 */
std::string mactostr
(
	LogEntry *value
);

/**
 * string to MAC address
 * @param retval must points to 6 bytes space
 * @param value may be incomplete(filled with zeroes)
 * @return 0..6. 6- all MAC address bytes has been read
 */
int strtomacaddress
(
	void *retval,
	const std::string &value
);

/**
 * MAC address to string
 */
std::string mactostr2
(
	const void *value,
	int size
);

#endif
