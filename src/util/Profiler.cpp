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
	map<string, Profile *>::const_iterator it;
	for ( it=profs.begin(); it != profs.end(); ++it )
	{
		delete it->second;
	}
}

void
Profiler::start(string name)
{
	Profile *prof = get(name);
	prof->start();
}

void
Profiler::stop(string name)
{
	map<string, Profile *>::iterator iter = profs.find(name);
	if ( iter == profs.end() ) {
		cerr<<name<<": no such Profile started";
		return;
	}
	iter->second->stop();
}

Profile *
Profiler::get(string name)
{
	Profile *prof;
	map<string, Profile *>::iterator iter = profs.find(name);
	if ( iter == profs.end() ) {
		prof = new Profile(name);
		profs.insert(pair<string, Profile *>(name, prof));
	} else {
		prof = iter->second;
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
	map<string, Profile *>::const_iterator it;
	for ( it=prof.profs.begin(); it != prof.profs.end(); ++it )
	{
		os<<*(it->second)<<endl;
	}
	return os;
}


} // namespace geos.util
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/06/12 11:29:24  strk
 * unsigned int => size_t
 *
 * Revision 1.9  2006/03/07 14:18:34  strk
 * Profiler singleton implemented with a function-static Profiler instance
 *
 * Revision 1.8  2006/03/06 19:40:48  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.7  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.6  2005/02/01 14:18:04  strk
 * Made profiler start/stop inline
 *
 * Revision 1.5  2005/02/01 13:44:59  strk
 * More profiling labels.
 *
 * Revision 1.4  2004/12/03 22:52:56  strk
 * enforced const return of CoordinateSequence::toVector() method to derivate classes.
 *
 * Revision 1.3  2004/11/08 12:15:35  strk
 * Added number of gathered timings in output.
 *
 * Revision 1.2  2004/11/08 11:19:39  strk
 * Profiler::get() always return a Profile (new if not existant).
 *
 * Revision 1.1  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 **********************************************************************/
