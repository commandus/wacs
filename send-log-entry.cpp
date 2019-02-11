#include <iostream>
#include <nanomsg/nn.h>
#include <nanomsg/bus.h>
#include "NanoMessage.h"
#include "errorcodes.h"
#include "send-log-entry.h"
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <netinet/in.h>

#include "log.h"
#include <string.h>

int openSocketLog
(
	int *socket,
	int *endpoint,
	const std::string &message_url
)
{
	if (!(socket && endpoint))
		return ERRCODE_WRONG_PARAM;
	*socket = nn_socket(AF_SP, NN_BUS);
	if (*socket < 0)
	{
		return ERRCODE_NN_CONNECT;
	}
	int linger = -1;
	nn_setsockopt(*socket, NN_SOL_SOCKET, NN_LINGER, &linger, sizeof (linger));
	*endpoint = nn_connect(*socket, message_url.c_str());
	if (*endpoint < 0)
	{
		LOG(ERROR) << ERR_NN_CONNECT << message_url << " " << errno << " " << nn_strerror(errno) << std::endl;
		return ERRCODE_NN_CONNECT;
	}

	// wait connection established
	struct nn_pollfd pollfd;
	pollfd.fd = *socket;
	pollfd.events = NN_POLLIN;
	nn_poll(&pollfd, 1, 100);

	if (*endpoint < 0)
	{
		LOG(ERROR) << ERR_NN_CONNECT << message_url;
		return ERRCODE_NN_CONNECT;
	}
// std::cerr << "openSocketLog socket: " << *socket << ", endpoint: " << *endpoint << std::endl;	
	return 0;
}

int closeSocketLog
(
	int socket,
	int endpoint
)
{
    int r = nn_shutdown(socket, endpoint);
// std::cerr << "closeSocketLog socket: " << socket << ", endpoint: " << endpoint << std::endl;	
    if (r)
    	LOG(ERROR) << ERR_NN_SHUTDOWN << " out " << errno << ": " << nn_strerror(errno) << std::endl;
	nn_close(socket);
	return r;
}

/**
 * @return 0- success
 */
int sendLogEntry
(
	const std::string &message_url,
	const LogEntry* value,
	int repeats,
	int verbosity
)
{
	LogEntry v;
	v.device_id = htons(value->device_id);
	v.ssi_signal = htons(value->ssi_signal);
	memmove(v.sa, value->sa, 6);
	int write_socket, eid;
	int r = openSocketLog(&write_socket, &eid, message_url);
	if (r)
		return r;
	r = sendLogEntrySocket(write_socket, value, repeats, verbosity);
	return closeSocketLog(write_socket, eid);
}

/**
 * @return 0- success
 */
int sendLogEntrySocket
(
	int socket,
	const LogEntry* value,
	int repeats,
	int verbosity
)
{
	LogEntry v;
	v.device_id = htons(value->device_id);
	v.ssi_signal = htons(value->ssi_signal);
	memmove(v.sa, value->sa, 6);
	for (int i = 0; i < repeats; i++)
	{
		// v.ssi_signal = htons(i);
		int bytes = nn_send(socket, &v, sizeof(LogEntry), 0);
		if (bytes < 0)
		{
			LOG(ERROR) << ERR_NN_SEND << " out " << errno << ": " << nn_strerror(errno) << std::endl;
		}
		else
		{
			if (verbosity > 2)
				LOG(INFO) << MSG_NN_SENT_SUCCESS << socket << " " << bytes << " bytes" << std::endl;
		}
	}
	struct nn_pollfd pollfd;
	pollfd.fd = socket;
	pollfd.events = NN_POLLIN;
	nn_poll(&pollfd, 1, 100); // sleep(1); // wait for flushing

	return 0;
}
