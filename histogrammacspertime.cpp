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

#include "histogrammacspertime.h"
#include "utilstring.h"
#include <sstream>

HistogramMacsPerTime::HistogramMacsPerTime(time_t astart, time_t afinish, int astep)
	: step(astep > 0 ? astep : 1), start(astart), finish(afinish)
{
	init();
}

HistogramMacsPerTime::~HistogramMacsPerTime()
{
}

size_t HistogramMacsPerTime::size()
{
	return (finish - start + 1) / step;
}

void HistogramMacsPerTime::init()
{
}

void HistogramMacsPerTime::put(time_t t, const uint8_t *sa)
{
	if ((t < start) || (t > finish))
		return;	// skip
	// get index
	int index = (t - start) / step;
	histogram[index] = histogram[index] + 1;
}

std::string HistogramMacsPerTime::toString()
{
	std::stringstream r;
	for (std::map<int, size_t>::const_iterator it(histogram.begin()); it != histogram.end(); ++it)
	{
		time_t t(start + (it->first * step));
		r << time_t2string(t) << "\t" << it->second << std::endl;
	}
	return r.str();
}

std::string HistogramMacsPerTime::toJSON()
{
	std::stringstream r;
	r << "[";
	for (std::map<int, size_t>::const_iterator it(histogram.begin()); it != histogram.end(); ++it)
	{
		time_t t(start + (it->first * step));
		// Uncomment for timestamp like 2019-01-31T15:48:00+09
		// r << "{\"t\":\"" << time_t2string(t) << "\", \"c\":" << it->second << "},";
		r << "{\"t\":" << t << ", \"c\":" << it->second << "},";
	}
	r << "]";
	std::string rr = r.str();
	if (rr.length() > 2)
		rr[rr.length() - 2] = ' ';	// remove last ','
	return rr;
}
