#include "wacsc-config.h"
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
#define progname "wacsc"

#define DEF_DB_PATH					"."
#define DEF_MODE					0664
#define DEF_FLAGS					0

#define DEF_COUNT					1000


WacscConfig::WacscConfig()
	: errorcode(0), cmd(0), verbosity(0), 
	path(getDefaultDatabasePath()), offset(0), count(0)
{
}
	
WacscConfig::WacscConfig
(
	int argc,
	char* argv[]
)
{
	errorcode = parseCmd(argc, argv);
}

WacscConfig::~WacscConfig()
{
}

static int parseCommand
(
	const char *value
)
{
	std::string v(value);
	if (v == "test")
		return CMD_SEND_TEST;
	if (v == "log")
		return CMD_LS_LOG;
	if (v == "probe")
		return CMD_LS_LAST_PROBE;
	if (v == "log-count")
		return CMD_COUNT_LOG;
	if (v == "probe-count")
		return CMD_COUNT_LAST_PROBE;
	if (v == "macs-per-time")
		return CMD_MACS_PER_TIME;
	
	return CMD_NONE;
}

/**
 * Parse command line into WacscConfig class
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int WacscConfig::parseCmd
(
	int argc,
	char* argv[]
)
{
	struct arg_str *a_cmd = arg_str1(NULL, NULL, "<cmd>", "Commands: test|log|probe|log-count|probe-count|macs-per-time");
	struct arg_int *a_repeats = arg_int0("n", "repeats", "<number>", "for test command. Default 1");
	// filter
	// MAC address
	struct arg_str *a_mac = arg_str0("a", "sa", "<MAC>", "MAC address");
	struct arg_str *a_start = arg_str0(NULL, "start", "<local time>", "e.g. 2019-01-01T00:00:00 or 1546300800 (GMT Unix seconds)");
	struct arg_str *a_finish = arg_str0(NULL, "finish", "<local time>", "e.g. 2019-12-31T23:59:59 or 1577836799. Default now.");

	struct arg_int *a_device_id = arg_int0("d", "deviceid", "<number>", "Devide identifier");
	struct arg_int *a_ssi_signal = arg_int0("b", "ssisignal", "<number>", "SSI signal");

	struct arg_str *a_message_url = arg_str0("i", "input", "<queue url>", "e.g. tcp://127.0.0.1:55555, ws://127.0.0.1:2019, ipc://tmp/wacs.nn. Default " DEF_QUEUE);
	
	struct arg_int *a_offset = arg_int0("o", "offset", "<number>", "Default 0");
	struct arg_int *a_count = arg_int0("c", "count", "<number>", "Default 1000");

	struct arg_str *a_db_path = arg_str0(NULL, "dbpath", "<path>", "Database path");
	struct arg_int *a_flags = arg_int0("f", "flags", "<number>", "LMDB flags. Default 0");
	struct arg_int *a_mode = arg_int0("m", "mode", "<number>", "LMDB file open mode. Default 0664");
	
	struct arg_int *a_step_seconds = arg_int0(NULL, "step", "<seconds>", "Valid with command macs-per-time. Default 1");

	// other
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 4, "0- quiet (default), 1- errors, 2- warnings, 3- debug, 4- debug libs");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_cmd, a_repeats,
		a_mac, a_start, a_finish,
		a_device_id, a_ssi_signal, a_message_url, 
		a_offset, a_count,
		a_db_path, a_flags, a_mode,
		a_step_seconds,
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

	if (a_repeats->count)
	{
		repeats = *a_repeats->ival;
		if (repeats <= 0) 
		{
			nerrors++;
			std::cerr << "Repeat count must be 1 or greater." << std::endl;
		}
	}
	else
		repeats = 1;
	
	if (a_cmd->count)
	{
		cmd = parseCommand(*a_cmd->sval);
	}
	if (cmd < 0)
	{
		std::cerr << "Unknown command: " << a_cmd->sval << std::endl;
		nerrors++;
	}

	if (a_mac->count)
		mac = std::string(*a_mac->sval);
	else
		mac = "";

	if (a_start->count)
		start = parseDate(*a_start->sval);
	else
		start = 0;
	if (a_finish->count)
		finish = parseDate(*a_finish->sval);
	else
		finish = time(NULL);
	if (a_device_id->count)
		device_id = *a_device_id->ival;
	else
		device_id = 0;

	if (a_step_seconds->count)
		step_seconds = *a_step_seconds->ival;
	else
		step_seconds = 1;

	if (a_ssi_signal->count)
		ssi_signal = *a_ssi_signal->ival;
	else
		ssi_signal = 0;
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

	if (a_offset->count)
		offset = *a_offset->ival;
	else
		offset = 0;

	if (a_count->count)
		count = *a_count->ival;
	else
		count = DEF_COUNT;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "wacs command line interface client" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

int WacscConfig::error()
{
	return errorcode;
}
