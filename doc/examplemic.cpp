/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *********************************************************************
 *
 * This file should document by example usage of the GEOS library.
 * It could actually be a live discuss-by-example board for
 * architectural design choices.
 *
 * 			--strk;
 *
 * DEBUGGING TIPS:
 *  use -D__USE_MALLOC at compile time for gcc 2.91, 2.95, 3.0 and 3.1
 *  and GLIBCXX_FORCE_NEW or GLIBCPP_FORCE_NEW at run time with gcc 3.2.2+
 *  to force libstdc++ avoid caching memory. This should remove some
 *  obscure reports from memory checkers like valgrind.
 *
 **********************************************************************/

#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTWriter.h>
#include <geos/util/GeometricShapeFactory.h>
#include <geos/geom/util/SineStarFactory.h>
#include <geos/util/GEOSException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/constants.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib> // exit()

// Set to 0 to skip section
#define GEOMETRIC_SHAPES 1
#define RELATIONAL_OPERATORS 1
#define COMBINATIONS 1
#define UNARY_OPERATIONS 1
#define LINEMERGE 1
#define POLYGONIZE 1


using namespace std;
using namespace geos;
using namespace geos::geom;
using namespace geos::operation::polygonize;
using namespace geos::operation::linemerge;
using geos::util::GEOSException;
using geos::util::IllegalArgumentException;


// Prototypes
void wkt_print_geoms(vector<const Geometry*>* geoms);
void wkt_print_ring_coordinates(vector<const Polygon*>* polys);


// This object will be used to construct our geometries.
// It might be bypassed by directly call geometry constructors,
// but that would be boring because you'd need to specify
// a PrecisionModel and a SRID everytime: those infos are
// cached inside a GeometryFactory object.
GeometryFactory::Ptr global_factory;

//#define DEBUG_STREAM_STATE 1


//
// This function tests writing and reading WKB
// TODO:
//	- compare input and output geometries for equality
//	- remove debugging lines (on stream state)
//
void
WKBtest(vector<const Geometry*>* geoms)
{
    stringstream s(ios_base::binary | ios_base::in | ios_base::out);
    io::WKBReader wkbReader(*global_factory);
    io::WKBWriter wkbWriter;
    Geometry* gout;

#if DEBUG_STREAM_STATE
    cout << "WKBtest: machine byte order: " << BYTE_ORDER << endl;
#endif


    size_t ngeoms = geoms->size();
    for(unsigned int i = 0; i < ngeoms; ++i) {
        const Geometry* gin = (*geoms)[i];

#if DEBUG_STREAM_STATE
        cout << "State of stream before WRITE: ";
        cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << endl;
#endif

#if DEBUG_STREAM_STATE
        cout << "State of stream after SEEKP: ";
        cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << endl;
#endif

        wkbWriter.write(*gin, s);
#if DEBUG_STREAM_STATE
        cout << "wkbWriter wrote and reached ";
        cout << "p:" << s.tellp() << " g:" << s.tellg() << endl;

        cout << "State of stream before DUMP: ";
        cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << endl;
#endif

#if DEBUG_STREAM_STATE
        cout << "State of stream after DUMP: ";
        cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << endl;
#endif

        s.seekg(0, ios::beg); // rewind reader pointer

#if DEBUG_STREAM_STATE
        cout << "State of stream before READ: ";
        cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << endl;
#endif

        gout = wkbReader.read(s).release();

#if DEBUG_STREAM_STATE
        cout << "State of stream after READ: ";
        cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << endl;
#endif

        const_cast<Geometry*>(gin)->normalize();
        gout->normalize();
        int failed = gin->compareTo(gout);
        if(failed) {
            cout << "{" << i << "} (WKB) ";
        }
        else {
            cout << "[" << i << "] (WKB) ";
        }

        io::WKBReader::printHEX(s, cout);
        cout << endl;

        if(failed) {
            io::WKTWriter wkt;
            cout << "  IN: " << wkt.write(gin) << endl;
            cout << " OUT: " << wkt.write(gout) << endl;
        }

        s.seekp(0, ios::beg); // rewind writer pointer

        delete gout;
    }

}


// This function will print given geometries in WKT
// format to stdout. As a side-effect, will test WKB
// output and input, using the WKBtest function.
void
wkt_print_geoms(vector<const Geometry*>* geoms)
{
    WKBtest(geoms); // test WKB parser

    // WKT-print given geometries
    io::WKTWriter* wkt = new io::WKTWriter();
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g = (*geoms)[i];
        string tmp = wkt->write(g);
        cout << "[" << i << "] (WKT) " << tmp << endl;
    }
    delete wkt;
}

void
wkt_print_ring_coordinates(vector<const Polygon*>* polys)
{
    // WKBtest(polys); // test WKB parser

    // WKT-print coordinates of given geometries
    io::WKTWriter* wkt = new io::WKTWriter();
    for(unsigned int i = 0; i < polys->size(); i++) {
        const Polygon* p = (*polys)[i];
        const LineString* exterior = p->getExteriorRing();
        string tmp = exterior->toString();
        cout << "[" << i << "] (Exterior Ring) " << tmp << endl;
        for(unsigned int j = 0; j < p->getNumInteriorRing(); j++) {
            const LineString* interior = p->getInteriorRingN(j);
            string tmp = interior->toString();
            cout << "[" << i << "." << j <<"] (Interior Ring) " << tmp << endl;
        }
    }
    delete wkt;
}

// This function will create a LinearRing
// geometry rapresenting a square with the given origin
// and side
LinearRing*
create_square_linearring(double xoffset, double yoffset, double side)
{
    // We will use a coordinate list to build the linearring
    CoordinateArraySequence* cl = new CoordinateArraySequence();

    cl->add(Coordinate(xoffset, yoffset));
    cl->add(Coordinate(xoffset, yoffset + side));
    cl->add(Coordinate(xoffset + side, yoffset + side));
    cl->add(Coordinate(xoffset + side, yoffset));
    cl->add(Coordinate(xoffset, yoffset));

    // Now that we have a CoordinateSequence we can create
    // the linearring.
    // The newly created LinearRing will take ownership
    // of the CoordinateSequence.
    LinearRing* lr = global_factory->createLinearRing(cl);

    // This is what you do if you want the new LinearRing
    // to make a copy of your CoordinateSequence:
    // LinearRing *lr = global_factory->createLinearRing(*cl);

    return lr; // our LinearRing
}

// This function will create a Polygon
// geometry rapresenting a square with the given origin
// and side and with a central hole 1/3 sided.
Polygon*
create_square_polygon(double xoffset, double yoffset, double side)
{
    // We need a LinearRing for the polygon shell
    LinearRing* outer = create_square_linearring(xoffset, yoffset, side);

    // And another for the hole
    LinearRing* inner = create_square_linearring(xoffset + (side / 3),
                        yoffset + (side / 3), (side / 3));

    // If we need to specify any hole, we do it using
    // a vector of LinearRing pointers
    vector<LinearRing*>* holes = new vector<LinearRing*>;

    // We add the newly created geometry to the vector
    // of holes.
    holes->push_back(inner);

    // And finally we call the polygon constructor.
    // Both the outer LinearRing and the vector of holes
    // will be referenced by the resulting Polygon object,
    // thus we CANNOT delete them, neither the holes, nor
    // the vector containing their pointers, nor the outer
    // LinearRing. Everything will be deleted at Polygon
    // deletion time (this is inconsistent with LinearRing
    // behaviour... what should we do?).
    Polygon* poly = global_factory->createPolygon(outer, holes);

    return poly;
}

// Start reading here
void
do_all()
{
    vector<const Geometry*>* geoms = new vector<const Geometry*>;
    vector<const Polygon*>* polys = new vector<const Polygon*>;
    vector<const Geometry*>* newgeoms;

    // Define a precision model using 0,0 as the reference origin
    // and 2.0 as coordinates scale.
    PrecisionModel* pm = new PrecisionModel(2.0, 0, 0);

    // Initialize global factory with defined PrecisionModel
    // and a SRID of -1 (undefined).
    global_factory = GeometryFactory::create(pm, -1);

    // We do not need PrecisionMode object anymore, it has
    // been copied to global_factory private storage
    delete pm;

////////////////////////////////////////////////////////////////////////
// GEOMETRY CREATION
////////////////////////////////////////////////////////////////////////

    // Read function bodies to see the magic behind them
    geoms->push_back(create_square_polygon(0, 0, 30));
    polys->push_back(create_square_polygon(0, 0, 30));

    // Print all geoms.
    cout << "--------HERE ARE THE BASE GEOMS ----------" << endl;
    wkt_print_geoms(geoms);

    // Print ring coordinates
    cout << "--------HERE ARE THE RING COORDINATES ----------" << endl;
    wkt_print_ring_coordinates(polys);

    /////////////////////////////////////////////
    // CLEANUP
    /////////////////////////////////////////////

    // Delete base geometries
    for(unsigned int i = 0; i < geoms->size(); i++) {
        delete(*geoms)[i];
    }
    delete geoms;

    // Delete base polygons
    for(unsigned int i = 0; i < polys->size(); i++) {
        delete(*polys)[i];
    }
    delete polys;
}

int
main()
{
    cout << "GEOS " << geosversion() << " ported from JTS " << jtsport() << endl;
    try {
        do_all();
    }
    // All exception thrown by GEOS are subclasses of this
    // one, so this is a catch-all
    catch(const GEOSException& exc) {
        cerr << "GEOS Exception: " << exc.what() << "\n";
        exit(1);
    }
    catch(const exception& e) {
        cerr << "Standard exception thrown: " << e.what() << endl;
        exit(1);
    }
    // and this is a catch-all non standard ;)
    catch(...) {
        cerr << "unknown exception trown!\n";
        exit(1);
    }

    // Unload is no more necessary
    //io::Unload::Release();

    exit(0);
}
