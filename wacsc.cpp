/**
 * Client (test,..)
 * @file wacsc.cpp
 */
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <signal.h>
#include <argtable3/argtable3.h>

#include "platform.h"
#include "utilstring.h"
#include "wacsc.h"
#include "send-log-entry.h"
#include "dblog.h"
#include "errorcodes.h"

static WacscConfig *config = NULL;

int reslt;

bool interruptFlag = false;

#ifdef _MSC_VER
void setSignalHandler(int signal)
{
}
#else
void signalHandler(int signal)
{
	switch(signal)
	{
	case SIGINT:
		if (interruptFlag)
		{
			if (config)
				delete config;
			exit(ERR_OK);
		}
		else
		{
			std::cerr << MSG_INTERRUPTED;
			interruptFlag = true;
		}
		break;
	case SIGHUP:
		break;
	default:
		std::cerr << MSG_SIGNAL << signal;
	}
}

void setSignalHandler(int signal)
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = &signalHandler;
	sigaction(signal, &action, NULL);
}
#endif

#ifdef _MSC_VER
void initWindows()
{
	// Initialize Winsock
	WSADATA wsaData;
	int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (r)
	{
		std::cerr << ERR_WSA_ERROR << r << std::endl;
		exit(ERRCODE_WSA_ERROR);
	}
}
#endif

static int sendTest
(
	WacscConfig *config
)
{
	LogEntry value;
	value.device_id = config->device_id;
	value.ssi_signal = config->ssi_signal;
	strtomacaddress(&value.sa, config->mac);
	return sendLogEntry(config->message_url, &value, config->repeats, config->verbosity);
}

static bool onLog
(
	void *env,
	LogKey *key,
	LogData *data
)
{
	std::cout
		<< mactostr(key->sa)
		<< "\t" << time_t2string(key->dt);
	if (data)
		std::cout
			<< "\t" << data->device_id
			<< "\t" << data->ssi_signal;
	std::cout << std::endl;
	return interruptFlag;
}

static int lsLog
(
	WacscConfig *config,
	void *onLogEnv
)
{
	struct dbenv env;
	int r = 0;
	if (!openDb(&env, config->path.c_str(), config->flags, config->mode))
	{
		std::cerr << ERR_LMDB_OPEN << config->path << std::endl;
		return ERRCODE_LMDB_OPEN;
	}

	uint8_t sa[6];
	strtomacaddress(&sa, config->mac);
	r = readLog(&env, config->mac.empty() ? NULL : sa, config->start, config->finish, onLog, onLogEnv);

	if (!closeDb(&env))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		r = ERRCODE_LMDB_CLOSE;
	}

	return r;
}

static int lsLastProbe
(
	WacscConfig *config,
	void *onLogEnv
)
{
	struct dbenv env;
	int r = 0;
	if (!openDb(&env, config->path.c_str(), config->flags, config->mode))
	{
		std::cerr << ERR_LMDB_OPEN << config->path << std::endl;
		return ERRCODE_LMDB_OPEN;
	}

	uint8_t sa[6];
	strtomacaddress(&sa, config->mac);
	r = readLastProbe(&env, config->mac.empty() ? NULL : sa, onLog, onLogEnv);

	if (!closeDb(&env))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		r = ERRCODE_LMDB_CLOSE;
	}

	return r;
}

int main(int argc, char** argv)
{
	// Signal handler
	setSignalHandler(SIGINT);
#ifdef _MSC_VER
	initWindows();
#endif
	reslt = 0;

	config = new WacscConfig(argc, argv);
	if (!config)
		exit(ERRCODE_NO_CONFIG);

	if (config->error() != 0)
	{
		std::cerr << ERR_NO_CONFIG;
		delete config;
		exit(config->error());
	}

	switch(config->cmd)
	{
	case CMD_SEND_TEST:
		reslt = sendTest(config);
		break;
	case CMD_LS_LOG:
		reslt = lsLog(config, NULL);
		break;
	case CMD_LS_LAST_PROBE:
		reslt = lsLastProbe(config, NULL);
		break;
	default:
		break;
	}

	delete config;
	return reslt;
}
