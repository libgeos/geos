/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef __XMLTESTER_H__
#define __XMLTESTER_H__

using namespace geos;

class XMLTester {

public:
	XMLTester();
	~XMLTester();
	void run(const string &testFile);
	void resultSummary(ostream &os) const;
	void resetCounters();

	/*
	 * Values:
	 *	0: Show case description, run tests, show result
	 *	1: Show parsed geometry values
	 *	2: Run predicates
	 *
	 * Return previously set verbosity level
	 */
	int setVerbosityLevel(int val);

private:
	enum {
		SHOW_RUN_INFO=1,
		SHOW_CASE,
		SHOW_TEST,
		SHOW_RESULT,
		SHOW_GEOMS,
		SHOW_GEOMS_FULL,
		PRED
	};

	void parsePrecisionModel();
	void parseCase();
	void parseTest();
	void runPredicates(const Geometry *a, const Geometry *b);
	Geometry *parseGeometry(const string &in);
	static string trimBlanks(const string &in);

	Geometry *gA;
	Geometry *gB;
	Geometry *gT;
	PrecisionModel *pm;

	GeometryFactory *factory;
	WKTReader *r;
	WKTWriter *w;
	WKBReader *br;
	CMarkupSTL xml;

	int verbose;
	int test_predicates;

	int failed;
	int succeeded;
	int caseCount;
	int testCount;
	int testFileCount;
	int totalTestCount;

};


#endif 
