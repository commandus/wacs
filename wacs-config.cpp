#include "wacs-config.h"
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

#define DEF_FILE_NAME "./wacs.db"

#include "platform.h"

#define progname "wacs"

#ifdef _MSC_VER
static std::string getDefaultDatabaseFileName(const std::string &filename)
{
	std::string r = filename;
	// Need a process with query permission set
	HANDLE hToken = 0;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		// Returns a path like C:/Documents and Settings/nibu if my user name is nibu
		char homedir[MAX_PATH];
		DWORD size = sizeof(homedir);
		if (GetUserProfileDirectoryA(hToken, homedir, &size) && (size > 0))
		{
			r = std::string(homedir, size - 1).append("\\").append(filename);
		}
		CloseHandle(hToken);
	}
	return r;
}
#else
/**
* https://stackoverflow.com/questions/2910377/get-home-directory-in-linux-c
*/
static  std::string getDefaultDatabaseFileName(const std::string &filename)
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	std::string r(homedir);
	return r + "/" + filename;
}
#endif

WacsConfig::WacsConfig()
	: errorcode(0), cmd(0), verbosity(0), 
	file_name(getDefaultDatabaseFileName(DEF_FILE_NAME))
{
}
	
WacsConfig::WacsConfig
(
	int argc,
	char* argv[]
)
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
	struct arg_str *a_file_name = arg_str0("c", "config", "<file>", "Configuration file. Default ~/" DEF_FILE_NAME);
	// other
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 4, "0- quiet (default), 1- errors, 2- warnings, 3- debug, 4- debug libs");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_file_name,
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

	if (a_file_name->count)
		file_name = *a_file_name->sval;
	else
		file_name = getDefaultDatabaseFileName(DEF_FILE_NAME);

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Web push notification command line interface client" << std::endl;
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
