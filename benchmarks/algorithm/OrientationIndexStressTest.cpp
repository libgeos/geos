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
    -v - displays the input and results from each test

A robust orientation index implementation should be internally consistent
- i.e. it should produce the same result for the 3 possible 
permutations of the input coordinates which have the same orientation:

p0-p1 / p2    p1-p2 / p0    p2-p0 / p1     

Also, the reverse orientations should themselves be consistent, 
and be opposite in sign to the forward orientation.

The robust implementation uses DoubleDouble arithmetic and a filter to improve computation time. 
It is compared to the simple Floating-Point orientation computation, which is not robust.
--------------------------------------------------------------*/

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
#include <geos/io/WKTWriter.h>

#include <iomanip>

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

bool isConsistent(std::string tag, Coordinate p0, Coordinate p1, Coordinate p2, 
        std::function<int(Coordinate p0, Coordinate p1, Coordinate p2)> orientFunc )
{
    int orient0 = orientFunc(p0, p1, p2);
    int orient1 = orientFunc(p1, p2, p0);
    int orient2 = orientFunc(p2, p0, p1);
    bool isConsistentForward = orient0 == orient1 && orient0 == orient2;

    int orientRev0 = orientFunc(p1, p0, p2);
    int orientRev1 = orientFunc(p0, p2, p1);
    int orientRev2 = orientFunc(p2, p1, p0);
    bool isConsistentRev = orientRev0 == orientRev1 && orientRev0 == orientRev2;

    bool isConsistent = isConsistentForward && isConsistentRev;
    if (isConsistent) {
        bool isOpposite = orient0 == -orientRev0;
        isConsistent &= isOpposite;
    }

    if (isVerbose) {
        std::string consistentInd = isConsistent ? "  " : "<!";
        std::cout << tag << ": " 
            << orientSym(orient0) << orientSym(orient1) << orientSym(orient2) << " "
            << orientSym(orientRev0) << orientSym(orientRev1) << orientSym(orientRev2) << " "
            << " " << consistentInd << "  ";
    }
    return isConsistent;
}

bool isConsistentDD(Coordinate p0, Coordinate p1, Coordinate p2)
{
    return isConsistent("DD", p0, p1, p2, 
    [](Coordinate p0, Coordinate p1, Coordinate p2) -> int {
        return Orientation::index(p0, p1, p2);
    });
}

bool isConsistentFP(Coordinate p0, Coordinate p1, Coordinate p2)
{
    return isConsistent("FP", p0, p1, p2, 
        [](Coordinate p0, Coordinate p1, Coordinate p2) -> int {
            return orientationIndexFP(p0, p1, p2);
        });
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

    bool isCorrectDD = isConsistentDD(p0, p1, p2);
    bool isCorrectFP = isConsistentFP(p0, p1, p2);

    if (isVerbose) {
        std::cout << std::setprecision(20) << "   " 
            << "LINESTRING ( " << p0.x << " " << p0.y << ", " << p1.x << " " << p1.y << " )"
            << "  - " << "POINT ( " << p2.x << " " << p2.y << " )"
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