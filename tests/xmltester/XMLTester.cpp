/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifdef _MSC_VER
# if defined(GEOS_DEBUG_MSVC_USE_VLD) && !defined(GEOS_TEST_USE_STACKWALKER)
#  include <vld.h>
# else
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
#  include <crtdbg.h>
# endif
#pragma warning(disable : 4127)
#endif

#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/BinaryOp.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/util.h>
//#include <geos/geomgraph.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/opRelate.h>
#include <geos/opPolygonize.h>
#include <geos/opLinemerge.h>
#include <geos/profiler.h>
#include <geos/unload.h>
#include <geos/opValid.h>
#include "XMLTester.h"
#include "BufferResultMatcher.h"
#include "SingleSidedBufferResultMatcher.h"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstring>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#if defined(_MSC_VER) && defined(GEOS_TEST_USE_STACKWALKER)
#include <windows.h>
#include "Stackwalker.h"
#endif

// Geometry methods do use BinaryOp internally
#undef USE_BINARYOP

using namespace geos;
using namespace geos::operation::polygonize;
using namespace geos::operation::linemerge;
using std::runtime_error;

namespace {

// a utility function defining a very simple method to indent a line of text
const char * getIndent( unsigned int numIndents )
{
    static const char * pINDENT = "                                      + ";
    static const unsigned int LENGTH = strlen( pINDENT );

    if ( numIndents > LENGTH ) numIndents = LENGTH;

    return &pINDENT[ LENGTH-numIndents ];
}



void dump_to_stdout( const TiXmlNode * pParent, unsigned int indent = 0 )
{
    if ( !pParent ) return;

    const TiXmlText *pText;
    int t = pParent->Type();
    printf( "%s", getIndent( indent));

    switch ( t )
    {
    case TiXmlNode::DOCUMENT:
        printf( "Document" );
        break;

    case TiXmlNode::ELEMENT:
        printf( "Element \"%s\"", pParent->Value() );
        break;

    case TiXmlNode::COMMENT:
        printf( "Comment: \"%s\"", pParent->Value());
        break;

    case TiXmlNode::UNKNOWN:
        printf( "Unknown" );
        break;

    case TiXmlNode::TEXT:
        pText = pParent->ToText();
        printf( "Text: [%s]", pText->Value() );
        break;

    case TiXmlNode::DECLARATION:
        printf( "Declaration" );
        break;
    default:
        break;
    }
    printf( "\n" );

    const TiXmlNode * pChild;

    for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
    {
        dump_to_stdout( pChild, indent+2 );
    }
}

}

void
tolower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))std::tolower);
}

std::string
normalize_filename(const std::string& str)
{
	std::string newstring;

	std::string::size_type last_slash = str.find_last_of('/', str.size());
	if ( last_slash == std::string::npos ) newstring = str;
	else newstring = str.substr(last_slash+1);
	
	for (std::string::iterator i=newstring.begin(), e=newstring.end(); i!=e; ++i)
	{
		if ( *i == '.' ) *i = '_';
	}

	tolower(newstring);

	return newstring;
}

/* Could be an XMLTester class private but oh well.. */
static int
checkBufferSuccess(geom::Geometry const& gRes, geom::Geometry const& gRealRes, double dist)
{
	int success = 1;
	do
	{

		if ( gRes.getGeometryTypeId() != gRealRes.getGeometryTypeId() )
		{
			std::cerr << "Expected result is of type "
				<< gRes.getGeometryType()
				<< "; obtained result is of type "
				<< gRealRes.getGeometryType()
				<< std::endl;
			success=0;
			break;
		}

		// Is a buffer always an area ?
		if ( gRes.getDimension() != 2 )
		{
			std::cerr << "Don't know how to validate "
				<< "result of buffer operation "
				<< "when expected result is not an "
				<< "areal type."
				<< std::endl;
		}
		

		geos::xmltester::BufferResultMatcher matcher;
		if ( ! matcher.isBufferResultMatch(gRealRes,
						   gRes,
						   dist) )
		{
std::cerr << "BufferResultMatcher FAILED" << std::endl;
			success=0;
			break;
		}

	}
	while (0);

	return success;
}

static int
checkSingleSidedBufferSuccess(geom::Geometry& gRes,
		geom::Geometry& gRealRes, double dist)
{
	int success = 1;
	do
	{

		if ( gRes.getGeometryTypeId() != gRealRes.getGeometryTypeId() )
		{
			std::cerr << "Expected result is of type "
				<< gRes.getGeometryType()
				<< "; obtained result is of type "
				<< gRealRes.getGeometryType()
				<< std::endl;
			success=0;
			break;
		}

		geos::xmltester::SingleSidedBufferResultMatcher matcher;
		if ( ! matcher.isBufferResultMatch(gRealRes,
						   gRes,
						   dist) )
		{
std::cerr << "SingleSidedBufferResultMatcher FAILED" << std::endl;
			success=0;
			break;
		}

	}
	while (0);

	return success;
}

XMLTester::XMLTester()
	:
	gA(0),
	gB(0),
	gT(0),
	pm(0),
	factory(0),
	wktreader(0),
	wktwriter(0),
	wkbreader(0),
	wkbwriter(0),
	test_predicates(0),
	failed(0),
	succeeded(0),
	caseCount(0),
	testCount(0),
	testFileCount(0),
	totalTestCount(0),
	curr_file(NULL),
	testValidOutput(false),
	testValidInput(false),
	sqlOutput(false),
	HEXWKB_output(false)
{
	setVerbosityLevel(0);
}

int
XMLTester::setVerbosityLevel(int value)
{
	int old_value=verbose;

	verbose=value;

	return old_value;
}

/*private*/
void
XMLTester::printTest(bool success, const std::string& expected_result, const std::string& actual_result)
{
	if ( sqlOutput )
	{
		std::cout << "INSERT INTO \"" << normalize_filename(*curr_file) << "\" VALUES ("
		          << caseCount << ", "
		          << testCount << ", "
		          << "'" << opSignature << "', "
		          << "'" << curr_case_desc << "', ";

		std::string geomOut;

		if ( gA ) {
			std::cout << "'" << printGeom(gA) << "', ";
		} else {
			std::cout << "NULL, ";
		}

		if ( gB ) {
			std::cout << "'" << printGeom(gB) << "', ";
		} else {
			std::cout << "NULL, ";
		}

		std::cout << "'" << expected_result << "', "
		          << "'" << actual_result << "', ";

		if ( success ) std::cout << "'t'";
		else std::cout << "'f'";

		std::cout << ");" << std::endl;
	}

	else
	{
		std::cout << *curr_file <<":";
		std::cout << " case" << caseCount << ":";
		std::cout << " test" << testCount << ": "
			<< opSignature; 
		std::cout << ": " << (success?"ok.":"failed.")<<std::endl;

		std::cout << "\tDescription: " << curr_case_desc << std::endl;


		if ( gA ) {
			std::cout << "\tGeometry A: ";
			printGeom(std::cout, gA);
			std::cout << std::endl;
		}

		if ( gB ) {
			std::cout << "\tGeometry B: ";
			printGeom(std::cout, gB);
			std::cout << std::endl;
		}

		std::cout << "\tExpected result: "<<expected_result<<std::endl;
		std::cout << "\tObtained result: "<<actual_result<<std::endl;
		std::cout <<std::endl;
	}
}

void
XMLTester::run(const std::string &source)
{
	curr_file=&source;

	if ( sqlOutput )
	{
		std::cout << "CREATE TABLE \"" << normalize_filename(*curr_file) << "\"" 
		          << "( caseno integer, testno integer, " 
			  << " operation varchar, description varchar, "
			  << " a geometry, b geometry, expected geometry, "
			  << " obtained geometry, result bool )"

			  // NOTE: qgis 0.7.4 require oids for proper operations.
			  //       The 'WITH OIDS' parameter is supported back to
			  //       PostgreSQL 7.2, so if you run an older version
			  //       rebuild with the next line commented out.
			  << " WITH OIDS"

			  << ";" << std::endl;
	}

	++testFileCount;

	caseCount=0;

	if ( ! xml.LoadFile(source.c_str()) )
	{
		std::stringstream err;
		err << "Could not load " << source << ": " << xml.ErrorDesc();
		throw runtime_error(err.str());
	}

	//dump_to_stdout(&xml);

	const TiXmlNode* node = xml.FirstChild("run");

	if ( ! node )
	  throw(runtime_error("Document has no childs"));

	parseRun(node);

}

void 
XMLTester::resultSummary(std::ostream &os) const
{
	os<<"Files: "<<testFileCount<<std::endl;
	os<<"Tests: "<<totalTestCount<<std::endl;
	os<<"Failed: "<<failed<<std::endl;
	os<<"Succeeded: "<<succeeded<<std::endl;
}

void 
XMLTester::resetCounters() 
{
	testFileCount=totalTestCount=failed=succeeded=0;
}

void
XMLTester::parseRun(const TiXmlNode* node)
{
	using geos::geom::PrecisionModel;

	assert(node);

	//dump_to_stdout(node);

	// Look for precisionModel element
	const TiXmlElement* el = node->FirstChildElement("precisionModel");
	if ( el ) parsePrecisionModel(el);
	else pm.reset(new PrecisionModel());

	if (verbose > 1)
	{
		std::cerr << *curr_file <<": run: Precision Model: " << pm->toString() <<std::endl;
	}

	factory.reset(new geom::GeometryFactory(pm.get()));
	wktreader.reset(new io::WKTReader(factory.get()));
	wktwriter.reset(new io::WKTWriter());
	wkbreader.reset(new io::WKBReader(*factory));
	wkbwriter.reset(new io::WKBWriter());

	const TiXmlNode* casenode;
	for ( casenode = node->FirstChild("case");
	      casenode;
	      casenode = casenode->NextSibling("case") )
	{
		try {
			parseCase(casenode);
		} catch (const std::exception& exc) {
			std::cerr<<exc.what()<<std::endl;
		}
	}

}

void
XMLTester::parsePrecisionModel(const TiXmlElement* el)
{
	using geos::geom::PrecisionModel;

	//dump_to_stdout(el);

	/* This does not seem to work... */
	std::string type;
	const char* typeStr = el->Attribute("type");
	if ( typeStr ) type = typeStr;

	const char* scaleStr = el->Attribute("scale");

	if ( ! scaleStr ) {
		if ( type == "FLOATING_SINGLE" )
		{
			pm.reset(new PrecisionModel(PrecisionModel::FLOATING_SINGLE));
		}
		else
		{
			pm.reset(new PrecisionModel());
		}
	} else {

		char* stopstring;

		double scale = std::strtod(scaleStr, &stopstring);
		double offsetX = 0;
		double offsetY = 2;

		if ( ! el->QueryDoubleAttribute("offsetx", &offsetX) )
		{} // std::cerr << "No offsetx" << std::endl;

		if ( ! el->QueryDoubleAttribute("offsety", &offsetY) )
		{} // std::cerr << "No offsety" << std::endl;

		// NOTE: PrecisionModel discards offsets anyway...
		pm.reset(new PrecisionModel(scale, offsetX, offsetY));
	}
}


bool
XMLTester::testValid(const geom::Geometry* g, const std::string& label)
{
	operation::valid::IsValidOp ivo(g);
	bool valid = ivo.isValid();
	if ( ! valid )
	{
		operation::valid::TopologyValidationError *err = ivo.getValidationError();
		std::cerr << *curr_file << ":"
			  << " case" << caseCount << ":"
			  << " test" << testCount << ": "
			  << opSignature << ": " 
			  << " invalid geometry (" << label 
			  << "): " << err->toString() << std::endl;
	}
	return valid;
}

/**
 * Parse WKT or HEXWKB
 */
geom::Geometry *
XMLTester::parseGeometry(const std::string &in, const char* label)
{
	if ( ( ! wkbreader.get() ) || ( ! wktreader.get() ) )
		throw(runtime_error("No precision model specified"));

	std::stringstream is(in, std::ios_base::in);
	char first_char;

	// Remove leading spaces
	while (is.get(first_char) && std::isspace(first_char));
	is.unget();

	geom::Geometry* ret;

	switch (first_char)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			ret = wkbreader->readHEX(is);
			break;
		default:
			ret = wktreader->read(in);
			break;
	}

	if ( testValidInput ) testValid(ret, std::string(label));

	//ret->normalize();

	return ret;
}

std::string 
XMLTester::trimBlanks(const std::string &in)
{
	std::string out;
	std::string::size_type pos = in.find_first_not_of(" \t\n\r");
	if (pos!=std::string::npos) out=in.substr(pos);
	pos = out.find_last_not_of(" \t\n\r");
	if (pos!=std::string::npos) out=out.substr(0, pos+1);
	return out;
}

void
XMLTester::parseCase(const TiXmlNode* node)
{
	assert(node);

	std::string geomAin;
	std::string geomBin;
	std::string thrownException;

	gA=NULL;
	gB=NULL;


	//dump_to_stdout(node);

	curr_case_desc.clear();
	const TiXmlNode* txt = node->FirstChild("desc");
	if ( txt ) {
		txt = txt->FirstChild();
		if ( txt ) curr_case_desc = trimBlanks(txt->Value());
	}

	//std::cerr << "Desc: " << curr_case_desc << std::endl;


	try {
		const TiXmlNode *el = node->FirstChild("a");
		geomAin = el->FirstChild()->Value();
		geomAin = trimBlanks(geomAin);
		gA = parseGeometry(geomAin, "Geometry A");

		if ( 0 != (el = node->FirstChild("b")) )
		{
			geomBin = el->FirstChild()->Value();
			geomBin = trimBlanks(geomBin);
			gB = parseGeometry(geomBin, "Geometry B");
		}
	}
	catch (const std::exception &e) {
		thrownException = e.what();
	}
	catch (...) {
		thrownException = "Unknown exception";
	}

//std::cerr << "A: " << geomAin << std::endl;
//std::cerr << "B: " << geomBin << std::endl;


	if ( thrownException != "" )
	{
		std::cout << *curr_file <<":";
		std::cout << " case" << caseCount << ":";
		std::cout << " skipped ("<<thrownException<<")."<<std::endl;
		return;
	}

	++caseCount;
	testCount=0;

	const TiXmlNode* testnode;
	for ( testnode = node->FirstChild("test");
	      testnode;
	      testnode = testnode->NextSibling("test") )
	{
		parseTest(testnode);
	}

	totalTestCount+=testCount;
		
	delete gA;
	delete gB;
}

/*private*/
void
XMLTester::printGeom(std::ostream& os, const geom::Geometry *g)
{
	os << printGeom(g);
}

std::string
XMLTester::printGeom(const geom::Geometry *g)
{
	if ( HEXWKB_output )
	{
		std::stringstream s(std::ios_base::binary|std::ios_base::in|std::ios_base::out);
		wkbwriter->write(*g, s);
		std::stringstream s2;
		wkbreader->printHEX(s, s2);
		return s2.str();
	}
	else
	{
		return wktwriter->write(g);
	}
}

void
XMLTester::parseTest(const TiXmlNode* node)
{
	using namespace operation::overlay;

	typedef std::auto_ptr< geom::Geometry > GeomAutoPtr;

	int success=0; // no success by default
	std::string opName;
	std::string opArg1;
	std::string opArg2;
	std::string opArg3;
	std::string opArg4;
	std::string opRes;

	++testCount;

	const TiXmlNode* opnode = node->FirstChild("op");
	if ( ! opnode ) throw(runtime_error("case has no op"));

	//dump_to_stdout(opnode);

	const TiXmlElement* opel = opnode->ToElement();

	const char* tmp = opel->Attribute("name");
	if ( tmp ) opName = tmp;

	tmp = opel->Attribute("arg1");
	if ( tmp ) opArg1 = tmp;

	tmp = opel->Attribute("arg2");
	if ( tmp ) opArg2 = tmp;

	tmp = opel->Attribute("arg3");
	if ( tmp ) opArg3 = tmp;

	tmp = opel->Attribute("arg4");
	if ( tmp ) opArg4 = tmp;

	const TiXmlNode* resnode = opnode->FirstChild();
	if ( ! resnode )
	{
		std::stringstream tmp;
		tmp << "op of test " << testCount
		    << " of case " << caseCount
		    << " has no expected result child";
		throw(runtime_error(tmp.str()));
	}
	opRes = resnode->Value();

	// trim blanks
	opRes=trimBlanks(opRes);
	opName=trimBlanks(opName);
	tolower(opName);

	std::string opSig="";

	if ( opArg1 != "" ) opSig=opArg1;
	if ( opArg2 != "" ) {
		if ( opSig != "" ) opSig += ", ";
		opSig += opArg2;
	}
	if ( opArg3 != "" ) {
		if ( opSig != "" ) opSig += ", ";
		opSig += opArg3;
	}
	if ( opArg4 != "" ) {
		if ( opSig != "" ) opSig += ", ";
		opSig += opArg4;
	}

	opSignature = opName + "(" + opSig + ")";

	std::string actual_result="NONE";

	// expected_result will be modified by specific tests
	// if needed (geometry normalization, for example)
	std::string expected_result=opRes;

	try
	{

		util::Profile profile("op");

		if (opName=="relate")
		{
			std::auto_ptr<geom::IntersectionMatrix> im(gA->relate(gB));
			assert(im.get());

			if (im->matches(opArg3)) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;
		}

		else if (opName=="isvalid")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) {
				gT=gB;
			} 

			if (gT->isValid()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="intersection")
		{

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

#ifndef USE_BINARYOP
			GeomAutoPtr gRealRes(gA->intersection(gB));
#else
			GeomAutoPtr gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opINTERSECTION));
#endif

			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="union")
		{
			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));

			GeomAutoPtr gRealRes;
			if ( gB ) {
#ifndef USE_BINARYOP
				gRealRes.reset(gA->Union(gB));
#else
				gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opUNION));
#endif
			} else {
				gRealRes = gA->Union();
			}

			if (gRes->equals(gRealRes.get())) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="difference")
		{

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

#ifndef USE_BINARYOP
			GeomAutoPtr gRealRes(gA->difference(gB));
#else
			GeomAutoPtr gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opDIFFERENCE));
#endif
			
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="symdifference")
		{
			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

#ifndef USE_BINARYOP
			GeomAutoPtr gRealRes(gA->symDifference(gB));
#else
			GeomAutoPtr gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opSYMDIFFERENCE));
#endif

			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="intersects")
		{
			geom::Geometry *g1 = opArg1 == "B" ? gB : gA;
			geom::Geometry *g2 = opArg2 == "B" ? gB : gA;

			if (g1->intersects(g2)) actual_result="true";
			else actual_result="false";
			
			if (actual_result==opRes) success=1;
		}

		else if (opName=="contains")
		{
			geom::Geometry *g1 = opArg1 == "B" ? gB : gA;
			geom::Geometry *g2 = opArg2 == "B" ? gB : gA;

			if (g1->contains(g2)) actual_result="true";
			else actual_result="false";
			
			if (actual_result==opRes) success=1;
		}

		else if (opName=="within")
		{
			geom::Geometry *g1 = opArg1 == "B" ? gB : gA;
			geom::Geometry *g2 = opArg2 == "B" ? gB : gA;

			if (g1->within(g2)) actual_result="true";
			else actual_result="false";
			
			if (actual_result==opRes) success=1;
		}

		else if (opName=="covers")
		{
			geom::Geometry *g1 = opArg1 == "B" ? gB : gA;
			geom::Geometry *g2 = opArg2 == "B" ? gB : gA;

			if (g1->covers(g2)) actual_result="true";
			else actual_result="false";
			
			if (actual_result==opRes) success=1;
		}

		else if (opName=="coveredby")
		{
			geom::Geometry *g1 = opArg1 == "B" ? gB : gA;
			geom::Geometry *g2 = opArg2 == "B" ? gB : gA;

			if (g1->coveredBy(g2)) actual_result="true";
			else actual_result="false";
			
			if (actual_result==opRes) success=1;
		}

		else if (opName=="getboundary")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->getBoundary());
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="getcentroid")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->getCentroid());

			if ( gRealRes.get() ) gRealRes->normalize();
			else gRealRes.reset(factory->createPoint());
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput ) 
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="issimple")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			if (gT->isSimple()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="convexhull")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->convexHull());
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput ) 
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="buffer")
		{
			using namespace operation::buffer;

			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			profile.start();

			GeomAutoPtr gRealRes;
			double dist = std::atof(opArg2.c_str());

			BufferParameters params;
			if ( opArg3 != "" ) {
		params.setQuadrantSegments(std::atoi(opArg3.c_str()));
			}
			

			BufferOp op(gT, params);
			gRealRes.reset(op.getResultGeometry(dist));

			profile.stop();
			gRealRes->normalize();

			// Validate the buffer operation
			success = checkBufferSuccess(*gRes, *gRealRes, dist);

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="buffersinglesided")
		{
			using namespace operation::buffer;

			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			profile.start();

			GeomAutoPtr gRealRes;
			double dist = std::atof(opArg2.c_str());
				
			BufferParameters params ;
			params.setJoinStyle( BufferParameters::JOIN_ROUND ) ;
			if ( opArg3 != "" ) {
		params.setQuadrantSegments( std::atoi(opArg3.c_str()));
			}
			
			bool leftSide = true ;
			if ( opArg4 == "right" )
			{
				leftSide = false ;
			}

			BufferBuilder bufBuilder( params ) ;
			gRealRes.reset( bufBuilder.bufferLineSingleSided(
			                             gT, dist, leftSide ) ) ;

			profile.stop();
			gRealRes->normalize();

			// Validate the single sided buffer operation
			success = checkSingleSidedBufferSuccess(*gRes,
					*gRealRes, dist);

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="buffermitredjoin")
		{
			using namespace operation::buffer;

			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			profile.start();

			GeomAutoPtr gRealRes;
			double dist = std::atof(opArg2.c_str());

			BufferParameters params;
			params.setJoinStyle(BufferParameters::JOIN_MITRE);

			if ( opArg3 != "" ) {
		params.setQuadrantSegments(std::atoi(opArg3.c_str()));
			}

			BufferOp op(gT, params);
			gRealRes.reset(op.getResultGeometry(dist));

			profile.stop();
			gRealRes->normalize();

			// Validate the buffer operation
			success = checkBufferSuccess(*gRes, *gRealRes, dist);

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput ) 
				success &= testValid(gRealRes.get(), "result");
		}


		else if (opName=="getinteriorpoint")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->getInteriorPoint());
			if ( gRealRes.get() ) gRealRes->normalize();
			else gRealRes.reset(factory->createPoint());

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput ) 
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="iswithindistance")
		{
			double dist=std::atof(opArg3.c_str());
			if (gA->isWithinDistance(gB, dist)) {
				actual_result="true";
			} else {
				actual_result="false";
			}

			if (actual_result==opRes) success=1;

		}

		else if (opName=="polygonize")
		{

			GeomAutoPtr gRes(wktreader->read(opRes));
			gRes->normalize();

			Polygonizer plgnzr;
			plgnzr.add(gA);


            std::vector<geos::geom::Polygon *>*polys = plgnzr.getPolygons();
			std::vector<geom::Geometry *>*newgeoms = new std::vector<geom::Geometry *>;
			for (unsigned int i=0; i<polys->size(); i++)
				newgeoms->push_back((*polys)[i]);
			delete polys;

			GeomAutoPtr gRealRes(factory->createGeometryCollection(newgeoms));
			gRealRes->normalize();


			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput )
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="linemerge")
		{
			GeomAutoPtr gRes(wktreader->read(opRes));
			gRes->normalize();

			geom::Geometry *gT=gA;

			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			LineMerger merger;
			merger.add(gT);
			std::auto_ptr< std::vector<geom::LineString *> > lines ( merger.getMergedLineStrings() );
			std::vector<geom::Geometry *>*newgeoms = new std::vector<geom::Geometry *>(lines->begin(),
					lines->end());

			GeomAutoPtr gRealRes(factory->createGeometryCollection(newgeoms));
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

			if ( testValidOutput ) 
				success &= testValid(gRealRes.get(), "result");
		}

		else if (opName=="areatest")
		{
			char* rest;
			double toleratedDiff = std::strtod(opRes.c_str(), &rest);
			int validOut = 1;

			if ( rest == opRes.c_str() )
			{
				throw std::runtime_error("malformed testcase: missing tolerated area difference in 'areatest' op");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running intersection for areatest" << std::endl;
			}
#ifndef USE_BINARYOP
			GeomAutoPtr gI(gA->intersection(gB));
#else
			GeomAutoPtr gI = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opINTERSECTION));
#endif

			if ( testValidOutput )
			{
				validOut &= testValid(gI.get(), "areatest intersection");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running difference(A,B) for areatest" << std::endl;
			}

#ifndef USE_BINARYOP
			GeomAutoPtr gDab(gA->difference(gB));
#else
			GeomAutoPtr gDab = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opDIFFERENCE));
#endif

			if ( testValidOutput )
			{
				validOut &= testValid(gI.get(), "areatest difference(a,b)");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running difference(B,A) for areatest" << std::endl;
			}

#ifndef USE_BINARYOP
			GeomAutoPtr gDba(gB->difference(gA));
#else
			GeomAutoPtr gDba = BinaryOp(gB, gA,
					overlayOp(OverlayOp::opDIFFERENCE));
#endif

			if ( testValidOutput )
			{
				validOut &= testValid(gI.get(), "areatest difference(b,a)");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running symdifference for areatest" << std::endl;
			}

#ifndef USE_BINARYOP
			GeomAutoPtr gSD(gA->symDifference(gB));
#else
			GeomAutoPtr gSD = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opSYMDIFFERENCE));
#endif

			if ( testValidOutput )
			{
				validOut &= testValid(gI.get(), "areatest symdifference");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running union for areatest" << std::endl;
			}

#ifndef USE_BINARYOP
			GeomAutoPtr gU(gA->Union(gB));
#else
			GeomAutoPtr gU = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opUNION));
#endif

			double areaA = gA->getArea();
			double areaB = gB->getArea();
			double areaI = gI->getArea(); 
			double areaDab = gDab->getArea();
			double areaDba = gDba->getArea();
			double areaSD = gSD->getArea();
			double areaU = gU->getArea();

			double maxdiff = 0;
			std::string maxdiffop;

			// @ : symdifference
			// - : difference
			// + : union
			// ^ : intersection
		
			// A == ( A ^ B ) + ( A - B )
			double diff = std::fabs ( areaA - areaI - areaDab );
			if ( diff > maxdiff ) {
				maxdiffop = "A == ( A ^ B ) + ( A - B )";
				maxdiff = diff;
			}

			// B == ( A ^ B ) + ( B - A )
			diff = std::fabs ( areaB - areaI - areaDba );
			if ( diff > maxdiff ) {
				maxdiffop = "B == ( A ^ B ) + ( B - A )";
				maxdiff = diff;
			}

			//  ( A @ B ) == ( A - B ) + ( B - A )
			diff = std::fabs ( areaDab + areaDba - areaSD );
			if ( diff > maxdiff ) {
				maxdiffop = "( A @ B ) == ( A - B ) + ( B - A )";
				maxdiff = diff;
			}

			//  ( A u B ) == ( A ^ B ) + ( A @ B )
			diff = std::fabs ( areaI + areaSD - areaU );
			if ( diff > maxdiff ) {
				maxdiffop = "( A u B ) == ( A ^ B ) + ( A @ B )";
				maxdiff = diff;
			}

			if ( maxdiff <= toleratedDiff )
			{
				success = 1 && validOut;
			}

			std::stringstream tmp;
			tmp << maxdiffop << ": " << maxdiff;
			actual_result=tmp.str();
			expected_result=opRes;

		}
		else if (opName=="distance")
		{
			char* rest;
			double distE = std::strtod(opRes.c_str(), &rest);
			if ( rest == opRes.c_str() )
			{
				throw std::runtime_error("malformed testcase: missing expected result in 'distance' op");
			}

			geom::Geometry *g1 = opArg1 == "B" ? gB : gA;
			geom::Geometry *g2 = opArg2 == "B" ? gB : gA;
			double distO = g1->distance(g2);
			std::stringstream ss; ss << distO; 
			actual_result = ss.str();

			// TODO: Use a tolerance ?
			success = ( distO == distE ) ? 1 : 0;
		}

		else
		{
			std::cerr << *curr_file << ":";
			std::cerr << " case" << caseCount << ":";
			std::cerr << " test" << testCount << ": "
					  << opName << "(" << opSig << ")";
			std::cerr << ": skipped (unrecognized)." << std::endl;
			return;
		}

	}
	catch (const std::exception &e)
	{
		std::cerr<<"EXCEPTION on case "<<caseCount
			<<" test "<<testCount<<": "<<e.what()
			<<std::endl;
		actual_result = e.what();
	}
	catch (...)
	{
		std::cerr << "Unknown EXEPTION on case "
		          << caseCount
		          << std::endl;
		actual_result = "Unknown exception thrown";
	}

	if ( success ) ++succeeded;
	else ++failed;

	if ((!success && verbose) || verbose > 1)
	{
		printTest(!!success, expected_result, actual_result);
	}

	if (test_predicates && gB && gA) {
		runPredicates(gA, gB);
	}

}

void
XMLTester::runPredicates(const geom::Geometry *gA, const geom::Geometry *gB)
{
	std::cout << "\t    Equals:\tAB=" << (gA->equals(gB)?"T":"F") << ", BA=" << (gB->equals(gA)?"T":"F") << std::endl;
	std::cout << "\t  Disjoint:\tAB=" << (gA->disjoint(gB)?"T":"F") << ", BA=" << (gB->disjoint(gA)?"T":"F") << std::endl;
	std::cout << "\tIntersects:\tAB=" << (gA->intersects(gB)?"T":"F") << ", BA=" << (gB->intersects(gA)?"T":"F") << std::endl;
	std::cout << "\t   Touches:\tAB=" << (gA->touches(gB)?"T":"F") << ", BA=" << (gB->touches(gA)?"T":"F") << std::endl;
	std::cout << "\t   Crosses:\tAB=" << (gA->crosses(gB)?"T":"F") << ", BA=" << (gB->crosses(gA)?"T":"F") << std::endl;
	std::cout << "\t    Within:\tAB=" << (gA->within(gB)?"T":"F") << ", BA=" << (gB->within(gA)?"T":"F") << std::endl;
	std::cout << "\t  Contains:\tAB=" << (gA->contains(gB)?"T":"F") << ", BA=" << (gB->contains(gA)?"T":"F") << std::endl;
	std::cout << "\t  Overlaps:\tAB=" << (gA->overlaps(gB)?"T":"F") << ", BA=" << (gB->overlaps(gA)?"T":"F") << std::endl;
}

XMLTester::~XMLTester()
{
}


static void
usage(char *me, int exitcode, std::ostream &os)
{
	os << "Usage: " << me << " [options] <test> [<test> ...]" << std::endl;
	os << "Options: " << std::endl;
	os << " -v                  Verbose mode "
	   << "(multiple -v increment verbosity)" << std::endl
	   << "--test-valid-output  Test output validity" << std::endl
	   << "--test-valid-input   Test input validity" << std::endl
	   << "--sql-output         Produce SQL output" << std::endl
	   << "--wkb-output         Print Geometries as HEXWKB" << std::endl;

    std::exit(exitcode);
}

int
main(int argC, char* argV[])
{
	int verbose=0;
	bool sql_output=false;

#if defined(_MSC_VER) && defined(GEOS_TEST_USE_STACKWALKER)
	InitAllocCheck();
	{
#endif

	if ( argC < 2 ) usage(argV[0], 1, std::cerr);

	XMLTester tester;
	tester.setVerbosityLevel(verbose);

	for (int i=1; i<argC; ++i)
	{
		// increment verbosity level
		if ( ! std::strcmp(argV[i], "-v" ) )
		{
			++verbose;
			tester.setVerbosityLevel(verbose);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--test-valid-output" ) )
		{
			tester.testOutputValidity(true);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--sql-output" ) )
		{
			sql_output = true;
			tester.setSQLOutput(sql_output);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--wkb-output" ) )
		{
			sql_output = true;
			tester.setHEXWKBOutput(sql_output);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--test-valid-input" ) )
		{
			tester.testInputValidity(true);
			continue;
		}

		std::string source = argV[i];

		try {
			tester.run(source);
		} catch (const std::exception& exc) {
			std::cerr<<exc.what()<<std::endl;
		}
	}

	if ( ! sql_output ) tester.resultSummary(std::cout);
	else tester.resultSummary(std::cerr);

	io::Unload::Release();

	return tester.getFailuresCount();

#if defined(_MSC_VER) && defined(GEOS_TEST_USE_STACKWALKER)
	}
	DeInitAllocCheck();
#endif

}

/**********************************************************************
 * $Log: XMLTester.cpp,v $
 * Revision 1.38  2006/07/13 03:59:10  csavage
 * Changes to compile on VC++ - fully qualified polygon name.  Should also work on MingW, will test next.
 *
 * Revision 1.37  2006/06/19 20:48:35  strk
 * parseCase(): make sure to exit the <case> tag before returning
 *
 * Revision 1.36  2006/06/14 19:19:10  strk
 * Added support for "AreaTest" operations.
 *
 * Revision 1.35  2006/06/12 10:39:29  strk
 * don't print test file precision model if verbosity level < 2.
 *
 * Revision 1.34  2006/06/05 15:36:34  strk
 * Given OverlayOp funx code enum a name and renamed values to have a lowercase prefix. Drop all of noding headers from installed header set.
 *
 * Revision 1.33  2006/05/19 16:38:22  strk
 *         * tests/xmltester/XMLTester.cpp: report
 *         error on load of requested tests.
 *
 * Revision 1.32  2006/04/14 14:57:15  strk
 * XMLTester binary ops invoked using the new BinaryOp template function.
 *
 * Revision 1.31  2006/04/07 13:26:38  strk
 * Use of auto_ptr<> to prevent confusing leaks in tester
 *
 * Revision 1.30  2006/03/22 16:01:33  strk
 * indexBintree.h header split, classes renamed to match JTS
 *
 * Revision 1.29  2006/03/17 14:56:39  strk
 * Fixed filename normalizer for sql output
 **********************************************************************/

