// geos
#include <geos/platform.h>
#include <geos/geom/Triangle.h>
#include <geos/geom/Coordinate.h>
// std
#include <cmath>
#include <iostream>

using geos::geom::Triangle;
using geos::geom::Coordinate;
using namespace std;

int main()
{
	Coordinate a(5,7);
	Coordinate b(6,6);
	Coordinate c(2,-2);

	Coordinate d(3,3);
	Coordinate e(9,10);
	Coordinate f(6,7);

	Triangle t1(a,b,c);

	Triangle t2(d,e,f);

	//Test for circumcenter.

	Coordinate c1(0,0);
	t1.circumcentre(c1);
	Coordinate c2(0,0);
	t2.circumcentre(c2);
	cout << "CicumCenter of triangle ABC:: " << c1.x << " " << c1.y << endl;

	cout << "CicumCenter of triangle DEF:: " << c2.x << " " << c2.y << endl;

	return 0;
}
