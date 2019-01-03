#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v);

int getCurrentTimeOffset();
