/**
  * wacs PostgreSQL options
  * @file wacs-pg-config.h
  **/

#ifndef WACS_CONFIG_H
#define WACS_CONFIG_H	1

#include <string>
#include <vector>

#include <libpq-fe.h>

#include "snmp-params.h"

/**
 * Command line interface (CLI) tool configuration structure
 */
class WacsPgConfig
{
private:
	/**
	* Parse command line into WacsPgConfig class
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
	std::string path;

	// PostgreSQL
	std::string dbconn;
	std::string dboptionsfile;
	std::string dbname;
	std::string dbuser;
	std::string dbpassword;
	std::string dbhost;
	std::string dbport;
	std::string dbsocket;
	std::string dbcharset;
	int dbclientflags;

	bool snmp_agent;
	bool daemonize;
	int max_fd;										///< 0- use default max file descriptor count per process

	WacsPgConfig();
	WacsPgConfig
	(
		int argc,
		char* argv[]
	);
	~WacsPgConfig();
	int error();
	MonitoringParams *counter;
};

/**
 * Establish configured database connection
 */
PGconn *dbconnect(WacsPgConfig *config);

#endif
