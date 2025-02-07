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
 * Computes orientation index for a line segment and a point,
 * in a grid of contiguous FP numbers around the point.
 * Uses either DD (robust) or FP algorithms.
 * 
 * Examples:
 *    orientview  0 0   1 1   0.5 0.5
 *    orientview  0 2   2 0   0.4 1.6
 *    orientview -g   0 2   2 0   0.4 1.6
 *    orientview -g -f -v 0 2   2 0   0.4 1.6
 *    orientview -p   9 0   0 9  8.6 0.4 
 -------------------------------------------------------------*/

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>

#include <algorithm>
#include <random>
#include <vector>
#include <memory>
#include <iomanip>

using namespace geos::geom;
using geos::algorithm::Orientation;

bool isVerbose = false;
bool showGraph = false;
bool isFP = false;
bool isPermute = false;
double p0x, p0y, p1x, p1y, p2x, p2y;

void printUsage() {
    std::cout << "GEOS Orientation Index Viewer" << std::endl;
    std::cout << "Usage: orientview [ flags ] x0 y0 x1 y1 x2 y2" << std::endl;
    std::cout << "  -f - use FP instead of DD orientation algorithm" << std::endl;
    std::cout << "  -g - show grid of nearby points" << std::endl;
    std::cout << "  -p - compute all permutations of points" << std::endl;
    std::cout << "  -v - show verbose detail for each point" << std::endl;
    std::cout << "Example: orientview -g -p   0 2   2 0   0.4 1.6" << std::endl;
}

void parseFlag(char* arg) {
    char flag = arg[1];
    switch (flag) {
    case 'f': 
        isFP = true; break;
    case 'g': 
        showGraph = true; break;
    case 'p': 
        isPermute = true; break;
    case 'v': 
        isVerbose = true; break;
    }
}

void parseArgs(int argc, char** argv) {
    int i = 1;
    //-- parse flags
    while (argv[i][0] == '-' && isalpha(argv[i][1])) {
        parseFlag(argv[i]);
        i++;
    }
    //-- parse points
    if (argc - i < 6) {
        std::cerr << "3 coordinates must be specified";
        exit(EXIT_FAILURE);
    }
    // assert: there are 6 ordinates = 3 coordinates
    p0x = std::stod(argv[i]);
    p0y = std::stod(argv[i+1]);
    p1x = std::stod(argv[i+2]);
    p1y = std::stod(argv[i+3]);
    p2x = std::stod(argv[i+4]);
    p2y = std::stod(argv[i+5]);
}

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

double nextafterN(double x, double dir, size_t n) {
    for (size_t i = 0; i < n; i++) {
        x = nextafter(x, dir);
    }
    return x;
}

void computePoint(size_t ix, size_t iy, double x, double y, Coordinate p0, Coordinate p1, bool useFP, bool showDetail)
{
    Coordinate p(x, y);
    int index;
    if (useFP) {
        index = orientationIndexFP(p0, p1, p);
    }
    else {
        index = Orientation::index(p0, p1, p);
    }

    char c = '0';
    if (index < 0) c = '-';
    if (index > 0) c = '+';
    std::cout << c;

    (void)ix;
    (void)iy;
    if (showDetail) {
        std::cout << " " << (useFP ? "FP" : "DD")
            << " [ " << ix << ", " << iy << " ]  " 
            << std::setprecision(20)
            << "POINT ( " << x << " " << y << " ) -> " << index << std::endl;
    }
}

void computeGrid(double x0, double y0, double x1, double y1, double xp, double yp, size_t n) {
    Coordinate p0(x0, y0);
    Coordinate p1(x1, y1);

    std::cout << "LINESTRING ( " << x0 << " " << y0 << ", " << x1 << " " << y1 << " )" << std::endl;
    computePoint(0, 0, xp, yp, p0, p1, false, true);
    computePoint(0, 0, xp, yp, p0, p1, true, true);
    std::cout << std::endl;

    if (showGraph) {
        size_t nGrid = 2 * n + 1;
        std::cout << (isFP ? "Orientation FP" : "Orientation DD") << std::endl;

        //-- display grid limits
        double xmin = nextafterN(xp, xp - 1, n);
        double ymin = nextafterN(yp, xp - 1, n);
        double xmax = nextafterN(xp, xp + 1, n);
        double ymax = nextafterN(yp, yp + 1, n);
        std::cout << "Grid: "
            << std::setprecision(20)
            << "(" << xmin << " " << ymin << " ) - "
            << "(" << xmax << " " << ymax << " )"
            << std::endl;

        //-- display grid size for X and Y
        double xdel = nextafter(xp, xp - 1) - xp;
        double ydel = nextafter(yp, yp - 1) - yp;
        std::cout << "Size: "
            << std::setprecision(20)
            << "dX = " << xdel << "  dy = " << ydel 
            << std::endl;

        double y = nextafterN(yp, yp + 1, n);
        for (size_t iy = nGrid; iy > 0; iy--) {
            double x = nextafterN(xp, xp - 1, n);
            for (size_t ix = 0; ix <= nGrid; ix++) {
                computePoint(ix, iy, x, y, p0, p1, isFP, isVerbose);
                x = nextafter(x, x + 1);
            }
            std::cout << std::endl;
            y = nextafter(y, y - 1);
        }
    }
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        printUsage();
        exit(EXIT_SUCCESS);
    }
    parseArgs(argc, argv);
    const int gridSize = 20;
    computeGrid(p0x, p0y, p1x, p1y, p2x, p2y,     gridSize);

    if (isPermute) {
        std::cout << std::endl;
        computeGrid(p1x, p1y, p2x, p2y, p0x, p0y,     gridSize);

        std::cout << std::endl;
        computeGrid(p2x, p2y, p0x, p0y, p1x, p1y,     gridSize);
    }

    
}
