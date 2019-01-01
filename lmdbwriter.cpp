/**
 *	Write to the LMDB database
 */
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

#include "errorcodes.h"
#include "lmdbwriter.h"

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

#define BUF_SIZE	1024

#define LOG(verbosity) \
	std::cerr

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

static size_t getKey
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


/**
 * @brief Write LMDB loop
 * @param config
 * @return  0- success
 *          >0- error (see errorcodes.h)
 */
int run
(
	WacsConfig *config
)
{
START:
	config->stop_request = 0;
	int accept_socket = nn_socket(AF_SP, NN_SUB);
	int r = nn_setsockopt(accept_socket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
	if (r < 0)
	{
		LOG(ERROR) << ERR_NN_SUBSCRIBE << config->message_url << " " << errno << " " << strerror(errno);
		return ERRCODE_NN_SUBSCRIBE;
	}

	int eid = nn_connect(accept_socket, config->message_url.c_str());

	if (eid < 0)
	{
		LOG(ERROR) << ERR_NN_CONNECT << config->message_url;
		return ERRCODE_NN_CONNECT;
	}

	struct dbenv env;

	if (!open_lmdb(&env, config))
	{
		LOG(ERROR) << ERR_LMDB_OPEN << config->path;
		return ERRCODE_LMDB_OPEN;
	}


	void *buffer = malloc(BUF_SIZE);
	if (!buffer)
	{
		LOG(ERROR) << ERR_NO_MEMORY;
		return ERRCODE_NO_MEMORY;
	}

    while (!config->stop_request)
    {
    	int bytes;
   		bytes = nn_recv(accept_socket, &buffer, NN_MSG, 0);
    	if (bytes < 0)
    	{
			if (errno == EINTR) 
			{
				LOG(ERROR) << ERR_INTERRUPTED;
				config->stop_request = true;
				break;
			}
			else
				LOG(ERROR) << ERR_NN_RECV << errno << " " << strerror(errno);
    		continue;
    	}
		// put_db(&env, &options, buffer, bytes, &messageTypeNAddress, m);
		nn_freemsg(buffer);
    }

	r = 0;

	if (!close_lmdb(&env))
	{
		LOG(ERROR) << ERR_LMDB_CLOSE << config->path;
		r = ERRCODE_LMDB_CLOSE;
	}

	r = nn_shutdown(accept_socket, eid);
	if (r)
	{
		LOG(ERROR) << ERR_NN_SHUTDOWN << config->message_url;
		r = ERRCODE_NN_SHUTDOWN;

	}

	close(accept_socket);
	accept_socket = 0;

	return r;
}

/**
 *  @brief Stop writer
 *  @param config
 *  @return 0- success
 *          >0- config is not initialized yet
 */
int stop
(
		WacsConfig *config
)
{
    if (!config)
        return ERRCODE_NO_CONFIG;
    config->stop_request = 1;
    return ERR_OK;
}

int reload(WacsConfig *config)
{
	if (!config)
		return ERRCODE_NO_CONFIG;
	LOG(ERROR) << MSG_RELOAD_BEGIN;
	config->stop_request = 2;
	return ERR_OK;
}
