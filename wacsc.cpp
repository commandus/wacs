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
#include "dblog.h"
#include "errorcodes.h"
#include "send-log-entry.h"
#include "histogrammacspertime.h"

static WacscConfig *config = NULL;

int reslt;

struct ReqEnv
{
	bool interruptFlag;
	size_t position;
	size_t offset;
	size_t count;
	size_t sum;
} ReqEnv;

static struct ReqEnv reqEnv;

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
		if (reqEnv.interruptFlag)
		{
			if (config) 
			{
				delete config;
			}
			exit(ERR_OK);
		}
		else
		{
			std::cerr << MSG_INTERRUPTED;
			reqEnv.interruptFlag = true;
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
	int macSize = strtomacaddress(&value.sa, config->mac);
	if (macSize == 6)
		return sendLogEntry(config->message_url, &value, config->repeats, config->verbosity);
	else
		return ERRCODE_WRONG_PARAM;
}

static bool onLog
(
	void *env,
	LogKey *key,
	LogData *data
)
{
	if (!env)
		return true;
	struct ReqEnv *e = (struct ReqEnv *) env; 
	// paginate
	if (e->position < e->offset)
	{
		e->position++;
		return false; // skip
	}
	if (e->position >= e->offset + e->count)
		return true; // all done
	e->position++;

	std::cout
		<< mactostr(key->sa)
		<< "\t" << time_t2string(key->dt);
	if (data)
		std::cout
			<< "\t" << data->device_id
			<< "\t" << data->ssi_signal;
	std::cout << std::endl;
	return e->interruptFlag;	// interrupt sigmal received
}

static bool onLogCount
(
	void *env,
	LogKey *key,
	LogData *data
)
{
	if (!env)
		return true;
	struct ReqEnv *e = (struct ReqEnv *) env; 
	e->sum++;
	return e->interruptFlag;	// interrupt sigmal received
}

static bool onLogHistogram
(
	void *env,
	LogKey *key,
	LogData *data
)
{
	if (!env)
		return true;
	HistogramMacsPerTime *h = (HistogramMacsPerTime *) env; 
	h->put(key->dt, key->sa);
	return reqEnv.interruptFlag;
}

static int lsLog
(
	WacscConfig *config,
	OnLog onLog,
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
	int macSize = strtomacaddress(&sa, config->mac);
	r = readLog(&env, config->mac.empty() ? NULL : sa, macSize, config->start, config->finish, onLog, onLogEnv);

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
	OnLog onLog,
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
	int macSize = strtomacaddress(&sa, config->mac);
	r = readLastProbe(&env, config->mac.empty() ? NULL : sa, macSize, 
		config->start, config->finish, onLog, onLogEnv);

	if (!closeDb(&env))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		r = ERRCODE_LMDB_CLOSE;
	}

	return r;
}

static int macsPerTime
(
	HistogramMacsPerTime *retval,
	WacscConfig *config
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
	int macSize = strtomacaddress(&sa, config->mac);
	r = readLog(&env, config->mac.empty() ? NULL : sa, macSize, config->start, config->finish, onLogHistogram, retval);

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

	reqEnv.interruptFlag = false;
	reqEnv.position = 0;
	reqEnv.offset = config->offset;
	reqEnv.count = config->count;
	reqEnv.sum = 0;

	if (config->verbosity > 2)
		std::cerr << "start: " << config->start
		<< ", finish: " << config->finish
		<< ", step: " << config->step_seconds
		<< std::endl;

	switch(config->cmd)
	{
	case CMD_SEND_TEST:
		reslt = sendTest(config);
		break;
	case CMD_LS_LOG:
		reslt = lsLog(config, onLog, &reqEnv);
		break;
	case CMD_LS_LAST_PROBE:
		reslt = lsLastProbe(config, onLog, &reqEnv);
		break;
	case CMD_COUNT_LOG:
		reslt = lsLog(config, onLogCount, &reqEnv);
		std::cout << reqEnv.sum << std::endl;
		break;
	case CMD_COUNT_LAST_PROBE:
		reslt = lsLastProbe(config, onLogCount, &reqEnv);
		std::cout << reqEnv.sum << std::endl;
		break;
	case CMD_MACS_PER_TIME:
		{
			HistogramMacsPerTime reslt(config->start, config->finish, config->step_seconds);
			macsPerTime(&reslt, config);
			std::cout << reslt.toString() << std::endl;
		}
		break;
	default:
		break;
	}

	delete config;
	return reslt;
}
