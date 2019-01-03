#include "utilstring.h"
#include <stdlib.h>
#include <string.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for tm_gmtoff and tm_zone */
#endif
#include <time.h>

#include "platform.h"

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
