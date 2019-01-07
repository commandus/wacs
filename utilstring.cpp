#include "utilstring.h"
#include <stdlib.h>
#include <string.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for tm_gmtoff and tm_zone */
#endif
#include <time.h>

#include "platform.h"

#ifndef _MSC_VER
#define TMSIZE sizeof(struct tm)
#define localtime_s(tm, time) memmove(tm, localtime(time), TMSIZE)
#endif


#ifdef _MSC_VER
#include "Userenv.h"
#else
#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>
#endif

const static char *dateformat = "%FT%T";

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v)
{
        struct tm tmd;
        memset(&tmd, 0, sizeof(struct tm));

        time_t r;
        if (strptime(v, dateformat, &tmd) == NULL)
                r = strtol(v, NULL, 0);
        else
                r = mktime(&tmd);
        return r;
}

int getCurrentTimeOffset()
{
		time_t t = time(NULL);
		struct tm lt = {0};
		localtime_r(&t, &lt);
		return lt.tm_gmtoff;
}

std::string time_t2string
(
	time_t value
)
{
	struct tm tm;
	localtime_s(&tm, &value);
	char dt[32];
	strftime(dt, sizeof(dt), "%FT%T%Z", &tm);
	return std::string(dt);
}

#ifdef _MSC_VER
std::string getDefaultDatabasePath()
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
std::string getDefaultDatabasePath()
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	std::string r(homedir);
	return r;
}
#endif

