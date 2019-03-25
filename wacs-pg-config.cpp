#include "wacs-pg-config.h"
#include <iostream>
#include <argtable3/argtable3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#include "platform.h"
#include "utilstring.h"

#define progname "wacs-pg"

// PostgreSQL
#define DEF_DB_HOST             	"localhost"
#define DEF_DB_PORT             	"5432"
#define DEF_DATABASESOCKET          ""
#define DEF_DATABASECHARSET         "utf8"
#define DEF_DATABASECLIENTFLAGS     0

WacsPgConfig::WacsPgConfig()
	: errorcode(0), cmd(0), verbosity(0), counter(NULL)
{
}
	
WacsPgConfig::WacsPgConfig
(
	int argc,
	char* argv[]
)
	: counter(NULL)
{
	
	errorcode = parseCmd(argc, argv);
}

WacsPgConfig::~WacsPgConfig()
{
}

/**
 * Parse command line into WacsPgConfig class
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int WacsPgConfig::parseCmd
(
	int argc,
	char* argv[]
)
{
	struct arg_str *a_message_url = arg_str0("i", "input", "<queue url>", "e.g. tcp://127.0.0.1:55555, ws://127.0.0.1:2019, ipc://tmp/wacs.nn. Default " DEF_QUEUE);

	// database connection
	struct arg_str *a_conninfo = arg_str0(NULL, "conninfo", "<string>", "database connection");
	struct arg_str *a_user = arg_str0(NULL, "user", "<login>", "database login");
	struct arg_str *a_database = arg_str0(NULL, "database", "<scheme>", "database scheme");
	struct arg_str *a_password = arg_str0(NULL, "password", "<password>", "database user password");
	struct arg_str *a_host = arg_str0(NULL, "host", "<host>", "database host. Default localhost");
	struct arg_str *a_dbport = arg_str0(NULL, "port", "<integer>", "database port. Default 5432");
	struct arg_file *a_optionsfile = arg_file0(NULL, "options-file", "<file>", "database options file");
	struct arg_str *a_dbsocket = arg_str0(NULL, "dbsocket", "<socket>", "database socket. Default none.");
	struct arg_str *a_dbcharset = arg_str0(NULL, "dbcharset", "<charset>", "database client charset. Default utf8.");
	struct arg_int *a_dbclientflags = arg_int0(NULL, "dbclientflags", "<number>", "database client flags. Default 0.");

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
		a_conninfo, a_user, a_database, a_password, a_host, a_dbport, a_optionsfile, a_dbsocket, a_dbcharset, a_dbclientflags,
		a_snmp_agent,
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

	dbconn = *a_conninfo->sval;
	if (a_host->count)
		dbhost = *a_host->sval;
	else
		dbhost = DEF_DB_HOST;

	if (a_dbport->count)
		dbport = *a_dbport->sval;
	else
		dbport = DEF_DB_PORT;

	dboptionsfile = *a_optionsfile->filename;
	dbname = *a_database->sval;
	dbuser = *a_user->sval;
	dbpassword = *a_password->sval;
	if (a_dbsocket->count)
		dbsocket = *a_dbsocket->sval;
	else
		dbsocket = DEF_DATABASESOCKET;

	if (a_dbcharset->count)
		dbcharset = *a_dbcharset->sval;
	else
		dbcharset = DEF_DATABASECHARSET;

	if (a_dbclientflags->count)
		dbclientflags = *a_dbclientflags->ival;
	else
		dbclientflags = DEF_DATABASECLIENTFLAGS;

	snmp_agent = a_snmp_agent->count > 0;
	daemonize = a_daemonize->count > 0;
	
	if (a_max_fd->count > 0)
		max_fd = *a_max_fd->ival;
	else
		max_fd = 0;

	char wd[PATH_MAX];
	path = getcwd(wd, PATH_MAX);	

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

int WacsPgConfig::error()
{
	return errorcode;
}

/**
 * Establish configured database connection
 */
PGconn *dbconnect(WacsPgConfig *config)
{
	if (!config->dbconn.empty())
		return PQconnectdb(config->dbconn.c_str());
	else
		return PQsetdbLogin(config->dbhost.c_str(), config->dbport.c_str(), config->dboptionsfile.c_str(),
			NULL, config->dbname.c_str(), config->dbuser.c_str(), config->dbpassword.c_str());
}
