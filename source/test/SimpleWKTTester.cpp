// CTS.cpp : Testing class
//

#include <iostream>
#include <fstream>

#include "../headers/io.h"

using namespace std;

int main(int argc, char** argv)
{
    
	try {
		ofstream out("WKTOut");
		ifstream in("WKTIn");
		string instr;
		string outstr;
		WKTReader *r = new WKTReader(GeometryFactory(PrecisionModel(),10));
		WKTWriter *w=new WKTWriter();
		Geometry *g;

		cout << "Start Testing:" << endl;
		while(!in.eof()) {
			&getline(in,instr);
			if (instr!="") {
				g=r->read(instr);
				outstr=w->write(g);
				out << "----------" << endl;
				out << instr << endl;
				out << outstr << endl;
				out << "----------" << endl << endl;
			}
		}
		out.flush();
		out.close();
		cout << "End of Testing" << endl;

	} catch (char *message){
        cout << message << endl;
	} catch (ParseException pe) {
		cout << pe.toString() << endl;
	}

	return 0;
}

