// CTS.cpp : Testing class
//

#include "io.h"
#include <iostream>
using namespace std;

int main(int argc, char** argv)
{
	try {
	cout << "Start:" << endl;
	
/*	Coordinate c1(10.531,20.5e-5,30);
	Coordinate c2(c1);
	Coordinate c3;
	cout << c1.equals2D(c2) << endl;
	cout << c1.equals2D(c3) << endl;
	c3.setCoordinate(c1);
	cout << c1.equals2D(c3) << endl;

	Coordinate c4(10.531,20.5e-5);
	Coordinate c5(10.531,20.5e-5);
	cout << c1.equals2D(c4) << endl;
	cout << (c1==c4) << endl;
	cout << c4.equals2D(c5) << endl;
	cout << (c4==c5) << endl;

	cout << c1.toString() << endl;;
	cout << c4.toString() << endl;;

	Coordinate c0(10,20.5e-5,30);
	cout << c0.distance(c2) << endl;
*/

/*	cout << Dimension::toDimensionSymbol(-2) << endl;
	cout << Dimension::toDimensionSymbol(Dimension::DONTCARE) << endl;
	cout << Dimension::toDimensionSymbol(87) << endl;
	cout << Dimension::toDimensionSymbol(Dimension::DONTCARE) << endl;
*/
/*	cout << Dimension::toDimensionValue('T') << endl;
	cout << Dimension::toDimensionValue('*') << endl;
	cout << Dimension::toDimensionValue('t') << endl;
	cout << Dimension::toDimensionValue('Q') << endl;
	cout << Dimension::toDimensionValue('*') << endl;
*/

/*	Coordinate c1(10,20,30);
	Coordinate c2(40,50,60);
	Coordinate c3(1,2,3);
	Envelope e1;
	Envelope e2(6,7,8,9);
	Envelope e3(c1,c2);
	Envelope e4(c3);
	Envelope e5(e2);
	e2.setToNull();
*/

/*	Coordinate c1(10,20,30);
	Coordinate c2(40,50,60);
	LineSegment ls1;
	ls1.setCoordinates(c1,c2);
*/
/*	cout << Location::UNDEF << endl;
	cout << Location::toLocationSymbol(Location::UNDEF) << endl;
	cout << Location::toLocationSymbol(Location::EXTERIOR) << endl;
	cout << Location::toLocationSymbol(99) << endl;
	cout << Location::toLocationSymbol(Location::INTERIOR) << endl;
*/
/*	Coordinate c1(10,20,30);
	PrecisionModel pm;
	Point p(c1,pm,10);
*/

/*	Coordinate c1(10,20,30);
	Coordinate c2(1,2,3);
	CoordinateList cl1;
	CoordinateList cl2(c1);
	cl1.add(c1);
	c1.setCoordinate(c2);
*/

/*	Point p;
	cout << p.getNumPoints() << endl;
	cout << p.isEmpty() << endl;

*/
/*	Coordinate c2(1,2,3);
	PrecisionModel pm;
	Point p1(c2,pm,2);
	cout << p1.getNumPoints() << endl;
	cout << p1.isEmpty() << endl;
	CoordinateList cl=p1.getCoordinates();

	Point p2(p1);
*/

/*	Coordinate c1(10,20,30);
	PrecisionModel pm(10,5,1);
	cout << pm.toString() << endl;
	Coordinate c2=pm.toInternal(c1);
	cout << c1.toString() << endl;
	cout << c2.toString() << endl;
*/
/*	IntersectionMatrix im("TF*012TF*aaa");
	cout << im.matches("TF*012TF*") << endl;
	cout << im.matches("TF*012TFT") << endl;
//	cout << im.matches("TF*012TF*aaa") << endl;
	cout << im.toString() << endl;
	im.transpose();
	cout << im.toString() << endl;
*/
	
/*	LineString ls;
	Point p;
	cout << ls.isEquivalentClass(&ls) << endl;
	cout << ls.isEquivalentClass(&p) << endl;
*/

/*	Coordinate c1(0,0);
	Coordinate c2(100,0);
	Coordinate c3(100,100);
	Coordinate c4(0,100);

	Coordinate d1(25,25);
	Coordinate d2(75,25);
	Coordinate d3(75,75);
	Coordinate d4(25,75);

	CoordinateList outlist;
	outlist.add(c1);
	outlist.add(c2);
	outlist.add(c3);
	outlist.add(c4);
	outlist.add(c1);

	CoordinateList inlist(5);
	inlist.add(d1);
	inlist.add(d2);
	inlist.add(d3);
	inlist.add(d4);
	inlist.add(d1);

	int a=inlist.getSize();
	a=outlist.getSize();
	CoordinateList testlist(outlist);
	a=testlist.getSize();
	cout << outlist.toString() <<endl;
	cout << testlist.toString() <<endl;
	outlist.deleteAt(2);
	a=outlist.getSize();
	a=testlist.getSize();
	cout << outlist.toString() <<endl;
	cout << testlist.toString() <<endl;

	LinearRing outring(outlist,PrecisionModel(),100);
	LinearRing inring(inlist,PrecisionModel(),50);

	vector<Geometry *> h(1,&inring);
	Polygon p(&outring,&h,PrecisionModel(),10);

	a=p.getNumPoints();

	LineString ls1(inlist,PrecisionModel(),1);
	LineString ls2(outlist,PrecisionModel(),1);

	bool z=ls1.equalsExact(&p);
	z=ls1.equalsExact(&ls2);
	z=ls1.equalsExact(&ls1);

	Polygon p1(p);
	p.compareToSameClass(&p1);

	p.normalize();
*/
//	throw ParseException("Test Message","ErrorLoc");

//	Geometry g(PrecisionModel(),10);
//	GeometryFactory gf;
//	string a(typeid(gf).name());
//	cout << a << endl;

	WKTReader wr;
//	CoordinateList cl=wr.getCoordinates1();

//	cout << cl.toString() <<endl;

	GeometryFactory gf;
	Polygon p(gf.createPolygon(NULL,NULL));

	cout << "End" << endl;
	} catch (char *message){
        cout << message << endl;
	} catch (ParseException pe) {
		cout << pe.toString() << endl;
	}

	return 0;
}

