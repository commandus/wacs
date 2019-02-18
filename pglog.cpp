#include <iostream>
#include <sstream>
#include <string.h>
#include <netinet/in.h>

#include "pglog.h"
#include "errorcodes.h"
#include "hostapd-log-entry.h"

#include "log.h"

/**
 * @brief Open PostgreSQL connection
 * @param env created LMDB environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool openDb
(
	struct dbenv *env
)
{
	if (!env->dbconn.empty())
		env->conn = PQconnectdb(env->dbconn.c_str());
	else
		env->conn = PQsetdbLogin(env->dbhost.c_str(), env->dbport.c_str(), env->dboptionsfile.c_str(),
			NULL, env->dbname.c_str(), env->dbuser.c_str(), env->dbpassword.c_str());
	return (PQstatus(env->conn) == CONNECTION_OK);
}

/**
 * @brief Close PostgreSQL connection
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool closeDb
(
	struct dbenv *env
)
{
	PQfinish(env->conn);
	return true;
}

#define CHECK_STMT(error_message) \
		if (PQresultStatus(res) != PGRES_COMMAND_OK) \
		{ \
			LOG(ERROR) << ERR_DATABASE_STATEMENT_FAIL << error_message; \
			PQclear(res); \
			PQfinish(env->conn); \
			return ERRCODE_DATABASE_STATEMENT_FAIL; \
		}

/**
 * @brief Store input packet to the PostgreSQL
 * @param env database env
 * @param buffer buffer (LogEntry: device_id(0..255), ssi_signal(-128..127), MAC(6 bytes)
 * @param buffer_size buffer size
 * @return 0 - success
 */
int putLog
(
	struct dbenv *env,
	void *buffer,
	size_t size,
	int verbosity
)
{
	if (size < sizeof(LogEntry))
		return ERRCODE_NO_MEMORY;
	LogEntry *entry = (LogEntry *) buffer;
	// swap bytes if needed
	ntohLogEntry(entry);

	// start transaction
	PGresult *res = PQexec(env->conn, "BEGIN");
	CHECK_STMT("start transaction")
	std::stringstream ss;
	time_t dt = time(NULL);
	ss << "INSERT INTO loc(sa, dt, device_id, ssi_signal) VALUES ("
		<< "'" << mactostr(entry->sa) << "', "
		<<  dt << ", "
		<<  (int) entry->device_id << ", "
		<<  (int) entry->ssi_signal
		<< ")";
	// insert
	res = PQexec(env->conn, ss.str().c_str());
	CHECK_STMT("put")
	// commit
	res = PQexec(env->conn, "END");
	CHECK_STMT("commit transaction")
	return 0;
}

/**
 * @brief Store input log data to the PostgreSQL
 * @param env database env
 * @return 0 - success
 */
int putLogEntries
(
	struct dbenv *env,
	int verbosity,
	OnPutLogEntry onPutLogEntry,
	void *onPutLogEntryEnv
)
{
	// start transaction
	PGresult *res = PQexec(env->conn, "BEGIN");
	CHECK_STMT("start transaction")

	LogRecord entry;
	int c = 0;
	while (!onPutLogEntry(onPutLogEntryEnv, &entry)) {
		std::stringstream ss;
		time_t dt = time(NULL);
		ss << "INSERT INTO loc(sa, dt, device_id, ssi_signal) VALUES ("
			<< "'" << mactostr(entry.sa) << "', "
			<<  dt << ", "
			<<  (int) entry.device_id << ", "
			<<  (int) entry.ssi_signal
			<< ")";
		// insert
		res = PQexec(env->conn, ss.str().c_str());
		CHECK_STMT("put")
		c++;
	}

	// commit
	res = PQexec(env->conn, "END");
	CHECK_STMT("commit transaction")
	return c;
}

/**
 * @brief Read log data from the PostgreSQL
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0
 * @param start 0- no limit
 * @param finish 0- no limit
 * @param onLog callback
 * @param onLogEnv object passed to callback
 */
int readLog
(
	struct dbenv *env,
	const uint8_t *sa,			// MAC address
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
)
{
	if (!onLogEnv)
		return ERRCODE_WRONG_PARAM;
	struct ReqEnv* e = (struct ReqEnv*) onLogEnv;

	if (!onLog)
	{
		LOG(ERROR) << ERR_WRONG_PARAM << "onLog" << std::endl;
		return ERRCODE_WRONG_PARAM;
	}
	// start transaction
	PGresult *res = PQexec(env->conn, "BEGIN");
	CHECK_STMT("start transaction")

	int sz;
	if (saSize >= 6)
		sz = 6;
	else 
		if (saSize <= 0)
			sz = 0;
		else
			sz = saSize;
		
	if (finish == 0)
		finish = UINT32_MAX;
	if (finish < start)
	{
		uint32_t swap = start;
		start = finish;
		finish = swap;
	}

	std::stringstream ss;
	time_t dt = time(NULL);
	ss << "SELECT sa, dt, device_id, ssi_signal FROM loc WHERE sa LIKE "
		<< "'" << mactostr2(sa, saSize) << (saSize < 6 ? "%'" : "'")
		<<  " AND dt <= " << finish
		<<  " AND dt >= " << start;
	// insert
	res = PQexec(env->conn, ss.str().c_str());
	CHECK_STMT("read")
	int nrows = PQntuples(res);
	for (int r = 0; r < nrows; r++)
	{
		LogKey key1;
		strtomacaddress(key1.sa, PQgetvalue(res, r, 0));
		key1.dt = strtol(PQgetvalue(res, r, 1), NULL, 10);
		LogData data;
		data.device_id  = strtol(PQgetvalue(res, r, 2), NULL, 10);
		data.ssi_signal= strtol(PQgetvalue(res, r, 3), NULL, 10);
		if (onLog(onLogEnv, &key1, &data))
			break;
	}
	// commit
	res = PQexec(env->conn, "END");
	CHECK_STMT("commit transaction")
	return 0;
}

/**
 * @brief Remove log data from the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0
 * @param start 0- no limit
 * @param finish 0- no limit
 */
int rmLog
(
	struct dbenv *env,
	const uint8_t *sa,			// MAC address
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish
)
{
	return ERRCODE_NOT_IMPLEMENTED;
}

/**
 * @brief Read last probes from the LMDB
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0 
 * @param start 0- no limit
 * @param finish 0- no limit
 * @param onLog callback
 */
int readLastProbe
(
	struct dbenv *env,
	const uint8_t *sa,			///< MAC address filter
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
)
{
	return ERRCODE_NOT_IMPLEMENTED;
}

// Notification database routines 

/**
 * Get notification JSON string into retval
 * @param retval return value
 * @return empty string if not found
 */
int getNotification
(
	std::string &retval,
	struct dbenv *env,
	const uint8_t *sa			///< MAC address
)
{
	return ERRCODE_NOT_IMPLEMENTED;
}

/**
 * Put notification JSON string 
 * @param value return notification JSON string. If empty, clear
 * @return 0 - success
 */
int putNotification
(
	struct dbenv *env,
	const uint8_t *sa,			///< MAC address
	const std::string &value
)
{
	return ERRCODE_NOT_IMPLEMENTED;
}

/**
 * List notification JSON string 
 * @param value return notification JSON string 
 * @return 0 - success
 */
int lsNotification
(
	struct dbenv *env,
	OnNotification onNotification,
	void *onNotificationEnv
)
{
	return ERRCODE_NOT_IMPLEMENTED;
}

/**
 * Remove notification JSON string 
 * @return 0 - success
 */
int rmNotification
(
	struct dbenv *env,
	const uint8_t *sa,
	int sa_size
)
{
	return ERRCODE_NOT_IMPLEMENTED;
}
