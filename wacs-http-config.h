/**
  * wacs httpd options
  * @file wacs-http-config.h
  **/

#ifndef WACS_HTTP_CONFIG_H
#define WACS_HTTP_CONFIG_H	1

#include <string>
#include <vector>

/**
 * wacs httpd configuration structure
 */
class WacsHttpConfig
{
private:
	/**
	* Parse command line into WacsHttpConfig class
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
	int stop_request;
	std::string root_path;
	int port;										///< CMD_*
	int verbosity;									///< 0-quiet, 3- debug
	std::string path;								///< database files lock.mdb, data.mdb directory path
	int flags;
	int mode;
	bool daemonize;
	int max_fd;										///< 0- use default max file descriptor count per process

	WacsHttpConfig();
	WacsHttpConfig
	(
		int argc,
		char* argv[]
	);
	~WacsHttpConfig();
	int error();
};

#endif
