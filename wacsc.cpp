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
#include "wacsc.h"
#include "send-log-entry.h"
#include "errorcodes.h"

static WacscConfig *config = NULL;

int reslt;

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
		std::cerr << MSG_INTERRUPTED;
		if (config)
			delete config;
		exit(ERR_OK);
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
	value.device_id = 1;
	value.ssi_signal = 0xDEAD;
	strtomacaddress(&value.sa, "de:ad:be:ef:be:ef");
	return sendLogEntry(config->message_url, &value);
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
	case 1:
		reslt = sendTest(config);
		break;
	default:
		break;
	}

	delete config;
	return reslt;
}
