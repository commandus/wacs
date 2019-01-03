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
#include <nanomsg/bus.h>

#include "errorcodes.h"
#include "lmdbwriter.h"
#include "send-log-entry.h"

#include "dblog.h"
#include "log.h"

#define BUF_SIZE	128

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
	int accept_socket = nn_socket(AF_SP, NN_BUS);
	int eid = nn_bind(accept_socket, config->message_url.c_str());
	if (eid < 0)
	{
		LOG(ERROR) << ERR_NN_BIND << config->message_url << std::endl;
		return ERRCODE_NN_BIND;
	}
   	if (config->verbosity > 2)
		LOG(INFO) << MSG_BIND_SUCCESSFULLY << config->message_url << std::endl;

	struct dbenv env;

	if (!openDb(&env, config->path.c_str(), config->flags, config->mode))
	{
		LOG(ERROR) << ERR_LMDB_OPEN << config->path << std::endl;
		return ERRCODE_LMDB_OPEN;
	}

    while (!config->stop_request)
    {
		LogEntry e;
    	int bytes = nn_recv(accept_socket, &e, sizeof(LogEntry), 0);
    	if (bytes < 0)
    	{
			if (errno == EINTR) 
			{
				LOG(ERROR) << ERR_INTERRUPTED << std::endl;
				config->stop_request = true;
				break;
			}
			else
				LOG(ERROR) << ERR_NN_RECV << errno << " " << strerror(errno) << std::endl;
    		continue;
    	}
    	if (config->verbosity > 2)
			LOG(INFO) << MSG_RECEIVED << bytes << std::endl;
		putLog(&env, &e, bytes, config->verbosity);
    }
	
	int r = 0;

	if (!closeDb(&env))
	{
		LOG(ERROR) << ERR_LMDB_CLOSE << config->path << std::endl;
		r = ERRCODE_LMDB_CLOSE;
	}

	/*
	r = nn_shutdown(accept_socket, eid);
	if (r)
	{
		LOG(ERROR) << ERR_NN_SHUTDOWN << config->message_url << std::endl;
		r = ERRCODE_NN_SHUTDOWN;
	}
	*/

	nn_close(accept_socket);
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
	LOG(ERROR) << MSG_RELOAD_BEGIN << std::endl;
	config->stop_request = 2;
	return ERR_OK;
}
