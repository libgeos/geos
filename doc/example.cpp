// This file should document by example usage of the GEOS library.
// It could actually be a live discuss-by-example board for
// architectural design choices.
//
// 			--strk;
// 
// DEBUGGING TIPS:
//  use -D__USE_MALLOC at compile time for gcc 2.91, 2.95, 3.0 and 3.1
//  and GLIBCXX_FORCE_NEW or GLIBCPP_FORCE_NEW at run time with gcc 3.2.2+
//  to force libstdc++ avoid caching memory. This should remove some
//  obscure reports from memory checkers like valgrind.
//
// $Log$
// Revision 1.10  2003/11/06 17:41:41  strk
// Added Buffer,Intersection,Difference and Symdifference. Exception cleanup
//
// Revision 1.9  2003/11/05 21:52:57  strk
// Modified example.cpp to make use of vectors instead of Geometry * / int
// couples. Added LineString creation example. Added Makefile to compile it.
//
// Revision 1.8  2003/11/03 16:09:44  strk
// Removed comments about segfaults, made the simple collection creation call
// cleaner by use of the clone() method.
//
// Revision 1.7  2003/10/29 10:38:43  strk
// Added centroid computation example
//
// Revision 1.6  2003/10/21 16:16:46  strk
// Uncommented point creation lines. Updated comments about segfaults.
//
// Revision 1.5  2003/10/20 17:50:30  strk
// added Union example
//
// Revision 1.4  2003/10/09 11:20:12  strk
// moved Log to a better place
//
// Revision 1.3  2003/10/09 11:19:20  strk
// added convexHull and PrecisionModel
//
// 
#include <stdio.h>
#include <io.h>
#include <geom.h>
#include <unload.h>

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
	Point p0;
	Coordinate c;
       	c.x = x;
	c.y = y;
	// the geometry factory will copy coordinate
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
	CoordinateList *cl = new BasicCoordinateList(4);

	// Each coordinate in the list must be created,
	// passed to coordinate list setAt and then deleted.
	// Pretty boring uh ?

	Coordinate *c;

	c = new Coordinate(xoffset, yoffset);
	cl->setAt(*c ,0);
	delete c;

	c = new Coordinate(xoffset, yoffset+side);
	cl->setAt(*c ,1);
	delete c;

	c = new Coordinate(xoffset+side, yoffset+side);
	cl->setAt(*c ,2);
	delete c;

	c = new Coordinate(xoffset+side, yoffset);
	cl->setAt(*c ,3);
	delete c;

	// Now that we have a CoordinateList we can create 
	// the linestring.
	LineString *ls = global_factory->createLineString(cl);
	
	// We don't need our CoordinateList anymore, it has been 
	// copied inside the LineString object
	delete cl;

	return ls; // our LineString
}

// This function will create a LinearRing
// geometry rapresenting a square with the given origin 
// and side 
LinearRing *
create_square_linearring(double xoffset, double yoffset, double side)
{
	// We will use a coordinate list to build the linearring
	CoordinateList *cl = new BasicCoordinateList(5);

	// Each coordinate in the list must be created,
	// passed to coordinate list setAt and then deleted.
	// Pretty boring uh ?
	Coordinate *c;
	c = new Coordinate(xoffset, yoffset);
	cl->setAt(*c ,0);
	delete c;
	c = new Coordinate(xoffset+side, yoffset);
	cl->setAt(*c ,1);
	delete c;
	c = new Coordinate(xoffset+side, yoffset+side);
	cl->setAt(*c ,2);
	delete c;
	c = new Coordinate(xoffset, yoffset+side);
	cl->setAt(*c ,3);
	delete c;
	c = new Coordinate(xoffset, yoffset);
	cl->setAt(*c ,4);
	delete c;

	// Now that we have a CoordinateList we can create 
	// the linearring.
	LinearRing *lr = global_factory->createLinearRing(cl);
	
	// We don't need our CoordinateList anymore, it has been 
	// copied inside the LinearRing object
	delete cl;

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
// This function will create a GeoemtryCollection
// containing copies of all Geometries in given vector.
// NOTE that this functionality is not available in GEOS
// where every GeometryCollection constructor will
// take ownership of Geometries passed as arguments.
//
GeometryCollection *
create_simple_collection(vector<Geometry *> *geoms)
{
	// We need to construct a <Geometry *> vector
	// to use as argument to the factory function
	//
	// This vector will have to contain COPIES of
	// passed objects since GeometryCollection
	// constructors will take ownership of them.
	//
	vector<Geometry *> *collection = new vector<Geometry *>;
	for (int i=0; i<geoms->size(); i++) {
		const Geometry *g = (*geoms)[i];
		collection->push_back(g->clone());
	}

	GeometryCollection *ret =
		global_factory->createGeometryCollection(collection);

	// We can delete vector used to store geometries
	// ( I don't like it but this is what you should do )
	delete collection;

	return ret;
}


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

	/////////////////////////////////////////////
	// BASIC GEOMETRY CREATION
	/////////////////////////////////////////////

	// Read function bodies to see the magic behind them
	geoms->push_back(create_point(150, 350));
	geoms->push_back(create_ushaped_linestring(60,60,100));
	geoms->push_back(create_square_linearring(0,0,100));
	geoms->push_back(create_square_polygon(0,200,300));
	geoms->push_back(create_square_polygon(0,250,300));
	geoms->push_back(create_simple_collection(geoms));


	// Print all geoms.
	cout<<"--------HERE ARE THE BASE GEOMS ----------"<<endl;
	wkt_print_geoms(geoms);

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
	//for (int i=1; i<2; i++) {
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
