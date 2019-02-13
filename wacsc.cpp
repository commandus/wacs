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
#include <fstream>
#include <sstream>

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
	int r;
	for (std::vector<std::string>::const_iterator it (config->mac.begin()); it != config->mac.end(); ++it)
	{
		int macSize = strtomacaddress(&value.sa, *it);
		if (macSize == 6)
			r = sendLogEntry(config->message_url, &value, config->repeats, config->verbosity);
		else
			return ERRCODE_WRONG_PARAM;
	}
	return r;
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
	if ((e->count != -1) && (e->position >= e->offset + e->count))
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
	int rr = 0;
	if (!openDb(&env, config->path.c_str(), config->flags, config->mode))
	{
		std::cerr << ERR_LMDB_OPEN << config->path << std::endl;
		return ERRCODE_LMDB_OPEN;
	}
	for (std::vector<std::string>::const_iterator it (config->mac.begin()); it != config->mac.end(); ++it)
	{
		uint8_t sa[6];
		int macSize = strtomacaddress(&sa, *it);
		int r = readLog(&env, config->mac.empty() ? NULL : sa, macSize, config->start, config->finish, onLog, onLogEnv);
		if (r < 0) {
			rr =r;
			break;
		}
		else
			rr += r;
	}
	if (!closeDb(&env))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		return ERRCODE_LMDB_CLOSE;
	}
	return rr;
}

static int rmLog
(
	WacscConfig *config
)
{
	struct dbenv env;
	int rr = 0;
	if (!openDb(&env, config->path.c_str(), config->flags, config->mode))
	{
		std::cerr << ERR_LMDB_OPEN << config->path << std::endl;
		return ERRCODE_LMDB_OPEN;
	}
	for (std::vector<std::string>::const_iterator it (config->mac.begin()); it != config->mac.end(); ++it)
	{
		uint8_t sa[6];
		int macSize = strtomacaddress(&sa, *it);
		int r = rmLog(&env, config->mac.empty() ? NULL : sa, macSize, config->start, config->finish);
		if (r < 0) {
			rr =r;
			break;
		}
		else
			rr += r;
	}
	if (!closeDb(&env))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		return ERRCODE_LMDB_CLOSE;
	}
	return rr;
}

/**
 * line:
 * MAC date&time deviceId ssi_signal
 * 00:ec:0a:ed:5c:55	2019-01-31T15:48:49+09	1	-76
 */
static void parseLogLine
(
	LogRecord *retval,
	const std::string &line
)
{
	std::stringstream ss(line);
	std::string mac;
	std::string dt;
	ss 
		>> mac
		>> dt
		>> retval->device_id
		>> retval->ssi_signal;
	strtomacaddress(retval->sa, mac);
	retval->dt = parseDate(dt.c_str());
}

static bool onPutLogEntry
(
	void *env,
	LogRecord *retval
)
{
	if (!env)
		return true;
	std::ifstream *strm = (std::ifstream *) env;
	if (strm->bad())
		return true;
	std::string line;
	if (getline(*strm, line))
	{
		parseLogLine(retval, line);
#ifdef DEBUG
		std::cerr 
			<< mactostr(retval->sa) << "\t"
			<< time_t2string(retval->dt) << "\t"
			<< retval->device_id << "\t"
			<< retval->ssi_signal << std::endl;
#endif			
		return false;
	}
	return true;
}

static int loadLog
(
	WacscConfig *config
)
{
	struct dbenv env;
	if (!openDb(&env, config->path.c_str(), config->flags, config->mode))
	{
		std::cerr << ERR_LMDB_OPEN << config->path << std::endl;
		return ERRCODE_LMDB_OPEN;
	}
	std::istream *strm;
	if (!config->logFileName.empty())
		strm = new std::ifstream(config->logFileName.c_str());
	else
		strm = &std::cin;
	int r = putLogEntries(&env, config->verbosity, onPutLogEntry, strm);
	if (!config->logFileName.empty())
		delete strm;

	if (!closeDb(&env))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		return ERRCODE_LMDB_CLOSE;
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
	for (std::vector<std::string>::const_iterator it (config->mac.begin()); it != config->mac.end(); ++it)
	{
		uint8_t sa[6];
		int macSize = strtomacaddress(&sa, *it);
		r = readLastProbe(&env, config->mac.empty() ? NULL : sa, macSize, 
			config->start, config->finish, onLog, onLogEnv);
	}
	if (!closeDb(&env))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		return ERRCODE_LMDB_CLOSE;
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
	for (std::vector<std::string>::const_iterator it (config->mac.begin()); it != config->mac.end(); ++it)
	{
		uint8_t sa[6];
		int macSize = strtomacaddress(&sa, *it);
		r = readLog(&env, config->mac.empty() ? NULL : sa, macSize, config->start, config->finish, onLogHistogram, retval);
	}
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
	case CMD_REMOVE:
		{
			int c = rmLog(config);
			if (c < 0)
				std::cerr << "Error " << c << std::endl;
			else
				std::cout << c << " records removed." << std::endl;

		}
		break;
	case CMD_LOG_READ:
		{
			int c = loadLog(config);
			if (c < 0)
				std::cerr << "Error " << c << std::endl;
			else
				std::cout << c << " records added." << std::endl;
		}
		break;
	default:
		break;
	}

	delete config;
	return reslt;
}
