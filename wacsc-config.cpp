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

#define progname "wacsc"

#define DEF_QUEUE					"ipc:///tmp/wacs.nanomsg"
#define DEF_DB_PATH					"."
#define DEF_MODE					0664
#define DEF_FLAGS					0

#ifdef _MSC_VER
static std::string getDefaultDatabasePath()
{
	std::string r = "";
	// Need a process with query permission set
	HANDLE hToken = 0;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		// Returns a path like C:/Documents and Settings/nibu if my user name is nibu
		char homedir[MAX_PATH];
		DWORD size = sizeof(homedir);
		if (GetUserProfileDirectoryA(hToken, homedir, &size) && (size > 0))
		{
			r = std::string(homedir, size - 1);
		}
		CloseHandle(hToken);
	}
	return r;
}
#else
/**
* https://stackoverflow.com/questions/2910377/get-home-directory-in-linux-c
*/
static  std::string getDefaultDatabasePath()
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	std::string r(homedir);
	return r;
}
#endif

WacscConfig::WacscConfig()
	: errorcode(0), cmd(0), verbosity(0), 
	path(getDefaultDatabasePath())
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
		return 1;
	return -1;
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
	struct arg_str *a_cmd = arg_str1(NULL, NULL, "<cmd>", "Commands: test");
	struct arg_str *a_message_url = arg_str0("i", "input", "<queue url>", "Default " DEF_QUEUE);
	struct arg_str *a_db_path = arg_str0(NULL, "dbpath", "<path>", "Database path");
	struct arg_int *a_flags = arg_int0("f", "flags", "<number>", "LMDB flags. Default 0");
	struct arg_int *a_mode = arg_int0("m", "mode", "<number>", "LMDB file open mode. Default 0664");

	// other
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 4, "0- quiet (default), 1- errors, 2- warnings, 3- debug, 4- debug libs");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_cmd,
		a_message_url,
		a_db_path, a_flags, a_mode,
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
	if (a_cmd->count)
	{
		cmd = parseCommand(*a_cmd->sval);
	}
	if (cmd < 0)
	{
		std::cerr << "Unknown command: " << a_cmd->sval << std::endl;
		nerrors++;
	}
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