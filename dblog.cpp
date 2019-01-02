#include "dblog.h"
#include "iostream"
#include "errorcodes.h"
#include "hostapd-log-entry.h"
#include <string.h>

#define LOG(verbosity) \
	std::cerr

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
)
{
	int rc = mdb_env_create(&env->env);
	if (rc)
	{
		LOG(ERROR) << "mdb_env_create error " << rc << " " << mdb_strerror(rc);
		env->env = NULL;
		return false;
	}

	rc = mdb_env_open(env->env, config->path.c_str(), config->flags, config->mode);
	if (rc)
	{
		LOG(ERROR) << "mdb_env_open path: " << config->path.c_str() << " error " << rc << " " << mdb_strerror(rc);
		env->env = NULL;
		return false;
	}

	rc = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (rc)
	{
		LOG(ERROR) << "mdb_txn_begin error " << rc << " " << mdb_strerror(rc);
		env->env = NULL;
		return false;
	}


	rc = mdb_open(env->txn, NULL, 0, &env->dbi);
	if (rc)
	{
		LOG(ERROR) << "mdb_open error " << rc << " " << mdb_strerror(rc);
		env->env = NULL;
		return false;
	}

	rc = mdb_txn_commit(env->txn);

	return rc == 0;
}

/**
 * @brief Close LMDB database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool close_lmdb
(
	struct dbenv *env
)
{
	mdb_close(env->env, env->dbi);
	mdb_env_close(env->env);
	return true;
}

/**
 * @brief Store input packet to the LMDB
 * @param env database env
 * @param buffer buffer
 * @param buffer_size buffer size
 * @return 0 - success
 */
int put_db
(
	struct dbenv *env,
	void *buffer,
	size_t size
)
{
	if (size < sizeof(LogEntry))
		return ERRCODE_NO_MEMORY;
	LogEntry *entry = (LogEntry *) buffer;
	// swap bytes if needed
	ntohLogEntry(entry);

	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r)
	{
		LOG(ERROR) << ERR_LMDB_TXN_BEGIN << r;
		return ERRCODE_LMDB_TXN_BEGIN;
	}

	// log
	LogKey key;
	key.tag = 'L';
	key.dt = time(NULL);
	memmove(key.sa, entry->sa, sizeof(key.sa));
	MDB_val dbkey;
	dbkey.mv_size = sizeof(LogKey);
	dbkey.mv_data = &key;

	LogData data;
	data.device_id = entry->device_id;
	data.ssi_signal = entry->ssi_signal;
	MDB_val dbdata;
	dbdata.mv_size = sizeof(LogData);
	dbdata.mv_data = &data;

	r = mdb_put(env->txn, env->dbi, &dbkey, &dbdata, 0);
	{
		LOG(ERROR) << ERR_LMDB_PUT << r;
		return ERRCODE_LMDB_PUT;
	}

	// probe
	key.tag = 'P';
	dbkey.mv_size = sizeof(LastProbeKey);
	dbdata.mv_size = 0; 
	dbdata.mv_data = NULL;
	r = mdb_put(env->txn, env->dbi, &dbkey, &dbdata, 0);
	{
		LOG(ERROR) << ERR_LMDB_PUT << r;
		return ERRCODE_LMDB_PUT;
	}

	r = mdb_txn_commit(env->txn);
	if (r)
	{
		LOG(ERROR) << ERR_LMDB_TXN_COMMIT << r;
		return ERRCODE_LMDB_TXN_COMMIT;
	}
	return r;
}

