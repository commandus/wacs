/**
 * @file wacs-http.cpp
 */
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <signal.h>
#include <argtable3/argtable3.h>

#include "platform.h"
#include "wacs-http-config.h"
#include "errorcodes.h"
#include "daemonize.h"
#include "http-server.h"

#define DEAMON_NAME	"wacshttpd"

static WacsHttpConfig *config = NULL;

void stopNWait()
{
	if (config)
		stop(config);
}

void done()
{
	exit(0);
}

int reslt;

void runner()
{
	if (!config)
	{
		std::cerr << ERR_NO_CONFIG;
		return;
	}
	reslt = run(config);
}

#ifdef _MSC_VER
static void setSignalHandler(int signal)
{
}
#else
static void signalHandler(int signal)
{
	switch(signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stopNWait();
		done();
		break;
	case SIGHUP:
		std::cerr << MSG_RELOAD_CONFIG_REQUEST << std::endl;
		reload(config);
		break;
	default:
			std::cerr << MSG_SIGNAL << signal << std::endl;
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

void onLog
(
	void *env,
	int severity,
	const char *message
)
{
	std::cerr << message;
}

int main(int argc, char** argv)
{
	// Signal handler
	setSignalHandler(SIGINT);
#ifdef _MSC_VER
	initWindows();
#endif
    reslt = 0;

	config = new WacsHttpConfig(argc, argv);
	if (!config)
		exit(ERRCODE_NO_CONFIG);

    if (config->error() != 0)
	{
		std::cerr << ERR_NO_CONFIG;
		exit(config->error());
	}

    if (config->daemonize)
	{
		std::cerr << MSG_DAEMONIZE;
		Daemonize daemonize(DEAMON_NAME, config->path, runner, stopNWait, done, config->max_fd);
	}
	else
	{
		if (config->max_fd > 0)
			Daemonize::setFdLimit(config->max_fd);
		runner();
		done();
	}
	return reslt;
}
