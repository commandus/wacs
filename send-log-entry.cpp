#include <iostream>
#include <nanomsg/nn.h>
#include <nanomsg/bus.h>
#include "NanoMessage.h"
#include "errorcodes.h"
#include "send-log-entry.h"
#include <stdio.h>
#include <unistd.h>
#include <error.h>

#include "log.h"

int sendLogEntry
(
	const std::string &message_url,
	const LogEntry* value
)
{
	int write_socket = nn_socket(AF_SP, NN_BUS);
	int eid = nn_connect(write_socket, message_url.c_str());
	if (eid < 0)
	{
		LOG(ERROR) << ERR_NN_CONNECT << message_url << " " << errno << " " << nn_strerror(errno);
		return ERRCODE_NN_CONNECT;
	}

	if (eid < 0)
	{
		LOG(ERROR) << ERR_NN_BIND << message_url;
		return ERRCODE_NN_BIND;
	}

    int r = nn_shutdown(write_socket, eid);
    if (r)
    	LOG(ERROR) << ERR_NN_SHUTDOWN << " out " << errno << ": " << nn_strerror(errno);
	if (write_socket)
	{
		close(write_socket);
		write_socket = 0;
	}
	return 0;
}
