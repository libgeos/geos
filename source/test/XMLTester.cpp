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

int main(int argC, char* argV[]) {
	string source="d://test.xml";
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

	WKTReader r(GeometryFactory(PrecisionModel(),10));
	WKTWriter *w=new WKTWriter();
	Geometry *gA;
	Geometry *gB;

	CMarkupSTL xml;
	bool a=xml.Load(source.c_str());

	xml.ResetPos();
	a=xml.FindElem("run");
	a=xml.FindChildElem("precisionModel");
	precisionModel=xml.GetChildAttrib("type");
	cout << "Precision Model: " << precisionModel << endl;
	while (xml.FindChildElem("case")) {
		xml.IntoElem();
		testCount++;
		cout << "Test #" << testCount << endl;

		a=xml.FindChildElem("desc");
		desc=xml.GetChildData();
		cout << "\t" << desc << endl;

		a=xml.FindChildElem("a");
		geomAin=xml.GetChildData();
		gA=r.read(geomAin);
		geomAout=w->write(gA);
		cout << "\tGeometry A" << endl;
		cout << "\t\tIn:" << geomAin << endl;
		cout << "\t\tOut:" << geomAout << endl;

		a=xml.FindChildElem("b");
		geomBin=xml.GetChildData();
		gB=r.read(geomBin);
		geomBout=w->write(gB);
		cout << "\tGeometry B" << endl;
		cout << "\t\tIn:" << geomBin << endl;
		cout << "\t\tOut:" << geomBout << endl;

		a=xml.FindChildElem("test");
		xml.IntoElem();
		a=xml.FindChildElem("op");
		opName=xml.GetChildAttrib("name");
		opSig=xml.GetChildAttrib("arg3");
		opRes=xml.GetChildData();
		cout << "\tOperation '" << opName << "[" << opSig <<"]' should be " << opRes << endl;
		if (opName=="relate") {
			IntersectionMatrix im(gA->relate(gB));
			cout << "\tResult: matrix='" << im.toString() << "' result=" << (im.matches(opSig)?"true":"false") <<endl;
		}

		xml.OutOfElem();
		xml.OutOfElem();
	}
	cout << "End Test";
}