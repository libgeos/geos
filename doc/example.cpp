// This file should document by example usage of the GEOS library.
// It could actually be a live discuss-by-example board for
// architectural design choices.
// 			--strk;
// 
// DEBUGGING TIPS:
//  use -D__USE_MALLOC at compile time for gcc 2.91, 2.95, 3.0 and 3.1
//  and GLIBCXX_FORCE_NEW or GLIBCPP_FORCE_NEW at run time with gcc 3.2.2+
//  to force libstdc++ avoid caching memory. This should remove some
//  obscure reports from memory checkers like valgrind.
//
// $Log$
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
wkt_print_geoms(int numgeoms, Geometry **geoms)
{
	// WKT-print given geometries
	WKTWriter *wkt = new WKTWriter();
	for (int i=0; i<numgeoms; i++) {
		//string tmp="test";
		string tmp=wkt->write(geoms[i]);
		cout<<tmp<<endl;
		//cout<<wkt->write(geoms[i])<<endl;
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
	LinearRing *lr = (LinearRing*) global_factory->createLinearRing(cl);
	
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

// This function will create a GeoemtryCollection
// containing the two given Geometries.
// Note that given Geometries will be referenced
// by returned object, thus deleted at its destruction
// time.
//
GeometryCollection *
create_simple_collection(Geometry *g1, Geometry *g2)
{
	// We need to construct a <Geometry *> vector
	// to use as argument to the factory function
	vector<Geometry *> *collection = new vector<Geometry *>;

	// Now, we need to make copies of the given args
	// we do it using copy constructor.
	collection->push_back(g1);
	collection->push_back(g2);

	GeometryCollection *ret =
		global_factory->createGeometryCollection(collection);


	// We HAVE to delete the vectore used to store
	// goemetry pointers, but created object will
	// delete pointed geometries, weird uh?
	delete collection;

	return ret;
}


// Start reading here
void do_all()
{
	int numgeoms = 4;
	Geometry *geoms[numgeoms];

	// Define a precision model using 0,0 as the reference origin
	// and 2.0 as coordinates scale.
	// Using 1.0 as scale will segfault, dunno why --strk;
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
	geoms[0] = create_square_linearring(0,0,100);
	geoms[1] = create_square_polygon(0,200,300);
	geoms[2] = create_square_polygon(0,250,300);

	// here we write this bad-looking code to copy
	// geometries before putting them in a collection
	// object, since it will take responsability about
	// passed arguments life.
	geoms[3] = create_simple_collection(
			new LinearRing(*((LinearRing *)geoms[0])),
			new Polygon(*((Polygon *)geoms[1])));

	// WKT-printing a point geometry segfaults !!
	//geoms[4] = create_point(150, 350);

	// Print all geoms.
	cout<<"--------HERE ARE THE BASE GEOMS ----------"<<endl;
	wkt_print_geoms(numgeoms, geoms);

	/////////////////////////////////////////////
	// CONVEX HULL
	/////////////////////////////////////////////
	
	// Make convex hulls of geometries
	Geometry *hulls[numgeoms];
	for (int i=0; i<numgeoms; i++) {
		hulls[i] = geoms[i]->convexHull();
	}

	// Print all convex hulls
	cout<<"--------HERE COMES THE HULLS----------"<<endl;
	wkt_print_geoms(numgeoms, hulls);

	// Delete the hulls
	for (int i=0; i<numgeoms; i++) {
		delete hulls[i];
	}
	
	/////////////////////////////////////////////
	// UNION
	/////////////////////////////////////////////
	
	// Make unions of all geoms
	Geometry *unions[numgeoms*numgeoms];
	int numunions=0;
	for (int i=0; i<numgeoms-1; i++) {
		Geometry *g1 = geoms[i];
		for (int j=i+1; j<numgeoms; j++) {
			Geometry *g2 = geoms[j];
			try {
				unions[numunions] = g1->Union(g2);
				numunions++;
			}
			// It's illegal to union a collection ...
			catch (IllegalArgumentException *ill) {
				//cerr <<ill->toString()<<"\n";
				delete ill;
			}
			catch (GEOSException *exc) {
				cerr <<"GEOS Exception: "<<exc->toString()<<"\n";
			}
		}
	}

	// Print all unions
	cout<<"--------HERE COMES THE UNIONS----------"<<endl;
	wkt_print_geoms(numunions, unions);

	// Delete the resulting geoms
	for (int i=0; i<numunions; i++) {
		delete unions[i];
	}
	
	/////////////////////////////////////////////
	// CLEANUP
	/////////////////////////////////////////////

	// Delete base geometries 
	for (int i=0; i<numgeoms; i++) {
		delete geoms[i];
	}

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
