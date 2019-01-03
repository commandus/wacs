/**
  * wacsc options
  * @file wacsc-config.h
  **/

#ifndef WACSC_CONFIG_H
#define WACSC_CONFIG_H	1

#include <string>
#include <vector>

#define	CMD_NONE			-1
#define	CMD_SEND_TEST		1
#define	CMD_LS_LOG			2
#define	CMD_LS_LAST_PROBE	3

/**
 * Command line interface (CLI) tool configuration structure
 */
class WacscConfig
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
	std::string message_url;
	int cmd;										///< CMD_*
	int verbosity;									///< 0-quiet, 3- debug
	std::string path;								///< database files lock.mdb, data.mdb directory path
	int flags;
	int mode;

	WacscConfig();
	WacscConfig
	(
		int argc,
		char* argv[]
	);
	~WacscConfig();
	int error();
};

#endif
