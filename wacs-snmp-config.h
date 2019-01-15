/**
  * wacs-snmp options
  * @file wacs-snmp-config.h
  **/

#ifndef WACS_SNMP_CONFIG_H
#define WACS_SNMP_CONFIG_H	1

#include <string>

/**
 * SNMP command line interface (CLI) tool configuration structure
 */
class WacsSNMPConfig
{
private:
	int errorcode;
	/**
	* Parse command line into WacsSNMPConfig class
	* Return 0- success
	*        1- show help and exit, or command syntax error
	*        2- output file does not exists or can not open to write
	**/
	int parseCmd
	(
		int argc,
		char* argv[]
	);
public:
	std::string peer;
	std::string community;
	int verbosity;									///< 0-quiet, 3- debug

	WacsSNMPConfig();
	WacsSNMPConfig
	(
		int argc,
		char* argv[]
	);
	~WacsSNMPConfig();
	int error();
};

#endif
