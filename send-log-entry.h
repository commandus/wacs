#include <string>
#include "hostapd-log-entry.h"

int sendLogEntry
(
	const std::string &message_url,
	const LogEntry* value,
	int repeats,
	int verbosity
);
