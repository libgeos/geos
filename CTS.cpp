// CTS.cpp : Testing class
//

#include "geom.h"
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

//	Polygon p;


	cout << "End" << endl;
	} catch (char *message){
        cout << message << endl;
	}

	return 0;
}

