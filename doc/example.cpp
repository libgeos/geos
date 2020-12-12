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


using namespace geos;
using namespace geos::geom;
using namespace geos::operation::polygonize;
using namespace geos::operation::linemerge;
using geos::util::GEOSException;
using geos::util::IllegalArgumentException;


// Prototypes
void wkt_print_geoms(std::vector<const Geometry*>* geoms);


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
WKBtest(std::vector<const Geometry*>* geoms)
{
    std::stringstream s(std::ios_base::binary | std::ios_base::in | std::ios_base::out);
    io::WKBReader wkbReader(*global_factory);
    io::WKBWriter wkbWriter;
    Geometry* gout;

#if DEBUG_STREAM_STATE
    std::cout << "WKBtest: machine byte order: " << BYTE_ORDER << std::endl;
#endif


    std::size_t ngeoms = geoms->size();
    for(unsigned int i = 0; i < ngeoms; ++i) {
        const Geometry* gin = (*geoms)[i];

#if DEBUG_STREAM_STATE
        std::cout << "State of stream before WRITE: ";
        std::cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << std::endl;
#endif

#if DEBUG_STREAM_STATE
        std::cout << "State of stream after SEEKP: ";
        std::cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << std::endl;
#endif

        wkbWriter.write(*gin, s);
#if DEBUG_STREAM_STATE
        std::cout << "wkbWriter wrote and reached ";
        std::cout << "p:" << s.tellp() << " g:" << s.tellg() << std::endl;

        std::cout << "State of stream before DUMP: ";
        std::cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << std::endl;
#endif

#if DEBUG_STREAM_STATE
        std::cout << "State of stream after DUMP: ";
        std::cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << std::endl;
#endif

        s.seekg(0, std::ios::beg); // rewind reader pointer

#if DEBUG_STREAM_STATE
        std::cout << "State of stream before READ: ";
        std::cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << std::endl;
#endif

        gout = wkbReader.read(s).release();

#if DEBUG_STREAM_STATE
        std::cout << "State of stream after READ: ";
        std::cout << "p:" << s.tellp() << " g:" << s.tellg() <<
             " good:" << s.good() <<
             " eof:" << s.eof() <<
             " bad:" << s.bad() <<
             " fail:" << s.fail() << std::endl;
#endif

        const_cast<Geometry*>(gin)->normalize();
        gout->normalize();
        int failed = gin->compareTo(gout);
        if(failed) {
            std::cout << "{" << i << "} (WKB) ";
        }
        else {
            std::cout << "[" << i << "] (WKB) ";
        }

        io::WKBReader::printHEX(s, std::cout);
        std::cout << std::endl;

        if(failed) {
            io::WKTWriter wkt;
            std::cout << "  IN: " << wkt.write(gin) << std::endl;
            std::cout << " OUT: " << wkt.write(gout) << std::endl;
        }

        s.seekp(0, std::ios::beg); // rewind writer pointer

        delete gout;
    }

}


// This function will print given geometries in WKT
// format to stdout. As a side-effect, will test WKB
// output and input, using the WKBtest function.
void
wkt_print_geoms(std::vector<const Geometry*>* geoms)
{
    WKBtest(geoms); // test WKB parser

    // WKT-print given geometries
    io::WKTWriter* wkt = new io::WKTWriter();
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g = (*geoms)[i];
        std::string tmp = wkt->write(g);
        std::cout << "[" << i << "] (WKT) " << tmp << std::endl;
    }
    delete wkt;
}

// This is the simpler geometry you can get: a point.
Point*
create_point(double x, double y)
{
    Coordinate c(x, y);
    Point* p = global_factory->createPoint(c);
    return p;
}

// This function will create a LinearString
// geometry with the shape of the letter U
// having top-left corner at given coordinates
// and 'side' height and width
LineString*
create_ushaped_linestring(double xoffset, double yoffset, double side)
{
    // We will use a coordinate list to build the linestring
    CoordinateArraySequence* cl = new CoordinateArraySequence();

    cl->add(Coordinate(xoffset, yoffset));
    cl->add(Coordinate(xoffset, yoffset + side));
    cl->add(Coordinate(xoffset + side, yoffset + side));
    cl->add(Coordinate(xoffset + side, yoffset));

    // Now that we have a CoordinateSequence we can create
    // the linestring.
    // The newly created LineString will take ownership
    // of the CoordinateSequence.
    LineString* ls = global_factory->createLineString(cl);

    // This is what you do if you want the new LineString
    // to make a copy of your CoordinateSequence:
    // LineString *ls = global_factory->createLineString(*cl);

    return ls; // our LineString
}

// This function will create a LinearRing
// geometry representing a square with the given origin
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
// geometry representing a square with the given origin
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
    std::vector<LinearRing*>* holes = new std::vector<LinearRing*>;

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

//
// This function will create a GeometryCollection
// containing copies of all Geometries in given vector.
//
GeometryCollection*
create_simple_collection(std::vector<const Geometry*>* geoms)
{
    return global_factory->createGeometryCollection(*geoms);
    // if you wanted to transfer ownership of vector end
    // its elements you should have call:
    // return global_factory->createGeometryCollection(geoms);
}

//
// This function uses GeometricShapeFactory to render
// a circle having given center and radius
//
Polygon*
create_circle(double centerX, double centerY, double radius)
{
    geos::util::GeometricShapeFactory shapefactory(global_factory.get());
    shapefactory.setCentre(Coordinate(centerX, centerY));
    shapefactory.setSize(radius);
    // same as:
    //	shapefactory.setHeight(radius);
    //	shapefactory.setWidth(radius);
    return shapefactory.createCircle().release();
}

//
// This function uses GeometricShapeFactory to render
// an ellipse having given center and axis size
//
Polygon*
create_ellipse(double centerX, double centerY, double width, double height)
{
    geos::util::GeometricShapeFactory shapefactory(global_factory.get());
    shapefactory.setCentre(Coordinate(centerX, centerY));
    shapefactory.setHeight(height);
    shapefactory.setWidth(width);
    return shapefactory.createCircle().release();
}

//
// This function uses GeometricShapeFactory to render
// a rectangle having lower-left corner at given coordinates
// and given sizes.
//
Polygon*
create_rectangle(double llX, double llY, double width, double height)
{
    geos::util::GeometricShapeFactory shapefactory(global_factory.get());
    shapefactory.setBase(Coordinate(llX, llY));
    shapefactory.setHeight(height);
    shapefactory.setWidth(width);
    shapefactory.setNumPoints(4); // we don't need more then 4 points for a rectangle...
    // can use setSize for a square
    return shapefactory.createRectangle().release();
}

//
// This function uses GeometricShapeFactory to render
// an arc having lower-left corner at given coordinates,
// given sizes and given angles.
//
LineString*
create_arc(double llX, double llY, double width, double height, double startang, double endang)
{
    geos::util::GeometricShapeFactory shapefactory(global_factory.get());
    shapefactory.setBase(Coordinate(llX, llY));
    shapefactory.setHeight(height);
    shapefactory.setWidth(width);
    // shapefactory.setNumPoints(100); // the default (100 pts)
    // can use setSize for a square
    return shapefactory.createArc(startang, endang).release();
}

std::unique_ptr<Polygon>
create_sinestar(double cx, double cy, double size, int nArms, double armLenRat)
{
    geos::geom::util::SineStarFactory fact(global_factory.get());
    fact.setCentre(Coordinate(cx, cy));
    fact.setSize(size);
    fact.setNumPoints(nArms * 5);
    fact.setArmLengthRatio(armLenRat);
    fact.setNumArms(nArms);
    return fact.createSineStar();
}

// Start reading here
void
do_all()
{
    std::vector<const Geometry*>* geoms = new std::vector<const Geometry*>;
    std::vector<const Geometry*>* newgeoms;

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
    geoms->push_back(create_point(150, 350));
    geoms->push_back(create_square_linearring(0, 0, 100));
    geoms->push_back(create_ushaped_linestring(60, 60, 100));
    geoms->push_back(create_square_linearring(0, 0, 100));
    geoms->push_back(create_square_polygon(0, 200, 300));
    geoms->push_back(create_square_polygon(0, 250, 300));
    geoms->push_back(create_simple_collection(geoms));

#if GEOMETRIC_SHAPES
    // These ones use a GeometricShapeFactory
    geoms->push_back(create_circle(0, 0, 10));
    geoms->push_back(create_ellipse(0, 0, 8, 12));
    geoms->push_back(create_rectangle(-5, -5, 10, 10)); // a square
    geoms->push_back(create_rectangle(-5, -5, 10, 20)); // a rectangle
    // The upper-right quarter of a vertical ellipse
    geoms->push_back(create_arc(0, 0, 10, 20, 0, MATH_PI / 2));
    geoms->push_back(create_sinestar(10, 10, 100, 5, 2).release()); // a sine star
#endif

    // Print all geoms.
    std::cout << "--------HERE ARE THE BASE GEOMS ----------" << std::endl;
    wkt_print_geoms(geoms);


#if UNARY_OPERATIONS

////////////////////////////////////////////////////////////////////////
// UNARY OPERATIONS
////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////
    // CENTROID
    /////////////////////////////////////////////

    // Find centroid of each base geometry
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g = (*geoms)[i];
        newgeoms->push_back(g->getCentroid().release());
    }

    // Print all convex hulls
    std::cout << std::endl << "------- AND HERE ARE THEIR CENTROIDS -----" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the centroids
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

    /////////////////////////////////////////////
    // BUFFER
    /////////////////////////////////////////////

    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g = (*geoms)[i];
        try {
            Geometry* g2 = g->buffer(10).release();
            newgeoms->push_back(g2);
        }
        catch(const GEOSException& exc) {
            std::cerr << "GEOS Exception: geometry " << i << "->buffer(10): " << exc.what() << "\n";
        }
    }

    std::cout << std::endl << "--------HERE COMES THE BUFFERED GEOMS ----------" << std::endl;
    wkt_print_geoms(newgeoms);

    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

    /////////////////////////////////////////////
    // CONVEX HULL
    /////////////////////////////////////////////

    // Make convex hulls of geometries
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g = (*geoms)[i];
        newgeoms->push_back(g->convexHull().release());
    }

    // Print all convex hulls
    std::cout << std::endl << "--------HERE COMES THE HULLS----------" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the hulls
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

#endif // UNARY_OPERATIONS

#if RELATIONAL_OPERATORS

////////////////////////////////////////////////////////////////////////
// RELATIONAL OPERATORS
////////////////////////////////////////////////////////////////////////

    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "RELATIONAL OPERATORS" << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;

    /////////////////////////////////////////////
    // DISJOINT
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "   DISJOINT   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->disjoint(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // TOUCHES
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "    TOUCHES   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->touches(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // INTERSECTS
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << " INTERSECTS   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->intersects(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // CROSSES
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "    CROSSES   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->crosses(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // WITHIN
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "     WITHIN   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->within(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // CONTAINS
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "   CONTAINS   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->contains(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // OVERLAPS
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "   OVERLAPS   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->overlaps(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // RELATE
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "     RELATE   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                // second argument is intersectionPattern
                std::string pattern = "212101212";
                if(g1->relate(g2, pattern)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }

                // get the intersectionMatrix itself
                auto im = g1->relate(g2);
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // EQUALS
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "     EQUALS   ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                if(g1->equals(g2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // EQUALS_EXACT
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "EQUALS_EXACT  ";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                // second argument is a tolerance
                if(g1->equalsExact(g2, 0.5)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    /////////////////////////////////////////////
    // IS_WITHIN_DISTANCE
    /////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "IS_WITHIN_DIST";
    for(unsigned int i = 0; i < geoms->size(); i++) {
        std::cout << "\t[" << i << "]";
    }
    std::cout << std::endl;
    for(unsigned int i = 0; i < geoms->size(); i++) {
        const Geometry* g1 = (*geoms)[i];
        std::cout << "      [" << i << "]\t";
        for(unsigned int j = 0; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                // second argument is the distance
                if(g1->isWithinDistance(g2, 2)) {
                    std::cout << " 1\t";
                }
                else {
                    std::cout << " 0\t";
                }
            }
            // Geometry Collection is not a valid argument
            catch(const IllegalArgumentException&) {
                std::cout << " X\t";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }

#endif // RELATIONAL_OPERATORS

#if COMBINATIONS

////////////////////////////////////////////////////////////////////////
// COMBINATIONS
////////////////////////////////////////////////////////////////////////

    std::cout << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "COMBINATIONS" << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;

    /////////////////////////////////////////////
    // UNION
    /////////////////////////////////////////////

    // Make unions of all geoms
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < geoms->size() - 1; i++) {
        const Geometry* g1 = (*geoms)[i];
        for(unsigned int j = i + 1; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                Geometry* g3 = g1->Union(g2).release();
                newgeoms->push_back(g3);
            }
            // It's illegal to union a collection ...
            catch(const IllegalArgumentException&) {
                //std::cerr <<ill.toString()<<"\n";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
    }

    // Print all unions
    std::cout << std::endl << "----- AND HERE ARE SOME UNION COMBINATIONS ------" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the resulting geoms
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;


    /////////////////////////////////////////////
    // INTERSECTION
    /////////////////////////////////////////////

    // Compute intersection of adjacent geometries
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < geoms->size() - 1; i++) {
        const Geometry* g1 = (*geoms)[i];
        for(unsigned int j = i + 1; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                Geometry* g3 = g1->intersection(g2).release();
                newgeoms->push_back(g3);
            }
            // Collection are illegal as intersection argument
            catch(const IllegalArgumentException&) {
                //std::cerr <<ill.toString()<<"\n";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
    }

    std::cout << std::endl << "----- HERE ARE SOME INTERSECTIONS COMBINATIONS ------" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the resulting geoms
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

    /////////////////////////////////////////////
    // DIFFERENCE
    /////////////////////////////////////////////

    // Compute difference of adjacent geometries
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < geoms->size() - 1; i++) {
        const Geometry* g1 = (*geoms)[i];
        for(unsigned int j = i + 1; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                Geometry* g3 = g1->difference(g2).release();
                newgeoms->push_back(g3);
            }
            // Collection are illegal as difference argument
            catch(const IllegalArgumentException&) {
                //std::cerr <<ill.toString()<<"\n";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
    }

    std::cout << std::endl << "----- HERE ARE SOME DIFFERENCE COMBINATIONS ------" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the resulting geoms
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

    /////////////////////////////////////////////
    // SYMMETRIC DIFFERENCE
    /////////////////////////////////////////////

    // Compute symmetric difference of adjacent geometries
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < geoms->size() - 1; i++) {
        const Geometry* g1 = (*geoms)[i];
        for(unsigned int j = i + 1; j < geoms->size(); j++) {
            const Geometry* g2 = (*geoms)[j];
            try {
                Geometry* g3 = g1->symDifference(g2).release();
                newgeoms->push_back(g3);
            }
            // Collection are illegal as symdifference argument
            catch(const IllegalArgumentException&) {
                //std::cerr <<ill.toString()<<"\n";
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
    }

    std::cout << std::endl << "----- HERE ARE SYMMETRIC DIFFERENCES ------" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the resulting geoms
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

#endif // COMBINATIONS

#if LINEMERGE

    /////////////////////////////////////////////
    // LINEMERGE
    /////////////////////////////////////////////
    LineMerger lm;
    lm.add(geoms);
    auto mls = lm.getMergedLineStrings();
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < mls.size(); i++) {
        newgeoms->push_back(mls[i].release());
    }

    std::cout << std::endl << "----- HERE IS THE LINEMERGE OUTPUT ------" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the resulting geoms
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

#endif // LINEMERGE

#if POLYGONIZE

    /////////////////////////////////////////////
    // POLYGONIZE
    /////////////////////////////////////////////
    Polygonizer plgnzr;
    plgnzr.add(geoms);
    auto polys = plgnzr.getPolygons();
    newgeoms = new std::vector<const Geometry*>;
    for(unsigned int i = 0; i < polys.size(); i++) {
        newgeoms->push_back(polys[i].release());
    }

    std::cout << std::endl << "----- HERE IS POLYGONIZE OUTPUT ------" << std::endl;
    wkt_print_geoms(newgeoms);

    // Delete the resulting geoms
    for(unsigned int i = 0; i < newgeoms->size(); i++) {
        delete(*newgeoms)[i];
    }
    delete newgeoms;

#endif // POLYGONIZE

    /////////////////////////////////////////////
    // CLEANUP
    /////////////////////////////////////////////

    // Delete base geometries
    for(unsigned int i = 0; i < geoms->size(); i++) {
        delete(*geoms)[i];
    }
    delete geoms;
}

int
main()
{
    std::cout << "GEOS " << geosversion() << " ported from JTS " << jtsport() << std::endl;
    try {
        do_all();
    }
    // All exception thrown by GEOS are subclasses of this
    // one, so this is a catch-all
    catch(const GEOSException& exc) {
        std::cerr << "GEOS Exception: " << exc.what() << "\n";
        exit(1);
    }
    catch(const std::exception& e) {
        std::cerr << "Standard exception thrown: " << e.what() << std::endl;
        exit(1);
    }
    // and this is a catch-all non standard ;)
    catch(...) {
        std::cerr << "unknown exception trown!\n";
        exit(1);
    }

    // Unload is no more necessary
    //io::Unload::Release();

    exit(0);
}
