#ifdef USE_LMDB
#include "lmdb.h"
#else
#include "mdbx.h"

#define MDB_SET_RANGE MDBX_SET_RANGE
#define MDB_NEXT MDBX_NEXT
#define MDB_SUCCESS MDBX_SUCCESS
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
#define mdb_strerror mdbx_strerror
#define mdb_open mdbx_dbi_open
#define mdb_close mdbx_dbi_close
#define mdb_put mdbx_put
#define mdb_cursor_get mdbx_cursor_get

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
	int16_t ssi_signal;	// dB
} LogData;

typedef struct 
{
	uint8_t tag;		// 'P'- last_probe
	uint8_t sa[6];		// MAC address
} LastProbeKey;

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

/**
 * @brief Store input log data to the LMDB
 * @param env database env
 * @param buffer buffer
 * @param size buffer size
 * @return 0 - success
 */
int putLog
(
	struct dbenv *env,
	void *buffer,
	size_t size
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
 * @brief Store input log data to the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param start 0- no limit
 * @param finish 0- no limit
 * @param onLog callback
 */
int readLog
(
	struct dbenv *env,
	uint8_t *sa,			// MAC address
	time_t start,			// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog
);
