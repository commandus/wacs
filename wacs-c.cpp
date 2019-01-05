#include "wacs-c.h"

EXPORT_C int sendLogEntryC
(
	const std::string &message_url,
	const LogEntry* value,
	int repeats,
	int verbosity
)
{
	return sendLogEntryC
	(
		message_url,
		value,
		repeats,
		verbosity
	);
}

