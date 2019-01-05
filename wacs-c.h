#ifndef WACS_C_H_
#define WACS_C_H_	1

#include "send-log-entry.h"

/**
 * C wrapper
 */
#ifdef __cplusplus
extern "C" {
#endif
	

#ifdef _MSC_VER
#ifdef EXPORT_C_DLL
#define EXPORT_C extern "C" __declspec(dllexport)
#else
#define EXPORT_C
#endif
#else
#define EXPORT_C
#endif

EXPORT_C int sendLogEntryC
(
	const std::string &message_url,
	const LogEntry* value,
	int repeats,
	int verbosity
);

#ifdef __cplusplus
}
#endif
#endif

