// CTS.cpp : Testing class
//

#include <iostream>

#include "../headers/io.h"
#include "../headers/geom.h"

using namespace std;


int main(int argc, char** argv)
{
	try {
	cout << "Start:" << endl;
	

	cout << "End" << endl;
	} catch (char *message){
        cout << message << endl;
	} catch (ParseException pe) {
		cout << pe.toString() << endl;
	}

	return 0;
}

