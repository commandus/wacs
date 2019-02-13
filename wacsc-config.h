/**
  * wacsc options
  * @file wacsc-config.h
  **/

#ifndef WACSC_CONFIG_H
#define WACSC_CONFIG_H	1

#include <string>
#include <vector>
#include <inttypes.h>

#define	CMD_NONE				-1
#define	CMD_SEND_TEST			1
#define	CMD_LS_LOG				2
#define	CMD_LS_LAST_PROBE		3
#define	CMD_COUNT_LOG			4
#define	CMD_COUNT_LAST_PROBE	5
#define	CMD_MACS_PER_TIME		6
#define	CMD_REMOVE				7
#define	CMD_LOG_READ			8


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
	int repeats;
	std::vector<std::string> mac;					///< MAC addresses
	time_t start;
	time_t finish;
	uint16_t device_id;
	int16_t ssi_signal;

	int verbosity;									///< 0-quiet, 3- debug
	std::string path;								///< database files lock.mdb, data.mdb directory path
	int flags;
	int mode;

	size_t offset;									///< -o
	int count;										///< -c. -1 - unlimited

	int step_seconds;								///< --step
	
	std::string logFileName;						///< file name to read log. Default stdin

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
