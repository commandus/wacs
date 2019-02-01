#include "wacs-c.h"
#include "send-log-entry.h"
#include <string.h>
#include <iostream>

EXPORT_C int sendLogEntryC
(
	const char *message_url,
	unsigned int device_id,	// 0..255 AP identifier
	int ssi_signal,			// dB
	const unsigned char* sa,		// MAC address
	int repeats,
	int verbosity
)
{
	LogEntry e;
	e.device_id = device_id;
	e.ssi_signal = ssi_signal;
	if (sa)
		memmove(e.sa, sa, 6);
	return sendLogEntry
	(
		std::string(message_url),
		&e,
		repeats,
		verbosity
	);
}

EXPORT_C int sendLogEntrySocketC
(
	int socket,
	unsigned int device_id,	// 0..255 AP identifier
	int ssi_signal,			// dB
	const unsigned char* sa,		// MAC address
	int repeats,
	int verbosity
)
{
	LogEntry e;
	e.device_id = device_id;
	e.ssi_signal = ssi_signal;
	if (sa)
		memmove(e.sa, sa, 6);
	return sendLogEntrySocket
	(
		socket,
		&e,
		repeats,
		verbosity
	);
}

EXPORT_C int openSocketLogC
(
	int *socket,
	int *endpoint,
	const char *message_url
)
{
	return openSocketLog(
		socket,
		endpoint,
		message_url
	);
}

EXPORT_C int closeSocketLogC
(
	int socket,
	int endpoint
)
{
	return closeSocketLog(socket, endpoint);
}
