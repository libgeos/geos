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
 **********************************************************************
 * $Log$
 * Revision 1.22  2004/07/16 10:00:45  strk
 * Bug fixed in GeometricShapeFactory examples.
 * Added example of GeometricShapeFactory::createArc.
 *
 * Revision 1.21  2004/07/15 13:41:54  strk
 * Added createRectangle example.
 *
 * Revision 1.20  2004/07/14 21:15:49  strk
 * Added GeometricShapeFactory example: createCircle.
 * Added simple filter to send example output to a postgis table.
 *
 * Revision 1.19  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.18  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.17  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.16  2004/06/15 20:42:43  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.15  2004/04/13 14:28:37  strk
 * Removed spurious line
 *
 * Revision 1.14  2003/11/13 11:57:23  strk
 * bug fixed in relate call
 *
 * Revision 1.13  2003/11/12 22:03:54  strk
 * added relational operators
 *
 * Revision 1.12  2003/11/12 11:08:39  strk
 * removed old changelog, moved comments in the nice standard frame
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
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

#include <stdio.h>
#include <geos.h>

using namespace geos;

// This object will be used to construct our geometries.
// It might be bypassed by directly call geometry constructors,
// but that would be boring because you'd need to specify
// a PrecisionModel and a SRID everytime: those infos are
// cached inside a GeometryFactory object.
GeometryFactory *global_factory;

// This function will print given geometries in WKT
// format to stdout.
void
wkt_print_geoms(vector<Geometry *> *geoms)
{
	// WKT-print given geometries
	WKTWriter *wkt = new WKTWriter();
	for (int i=0; i<geoms->size(); i++) {
		const Geometry *g = (*geoms)[i];
		string tmp=wkt->write(g);
		cout<<"["<<i<<"] "<<tmp<<endl;
	}
	delete wkt;
}

// This is the simpler geometry you can get: a point.
Point *
create_point(double x, double y)
{
	Coordinate c(x, y);
	Point *p = global_factory->createPoint(c);
	return p;
}

// This function will create a LinearString
// geometry with the shape of the letter U
// having top-left corner at given coordinates
// and 'side' height and width 
LineString *
create_ushaped_linestring(double xoffset, double yoffset, double side)
{
	// We will use a coordinate list to build the linestring
	CoordinateSequence *cl = new DefaultCoordinateSequence();

	cl->add(Coordinate(xoffset, yoffset));
	cl->add(Coordinate(xoffset, yoffset+side));
	cl->add(Coordinate(xoffset+side, yoffset+side));
	cl->add(Coordinate(xoffset+side, yoffset));

	// Now that we have a CoordinateSequence we can create 
	// the linestring.
	// The newly created LineString will take ownership
	// of the CoordinateSequence.
	LineString *ls = global_factory->createLineString(cl);

	// This is what you do if you want the new LineString
	// to make a copy of your CoordinateSequence:
	// LineString *ls = global_factory->createLineString(*cl);
	
	return ls; // our LineString
}

// This function will create a LinearRing
// geometry rapresenting a square with the given origin 
// and side 
LinearRing *
create_square_linearring(double xoffset, double yoffset, double side)
{
	// We will use a coordinate list to build the linearring
	CoordinateSequence *cl = new DefaultCoordinateSequence();

	cl->add(Coordinate(xoffset, yoffset));
	cl->add(Coordinate(xoffset, yoffset+side));
	cl->add(Coordinate(xoffset+side, yoffset+side));
	cl->add(Coordinate(xoffset+side, yoffset));
	cl->add(Coordinate(xoffset, yoffset));

	// Now that we have a CoordinateSequence we can create 
	// the linearring.
	// The newly created LinearRing will take ownership
	// of the CoordinateSequence.
	LinearRing *lr = global_factory->createLinearRing(cl);

	// This is what you do if you want the new LinearRing
	// to make a copy of your CoordinateSequence:
	// LinearRing *lr = global_factory->createLinearRing(*cl);
	
	return lr; // our LinearRing
}

// This function will create a Polygon
// geometry rapresenting a square with the given origin 
// and side and with a central hole 1/3 sided.
Polygon *
create_square_polygon(double xoffset, double yoffset, double side)
{
	// We need a LinearRing for the polygon shell 
	LinearRing *outer = create_square_linearring(xoffset,yoffset,side);

	// And another for the hole 
	LinearRing *inner = create_square_linearring(xoffset+(side/3),
			yoffset+(side/3),(side/3));
	
	// If we need to specify any hole, we do it using
	// a vector of Geometry pointers (I don't know why
	// not LinearRings)
	vector<Geometry *> *holes = new vector<Geometry *>;

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
	Polygon *poly = global_factory->createPolygon(outer, holes);

	return poly;
}

//
// This function will create a GeometryCollection
// containing copies of all Geometries in given vector.
//
GeometryCollection *
create_simple_collection(vector<Geometry *> *geoms)
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
Polygon *
create_circle(double centerX, double centerY, double radius)
{
	GeometricShapeFactory shapefactory(global_factory);
	shapefactory.setCentre(Coordinate(centerX, centerY));
	shapefactory.setSize(radius);
	// same as:
	//	shapefactory.setHeight(radius);
	//	shapefactory.setWidth(radius);
	return shapefactory.createCircle();
};

//
// This function uses GeometricShapeFactory to render
// an ellipse having given center and axis size
//
Polygon *
create_ellipse(double centerX, double centerY, double width, double height)
{
	GeometricShapeFactory shapefactory(global_factory);
	shapefactory.setCentre(Coordinate(centerX, centerY));
	shapefactory.setHeight(height);
	shapefactory.setWidth(width);
	return shapefactory.createCircle();
};

//
// This function uses GeometricShapeFactory to render
// a rectangle having lower-left corner at given coordinates
// and given sizes.
//
Polygon *
create_rectangle(double llX, double llY, double width, double height)
{
	GeometricShapeFactory shapefactory(global_factory);
	shapefactory.setBase(Coordinate(llX, llY));
	shapefactory.setHeight(height);
	shapefactory.setWidth(width);
	shapefactory.setNumPoints(4); // we don't need more then 4 points for a rectangle...
	// can use setSize for a square
	return shapefactory.createRectangle();
};

//
// This function uses GeometricShapeFactory to render
// an arc having lower-left corner at given coordinates,
// given sizes and given angles. 
//
LineString *
create_arc(double llX, double llY, double width, double height, double startang, double endang)
{
	GeometricShapeFactory shapefactory(global_factory);
	shapefactory.setBase(Coordinate(llX, llY));
	shapefactory.setHeight(height);
	shapefactory.setWidth(width);
	// shapefactory.setNumPoints(100); // the default (100 pts)
	// can use setSize for a square
	return shapefactory.createArc(startang, endang);
};

// Start reading here
void do_all()
{
	vector<Geometry *> *geoms = new vector<Geometry *>;
	vector<Geometry *> *newgeoms;
	Geometry *geom;

	// Define a precision model using 0,0 as the reference origin
	// and 2.0 as coordinates scale.
	PrecisionModel *pm = new PrecisionModel(2.0, 0, 0);

	// Initialize global factory with defined PrecisionModel
	// and a SRID of -1 (undefined).
	global_factory = new GeometryFactory(pm, -1);

	// We do not need PrecisionMode object anymore, it has
	// been copied to global_factory private storage
	delete pm;

////////////////////////////////////////////////////////////////////////
// GEOMETRY CREATION
////////////////////////////////////////////////////////////////////////

	// Read function bodies to see the magic behind them
	geoms->push_back(create_point(150, 350));
	geoms->push_back(create_ushaped_linestring(60,60,100));
	geoms->push_back(create_square_linearring(0,0,100));
	geoms->push_back(create_square_polygon(0,200,300));
	geoms->push_back(create_square_polygon(0,250,300));
	geoms->push_back(create_simple_collection(geoms));

	// These ones use a GeometricShapeFactory
	geoms->push_back(create_circle(0, 0, 10));
	geoms->push_back(create_ellipse(0, 0, 8, 12));
	geoms->push_back(create_rectangle(-5, -5, 10, 10)); // a square
	geoms->push_back(create_rectangle(-5, -5, 10, 20)); // a rectangle
	// The upper-right quarter of a vertical ellipse
	geoms->push_back(create_arc(0, 0, 10, 20, 0, M_PI/2));

	// Print all geoms.
	cout<<"--------HERE ARE THE BASE GEOMS ----------"<<endl;
	wkt_print_geoms(geoms);


////////////////////////////////////////////////////////////////////////
// UNARY OPERATIONS
////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////
	// CENTROID
	/////////////////////////////////////////////
	
	// Find centroid of each base geometry
	newgeoms = new vector<Geometry *>;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g = (*geoms)[i];
		newgeoms->push_back( g->getCentroid() );
	}

	// Print all convex hulls
	cout<<endl<<"------- AND HERE ARE THEIR CENTROIDS -----"<<endl;
	wkt_print_geoms(newgeoms);

	// Delete the centroids
	for (int i=0; i<newgeoms->size(); i++) {
		delete (*newgeoms)[i];
	}
	delete newgeoms;
	
	/////////////////////////////////////////////
	// BUFFER
	/////////////////////////////////////////////
	
	newgeoms = new vector<Geometry *>;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g = (*geoms)[i];
		try {
			Geometry *g2 = g->buffer(10);
			newgeoms->push_back(g2);
		}
		catch (GEOSException *exc) {
			cerr <<"GEOS Exception: geometry "<<i<<"->buffer(10): "<<exc->toString()<<"\n";
			delete exc;
		}
	}

	cout<<endl<<"--------HERE COMES THE BUFFERED GEOMS ----------"<<endl;
	wkt_print_geoms(newgeoms);

	for (int i=0; i<newgeoms->size(); i++) {
		delete (*newgeoms)[i];
	}
	delete newgeoms;
	
	/////////////////////////////////////////////
	// CONVEX HULL
	/////////////////////////////////////////////
	
	// Make convex hulls of geometries
	newgeoms = new vector<Geometry *>;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g = (*geoms)[i];
		newgeoms->push_back( g->convexHull() );
	}

	// Print all convex hulls
	cout<<endl<<"--------HERE COMES THE HULLS----------"<<endl;
	wkt_print_geoms(newgeoms);

	// Delete the hulls
	for (int i=0; i<newgeoms->size(); i++) {
		delete (*newgeoms)[i];
	}
	delete newgeoms;

////////////////////////////////////////////////////////////////////////
// RELATIONAL OPERATORS
////////////////////////////////////////////////////////////////////////

cout<<"-------------------------------------------------------------------------------"<<endl;
cout<<"RELATIONAL OPERATORS"<<endl;
cout<<"-------------------------------------------------------------------------------"<<endl;

	/////////////////////////////////////////////
	// DISJOINT
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"   DISJOINT   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->disjoint(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}
	
	/////////////////////////////////////////////
	// TOUCHES
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"    TOUCHES   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->touches(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// INTERSECTS
	/////////////////////////////////////////////

	cout<<endl;
	cout<<" INTERSECTS   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->intersects(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// CROSSES
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"    CROSSES   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->crosses(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// WITHIN
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"     WITHIN   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->within(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// CONTAINS
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"   CONTAINS   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->contains(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// OVERLAPS
	/////////////////////////////////////////////
	
	cout<<endl;
	cout<<"   OVERLAPS   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->overlaps(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// RELATE
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"     RELATE   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			IntersectionMatrix *im=NULL;
			try {
				// second argument is intersectionPattern
				if ( g1->relate(g2, "212101212") ) cout<<" 1\t";
				else cout<<" 0\t";
				
				// get the intersectionMatrix itself
				im=g1->relate(g2);
				delete im; // delete afterwards
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// EQUALS
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"     EQUALS   ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				if ( g1->equals(g2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// EQUALS_EXACT
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"EQUALS_EXACT  ";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				// second argument is a tolerance
				if ( g1->equalsExact(g2, 0.5) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

	/////////////////////////////////////////////
	// IS_WITHIN_DISTANCE
	/////////////////////////////////////////////

	cout<<endl;
	cout<<"IS_WITHIN_DIST";
	for (int i=0; i<geoms->size(); i++) {
		cout<<"\t["<<i<<"]";
	}
	cout<<endl;
	for (int i=0; i<geoms->size(); i++) {
		Geometry *g1 = (*geoms)[i];
		cout<<"      ["<<i<<"]\t";
		for (int j=0; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				// second argument is the distance
				if ( g1->isWithinDistance(g2,2) ) cout<<" 1\t";
				else cout<<" 0\t";
			}
			// Geometry Collection is not a valid argument
			catch (IllegalArgumentException *exc) {
				cout<<" X\t";
				delete exc;
			}
			catch (GEOSException *exc) {
				cerr<<exc->toString()<<endl;
				delete exc;
			}
		}
		cout<<endl;
	}

////////////////////////////////////////////////////////////////////////
// COMBINATIONS
////////////////////////////////////////////////////////////////////////

cout<<endl;
cout<<"-------------------------------------------------------------------------------"<<endl;
cout<<"COMBINATIONS"<<endl;
cout<<"-------------------------------------------------------------------------------"<<endl;

	/////////////////////////////////////////////
	// UNION
	/////////////////////////////////////////////

	// Make unions of all geoms
	newgeoms = new vector<Geometry *>;
	for (int i=0; i<geoms->size()-1; i++) {
		Geometry *g1 = (*geoms)[i];
		for (int j=i+1; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				Geometry *g3 = g1->Union(g2);
				newgeoms->push_back(g3);
			}
			// It's illegal to union a collection ...
			catch (IllegalArgumentException *ill) {
				//cerr <<ill->toString()<<"\n";
				delete ill;
			}
			catch (GEOSException *exc) {
				cerr <<"GEOS Exception: "<<exc->toString()<<"\n";
				delete exc;
			}
		}
	}

	// Print all unions
	cout<<endl<<"----- AND HERE ARE SOME UNION COMBINATIONS ------"<<endl;
	wkt_print_geoms(newgeoms);

	// Delete the resulting geoms
	for (int i=0; i<newgeoms->size(); i++) {
		delete (*newgeoms)[i];
	}
	delete newgeoms;
	

	/////////////////////////////////////////////
	// INTERSECTION
	/////////////////////////////////////////////
	
	// Compute intersection of adhiacent geometries
	newgeoms = new vector<Geometry *>;
	for (int i=0; i<geoms->size()-1; i++) {
		Geometry *g1 = (*geoms)[i];
		for (int j=i+1; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				Geometry *g3 = g1->intersection(g2);
				newgeoms->push_back(g3);
			}
			// Collection are illegal as intersection argument
			catch (IllegalArgumentException *ill) {
				//cerr <<ill->toString()<<"\n";
				delete ill;
			}
			catch (GEOSException *exc) {
				cerr <<"GEOS Exception: "<<exc->toString()<<"\n";
				delete exc;
			}
		}
	}

	cout<<endl<<"----- HERE ARE SOME INTERSECTIONS COMBINATIONS ------"<<endl;
	wkt_print_geoms(newgeoms);

	// Delete the resulting geoms
	for (int i=0; i<newgeoms->size(); i++) {
		delete (*newgeoms)[i];
	}
	delete newgeoms;

	/////////////////////////////////////////////
	// DIFFERENCE
	/////////////////////////////////////////////
	
	// Compute difference of adhiacent geometries
	newgeoms = new vector<Geometry *>;
	for (int i=0; i<geoms->size()-1; i++) {
		Geometry *g1 = (*geoms)[i];
		for (int j=i+1; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				Geometry *g3 = g1->difference(g2);
				newgeoms->push_back(g3);
			}
			// Collection are illegal as difference argument
			catch (IllegalArgumentException *ill) {
				//cerr <<ill->toString()<<"\n";
				delete ill;
			}
			catch (GEOSException *exc) {
				cerr <<"GEOS Exception: "<<exc->toString()<<"\n";
				delete exc;
			}
		}
	}

	cout<<endl<<"----- HERE ARE SOME DIFFERENCE COMBINATIONS ------"<<endl;
	wkt_print_geoms(newgeoms);

	// Delete the resulting geoms
	for (int i=0; i<newgeoms->size(); i++) {
		delete (*newgeoms)[i];
	}
	delete newgeoms;
	
	/////////////////////////////////////////////
	// SYMMETRIC DIFFERENCE
	/////////////////////////////////////////////
	
	// Compute symmetric difference of adhiacent geometries
	newgeoms = new vector<Geometry *>;
	for (int i=0; i<geoms->size()-1; i++) {
		Geometry *g1 = (*geoms)[i];
		for (int j=i+1; j<geoms->size(); j++) {
			Geometry *g2 = (*geoms)[j];
			try {
				Geometry *g3 = g1->symDifference(g2);
				newgeoms->push_back(g3);
			}
			// Collection are illegal as symdifference argument
			catch (IllegalArgumentException *ill) {
				//cerr <<ill->toString()<<"\n";
				delete ill;
			}
			catch (GEOSException *exc) {
				cerr <<"GEOS Exception: "<<exc->toString()<<"\n";
				delete exc;
			}
		}
	}

	cout<<endl<<"----- HERE ARE SYMMETRIC DIFFERENCES ------"<<endl;
	wkt_print_geoms(newgeoms);

	// Delete the resulting geoms
	for (int i=0; i<newgeoms->size(); i++) {
		delete (*newgeoms)[i];
	}
	delete newgeoms;

	/////////////////////////////////////////////
	// CLEANUP
	/////////////////////////////////////////////

	// Delete base geometries 
	for (int i=0; i<geoms->size(); i++) {
		delete (*geoms)[i];
	}
	delete geoms;

	delete global_factory;
}

main()
{
	try
	{
		do_all();
	}
	// All exception thrown by GEOS are subclasses of this
	// one, so this is a catch-all 
	catch (GEOSException *exc)
	{
		cerr <<"GEOS Exception: "<<exc->toString()<<"\n";
		delete exc;
		exit(1);
	}
	// and this is a catch-all non standard ;)
	catch (...)
	{
		cerr <<"unknown exception trown!\n";
		exit(1);
	}

	// This is not really needed but to make
	// memory checker like valgrind quiet
	// about static heap-allocated data.
	Unload::Release();
}
