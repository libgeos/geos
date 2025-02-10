/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

/*--------------------------------------------------------------
Stress test for the Orientation Index implementation.

Usage: stress_orientation [ -v ]

A robust orientation index implementation should be consistent
- i.e. it should produce the same result for the 3 possible 
permutations of the input coordinates which have the same orientation:

p0-p1 / p2    p1-p2 / p0    p2-p0 / p1     

The robust implementation uses DoubleDouble arithmetic and a filter to improve computation time. 
It is compared to 
the simple floating-point orientation computation, which is not consistent.
--------------------------------------------------------------*/

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
#include <geos/io/WKTWriter.h>

using namespace geos::algorithm;
using namespace geos::geom;
using namespace geos::io;

int
orientationIndexFP(const Coordinate& p1, const Coordinate& p2,const Coordinate& q){
	double dx1 = p2.x-p1.x;
	double dy1 = p2.y-p1.y;
	double dx2 = q.x-p2.x;
	double dy2 = q.y-p2.y;
	double det = dx1 * dy2 - dx2 * dy1;
	if (det > 0.0) return 1;
	if (det < 0.0) return-1;
	return 0;
}

bool isVerbose = false;

std::size_t failDD;
std::size_t failFP;

void parseFlag(char* arg) {
    char flag = arg[1];
    switch (flag) {
    case 'v': 
        isVerbose = true; break;
    }
}

void parseArgs(int argc, char** argv) {
    if (argc <= 1) return;
    int i = 1;
    //-- parse flags
    while (i < argc && argv[i][0] == '-' && isalpha(argv[i][1])) {
        parseFlag(argv[i]);
        i++;
    }
}

char orientSym(int orientationIndex) {
    if (orientationIndex < 0) return '-';
    if (orientationIndex > 0) return '+';
    return '0';
}

void report(std::string name, int orient0, int orient1,
    int orient2) {

    std::string consistentInd = (orient0 == orient1 && orient0 == orient2) ? "  " : "<!";
    std::cout << name << ": "
    << orientSym(orient0) << "  "  
    << orientSym(orient1) << "  " 
    << orientSym(orient2) << "  "
    << consistentInd << "  ";
}

bool isAllOrientationsEqualDD(Coordinate p0, Coordinate p1, Coordinate p2)
{
    int orient0 = Orientation::index(p0, p1, p2);
    int orient1 = Orientation::index(p1, p2, p0);
    int orient2 = Orientation::index(p2, p0, p1);

    if (isVerbose) {
        report("DD", orient0, orient1, orient2);
    }

    return orient0 == orient1 && orient0 == orient2;
}

bool isAllOrientationsEqualFP(Coordinate p0, Coordinate p1, Coordinate p2)
{
    int orient0 = orientationIndexFP(p0, p1, p2);
    int orient1 = orientationIndexFP(p1, p2, p0);
    int orient2 = orientationIndexFP(p2, p0, p1);
    if (isVerbose) {
        report("FP", orient0, orient1, orient2);
    }
    return orient0 == orient1 && orient0 == orient2;
}

Coordinate randomCoord() {
    double x = (10.0 * random()) / RAND_MAX;
    double y = (10.0 * random()) / RAND_MAX;
    return Coordinate(x, y);
}

void runTest()
{
    Coordinate p0 = randomCoord();
    Coordinate p1 = randomCoord();

    Coordinate p2 = Coordinate(LineSegment::midPoint(p0, p1));

    bool isCorrectDD = isAllOrientationsEqualDD(p0, p1, p2);
    bool isCorrectFP = isAllOrientationsEqualFP(p0, p1, p2);

    if (isVerbose) {
        std::cout << "   " << WKTWriter::toLineString(p0, p1) << " - " << WKTWriter::toPoint(p2)
            << std::endl;
    }

    if (! isCorrectDD) failDD++;
    if (! isCorrectFP) failFP++;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        parseArgs(argc, argv);
    }

    srand (static_cast <unsigned> (time(0)));

    int i = 0;
    while (true) {
        runTest();
        i++;
        
        if (i % 1000 == 0) {
            std::cout << "Num tests: " <<  i 
                << "  DD fail = " << failDD << " (" << (100.0 * failDD / (double) i) << "%)"
                << "  FP fail = " << failFP << " (" << (100.0 * failFP / (double) i) << "%)"
                << std::endl;
        }
    }
}