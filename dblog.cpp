#include "dblog.h"
#include "iostream"
#include "errorcodes.h"
#include "hostapd-log-entry.h"
#include <string.h>

#include "log.h"

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

)
{
	int rc = mdb_env_create(&env->env);
	if (rc)
	{
		LOG(ERROR) << "mdb_env_create error " << rc << " " << mdb_strerror(rc);
		env->env = NULL;
		return false;
	}

	rc = mdb_env_open(env->env, path, flags, mode);
	if (rc)
	{
		LOG(ERROR) << "mdb_env_open path: " << path << " error " << rc << " " << mdb_strerror(rc);
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
bool closeDb
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
int putLog
(
	struct dbenv *env,
	void *buffer,
	size_t size,
	int verbosity
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
#if __BYTE_ORDER == __LITTLE_ENDIAN	
	key.dt = htobe32(time(NULL));
#else
	key.dt = time(NULL);
#endif	
	memmove(key.sa, entry->sa, 6);
	MDB_val dbkey;
	dbkey.mv_size = sizeof(LogKey);
	dbkey.mv_data = &key;

	LogData data;
	data.device_id = entry->device_id;
	data.ssi_signal = entry->ssi_signal;
	MDB_val dbdata;
	dbdata.mv_size = sizeof(LogData);
	dbdata.mv_data = &data;
	
	if (verbosity > 2)
		LOG(INFO) << MSG_RECEIVED << size 
			<< ", MAC: " << mactostr(key.sa)
			<< ", device_id: " << entry->device_id
			<< ", ssi_signal: " << entry->ssi_signal
			<< std::endl;

	r = mdb_put(env->txn, env->dbi, &dbkey, &dbdata, 0);
	if (r)
	{
		mdb_txn_abort(env->txn);
		LOG(ERROR) << ERR_LMDB_PUT << r;
		return ERRCODE_LMDB_PUT;
	}

	// probe
	key.tag = 'P';
	dbkey.mv_size = sizeof(LastProbeKey);
	dbdata.mv_size = 0; 
	dbdata.mv_data = NULL;
	r = mdb_put(env->txn, env->dbi, &dbkey, &dbdata, 0);
	if (r)
	{
		mdb_txn_abort(env->txn);
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
	const uint8_t *sa,			// MAC address
	time_t start,			// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog
)
{
	if (!onLog)
	{
		LOG(ERROR) << ERR_WRONG_PARAM << "onLog" << std::endl;
		return ERRCODE_WRONG_PARAM;
	}
	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r)
	{
		LOG(ERROR) << ERR_LMDB_TXN_BEGIN << r << std::endl;
		return ERRCODE_LMDB_TXN_BEGIN;
	}

	LogKey key;
	key.tag = 'L';
#if __BYTE_ORDER == __LITTLE_ENDIAN	
	key.dt = be32toh(start);
#else
	key.dt = start;
#endif	

	if (sa)
		memmove(key.sa, sa, 6);
	else
		memset(key.sa, 0, 6);

	MDB_val dbkey;
	dbkey.mv_size = sizeof(LogKey);
	dbkey.mv_data = &key;

	// Get the last key
	MDB_cursor *cursor;
	MDB_val dbval;
	r = mdb_cursor_open(env->txn, env->dbi, &cursor);
	if (r != MDB_SUCCESS) 
	{
		LOG(ERROR) << ERR_LMDB_OPEN << r << ": " << strerror(r) << std::endl;
		mdb_txn_commit(env->txn);
		return r;
	}
	r = mdb_cursor_get(cursor, &dbkey, &dbval, MDB_SET_RANGE);
	if (r != MDB_SUCCESS) 
	{
		LOG(ERROR) << ERR_LMDB_GET << r << ": " << strerror(r) << std::endl;
		mdb_txn_commit(env->txn);
		return r;
	}

	MDB_cursor_op dir;
	if (finish >= start)
		dir = MDB_NEXT;
	else
		dir = MDB_PREV;

	do {
		if ((dbval.mv_size < sizeof(LogData)) || (dbkey.mv_size < sizeof(LogKey)))
			continue;
		LogKey key1;
		memmove(key1.sa, ((LogKey*) dbkey.mv_data)->sa, 6);
#if __BYTE_ORDER == __LITTLE_ENDIAN	
		key1.dt = be32toh(((LogKey*) dbkey.mv_data)->dt);
#else
		key1.dt = ((LogKey*) dbkey.mv_data)->dt;
#endif	
		if (memcmp(key1.sa, sa, 6) != 0)
			break;

		if (finish > start) 
		{
			if (key1.dt > finish)
				break;
			if (key1.dt < start)
				continue;
		}
		else
		{
			if (key1.dt < finish)
				break;
			if (key1.dt > start)
				continue;
		}

		LogData data;
		data.device_id = ((LogData *) dbval.mv_data)->device_id;
		data.ssi_signal = ((LogData *) dbval.mv_data)->ssi_signal;
		if (onLog(NULL, &key1, &data))
			break;
	} while (mdb_cursor_get(cursor, &dbkey, &dbval, dir) == MDB_SUCCESS);

	r = mdb_txn_commit(env->txn);
	if (r)
	{
		LOG(ERROR) << ERR_LMDB_TXN_COMMIT << r << std::endl;
		return ERRCODE_LMDB_TXN_COMMIT;
	}
	return r;
}
