/*
 * Copyright (c) 2019 Andrei Ivanov andrei.i.ivanov@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef HISTOGRAMMACSPERTIME_H
#define HISTOGRAMMACSPERTIME_H

#include <inttypes.h>
#include <string>
#include <map>

/**
 */
class HistogramMacsPerTime
{
private:
	int step;
	time_t start;
	time_t finish;
	std::map<int, size_t> histogram;
	void init();
public:	
	HistogramMacsPerTime(time_t astart, time_t afinish, int astep);
	~HistogramMacsPerTime();
	size_t size();
	void put(time_t t, const uint8_t *sa);
	std::string toString();
	std::string toJSON();
};

#endif // HISTOGRAMMACSPERTIME_H
