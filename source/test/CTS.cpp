// CTS.cpp : Testing class
//

#include <iostream>

#include "../headers/io.h"
#include "../headers/geom.h"

using namespace std;
using namespace geos;

int main(int argc, char** argv)
{
	try {
	cout << "Start:" << endl;
	

	cout << "End" << endl;
	} catch (GEOSException *ge) {
		cout << ge->toString() << endl;
	}

	return 0;
}

