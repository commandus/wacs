#include "dblog.h"
#include "iostream"
#include "errorcodes.h"

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

size_t getKey
(
	int mode, 
	void *keybuffer,
	size_t keysize,
	void *buffer,
	size_t size
)
{
	switch (mode) {
		case 0:
			return 10;
		case 1:
			return 4;
	}
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
	int buffer_size
)
{
	MDB_val key, data;
	char keybuffer[2048];

	key.mv_size = getKey(0, keybuffer, sizeof(keybuffer), buffer, buffer_size);
	if (key.mv_size == 0)
		return 0;	// No key, no store
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r)
	{
		LOG(ERROR) << ERR_LMDB_TXN_BEGIN << r;
		return ERRCODE_LMDB_TXN_BEGIN;
	}

	key.mv_data = keybuffer;
	data.mv_size = buffer_size;
	data.mv_data = buffer;

	r = mdb_put(env->txn, env->dbi, &key, &data, 0);
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

