#include "MarkupSTL.h"
//#include "io.h"
#include <string>
#include <iostream>
#include <fstream>
#include "util.h"
#include "graph.h"
#include "io.h"
#include "opRelate.h"

using namespace std;

#define TEST_DESCR 1
#define GEOM_A_IN 2
#define GEOM_A_OUT 4
#define GEOM_B_IN 8
#define GEOM_B_OUT 16
#define TEST_OP 32
#define TEST_RESULT 64
#define PRED 128

int main(int argC, char* argV[]) {
//	int out=TEST_DESCR+GEOM_A_IN+GEOM_A_OUT+GEOM_B_IN+GEOM_B_OUT+TEST_OP+TEST_RESULT;
//	int out=TEST_DESCR+GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT;
//	int out=GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT+PRED;
//	int out=TEST_DESCR+GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT;
	int out=TEST_DESCR+TEST_RESULT;
	int failed=0;
//	string source="d://test.xml";
    string source="./test.xml";
	string precisionModel="";
	string desc="";
	string geomAin="";
	string geomBin="";
	string geomAout="";
	string geomBout="";
	string opName="";
	string opSig="";
	string opRes="";
	int testCount=0;

	WKTReader *r = new WKTReader(GeometryFactory(PrecisionModel(),10));
	WKTWriter *w=new WKTWriter();
	Geometry *gA;
	Geometry *gB;

	CMarkupSTL xml;
	bool a=xml.Load(source.c_str());

    cout << "xml.ResetPos()\n";
	xml.ResetPos();
    cout << "xml.FindElem(\"run\")\n";
	a=xml.FindElem("run");
    cout << a << endl;
	a=xml.FindChildElem("precisionModel");
    cout << a << endl;
	precisionModel=xml.GetChildAttrib("type");
	cout << "Precision Model: " << precisionModel << endl;
	while (xml.FindChildElem("case")) {
		xml.IntoElem();
		testCount++;
		cout << "Test #" << testCount << endl;

		a=xml.FindChildElem("desc");
        cout << a << endl;
		desc=xml.GetChildData();
        cout << "desc = " << desc << endl;
		if (out & TEST_DESCR)
			cout << "\t" << desc << endl;

		a=xml.FindChildElem("a");
        cout << "xml.FindChildElem(\"a\") =  " << a << endl;
		geomAin=xml.GetChildData();
        cout << "xml.GetChildData() = " << geomAin << endl;
		gA=r->read(geomAin);
        cout << "read(geomAin) = " << gA << endl;
		geomAout=w->write(gA);
        cout << "if (out &(GEOM_A_IN | GEOM_A_OUT))" << endl;
		if (out &(GEOM_A_IN | GEOM_A_OUT)) {
			cout << "\tGeometry A" << endl;
			if (out & GEOM_A_IN)
				cout << "\t\tIn:" << geomAin << endl;
			if (out & GEOM_A_OUT)
				cout << "\t\tOut:" << geomAout << endl;
		}

		a=xml.FindChildElem("b");
        cout << "xml.FindChildElem(\"b\") = " << a << endl;
		geomBin=xml.GetChildData();
		gB=r->read(geomBin);
		geomBout=w->write(gB);
        cout << "if (out &(GEOM_B_IN | GEOM_B_OUT))" << endl;
		if (out &(GEOM_B_IN | GEOM_B_OUT)) {
			cout << "\tGeometry B" << endl;
			if (out & GEOM_B_IN)
				cout << "\t\tIn:" << geomBin << endl;
			if (out & GEOM_B_OUT)
				cout << "\t\tOut:" << geomBout << endl;
		}

		a=xml.FindChildElem("test");
		xml.IntoElem();
        cout << "a=xml.FindChildElem(\"op\")";
        a=xml.FindChildElem("op");
        cout << a << endl;
		opName=xml.GetChildAttrib("name");
		opSig=xml.GetChildAttrib("arg3");
		opRes=xml.GetChildData();
		if (out & TEST_OP)
			cout << "\tOperation '" << opName << "[" << opSig <<"]' should be " << opRes << endl;
		if (opName=="relate") {
			IntersectionMatrix im(gA->relate(gB));
			if (out & TEST_RESULT)
				cout << "\tResult: matrix='" << im.toString() << "' result=" << (im.matches(opSig)?"true":"false") <<endl;
			if (!im.matches(opSig)) failed++;
		}
		if (out & PRED) {
			cout << "\tEquals:\t\tAB=" << (gA->equals(gB)?"T":"F") << ", BA=" << (gB->equals(gA)?"T":"F") << endl;
			cout << "\tDisjoint:\tAB=" << (gA->disjoint(gB)?"T":"F") << ", BA=" << (gB->disjoint(gA)?"T":"F") << endl;
			cout << "\tIntersects:\tAB=" << (gA->intersects(gB)?"T":"F") << ", BA=" << (gB->intersects(gA)?"T":"F") << endl;
			cout << "\tTouches:\tAB=" << (gA->touches(gB)?"T":"F") << ", BA=" << (gB->touches(gA)?"T":"F") << endl;
			cout << "\tCrosses:\tAB=" << (gA->crosses(gB)?"T":"F") << ", BA=" << (gB->crosses(gA)?"T":"F") << endl;
			cout << "\tWithin:\t\tAB=" << (gA->within(gB)?"T":"F") << ", BA=" << (gB->within(gA)?"T":"F") << endl;
			cout << "\tContains:\tAB=" << (gA->contains(gB)?"T":"F") << ", BA=" << (gB->contains(gA)?"T":"F") << endl;
			cout << "\tOverlaps:\tAB=" << (gA->overlaps(gB)?"T":"F") << ", BA=" << (gB->overlaps(gA)?"T":"F") << endl;
		}

		xml.OutOfElem();
		xml.OutOfElem();
	}
	cout << "Failed: ";
	cout << failed << endl;
	cout << "End Test";
}