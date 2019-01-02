#include "wacs-config.h"

#ifdef USE_LMDB
#include "lmdb.h"
#else
#include "mdbx.h"
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

/**
 * @brief Opens LMDB database file
 * @param env created LMDB environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool open_lmdb
(
	struct dbenv *env,
	WacsConfig *config
);

/**
 * @brief Close LMDB database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool close_lmdb
(
	struct dbenv *env
);

size_t getKey
(
	int mode, 
	void *keybuffer,
	size_t keysize,
	void *buffer,
	size_t size
);

/**
 * @brief Store input log data to the LMDB
 * @param env database env
 * @param buffer buffer
 * @param buffer_size buffer size
 * @return 0 - success
 */
int put_db
(
	struct dbenv *env,
	void *buffer,
	int buffer_size
);
