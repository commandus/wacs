/**
 * @file wacs.cpp
 */
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <signal.h>
#include <argtable3/argtable3.h>

#include "platform.h"
#include "wacs.h"

#define ERR_WSA		-1

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
		std::cerr << MSG_INTERRUPTED << std::endl;
		break;
	default:
		break;
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
		std::cerr << "WSAStartup error %d" << r << std::endl;
		exit(ERR_WSA);
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
	// In windows, this will init the winsock stuff

	WacsConfig config(argc, argv);
	if (config.error())
		exit(config.error());

	switch (config.cmd)
	{
		default:
			break;
	}
	return 0;
}
