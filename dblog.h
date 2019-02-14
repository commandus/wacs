#ifdef USE_LMDB
#include "lmdb.h"
#else
#include "mdbx.h"

#define MDB_SET_RANGE MDBX_SET_RANGE
#define MDB_NEXT MDBX_NEXT
#define MDB_PREV MDBX_PREV
#define MDB_SUCCESS MDBX_SUCCESS
#define MDB_cursor_op MDBX_cursor_op
#define MDB_env	MDBX_env
#define MDB_dbi	MDBX_dbi
#define MDB_txn	MDBX_txn
#define MDB_cursor	MDBX_cursor
#define MDB_val	MDBX_val
#define mdb_env_create	mdbx_env_create
#define mdb_env_open mdbx_env_open
#define mdb_env_close mdbx_env_close
#define mdb_txn_begin mdbx_txn_begin
#define mdb_txn_commit mdbx_txn_commit
#define mdb_txn_abort mdbx_txn_abort
#define mdb_strerror mdbx_strerror
#define mdb_open mdbx_dbi_open
#define mdb_close mdbx_dbi_close
#define mdb_put mdbx_put
#define mdb_cursor_open mdbx_cursor_open
#define mdb_cursor_get mdbx_cursor_get
#define mdb_cursor_del mdbx_cursor_del

#define mv_size iov_len
#define mv_data iov_base

#endif

/**
 * @brief LMDB environment(transaction, cursor)
 */
typedef struct dbenv {
	MDB_env *env;
	MDB_dbi dbi;
	MDB_txn *txn;
	MDB_cursor *cursor;
} dbenv;

typedef struct 
{
	uint8_t tag;		// 'L'- log entry
	uint8_t sa[6];		// MAC address
	time_t dt;			// time, seconds since Unix epoch 
} LogKey;

typedef struct 
{
	uint16_t device_id;	// 0
	int8_t ssi_signal;	// dBm
} LogData;

typedef struct 
{
	uint8_t tag;		// 'P'- last_probe
	uint8_t sa[6];		// MAC address
} LastProbeKey;

typedef struct 
{
	uint8_t sa[6];		// MAC address
	uint32_t dt;		// Date & Time
	uint16_t device_id;	// 0..255 AP identifier
	int8_t ssi_signal;	// dBm
} LogRecord;

/**
 * @brief Opens LMDB database file
 * @param env created LMDB environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool openDb
(
	struct dbenv *env,
	const char *path,
	int flags,
	int mode
);

/**
 * @brief Close LMDB database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool closeDb
(
	struct dbenv *env
);

// callback, return true - stop request, false- continue
typedef bool (*OnLog)
(
	void *env,
	LogKey *key,
	LogData *data
);

// callback, return true - stop request, false- continue
typedef bool (*OnLastProbe)
(
	void *env,
	LastProbeKey *key
);

/**
 * callback for putLogEntries, return true - stop request, false- continue
 * @param retval return date&time, device id, ssi signal
 */
typedef bool (*OnPutLogEntry)
(
	void *env,
	LogRecord *rec
);

/**
 * @brief Store input log data to the LMDB
 * @param env database env
 * @return 0 - success
 */
int putLog
(
	struct dbenv *env,
	void *buffer,
	size_t size,
	int verbosity
);

/**
 * @brief Store input log data to the LMDB
 * @param env database env
 * @param buffer buffer (LogEntry: device_id(0..255), ssi_signal(-128..127), MAC(6 bytes)
 * @param size buffer size
 * @return 0 - success
 */
int putLogEntries
(
	struct dbenv *env,
	int verbosity,
	OnPutLogEntry onPutLogEntry,
	void *onPutLogEntryEnv
);

/**
 * @brief Read log data from the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0 
 * @param start 0- no limit
 * @param finish 0- no limit
 * @param onLog callback
 * @param onLogEnv object passed to callback
 */
int readLog
(
	struct dbenv *env,
	const uint8_t *sa,			///< MAC address
	int saSize,
	time_t start,				///< time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
);

/**
 * @brief remove log data from the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0 
 * @param start 0- no limit
 * @param finish 0- no limit
 */
int rmLog
(
	struct dbenv *env,
	const uint8_t *sa,			///< MAC address
	int saSize,
	time_t start,				///< time, seconds since Unix epoch 
	time_t finish
);

/**
 * @brief Read last probes from the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0
 * @param start 0- no limit
 * @param finish 0- no limit
 * @param onLog callback
 * @param onLogEnv object passed to callback
 */
int readLastProbe
(
	struct dbenv *env,
	const uint8_t *sa,			///< MAC address
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
);
