/**
  * wpn options
  * @file wpn-config.h
  **/

#ifndef WPN_CONFIG_H
#define WPN_CONFIG_H	1

#include <string>
#include <vector>

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
	int cmd;										///< CMD_*
	int verbosity;									///< 0-quiet, 3- debug
	std::string file_name;							///< config file, e.g. https://sure-phone.firebaseio.com"

	WacsConfig();
	WacsConfig
	(
		int argc,
		char* argv[]
	);
	~WacsConfig();
	int error();
};

#endif
