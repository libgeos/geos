/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/profiler.h>
#include <iostream>
#include <map>
#include <string>
#include <utility>

using namespace std;

namespace geos {
namespace util { // geos.util

Profile::Profile(string newname)
{
	name = newname;
	totaltime = 0;
	min = max = avg = 0;
}

Profile::~Profile()
{
}

#if 0
void
Profile::start()
{
	gettimeofday(&starttime, NULL);
}

void
Profile::stop()
{
	gettimeofday(&stoptime, NULL);
	double elapsed = 1000000*(stoptime.tv_sec-starttime.tv_sec)+
		(stoptime.tv_usec-starttime.tv_usec);

	timings.push_back(elapsed);
	totaltime += elapsed;
	if ( timings.size() == 1 ) max = min = elapsed;
	else
	{
		if ( elapsed > max ) max = elapsed;
		if ( elapsed < min ) min = elapsed;
	}
	avg = totaltime / timings.size();
}
#endif

double
Profile::getMax() const
{
	return max;
}

double
Profile::getMin() const
{
	return min;
}

double
Profile::getAvg() const
{
	return avg;
}

double
Profile::getTot() const
{
	return totaltime;
}

size_t
Profile::getNumTimings() const
{
	return timings.size();
}

Profiler::Profiler()
{
}

Profiler::~Profiler()
{
	for (auto &prof : profs)
	{
		delete prof.second;
	}
}

void
Profiler::start(string name)
{
	auto prof = get(name);
	prof->start();
}

void
Profiler::stop(string name)
{
	auto prof = profs.find(name);
	if ( prof == profs.end() ) {
		cerr<<name<<": no such Profile started";
		return;
	}
	prof->second->stop();
}

Profile *
Profiler::get(string name)
{
	Profile *prof{nullptr};
	auto has_prof = profs.find(name);
	if ( has_prof == profs.end() ) {
		prof = new Profile(name);
		profs.insert(pair<string, Profile *>(name, prof));
	} else {
		prof = has_prof->second;
	}
	return prof;
}

Profiler *
Profiler::instance()
{
	static Profiler internal_profiler;
	return &internal_profiler;
}


ostream&
operator<< (ostream &os, const Profile &prof)
{
	os << " num:"<<prof.getNumTimings()<<" min:"<<
		prof.getMin()<<" max:"<<prof.getMax()<<
		" avg:"<<prof.getAvg()<<" tot:"<<prof.getTot()<<
		" ["<<prof.name<<"]";
	return os;
}

ostream&
operator<< (ostream &os, const Profiler &prof)
{
	for (const auto elem : prof.profs)
	{
		os << elem.second <<endl;
	}
	return os;
}


} // namespace geos.util
} // namespace geos
