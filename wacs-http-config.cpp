#include "wacs-http-config.h"
#include <iostream>
#include <argtable3/argtable3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#include "platform.h"
#include "utilstring.h"

#define progname "wacs-http"

#define DEF_ROOT					"."
#define DEF_PORT					55550
#define DEF_PORT_STR				"55550"
#define DEF_DB_PATH					"."
#define DEF_MODE					0664
#define DEF_FLAGS					0

WacsHttpConfig::WacsHttpConfig()
	: errorcode(0), port(DEF_PORT), verbosity(0), 
	path(getDefaultDatabasePath()), root_path(getDefaultDatabasePath())
{
}
	
WacsHttpConfig::WacsHttpConfig
(
	int argc,
	char* argv[]
)
{
	errorcode = parseCmd(argc, argv);
}

WacsHttpConfig::~WacsHttpConfig()
{
}

/**
 * Parse command line into WacsHttpConfig class
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int WacsHttpConfig::parseCmd
(
	int argc,
	char* argv[]
)
{
	struct arg_str *a_root_path = arg_str0("d", "root", "<path>", "Default " DEF_ROOT);
	struct arg_int *a_port = arg_int0("p", "port", "<number>", "Default " DEF_PORT_STR);
	struct arg_str *a_db_path = arg_str0(NULL, "dbpath", "<path>", "Database path");
	struct arg_int *a_flags = arg_int0("f", "flags", "<number>", "LMDB flags. Default 0");
	struct arg_int *a_mode = arg_int0("m", "mode", "<number>", "LMDB file open mode. Default 0664");

	// deamon
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "Start as daemon/service");
	struct arg_int *a_max_fd = arg_int0(NULL, "maxfd", "<number>", "Set max file descriptors. 0- use default (1024).");

	// other
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 4, "0- quiet (default), 1- errors, 2- warnings, 3- debug, 4- debug libs");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_root_path, a_port,
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

	if (a_root_path->count)
		root_path = *a_root_path->sval;
	else
		root_path = DEF_ROOT;
	if (a_port->count)
		port = *a_port->ival;
	else
		port = DEF_PORT;

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
		std::cerr << "wacs http listener" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

int WacsHttpConfig::error()
{
	return errorcode;
}
