#include "wacs-c.h"
#include "send-log-entry.h"
#include <string.h>

EXPORT_C int sendLogEntryC
(
	const char *message_url,
	unsigned int device_id,	// 0..255 AP identifier
	int ssi_signal,			// dB
	unsigned char* sa,		// MAC address
	int repeats,
	int verbosity
)
{
	LogEntry e;
	e.device_id = device_id;
	e.ssi_signal = ssi_signal;
	memmove(e.sa, sa, 6);
	return sendLogEntry
	(
		std::string(message_url),
		&e,
		repeats,
		verbosity
	);
}

