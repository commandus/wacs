#include <string>
#include <libpq-fe.h>

/**
 * @brief PostgreSQL environment(transaction, cursor)
 */

class dbenv {
public:	
	PGconn *conn;
	// PostgreSQL options 
	std::string dbconn;
	std::string dboptionsfile;
	std::string dbname;
	std::string dbuser;
	std::string dbpassword;
	std::string dbhost;
	std::string dbport;
	std::string dbsocket;
	std::string dbcharset;
	int dbclientflags;
};

typedef struct 
{
	uint8_t tag;		// 'L'- log entry
	uint8_t sa[6];		// MAC address
	time_t dt;			// time, seconds since Unix epoch 
} LogKey;

typedef struct 
{
	uint16_t device_id;	// 0
	int8_t ssi_signal;	// dBm
} LogData;

typedef struct 
{
	uint8_t tag;		// 'P'- last_probe
	uint8_t sa[6];		// MAC address
} LastProbeKey;

typedef struct 
{
	uint8_t sa[6];		// MAC address
	uint32_t dt;		// Date & Time
	uint16_t device_id;	// 0..255 AP identifier
	int8_t ssi_signal;	// dBm
} LogRecord;

/**
 * @brief Open PostgreSQL database file
 * @param env created PostgreSQL environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool openDb
(
	dbenv *env
);

/**
 * @brief Close PostgreSQL database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool closeDb
(
	dbenv *env
);

// callback, return true - stop request, false- continue
typedef bool (*OnLog)
(
	void *env,
	LogKey *key,
	LogData *data
);

// callback, return true - stop request, false- continue
typedef bool (*OnLastProbe)
(
	void *env,
	LastProbeKey *key
);

/**
 * callback for putLogEntries, return true - stop request, false- continue
 * @param retval return date&time, device id, ssi signal
 */
typedef bool (*OnPutLogEntry)
(
	void *env,
	LogRecord *rec
);

// callback, return true - stop request, false- continue
typedef bool (*OnNotification)
(
	void *env,
	const char *sa,
	const std::string &data
);

/**
 * @brief Store input log data to the PostgreSQL
 * @param env database env
 * @return 0 - success
 */
int putLog
(
	dbenv *env,
	void *buffer,
	size_t size,
	int verbosity
);

/**
 * @brief Store input log data to the PostgreSQL
 * @param env database env
 * @param buffer buffer (LogEntry: device_id(0..255), ssi_signal(-128..127), MAC(6 bytes)
 * @param size buffer size
 * @return 0 - success
 */
int putLogEntries
(
	dbenv *env,
	int verbosity,
	OnPutLogEntry onPutLogEntry,
	void *onPutLogEntryEnv
);

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
	dbenv *env,
	const uint8_t *sa,			///< MAC address
	int saSize,
	time_t start,				///< time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
);

/**
 * @brief remove log data from the PostgreSQL
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0 
 * @param start 0- no limit
 * @param finish 0- no limit
 */
int rmLog
(
	dbenv *env,
	const uint8_t *sa,			///< MAC address
	int saSize,
	time_t start,				///< time, seconds since Unix epoch 
	time_t finish
);

/**
 * @brief Read last probes from the PostgreSQL
 * @param env database env
 * @param sa can be NULL
 * @param saSize can be 0
 * @param start 0- no limit
 * @param finish 0- no limit
 * @param onLog callback
 * @param onLogEnv object passed to callback
 */
int readLastProbe
(
	dbenv *env,
	const uint8_t *sa,			///< MAC address
	int saSize,
	time_t start,				// time, seconds since Unix epoch 
	time_t finish,
	OnLog onLog,
	void *onLogEnv
);

// Notification database routines 

/**
 * Get notification JSON string into retval
 * @param retval return value
 * @return empty string if not found
 */
int getNotification
(
	std::string &retval,
	dbenv *env,
	const uint8_t *sa			///< MAC address
);

/**
 * Put notification JSON string 
 * @param value return notification JSON string. If empty, clear
 * @return 0 - success
 */
int putNotification
(
	dbenv *env,
	const uint8_t *sa,			///< MAC address
	const std::string &value
);

/**
 * Remove notification JSON string 
 * @return 0 - success
 */
int rmNotification
(
	dbenv *env,
	const uint8_t *sa,
	int sa_size
);

/**
 * List notification JSON string 
 * @param value return notification JSON string 
 * @return 0 - success
 */
int lsNotification
(
	dbenv *env,
	OnNotification onNotification,
	void *onNotificationEnv
);
