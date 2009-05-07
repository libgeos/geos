#include <vector>

#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/Coordinate.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/LinearRing.h"
#include "geos/geom/Polygon.h"
#include "geos/geom/MultiPolygon.h"

using namespace geos::geom;
using namespace std;

int main() {
 GeometryFactory factory;

 vector< Geometry * > *polys1 = new vector<Geometry*>();
 vector< Geometry * > *polys2 = new vector<Geometry*>();
 vector< Geometry * > *holes1 = new vector<Geometry*>();

 CoordinateArraySequence coords1;
 coords1.add(Coordinate(1, 1));
 coords1.add(Coordinate(1, 5));
 coords1.add(Coordinate(5, 5));
 coords1.add(Coordinate(5, 1));
 coords1.add(Coordinate(1, 1));
 holes1->push_back( factory.createLinearRing() );
 polys1->push_back( factory.createPolygon(factory.createLinearRing(coords1), holes1) );

 CoordinateArraySequence coords2;
 coords2.add(Coordinate(3, 3));
 coords2.add(Coordinate(3, 4));
 coords2.add(Coordinate(4, 4));
 coords2.add(Coordinate(4, 3));
 coords2.add(Coordinate(3, 3));
 polys2->push_back( factory.createPolygon(factory.createLinearRing(coords2), new vector<Geometry*>) );

 MultiPolygon *mpoly1 = factory.createMultiPolygon(polys1);
 MultiPolygon *mpoly2 = factory.createMultiPolygon(polys2);

 cout << "      Mpoly1: " << mpoly1->toString() << endl;
 cout << "      Mpoly2: " << mpoly2->toString() << endl;
 Geometry *intersection = mpoly1->intersection(mpoly2);
 cout << "Intersection: " << intersection->toString() << endl;

 delete mpoly1;
 delete mpoly2;
 delete intersection;
} 
