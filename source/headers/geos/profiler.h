/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_PROFILER_H
#define GEOS_PROFILER_H

#include <memory>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#ifndef _MSC_VER
#  include <sys/time.h>
#endif
#include <geos/timeval.h>

#ifndef PROFILE
#define PROFILE 0
#endif

using namespace std;

namespace geos {


/*
 * \class Profile utils.h geos.h
 *
 * \brief Profile statistics
 */
class Profile {
public:
	/** \brief Create a named profile */
	Profile(string name);

	/** \brief Destructor */
	~Profile();

	/** \brief start a new timer */
	void start();

	/** \brief stop current timer */
	void stop();

	/** \brief Return Max stored timing */
	double getMax() const;

	/** \brief Return Min stored timing */
	double getMin() const;

	/** \brief Return total timing */
	double getTot() const;

	/** \brief Return average timing */
	double getAvg() const;

	/** \brief Return number of timings */
	unsigned int getNumTimings() const;

	/** \brief Profile name */
	string name;


private:

	/* \brief current start and stop times */
	struct timeval starttime, stoptime;

	/* \brief actual times */
	vector<double> timings;

	/* \brief total time */
	double totaltime;

	/* \brief max time */
	double max;

	/* \brief max time */
	double min;

	/* \brief max time */
	double avg;

};

/*
 * \class Profiler utils.h geos.h
 *
 * \brief Profiling class
 *
 */
class Profiler {

public:

	Profiler();
	~Profiler();

	/**
	 * \brief
	 * Return the singleton instance of the
	 * profiler.
	 */
	static Profiler *instance(void);

	/**
	 * \brief
	 * Start timer for named task. The task is
	 * created if does not exist.
	 */
	void start(string name);

	/**
	 * \brief
	 * Stop timer for named task. 
	 * Elapsed time is registered in the given task.
	 */
	void stop(string name);

	/** \brief get Profile of named task */
	Profile *get(string name);

	map<string, Profile *> profs;
};


/** \brief Return a string representing the Profile */
ostream& operator<< (ostream& os, const Profile&);

/** \brief Return a string representing the Profiler */
ostream& operator<< (ostream& os, const Profiler&);

}
#endif // ndef GEOS_PROFILER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2004/12/03 16:21:07  frank
 * dont try for sys/time.h with MSVC
 *
 * Revision 1.3  2004/11/30 16:44:16  strk
 * Added gettimeofday implementation for win32, curtesy of Wu Yongwei.
 *
 * Revision 1.2  2004/11/04 08:49:13  strk
 * Unlinked new documentation.
 *
 * Revision 1.1  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 **********************************************************************/
