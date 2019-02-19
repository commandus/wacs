/**
  * wacs options
  * @file wacs-config.h
  **/

#ifndef WACS_CONFIG_H
#define WACS_CONFIG_H	1

#include <string>
#include <vector>

#include "snmp-params.h"

/**
 * Command line interface (CLI) tool configuration structure
 */
class WacsConfig
{
private:
	/**
	* Parse command line into WacsConfig class
	* Return 0- success
	*        1- show help and exit, or command syntax error
	*        2- output file does not exists or can not open to write
	**/
	int parseCmd
	(
		int argc,
		char* argv[]
	);
	int errorcode;
public:
	bool stop_request;
	std::string message_url;
	int cmd;										///< CMD_*
	int verbosity;									///< 0-quiet, 3- debug
	std::string path;								///< database files lock.mdb, data.mdb directory path
	int flags;
	int mode;
	int queue;										///< MQ

	bool snmp_agent;
	bool daemonize;
	int max_fd;										///< 0- use default max file descriptor count per process

	WacsConfig();
	WacsConfig
	(
		int argc,
		char* argv[]
	);
	~WacsConfig();
	int error();
	MonitoringParams *counter;
};

#endif
