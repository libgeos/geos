#ifndef GEOS_UTIL_H
#define GEOS_UTIL_H

#include <memory>
#include <string>
#include "platform.h"
#include "geom.h"

using namespace std;

namespace geos {

class GEOSException {
public:
	GEOSException();
	GEOSException(string msg);
	GEOSException(string nname,string msg);
	virtual ~GEOSException();
	virtual string toString();
	virtual void setName(string nname);
	virtual void setMessage(string msg);
protected:
	string txt;
	string name;
};

class AssertionFailedException: public GEOSException {
public:
	AssertionFailedException();
	AssertionFailedException(string msg);
	~AssertionFailedException();
};

class IllegalArgumentException: public GEOSException {
public:
	IllegalArgumentException();
	IllegalArgumentException(string msg);
	~IllegalArgumentException();
};

class UnsupportedOperationException: public GEOSException {
public:
	UnsupportedOperationException();
	UnsupportedOperationException(string msg);
	~UnsupportedOperationException();
};

class Coordinate;
class Assert {
public:
	static void isTrue(bool assertion);
	static void isTrue(bool assertion, string message);

	static void equals(Coordinate& expectedValue, Coordinate& actualValue);
	static void equals(Coordinate& expectedValue, Coordinate& actualValue, string message);

	static void shouldNeverReachHere();
	static void shouldNeverReachHere(string message);
};

class CoordinateArrayFilter:public CoordinateFilter {
public:
	CoordinateList* pts;
	int n;
	CoordinateArrayFilter(int size);
	virtual CoordinateList* getCoordinates();
	virtual void filter(Coordinate &coord);
};

class UniqueCoordinateArrayFilter:public CoordinateFilter {
public:
	CoordinateList *list;
	UniqueCoordinateArrayFilter();
	virtual ~UniqueCoordinateArrayFilter();
	virtual CoordinateList* getCoordinates();
	virtual void filter(Coordinate &coord);
};

}
#endif
