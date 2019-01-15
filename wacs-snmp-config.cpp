#include "wacs-snmp-config.h"
#include <iostream>
#include <argtable3/argtable3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#ifdef _MSC_VER
#include "Userenv.h"
#else
#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>
#endif

#include "platform.h"
#include "utilstring.h"
#define progname		"wacs-snmp"

#define DEF_PEER		"127.0.0.1"
#define DEF_COMMUNITY	"private"

WacsSNMPConfig::WacsSNMPConfig()
	: errorcode(0), peer(DEF_PEER), community(DEF_COMMUNITY), verbosity(0)
{
}
	
WacsSNMPConfig::WacsSNMPConfig
(
	int argc,
	char* argv[]
)
{
	errorcode = parseCmd(argc, argv);
}

WacsSNMPConfig::~WacsSNMPConfig()
{
}


/**
 * Parse command line into WacsSNMPConfig class
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int WacsSNMPConfig::parseCmd
(
	int argc,
	char* argv[]
)
{
	struct arg_str *a_peer = arg_str0(NULL, NULL, "<peer>", "IP address e.g. 127.0.0.1. Default " DEF_PEER);
	struct arg_str *a_community = arg_str0("c", "community", "<name>", "Default " DEF_COMMUNITY);
	// other
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 4, "0- quiet (default), 1- errors, 2- warnings, 3- debug, 4- debug libs");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_peer, a_community,
		a_verbosity, a_help, a_end 
	};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

	verbosity = a_verbosity->count;

	if (a_peer->count)
	{
		peer = std::string(*a_peer->sval);
	}
	if (peer.empty()) 
	{
		peer = DEF_PEER;
	}

	if (a_community->count)
	{
		community = std::string(*a_community->sval);
	}
	if (community.empty()) 
	{
		community = DEF_COMMUNITY;
	}

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "wacs-snmp monitoring tool" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

int WacsSNMPConfig::error()
{
	return errorcode;
}
