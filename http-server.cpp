/**
 *	HTTP server
 */
#include <algorithm>
#include <iostream>
#include <cstring>
#include <sstream>
#include <stdio.h>

#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>

#include "errorcodes.h"
#include "http-server.h"
#include "send-log-entry.h"

#include "dblog.h"
#include "log.h"

#include <sys/stat.h>

struct MHD_Daemon *mhdDaemon;

const static char *MSG404 = "404 not found";
const static char *MSG500 = "500 Internal error";

const static char *CT_HTML = "text/html;charset=UTF-8";
const static char *CT_JSON = "text/javascript;charset=UTF-8";
const static char *CT_KML = "application/vnd.google-earth.kml+xml";
const static char *CT_TILE = "image/png";
const static char *CT_CSS = "text/css";
const static char *CT_TEXT = "text/plain;charset=UTF-8";
const static char *CT_TTF = "font/ttf";
const static char *CT_BIN = "application/octet";

typedef enum
{
	RT_LOG = 0,			//< List of coordinates
	RT_LAST_LOG = 1,	//< List of device and their states
	RT_UNKNOWN = 100	//< file request
} RequestType;

#define PATH_COUNT 2
static const char *PATHS[PATH_COUNT] = 
{
	"/log",
	"/last"
};

static const char* queryParamNames[] = {
	"start",
	"finish",
	"sa",
	"offset",
	"size"
};

class RequestParams
{
public:
	RequestType requestType;
	time_t start;
	time_t finish;
	std::string sa;
	int offset;
	int size;
	RequestParams
	(
		struct MHD_Connection *connection,
		const char *url
	) 
		: requestType(parseRequestType(url))
	{
		parseUriParams(connection);
	}
private:
	static RequestType parseRequestType(const char *url)
	{
		int i;
		for (i = 0; i < PATH_COUNT; i++)
		{
			if (strcmp(PATHS[i], url) == 0)
				return (RequestType) i;
		}
		return RT_UNKNOWN;
	}

	static const char *requestTypeString(RequestType value)
	{
		if (value == RT_UNKNOWN)
			return "Unknown";
		if ((value >= 0) && (value < PATH_COUNT))
			return PATHS[(int) value];
		else
			return "???";
	}
	
	void parseUriParams
	(
		struct MHD_Connection *connection
	)
	{
		const char *v = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, queryParamNames[2]);
		if (v)
			sa = v;
		else
			sa = "";
		v = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, queryParamNames[0]);
		if (v)
			start = strtol(v, NULL, 0);
		else
			start = 0;
		v = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, queryParamNames[1]);
		if (v)
			finish = strtol(v, NULL, 0);
		else
			finish = time(NULL);
	}

};

class HttpEnv
{
public:
	WacsHttpConfig *config;
};

struct ReqEnv
{
	struct dbenv *dbEnv;
	std::stringstream *retval;
} ReqEnv;

static bool onReqLog
(
	void *env,
	LogKey *key,
	LogData *data
)
{
	if (!env)
		return true;
	struct ReqEnv *req = (struct ReqEnv *) env;
	*req->retval << "{\"sa\":\"" << mactostr(key->sa) << "\",\"dt\":" << key->dt;
	if (data)
		*req->retval
			<< ",\"device_id\":" << data->device_id
			<< ",\"ssi_signal\":" << data->ssi_signal;
	*req->retval			
		<< "},";
	return false;
}

static std::string lsLog
(
	const WacsHttpConfig *config,
	const RequestParams *params
)
{
	struct ReqEnv env;
	struct dbenv db;
	std::stringstream ss;
	env.dbEnv = &db;
	env.retval = &ss;
	if (!openDb(env.dbEnv, config->path.c_str(), config->flags, config->mode))
	{
		std::cerr << ERR_LMDB_OPEN << config->path << std::endl;
		return "";
	}

	uint8_t sa[6];
	strtomacaddress(&sa, params->sa);
	ss << "[";
	readLog(env.dbEnv, params->sa.empty() ? NULL : sa, params->start, params->finish, onReqLog, (void *) &env);
	ss << "]";
	if (!closeDb(env.dbEnv))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		return "";
	}
	std::string r(ss.str());
	if (r.size() > 2)
		r[r.size() - 2] = ' ';	// remove last ','
	return r;
}

static std::string lsLastProbe
(
	const WacsHttpConfig *config,
	const RequestParams *params
)
{
	struct ReqEnv env;
	struct dbenv db;
	std::stringstream ss;
	env.dbEnv = &db;
	env.retval = &ss;
	if (!openDb(env.dbEnv, config->path.c_str(), config->flags, config->mode))
	{
		std::cerr << ERR_LMDB_OPEN << config->path << std::endl;
		return "";
	}

	uint8_t sa[6];
	strtomacaddress(&sa, params->sa);
	ss << "[";
	readLastProbe(env.dbEnv, params->sa.empty() ? NULL : sa, onReqLog, &env);
	ss << "]";
	if (!closeDb(env.dbEnv))
	{
		std::cerr << ERR_LMDB_CLOSE << config->path << std::endl;
		return "";
	}
	std::string r(ss.str());
	if (r.size() > 2)
		r[r.size() - 2] = ' ';	// remove last ','
	return r;
}

const char *mimeTypeByFileExtention
(
	const std::string &filename
)
{
	std::string ext = filename.substr(filename.find_last_of(".") + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext == "html")
		return CT_HTML;
	else
		if (ext == "htm")
			return CT_HTML;
	else
		if (ext == "js")
			return CT_JSON;
	else
		if (ext == "css")
			return CT_CSS;
	else
		if (ext == "txt")
			return CT_TEXT;
	else
		if (ext == "ttf")
			return CT_TTF;
	else
		return CT_BIN;
}

static ssize_t file_reader_callback(void *cls, uint64_t pos, char *buf, size_t max)
{
	FILE *file = (FILE *) cls;
	(void) fseek (file, (long) pos, SEEK_SET);
	return fread (buf, 1, max, file);
}

static void free_file_reader_callback(void *cls)
{
	fclose ((FILE *) cls);
}

static int processFile
(
	struct MHD_Connection *connection,
	const std::string &filename
)
{
	struct MHD_Response *response;
	int ret;
	FILE *file;
	struct stat buf;

	const char *fn = filename.c_str();

	if (stat(fn, &buf) == 0)
		file = fopen(fn, "rb");
	else
		file = NULL;
	if (file == NULL)
	{
		response = MHD_create_response_from_buffer(strlen(MSG404), (void *) MSG404, MHD_RESPMEM_PERSISTENT);
		ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
		MHD_destroy_response (response);
	}
	else
	{
		response = MHD_create_response_from_callback(buf.st_size, 32 * 1024,
			&file_reader_callback, file, &free_file_reader_callback);
		if (NULL == response)
		{
			fclose (file);
			return MHD_NO;
		}

		MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, mimeTypeByFileExtention(filename));
		ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
		MHD_destroy_response (response);
	}
	return ret;
}

/**
 * Translate Url to the file name
 */
static std::string buildFileName
(
	const std::string &dirRoot,
	const char *url
)
{
	std::stringstream r;
	r << dirRoot;
	if (url)
	{
		r << url;
		int l = strlen(url);
		if (l && (url[l - 1] == '/'))
			r << "index.html";
	}
	return r.str();
}

static int httpHandler
(
	void *cls,
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data,
	size_t * upload_data_size,
	void ** ptr
) 
{
	static int dummy;
	const char *page = "/last /log?sa=&s=&f=";
	struct MHD_Response *response;
	int ret;

	if (strcmp(method, "GET") != 0)
		return MHD_NO; /* unexpected method */
	if (&dummy != *ptr)
	{
		// The first time only the headers are valid, do not respond in the first round...
		*ptr = &dummy;
		return MHD_YES;
	}

	if (*upload_data_size != 0)
		return MHD_NO; // upload data in a GET!?

	RequestParams params(connection, url);

	*ptr = NULL; /* clear context pointer */

	HttpEnv *env = (HttpEnv*) cls;
	std::string data = "";
	switch (params.requestType)
	{
	case RT_LOG:
		data = lsLog(env->config, &params);
		if (data.empty())
			response = MHD_create_response_from_buffer(strlen(MSG500), (void *) MSG500, MHD_RESPMEM_PERSISTENT);
		else
			response = MHD_create_response_from_buffer(data.size(), (void *) data.c_str(), MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, CT_JSON);
		break;
	case RT_LAST_LOG:
		data = lsLastProbe(env->config, &params);
		if (data.empty())
			response = MHD_create_response_from_buffer(strlen(MSG500), (void *) MSG500, MHD_RESPMEM_PERSISTENT);
		else
			response = MHD_create_response_from_buffer(data.size(), (void *) data.c_str(), MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, CT_JSON);
		break;
	default:
		{
			std::string fn = buildFileName(env->config->root_path, url);
			return processFile(connection, fn);
		}
	};
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	return ret;
}

static HttpEnv httpEnv;
/**
 * @brief Run HTTP server
 * @param config
 * @return  0- success
 *          >0- error (see errorcodes.h)
 */
int run
(
	WacsHttpConfig *config
)
{
	config->stop_request = 0;
	httpEnv.config = config;
	mhdDaemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, config->port, NULL, NULL, &httpHandler, (void *) &httpEnv, MHD_OPTION_END);
	if (!mhdDaemon)
		return ERRCODE_EXEC;
	while (!config->stop_request)
	{
		sleep(2);
	}
	return ERR_OK;
}

/**
 *  @brief Stop writer
 *  @param config
 *  @return 0- success,  >0- config is not initialized yet
 */
int stop
(
	WacsHttpConfig *config
)
{
	MHD_stop_daemon(mhdDaemon);
    if (!config)
        return ERRCODE_NO_CONFIG;
    config->stop_request = 1;
    return ERR_OK;
}

int reload(WacsHttpConfig *config)
{
	if (!config)
		return ERRCODE_NO_CONFIG;
	LOG(ERROR) << MSG_RELOAD_BEGIN << std::endl;
	config->stop_request = 2;
	return ERR_OK;
}
