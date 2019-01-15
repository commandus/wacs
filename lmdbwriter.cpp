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

#include "dblog.h"
#include "log.h"

#define USE_SNMP 1
#ifdef USE_SNMP
#include "utilsnmp.h"
#include "snmp-params.h"
#include "snmpagent-wacs.h"
#include "snmp-params.h"
#endif

#define BUF_SIZE	128

#define snmp_name	"wacs"

static void snmpInitialize
(
	WacsConfig *config
)
{
#ifdef USE_SNMP
	#define DEF_FILE_NAME "/mdbx.dat"

	if (config)
	{
		config->counter = getInstance();
		config->counter->clear();
		config->counter->databasefilename = config->path + DEF_FILE_NAME;
		if (config->snmp_agent)
			snmpInit(snmp_name, config->snmp_agent, config->verbosity, &config->stop_request, init_mibs);
	}
#endif	
}

static void snmpDone
(
	WacsConfig *config
)
{
#ifdef USE_SNMP
	if (config->snmp_agent > 0)
		snmpDone(snmp_name);
#endif	
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
	snmpInitialize(config);
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

	int recv_timeout = 1000;	// 1s
	if (nn_setsockopt(accept_socket, NN_SOL_SOCKET, NN_RCVTIMEO, &recv_timeout, sizeof (recv_timeout)) < 0) 
	{
		LOG(ERROR) << ERR_NN_SET_SOCKET_OPTION << config->message_url << " timeout " << recv_timeout << std::endl;
		return ERRCODE_NN_BIND;
	}
	// sleep (1); // wait for connections

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
			switch (errno) {
				case EINTR:
					LOG(ERROR) << ERR_INTERRUPTED << std::endl;
					config->stop_request = true;
					break;
				case ETIMEDOUT:
					break;
				default:
					LOG(ERROR) << ERR_NN_RECV << errno << " " << strerror(errno) << std::endl;
					break;
			}
		} 
		else
		{
			if (config->verbosity > 2)
				LOG(INFO) << MSG_RECEIVED << bytes << std::endl;
			putLog(&env, &e, bytes, config->verbosity);
#ifdef USE_SNMP
			if ((config->counter != NULL) && config->snmp_agent)
				config->counter->put(&e);
#endif			
		}
    }

	int r = 0;

	if (!closeDb(&env))
	{
		LOG(ERROR) << ERR_LMDB_CLOSE << config->path << std::endl;
		r = ERRCODE_LMDB_CLOSE;
	}

	r = nn_shutdown(accept_socket, eid);
	if (r)
	{
		LOG(ERROR) << ERR_NN_SHUTDOWN << config->message_url << std::endl;
		r = ERRCODE_NN_SHUTDOWN;
	}

	nn_close(accept_socket);
	accept_socket = 0;
	snmpDone(config);
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
