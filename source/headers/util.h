#ifndef GEOS_UTIL_H
#define GEOS_UTIL_H

#include <string>
#include "platform.h"

using namespace std;

class AssertionFailedException {
public:
	AssertionFailedException();
	AssertionFailedException(string msg);
	~AssertionFailedException();
	string toString();
private:
	string txt;
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

#endif
