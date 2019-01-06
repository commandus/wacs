#ifndef WACS_C_H_
#define WACS_C_H_	1

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
	const char *message_url,
	unsigned int device_id,	// 0..255 AP identifier
	int ssi_signal,			// dB
	unsigned char* sa,		// MAC address
	int repeats,
	int verbosity
);

#ifdef __cplusplus
}
#endif
#endif

