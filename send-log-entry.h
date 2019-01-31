#include <string>
#include "hostapd-log-entry.h"

int openSocketLog
(
	int *socket,
	int *endpoint,
	const std::string &message_url
);

int closeSocketLog
(
	int socket,
	int endpoint
);

int sendLogEntry
(
	const std::string &message_url,
	const LogEntry* value,
	int repeats,
	int verbosity
);

int sendLogEntrySocket
(
	int socket,
	const LogEntry* value,
	int repeats,
	int verbosity
);
