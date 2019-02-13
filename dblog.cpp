#include "dblog.h"
#include "iostream"
#include "errorcodes.h"
#include "hostapd-log-entry.h"
#include <string.h>
#include <netinet/in.h>

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
 * @param buffer buffer (LogEntry: device_id(0..255), ssi_signal(-32768..23767), MAC(6 bytes)
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
	dbdata.mv_size = sizeof(key.dt); 
	dbdata.mv_data = &(key.dt);
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
 * @return 0 - success
 */
int putLogEntries
(
	struct dbenv *env,
	int verbosity,
	OnPutLogEntry onPutLogEntry,
	void *onPutLogEntryEnv
)
{
	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r)
	{
		LOG(ERROR) << ERR_LMDB_TXN_BEGIN << r;
		return ERRCODE_LMDB_TXN_BEGIN;
	}

	LogRecord record;
	int c = 0;
	while (!onPutLogEntry(onPutLogEntryEnv, &record)) {
		// swap bytes if needed
		LogKey key;
		key.tag = 'L';
		memmove(key.sa, record.sa, 6);

		LogData data;
		data.device_id = record.device_id;
		data.ssi_signal = record.ssi_signal;	// do not htobe16
#if BYTE_ORDER == LITTLE_ENDIAN
		key.dt = htobe32(record.dt);
#endif	
		MDB_val dbkey;
		dbkey.mv_size = sizeof(LogKey);
		dbkey.mv_data = &key;
		
		MDB_val dbdata;
		dbdata.mv_size = sizeof(LogData);
		dbdata.mv_data = &data;
		
		if (verbosity > 2)
			LOG(INFO) << MSG_RECEIVED
				<< ", MAC: " << mactostr(key.sa)
				<< ", device_id: " << record.device_id
				<< ", ssi_signal: " << record.ssi_signal
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
		dbdata.mv_size = sizeof(key.dt); 
		dbdata.mv_data = &(key.dt);
		r = mdb_put(env->txn, env->dbi, &dbkey, &dbdata, 0);
		
		c++;
	}

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
	return c;
}

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
	const uint8_t *sa,			// MAC address
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
)
{
	if (!onLogEnv)
		return ERRCODE_WRONG_PARAM;
	struct ReqEnv* e = (struct ReqEnv*) onLogEnv;

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
	key.dt = htobe32(start);
#else
	key.dt = start;
#endif	
	int sz;
	if (saSize >= 6)
		sz = 6;
	else 
		if (saSize <= 0)
			sz = 0;
		else
			sz = saSize;
		
	memset(key.sa, 0, 6);
	if (sa)
		memmove(key.sa, sa, sz);

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
		if (sa)
			if (memcmp(key1.sa, sa, sz) != 0)
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
		if (onLog(onLogEnv, &key1, &data))
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

/**
 * @brief Remove log data from the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0
 * @param start 0- no limit
 * @param finish 0- no limit
 */
int rmLog
(
	struct dbenv *env,
	const uint8_t *sa,			// MAC address
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish
)
{
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
	key.dt = htobe32(start);
#else
	key.dt = start;
#endif	
	int sz;
	if (saSize >= 6)
		sz = 6;
	else 
		if (saSize <= 0)
			sz = 0;
		else
			sz = saSize;
		
	memset(key.sa, 0, 6);
	if (sa)
		memmove(key.sa, sa, sz);

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

	int cnt = 0;
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
		if (sa)
			if (memcmp(key1.sa, sa, sz) != 0)
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
		mdb_cursor_del(cursor, 0);
		cnt++;
	} while (mdb_cursor_get(cursor, &dbkey, &dbval, dir) == MDB_SUCCESS);

	r = mdb_txn_commit(env->txn);
	if (r)
	{
		LOG(ERROR) << ERR_LMDB_TXN_COMMIT << r << std::endl;
		return ERRCODE_LMDB_TXN_COMMIT;
	}
	if (r == 0)
		r = cnt;
	return r;
}

/**
 * @brief Read last probes from the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0 
 * @param start 0- no limit
 * @param finish 0- no limit
 * @param onLog callback
 */
int readLastProbe
(
	struct dbenv *env,
	const uint8_t *sa,			///< MAC address filter
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
)
{
	if (!onLogEnv)
		return ERRCODE_WRONG_PARAM;
	struct ReqEnv* e = (struct ReqEnv*) onLogEnv;

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

	LastProbeKey key;
	key.tag = 'P';

	int sz;
	if (saSize >= 6)
		sz = 6;
	else 
		if (saSize <= 0)
			sz = 0;
		else
			sz = saSize;

	memset(key.sa, 0, 6);
	if (sa && (sz > 0))
		memmove(key.sa, sa, sz);
	MDB_val dbkey;
	dbkey.mv_size = sizeof(LastProbeKey) - (6 - sz);	// exclude incomplete MAC address bytes
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

	MDB_cursor_op dir = MDB_NEXT;

	do {
		LogKey key1;
		if ((dbkey.mv_size < sizeof(LastProbeKey)) || (dbval.mv_size < sizeof(key1.dt)))
			continue;
		memmove(key1.sa, ((LastProbeKey*) dbkey.mv_data)->sa, 6);
		if (sa)
		{
			if (memcmp(key1.sa, sa, sz) != 0)
				break;
		}
#if __BYTE_ORDER == __LITTLE_ENDIAN	
		key1.dt = be32toh(*((time_t*) dbval.mv_data));
#else
		key1.dt = *((time_t*) dbval.mv_data);
#endif	
		if (finish > start) 
		{
			if ((key1.dt > finish) || (key1.dt < start))
				continue;
		}
		else
		{
			if ((key1.dt < finish) || (key1.dt > start))
				continue;
		}
		if (onLog(onLogEnv, &key1, NULL))
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
