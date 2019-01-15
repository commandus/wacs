#include "wacs-config.h"
#include <iostream>
#include <argtable3/argtable3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#include "platform.h"
#include "utilstring.h"

#define progname "wacs"

#define DEF_DB_PATH					"."
#define DEF_MODE					0664
#define DEF_FLAGS					0

WacsConfig::WacsConfig()
	: errorcode(0), cmd(0), verbosity(0), counter(NULL),
	path(getDefaultDatabasePath())
{
}
	
WacsConfig::WacsConfig
(
	int argc,
	char* argv[]
)
	: counter(NULL)
{
	
	errorcode = parseCmd(argc, argv);
}

WacsConfig::~WacsConfig()
{
}

/**
 * Parse command line into WacsConfig class
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int WacsConfig::parseCmd
(
	int argc,
	char* argv[]
)
{
	struct arg_str *a_message_url = arg_str0("i", "input", "<queue url>", "e.g. tcp://127.0.0.1:55555, ws://127.0.0.1:2019, ipc://tmp/wacs.nn. Default " DEF_QUEUE);
	struct arg_str *a_db_path = arg_str0(NULL, "dbpath", "<path>", "Database path");
	struct arg_int *a_flags = arg_int0("f", "flags", "<number>", "LMDB flags. Default 0");
	struct arg_int *a_mode = arg_int0("m", "mode", "<number>", "LMDB file open mode. Default 0664");

	// SNMP
	struct arg_lit *a_snmp_agent = arg_lit0(NULL, "snmp", "Run SNMP agent");
	
	// deamon
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "Start as daemon/service");
	struct arg_int *a_max_fd = arg_int0(NULL, "maxfd", "<number>", "Set max file descriptors. 0- use default (1024).");

	// other
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 4, "0- quiet (default), 1- errors, 2- warnings, 3- debug, 4- debug libs");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_message_url,
		a_db_path, a_flags, a_mode,
		a_daemonize, a_max_fd, 
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

	if (a_message_url->count)
		message_url = *a_message_url->sval;
	else
		message_url = DEF_QUEUE;

	if (a_db_path->count)
		path = *a_db_path->sval;
	else
		path = DEF_DB_PATH;
	char b[PATH_MAX];
	path = std::string(realpath(path.c_str(), b));

	if (a_mode->count)
		mode = *a_mode->ival;
	else
		mode = DEF_MODE;

	if (a_flags->count)
		flags = *a_flags->ival;
	else
		flags = DEF_FLAGS;

	snmp_agent = a_snmp_agent->count > 0;
	daemonize = a_daemonize->count > 0;
	if (a_max_fd > 0)
		max_fd = *a_max_fd->ival;
	else
		max_fd = 0;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "wacs listener" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

int WacsConfig::error()
{
	return errorcode;
}
