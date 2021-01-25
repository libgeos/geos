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
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/util/Densifier.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/overlay/snap/GeometrySnapper.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/operation/overlayng/UnaryUnionNG.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/polygonize/BuildArea.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/precision/MinimumClearance.h>
#include <geos/util.h>
#include <geos/util/Interrupt.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/operation/relate/RelateOp.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/profiler.h>
#include <geos/unload.h>
#include <geos/operation/valid/IsValidOp.h>
#include "XMLTester.h"
#include "BufferResultMatcher.h"
#include "SingleSidedBufferResultMatcher.h"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#include <signal.h>

#if defined(_MSC_VER) && defined(GEOS_TEST_USE_STACKWALKER)
#include <windows.h>
#include "Stackwalker.h"
#endif

using namespace geos;
using namespace geos::operation::polygonize;
using namespace geos::operation::linemerge;
using namespace geos::geom::prep;
using std::runtime_error;
using geos::operation::overlayng::OverlayNG;
using geos::operation::overlayng::UnaryUnionNG;
using geos::operation::overlayng::OverlayNGRobust;

namespace {

std::unique_ptr<const PreparedGeometry>
prepare(const geom::Geometry* g)
{
    return PreparedGeometryFactory::prepare(g);
}

// Asymmetric Rounding Algorithm  - equivalent to Java Math.round()
// Copy from geos/util/math.cpp
double
java_math_round(double val)
{
    double n;
    double f = std::fabs(std::modf(val, &n));

    if(val >= 0) {
        if(f < 0.5) {
            return std::floor(val);
        }
        else if(f > 0.5) {
            return std::ceil(val);
        }
        else {
            return (n + 1.0);
        }
    }
    else {
        if(f < 0.5) {
            return std::ceil(val);
        }
        else if(f > 0.5) {
            return std::floor(val);
        }
        else {
            return n;
        }
    }
} // java_math_round


#ifdef not_used
// a utility function defining a very simple method to indent a line of text
const char*
getIndent(unsigned int numIndents)
{
    static const char* pINDENT = "                                      + ";
    static const unsigned int LENGTH = static_cast<unsigned int>(strlen(pINDENT));

    if(numIndents > LENGTH) {
        numIndents = LENGTH;
    }

    return &pINDENT[ LENGTH - numIndents ];
}
#endif


// void dump_to_stdout( const tinyxml2::XMLNode * pParent, unsigned int indent = 0 )
// {
//     if ( !pParent ) return;

//     printf( "%s", getIndent( indent));

//     tinyxml2::XMLPrinter printer;
//     pParent->Accept(&printer);
// }


}

void
tolower(std::string& str)
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](char c){ return (char)std::tolower(c); }
        );
}

std::string
normalize_filename(const std::string& str)
{
    std::string newstring;

    std::string::size_type last_slash = str.find_last_of('/', str.size());
    if(last_slash == std::string::npos) {
        newstring = str;
    }
    else {
        newstring = str.substr(last_slash + 1);
    }

    for(std::string::iterator i = newstring.begin(), e = newstring.end(); i != e; ++i) {
        if(*i == '.') {
            *i = '_';
        }
    }

    tolower(newstring);

    return newstring;
}

static int
checkOverlaySuccess(geom::Geometry const& gRes, geom::Geometry const& gRealRes)
{
    double tol = operation::overlay::snap::GeometrySnapper::computeSizeBasedSnapTolerance(gRes);
    if(gRes.equals(&gRealRes)) {
        return 1;
    }
    std::cerr << "Using an overlay tolerance of " << tol << std::endl;
    if(gRes.equalsExact(&gRealRes, tol)) {
        return 1;
    }
    return 0;
}

/* Could be an XMLTester class private but oh well.. */
static int
checkBufferSuccess(geom::Geometry const& gRes, geom::Geometry const& gRealRes, double dist)
{

    using geos::xmltester::BufferResultMatcher;

    int success = 1;
    do {

        if(gRes.getGeometryTypeId() != gRealRes.getGeometryTypeId()) {
            std::cerr << "Expected result is of type "
                      << gRes.getGeometryType()
                      << "; obtained result is of type "
                      << gRealRes.getGeometryType()
                      << std::endl;
            success = 0;
            break;
        }

        // Is a buffer always an area ?
        if(gRes.getDimension() != 2) {
            std::cerr << "Don't know how to validate "
                      << "result of buffer operation "
                      << "when expected result is not an "
                      << "areal type."
                      << std::endl;
        }

        if(!BufferResultMatcher::isBufferResultMatch(gRealRes, gRes, dist)) {
            std::cerr << "BufferResultMatcher FAILED" << std::endl;
            success = 0;
            break;
        }

    }
    while(0);

    return success;
}

static int
checkSingleSidedBufferSuccess(geom::Geometry& gRes,
                              geom::Geometry& gRealRes, double dist)
{
    int success = 1;
    do {

        if(gRes.getGeometryTypeId() != gRealRes.getGeometryTypeId()) {
            std::cerr << "Expected result is of type "
                      << gRes.getGeometryType()
                      << "; obtained result is of type "
                      << gRealRes.getGeometryType()
                      << std::endl;
            success = 0;
            break;
        }

        geos::xmltester::SingleSidedBufferResultMatcher matcher;
        if(! matcher.isBufferResultMatch(gRealRes,
                                         gRes,
                                         dist)) {
            std::cerr << "SingleSidedBufferResultMatcher FAILED" << std::endl;
            success = 0;
            break;
        }

    }
    while(0);

    return success;
}

XMLTester::XMLTester()
    :
    gA(nullptr),
    gB(nullptr),
    pm(nullptr),
    factory(nullptr),
    wktreader(nullptr),
    wktwriter(nullptr),
    wkbreader(nullptr),
    wkbwriter(nullptr),
    test_predicates(0),
    failed(0),
    succeeded(0),
    caseCount(0),
    testCount(0),
    testFileCount(0),
    totalTestCount(0),
    curr_file(nullptr),
    testValidOutput(false),
    testValidInput(false),
    sqlOutput(false),
    HEXWKB_output(true)
{
    setVerbosityLevel(0);
}

int
XMLTester::setVerbosityLevel(int value)
{
    int old_value = verbose;

    verbose = value;

    return old_value;
}

/*private*/
void
XMLTester::printTest(bool success, const std::string& expected_result, const std::string& actual_result,
                     const util::Profile& prof)
{
    if(sqlOutput) {
        std::cout << "INSERT INTO \"" << normalize_filename(*curr_file) << "\" VALUES ("
                  << caseCount << ", "
                  << testCount << ", "
                  << "'" << opSignature << "', "
                  << "'" << curr_case_desc << "', ";

        std::string geomOut;

        if(gA) {
            std::cout << "'" << printGeom(gA) << "', ";
        }
        else {
            std::cout << "NULL, ";
        }

        if(gB) {
            std::cout << "'" << printGeom(gB) << "', ";
        }
        else {
            std::cout << "NULL, ";
        }

        std::cout << "'" << expected_result << "', "
                  << "'" << actual_result << "', ";

        if(success) {
            std::cout << "'t'";
        }
        else {
            std::cout << "'f'";
        }

        std::cout << ");" << std::endl;
    }

    else {
        std::cout << *curr_file << ":";
        std::cout << " case" << caseCount << ":";
        std::cout << " test" << testCount << ": "
                  << opSignature;
        std::cout << ": " << (success ? "ok." : "failed.");
        std::cout << " (" << std::setprecision(15) << java_math_round(prof.getTot() / 1000) << " ms)" << std::endl;

        // print geometry on failure for -v
        // print geometry no matter what for -v -v and above
        if (verbose > 1 || (verbose == 1 && !success)) {
            std::cout << "\tDescription: " << curr_case_desc << std::endl;

            if(gA) {
                std::cout << "\tGeometry A: ";
                printGeom(std::cout, gA);
                std::cout << std::endl;
            }

            if(gB) {
                std::cout << "\tGeometry B: ";
                printGeom(std::cout, gB);
                std::cout << std::endl;
            }

            std::cout << "\tExpected result: " << expected_result << std::endl;
            std::cout << "\tObtained result: " << actual_result << std::endl;
            std::cout << std::endl;
        }
    }
}

void
XMLTester::run(const std::string& source)
{
    curr_file = &source;

    if(sqlOutput) {
        std::cout << "CREATE TABLE \"" << normalize_filename(*curr_file) << "\""
                  << "( caseno integer, testno integer, "
                  << " operation varchar, description varchar, "
                  << " a geometry, b geometry, expected geometry, "
                  << " obtained geometry, result bool )"

                  // NOTE: qgis 0.7.4 require oids for proper operations.
                  //       The 'WITH OIDS' parameter is supported back to
                  //       PostgreSQL 7.2, so if you run an older version
                  //       rebuild with the next line commented out.
                  //<< " WITH OIDS"

                  << ";" << std::endl;
    }

    ++testFileCount;

    caseCount = 0;

    tinyxml2::XMLError e = xml.LoadFile(source.c_str());
    if(e) {
        std::stringstream err;
        err << "Could not load " << source << ": " << e << std::endl;
        xml.~XMLDocument(); // Deallocates various internal pools
        throw runtime_error(err.str());
    }

    //dump_to_stdout(&xml);

    const tinyxml2::XMLNode* node = xml.FirstChildElement("run");

    if(! node) {
        throw(runtime_error("Document has no childs"));
    }

    parseRun(node);

}

void
XMLTester::resultSummary(std::ostream& os) const
{
    os << "Files: " << testFileCount << std::endl;
    os << "Tests: " << totalTestCount << std::endl;
    os << "Failed: " << failed << std::endl;
    os << "Succeeded: " << succeeded << std::endl;
}

void
XMLTester::resetCounters()
{
    testFileCount = totalTestCount = failed = succeeded = 0;
}

void
XMLTester::parseRun(const tinyxml2::XMLNode* node)
{
    using geos::geom::PrecisionModel;

    assert(node);

    //dump_to_stdout(node);

    // Look for precisionModel element
    const tinyxml2::XMLElement* el = node->FirstChildElement("precisionModel");
    if(el) {
        parsePrecisionModel(el);
    }
    else {
        pm.reset(new PrecisionModel());
    }

    // Look for geometryOperation, if any
    usePrepared = false;
    el = node->FirstChildElement("geometryOperation");
    if(el) {
        const tinyxml2::XMLNode* txt = el->FirstChild();
        if(txt) {
            std::string op = trimBlanks(txt->Value());
            if(op.find("PreparedGeometryOperation")) {
                usePrepared = true;
            }
            else {
                std::cerr << *curr_file
                          << ": WARNING: unknown geometryOperation: "
                          << op << std::endl;
            }
        }
    }

    if(verbose > 1) {
        std::cerr << *curr_file << ": run: Precision Model: " << pm->toString();
        if(usePrepared) {
            std::cerr << " (prepared)";
        }
        std::cerr << std::endl;
    }


    factory = geom::GeometryFactory::create(pm.get());
    wktreader.reset(new io::WKTReader(factory.get()));
    wktwriter.reset(new io::WKTWriter());
    wktwriter->setTrim(true);
    wkbreader.reset(new io::WKBReader(*factory));
    wkbwriter.reset(new io::WKBWriter());

    const tinyxml2::XMLNode* casenode;
    for(casenode = node->FirstChildElement("case");
            casenode;
            casenode = casenode->NextSiblingElement("case")) {
        try {
            parseCase(casenode);
        }
        catch(const std::exception& exc) {
            std::cerr << exc.what() << std::endl;
        }
    }

}

void
XMLTester::parsePrecisionModel(const tinyxml2::XMLElement* el)
{
    using geos::geom::PrecisionModel;

    //dump_to_stdout(el);

    /* This does not seem to work... */
    std::string type;
    const char* typeStr = el->Attribute("type");
    if(typeStr) {
        type = typeStr;
    }

    const char* scaleStr = el->Attribute("scale");

    if(! scaleStr) {
        if(type == "FLOATING_SINGLE") {
            pm.reset(new PrecisionModel(PrecisionModel::FLOATING_SINGLE));
        }
        else {
            pm.reset(new PrecisionModel());
        }
    }
    else {

        char* stopstring;

        double scale = std::strtod(scaleStr, &stopstring);
        double offsetX = 0;
        double offsetY = 2;

        if(! el->QueryDoubleAttribute("offsetx", &offsetX))
        {} // std::cerr << "No offsetx" << std::endl;

        if(! el->QueryDoubleAttribute("offsety", &offsetY))
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
    if(! valid) {
        operation::valid::TopologyValidationError* err = ivo.getValidationError();
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
geom::Geometry*
XMLTester::parseGeometry(const std::string& in, const char* label)
{
    if((! wkbreader.get()) || (! wktreader.get())) {
        throw(runtime_error("No precision model specified"));
    }

    std::stringstream is(in, std::ios_base::in);
    char first_char;

    // Remove leading spaces
    while(is.get(first_char) && std::isspace(first_char));
    is.unget();

    std::unique_ptr<geom::Geometry> ret;

    switch(first_char) {
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

    if(testValidInput) {
        testValid(ret.get(), std::string(label));
    }

    //ret->normalize();

    return ret.release();
}

std::string
XMLTester::trimBlanks(const std::string& in)
{
    std::string out;
    std::string::size_type pos = in.find_first_not_of(" \t\n\r");
    if(pos != std::string::npos) {
        out = in.substr(pos);
    }
    pos = out.find_last_not_of(" \t\n\r");
    if(pos != std::string::npos) {
        out = out.substr(0, pos + 1);
    }
    return out;
}

void
XMLTester::parseCase(const tinyxml2::XMLNode* node)
{
    assert(node);

    std::string geomAin;
    std::string geomBin;
    std::string thrownException;

    gA = nullptr;
    gB = nullptr;


    //dump_to_stdout(node);

    curr_case_desc.clear();
    const tinyxml2::XMLNode* txt = node->FirstChildElement("desc");
    if(txt) {
        txt = txt->FirstChild();
        if(txt) {
            curr_case_desc = trimBlanks(txt->Value());
        }
    }

    //std::cerr << "Desc: " << curr_case_desc << std::endl;


    try {
        const tinyxml2::XMLNode* el = node->FirstChildElement("a");
        geomAin = el->FirstChild()->Value();
        geomAin = trimBlanks(geomAin);
        gA = parseGeometry(geomAin, "Geometry A");

        if(nullptr != (el = node->FirstChildElement("b"))) {
            geomBin = el->FirstChild()->Value();
            geomBin = trimBlanks(geomBin);
            gB = parseGeometry(geomBin, "Geometry B");
        }
    }
    catch(const std::exception& e) {
        thrownException = e.what();
    }
    catch(...) {
        thrownException = "Unknown exception";
    }

//std::cerr << "A: " << geomAin << std::endl;
//std::cerr << "B: " << geomBin << std::endl;


    if(thrownException != "") {
        std::cout << *curr_file << ":";
        std::cout << " case" << caseCount << ":";
        std::cout << " skipped (" << thrownException << ")." << std::endl;
        return;
    }

    ++caseCount;
    testCount = 0;

    const tinyxml2::XMLNode* testnode;
    for(testnode = node->FirstChildElement("test");
            testnode;
            testnode = testnode->NextSiblingElement("test")) {
        parseTest(testnode);
    }

    totalTestCount += testCount;

    delete gA;
    delete gB;
}

/*private*/
void
XMLTester::printGeom(std::ostream& os, const geom::Geometry* g)
{
    os << printGeom(g);
}

std::string
XMLTester::printGeom(const geom::Geometry* g)
{
    if(HEXWKB_output) {
        std::stringstream s(std::ios_base::binary | std::ios_base::in | std::ios_base::out);
        wkbwriter->write(*g, s);
        std::stringstream s2;
        wkbreader->printHEX(s, s2);
        return s2.str();
    }
    else {
        wktwriter->setRoundingPrecision(16);
        return wktwriter->write(g);
    }
}

/**
* Computes the maximum area delta value
* resulting from identity equations over the overlay operations.
* The delta value is normalized to the total area of the geometries.
* If the overlay operations are computed correctly
* the area delta is expected to be very small (e.g. < 1e-6).
*/
double
XMLTester::areaDelta(const geom::Geometry* a, const geom::Geometry* b, std::string& rsltMaxDiffOp, double maxDiff, std::stringstream& ss)
{
    double areaA = a == nullptr ? 0 : a->getArea();
    double areaB = b == nullptr ? 0 : b->getArea();

    // if an input is non-polygonal delta is 0
    if (areaA == 0 || areaB == 0)
      return 0;

    std::unique_ptr<geom::Geometry> geomU = OverlayNGRobust::Union(a, b);
    std::unique_ptr<geom::Geometry> geomI = OverlayNGRobust::Intersection(a, b);
    std::unique_ptr<geom::Geometry> geomDab = OverlayNGRobust::Difference(a, b);
    std::unique_ptr<geom::Geometry> geomDba = OverlayNGRobust::Difference(b, a);
    std::unique_ptr<geom::Geometry> geomSD = OverlayNGRobust::SymDifference(a, b);

    double areaU   = geomU->getArea();
    double areaI   = geomI->getArea();
    double areaDab = geomDab->getArea();
    double areaDba = geomDba->getArea();
    double areaSD  = geomSD->getArea();


    double maxDelta = 0;

    // & : intersection
    // - : difference
    // + : union
    // ^ : symdifference

    double delta = std::abs(areaA - areaI - areaDab);
    if (delta > maxDelta) {
        rsltMaxDiffOp = "A = ( A & B ) + ( A - B )";
        maxDelta = delta;
    }

    delta = std::abs(areaB - areaI - areaDba);
    if (delta > maxDelta) {
        rsltMaxDiffOp = "B = ( A & B ) + ( B - A )";
        maxDelta = delta;
    }

    delta = std::abs(areaDab + areaDba - areaSD);
    if (delta > maxDelta) {
        maxDelta = delta;
        rsltMaxDiffOp = "( A ^ B ) = ( A - B ) + ( B - A )";
    }

    delta = std::abs(areaI + areaSD - areaU);
    if (delta > maxDelta) {
        maxDelta = delta;
        rsltMaxDiffOp = "( A + B ) = ( A & B ) + ( A ^ B )";
    }

    delta = std::abs(areaU - areaI - areaDab - areaDba);
    if (delta > maxDelta) {
        maxDelta = delta;
        rsltMaxDiffOp = "( A + B ) = ( A & B ) + ( A - B ) + ( A - B )";
    }

    // normalize the area delta value
    double diffScore = maxDelta / (areaA + areaB);

    if (diffScore > maxDiff) {
        ss << std::endl << "A" << std::endl;
        ss << *a;
        ss << std::endl << "B" << std::endl;
        ss << *b;
        ss << std::endl << "geomU" << std::endl;
        ss << *geomU;
        ss << std::endl << "geomI" << std::endl;
        ss << *geomI;
        ss << std::endl << "geomDab" << std::endl;
        ss << *geomDab;
        ss << std::endl << "geomDba" << std::endl;
        ss << *geomDba;
        ss << std::endl << "geomSD" << std::endl;
        ss << *geomSD;
        ss << std::endl;
    }

    return diffScore;
}


void
XMLTester::parseTest(const tinyxml2::XMLNode* node)
{
    using namespace operation::overlay;

    typedef std::unique_ptr< geom::Geometry > GeomPtr;

    int success = 0; // no success by default
    std::string opName;
    std::string opArg1;
    std::string opArg2;
    std::string opArg3;
    std::string opArg4;
    std::string opRes;

    ++testCount;

    const tinyxml2::XMLNode* opnode = node->FirstChildElement("op");
    if(! opnode) {
        throw(runtime_error("case has no op"));
    }

    //dump_to_stdout(opnode);

    const tinyxml2::XMLElement* opel = opnode->ToElement();

    const char* tmp = opel->Attribute("name");
    if(tmp) {
        opName = tmp;
    }

    tmp = opel->Attribute("arg1");
    if(tmp) {
        opArg1 = tmp;
    }

    tmp = opel->Attribute("arg2");
    if(tmp) {
        opArg2 = tmp;
    }

    tmp = opel->Attribute("arg3");
    if(tmp) {
        opArg3 = tmp;
    }

    tmp = opel->Attribute("arg4");
    if(tmp) {
        opArg4 = tmp;
    }

    const tinyxml2::XMLNode* resnode = opnode->FirstChild();
    if(! resnode) {
        std::stringstream p_tmp;
        p_tmp << "op of test " << testCount
              << " of case " << caseCount
              << " has no expected result child";
        throw(runtime_error(p_tmp.str()));
    }
    opRes = resnode->Value();

    // trim blanks
    opRes = trimBlanks(opRes);
    opName = trimBlanks(opName);
    tolower(opName);

    std::string opSig = "";

    if(opArg1 != "") {
        opSig = opArg1;
    }
    if(opArg2 != "") {
        if(opSig != "") {
            opSig += ", ";
        }
        opSig += opArg2;
    }
    if(opArg3 != "") {
        if(opSig != "") {
            opSig += ", ";
        }
        opSig += opArg3;
    }
    if(opArg4 != "") {
        if(opSig != "") {
            opSig += ", ";
        }
        opSig += opArg4;
    }

    opSignature = opName + "(" + opSig + ")";

    std::string actual_result = "NONE";

    // expected_result will be modified by specific tests
    // if needed (geometry normalization, for example)
    std::string expected_result = opRes;

    util::Profile profile("op");

    try {
        if(opName == "relate") {
            std::unique_ptr<geom::IntersectionMatrix> im(gA->relate(gB));
            assert(im.get());

            if(im->matches(opArg3)) {
                actual_result = "true";
            }
            else {
                actual_result = "false";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }
        else if(opName == "relatestring") {
            std::unique_ptr<geom::IntersectionMatrix> im(gA->relate(gB));
            assert(im.get());

            actual_result = im->toString();

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "isvalid") {
            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            if(p_gT->isValid()) {
                actual_result = "true";
            }
            else {
                actual_result = "false";
            }

            if(actual_result == opRes) {
                success = 1;
            }

        }

        else if(opName == "intersection") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes(gA->intersection(gB));

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "intersectionng") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes = OverlayNG::overlay(gA, gB, OverlayNG::INTERSECTION);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "unionng") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes = OverlayNG::overlay(gA, gB, OverlayNG::UNION);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "differenceng") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            const geom::Geometry* dgA = gA;
            const geom::Geometry* dgB = gB;

            // Swap arguments if necessary
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                dgA = gB;
                dgB = gA;
            }

            GeomPtr gRealRes = OverlayNG::overlay(dgA, dgB, OverlayNG::DIFFERENCE);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "symdifferenceng") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes = OverlayNG::overlay(gA, gB, OverlayNG::SYMDIFFERENCE);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }


        else if(opName == "intersectionsr") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();
            double precision = 1.0;

            if(opArg3 != "") {
                precision = std::atof(opArg3.c_str());
            }

            profile.start();
            geom::PrecisionModel precMod(precision);
            GeomPtr gRealRes = OverlayNG::overlay(gA, gB, OverlayNG::INTERSECTION, &precMod);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "intersectionsin") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();
            double precision = 1.0;

            if(opArg3 != "") {
                precision = std::atof(opArg3.c_str());
            }

            profile.start();
            geom::PrecisionModel precMod(precision);
            GeomPtr gRealRes = OverlayNGRobust::Intersection(gA, gB);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }


        else if(opName == "unionsr") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();
            double precision = 1.0;
            GeomPtr gRealRes;

            if (gB) {
                geom::PrecisionModel precMod(precision);
                gRealRes = OverlayNG::overlay(gA, gB, OverlayNG::UNION, &precMod);
                if(opArg3 != "") {
                    precision = std::atof(opArg3.c_str());
                }
            }
            else {
                geom::PrecisionModel precMod(precision);

                // gRealRes = OverlayNG::geomunion(gA, &precMod);
                gRealRes = UnaryUnionNG::Union(gA, precMod);
                if(opArg2 != "") {
                    precision = std::atof(opArg2.c_str());
                }
            }

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "differencesr") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();
            double precision = 1.0;

            if(opArg3 != "") {
                precision = std::atof(opArg3.c_str());
            }

            const geom::Geometry* dgA = gA;
            const geom::Geometry* dgB = gB;

            // Swap arguments if necessary
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                dgA = gB;
                dgB = gA;
            }

            profile.start();
            geom::PrecisionModel precMod(precision);
            GeomPtr gRealRes = OverlayNG::overlay(dgA, dgB, OverlayNG::DIFFERENCE, &precMod);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }


        else if(opName == "symdifferencesr") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();
            double precision = 1.0;

            if(opArg3 != "") {
                precision = std::atof(opArg3.c_str());
            }

            profile.start();
            geom::PrecisionModel precMod(precision);
            GeomPtr gRealRes = OverlayNG::overlay(gA, gB, OverlayNG::SYMDIFFERENCE, &precMod);

            profile.stop();

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "densify") {
            geom::Geometry* p_gT = gA;

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            // gRes->normalize();

            geom::util::Densifier den(p_gT);
            double distanceTolerance = std::atof(opArg2.c_str());
            den.setDistanceTolerance(distanceTolerance);
            GeomPtr gRealRes = den.getResultGeometry();

            // gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

        }


        else if(opName == "union") {
            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes;
            if(gB) {
                gRealRes = gA->Union(gB);
            }
            else {
                gRealRes = gA->Union();
            }

            profile.stop();
            gRealRes->normalize();

            success = checkOverlaySuccess(*gRes, *gRealRes);

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "difference") {

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            GeomPtr gRealRes(gA->difference(gB));

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "symdifference") {
            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            GeomPtr gRealRes(gA->symDifference(gB));

            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "intersects") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->intersects(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->intersects(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "contains") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->contains(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->contains(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "overlaps") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->overlaps(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->overlaps(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "within") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->within(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->within(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "touches") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->touches(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->touches(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "crosses") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->crosses(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->crosses(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "disjoint") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->disjoint(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->disjoint(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "covers") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->covers(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->covers(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        // equalsTopo() is synomym for equals() in JTS
        else if(opName == "equalstopo") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(g1->equals(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "equalsexact") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(g1->equalsExact(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        // rather than implementing equalsnorm in the library,
        // we just do it in this one test case for now
        else if(opName == "equalsnorm") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            g1->normalize();
            g2->normalize();

            actual_result = "false";
            if(g1->equalsExact(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }


        else if(opName == "coveredby") {
            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;

            actual_result = "false";
            if(usePrepared) {
                if(prepare(g1)->coveredBy(g2)) {
                    actual_result = "true";
                }
            }
            else if(g1->coveredBy(g2)) {
                actual_result = "true";
            }

            if(actual_result == opRes) {
                success = 1;
            }
        }

        else if(opName == "getboundary") {
            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            GeomPtr gRealRes(p_gT->getBoundary());
            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "getcentroid") {
            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            GeomPtr gRealRes(p_gT->getCentroid());

            if(gRealRes.get()) {
                gRealRes->normalize();
            }
            else {
                gRealRes = factory->createPoint();
            }
            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "issimple") {
            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            if(p_gT->isSimple()) {
                actual_result = "true";
            }
            else {
                actual_result = "false";
            }

            if(actual_result == opRes) {
                success = 1;
            }

        }

        else if(opName == "convexhull") {
            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            GeomPtr gRealRes(p_gT->convexHull());
            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "buffer") {
            using namespace operation::buffer;

            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes;
            double dist = std::atof(opArg2.c_str());

            BufferParameters params;
            if(opArg3 != "") {
                params.setQuadrantSegments(std::atoi(opArg3.c_str()));
            }


            BufferOp op(p_gT, params);
            gRealRes.reset(op.getResultGeometry(dist));

            profile.stop();
            gRealRes->normalize();

            // Validate the buffer operation
            success = checkBufferSuccess(*gRes, *gRealRes, dist);

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "buffersinglesided") {
            using namespace operation::buffer;

            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes;
            double dist = std::atof(opArg2.c_str());

            BufferParameters params ;
            params.setJoinStyle(BufferParameters::JOIN_ROUND) ;
            if(opArg3 != "") {
                params.setQuadrantSegments(std::atoi(opArg3.c_str()));
            }

            bool leftSide = true ;
            if(opArg4 == "right") {
                leftSide = false ;
            }

            BufferBuilder bufBuilder(params) ;
            gRealRes.reset(bufBuilder.bufferLineSingleSided(
                               p_gT, dist, leftSide)) ;

            profile.stop();
            gRealRes->normalize();

            // Validate the single sided buffer operation
            success = checkSingleSidedBufferSuccess(*gRes,
                                                    *gRealRes, dist);

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "buffermitredjoin") {
            using namespace operation::buffer;

            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            profile.start();

            GeomPtr gRealRes;
            double dist = std::atof(opArg2.c_str());

            BufferParameters params;
            params.setJoinStyle(BufferParameters::JOIN_MITRE);

            if(opArg3 != "") {
                params.setQuadrantSegments(std::atoi(opArg3.c_str()));
            }

            BufferOp op(p_gT, params);
            gRealRes.reset(op.getResultGeometry(dist));

            profile.stop();
            gRealRes->normalize();

            // Validate the buffer operation
            success = checkBufferSuccess(*gRes, *gRealRes, dist);

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }


        else if(opName == "getinteriorpoint") {
            geom::Geometry* p_gT = gA;
            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            GeomPtr gRes(parseGeometry(opRes, "expected"));
            gRes->normalize();

            GeomPtr gRealRes(p_gT->getInteriorPoint());
            if(gRealRes.get()) {
                gRealRes->normalize();
            }
            else {
                gRealRes = factory->createPoint();
            }

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "iswithindistance") {
            double dist = std::atof(opArg3.c_str());
            if(gA->isWithinDistance(gB, dist)) {
                actual_result = "true";
            }
            else {
                actual_result = "false";
            }

            if(actual_result == opRes) {
                success = 1;
            }

        }

        else if(opName == "polygonize") {

            GeomPtr gRes(wktreader->read(opRes));
            gRes->normalize();

            Polygonizer plgnzr;
            plgnzr.add(gA);


            auto polys = plgnzr.getPolygons();
            std::vector<geom::Geometry*>* newgeoms = new std::vector<geom::Geometry*>;
            for(unsigned int i = 0; i < polys.size(); i++) {
                newgeoms->push_back(polys[i].release());
            }

            GeomPtr gRealRes(factory->createGeometryCollection(newgeoms));
            gRealRes->normalize();


            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "linemerge") {
            GeomPtr gRes(wktreader->read(opRes));
            gRes->normalize();

            geom::Geometry* p_gT = gA;

            if((opArg1 == "B" || opArg1 == "b") && gB) {
                p_gT = gB;
            }

            LineMerger merger;
            merger.add(p_gT);
            auto lines = merger.getMergedLineStrings();

            GeomPtr gRealRes(factory->createGeometryCollection(std::move(lines)));
            gRealRes->normalize();

            if(gRes->compareTo(gRealRes.get()) == 0) {
                success = 1;
            }

            actual_result = printGeom(gRealRes.get());
            expected_result = printGeom(gRes.get());

            if(testValidOutput) {
                success &= int(testValid(gRealRes.get(), "result"));
            }
        }

        else if(opName == "overlayareatest") {

            std::string maxDiffOp;
            std::stringstream p_tmp;
            double maxDiff = 1e-6;
            double areaDiff = areaDelta(gA, gB, maxDiffOp, maxDiff, p_tmp);

            // Debug output of actual geometries returned
            if (areaDiff < maxDiff && false) {
                std::cout << p_tmp.str();
            }

            p_tmp.str("");
            p_tmp << maxDiffOp << ": " << areaDiff;
            actual_result = p_tmp.str();
            p_tmp.str("");
            p_tmp << maxDiff;
            expected_result = p_tmp.str();

            if (areaDiff < maxDiff)
                success = 1;
        }

        else if(opName == "unionlength") {

            char* rest;
            GeomPtr result = OverlayNGRobust::Union(gA);
            double resultLength = result->getLength();
            double expectedLength = std::strtod(opRes.c_str(), &rest);
            if(rest == opRes.c_str()) {
                throw std::runtime_error("malformed testcase: missing expected length 'unionlength' op");
            }

            std::stringstream ss;
            ss << resultLength;
            actual_result = ss.str();

            if (std::abs(expectedLength-resultLength) / expectedLength < 1e-3) {
                success = 1;
            }

        }

        else if(opName == "unionarea") {

            char* rest;
            GeomPtr result = OverlayNGRobust::Union(gA);
            double resultArea  = result->getArea();
            double expectedArea = std::strtod(opRes.c_str(), &rest);
            if(rest == opRes.c_str()) {
                throw std::runtime_error("malformed testcase: missing expected area 'unionarea' op");
            }

            std::stringstream ss;
            ss << resultArea;
            actual_result = ss.str();

            if (std::abs(expectedArea-resultArea) / expectedArea < 1e-3) {
                success = 1;
            }

        }

        else if(opName == "areatest") {
            char* rest;
            double toleratedDiff = std::strtod(opRes.c_str(), &rest);
            int validOut = 1;

            if(rest == opRes.c_str()) {
                throw std::runtime_error("malformed testcase: missing tolerated area difference in 'areatest' op");
            }

            if(verbose > 1) {
                std::cerr << "Running intersection for areatest" << std::endl;
            }
            GeomPtr gI(gA->intersection(gB));

            if(testValidOutput) {
                validOut &= int(testValid(gI.get(), "areatest intersection"));
            }

            if(verbose > 1) {
                std::cerr << "Running difference(A,B) for areatest" << std::endl;
            }

            GeomPtr gDab(gA->difference(gB));

            if(testValidOutput) {
                validOut &= int(testValid(gI.get(), "areatest difference(a,b)"));
            }

            if(verbose > 1) {
                std::cerr << "Running difference(B,A) for areatest" << std::endl;
            }

            GeomPtr gDba(gB->difference(gA));

            if(testValidOutput) {
                validOut &= int(testValid(gI.get(), "areatest difference(b,a)"));
            }

            if(verbose > 1) {
                std::cerr << "Running symdifference for areatest" << std::endl;
            }

            GeomPtr gSD(gA->symDifference(gB));

            if(testValidOutput) {
                validOut &= int(testValid(gI.get(), "areatest symdifference"));
            }

            if(verbose > 1) {
                std::cerr << "Running union for areatest" << std::endl;
            }

            GeomPtr gU(gA->Union(gB));

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
            double diff = std::fabs(areaA - areaI - areaDab);
            if(diff > maxdiff) {
                maxdiffop = "A == ( A ^ B ) + ( A - B )";
                maxdiff = diff;
            }

            // B == ( A ^ B ) + ( B - A )
            diff = std::fabs(areaB - areaI - areaDba);
            if(diff > maxdiff) {
                maxdiffop = "B == ( A ^ B ) + ( B - A )";
                maxdiff = diff;
            }

            //  ( A @ B ) == ( A - B ) + ( B - A )
            diff = std::fabs(areaDab + areaDba - areaSD);
            if(diff > maxdiff) {
                maxdiffop = "( A @ B ) == ( A - B ) + ( B - A )";
                maxdiff = diff;
            }

            //  ( A u B ) == ( A ^ B ) + ( A @ B )
            diff = std::fabs(areaI + areaSD - areaU);
            if(diff > maxdiff) {
                maxdiffop = "( A u B ) == ( A ^ B ) + ( A @ B )";
                maxdiff = diff;
            }

            if(maxdiff <= toleratedDiff) {
                success = 1 && validOut;
            }

            std::stringstream p_tmp;
            p_tmp << maxdiffop << ": " << maxdiff;
            actual_result = p_tmp.str();
            expected_result = opRes;

        }
        else if(opName == "distance") {
            char* rest;
            double distE = std::strtod(opRes.c_str(), &rest);
            if(rest == opRes.c_str()) {
                throw std::runtime_error("malformed testcase: missing expected result in 'distance' op");
            }

            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            geom::Geometry* g2 = opArg2 == "B" ? gB : gA;
            double distO = g1->distance(g2);
            std::stringstream ss;
            ss << distO;
            actual_result = ss.str();

            // TODO: Use a tolerance ?
            success = (distO == distE) ? 1 : 0;
        }
        else if(opName == "minclearance") {
            char* rest;
            double minclearanceE = std::strtod(opRes.c_str(), &rest);
            if(rest == opRes.c_str()) {
                throw std::runtime_error("malformed testcase: missing expected result in 'minclearance' op");
            }

            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            precision::MinimumClearance mc(g1);

            double minclearanceO = mc.getDistance();
            std::stringstream ss;
            ss << minclearanceO;
            actual_result = ss.str();

            // Hack for Inf/1.7976931348623157E308 comparison
            if(minclearanceO > 1.7976931348623157E308) {
                minclearanceO = 1.7976931348623157E308;
            }

            // TODO: Use a tolerance ?
            success = (minclearanceO == minclearanceE) ? 1 : 0;
        }
        else if(opName == "minclearanceline") {

            double tol = 0.0000001;
            GeomPtr lineE(parseGeometry(opRes, "expected"));
            if(!lineE) {
                throw std::runtime_error("malformed testcase: missing expected result in 'minclearanceline' op");
            }

            geom::Geometry* g1 = opArg1 == "B" ? gB : gA;
            precision::MinimumClearance mc(g1);
            std::unique_ptr<geom::Geometry> lineO = mc.getLine();
            lineO.get()->normalize();
            lineE.get()->normalize();

            actual_result = printGeom(lineO.get());
            success = lineE.get()->equalsExact(lineO.get(), tol) ? 1 : 0;
        }

        else if (opName == "buildarea")
        {
            GeomPtr gExpected(parseGeometry(opRes, "expected"));
            gExpected->normalize();

            auto gGot = BuildArea().build(gA);
            if( gGot )
            {
                GeomPtr gRealRes(gGot.release());
                gRealRes->normalize();

                if (gExpected->equals(gRealRes.get())) success=1;

                actual_result=printGeom(gRealRes.get());
                expected_result=printGeom(gExpected.get());
                if( actual_result == expected_result ) success=1;

                if ( testValidOutput )
                    success &= int(testValid(gRealRes.get(), "result"));
            }
            else
            {
                success = false;
            }
        }

        else if (opName == "makevalid")
        {
            GeomPtr gExpected(parseGeometry(opRes, "expected"));
            gExpected->normalize();

            auto gGot = geos::operation::valid::MakeValid().build(gA);
            if( gGot )
            {
                GeomPtr gRealRes(gGot.release());
                gRealRes->normalize();

                if (gExpected->equals(gRealRes.get())) success=1;

                actual_result=printGeom(gRealRes.get());
                expected_result=printGeom(gExpected.get());
                if( actual_result == expected_result ) success=1;

                if ( testValidOutput )
                    success &= int(testValid(gRealRes.get(), "result"));
            }
            else
            {
                success = false;
            }
        }

        else {
            std::cerr << *curr_file << ":";
            std::cerr << " case" << caseCount << ":";
            std::cerr << " test" << testCount << ": "
                      << opName << "(" << opSig << ")";
            std::cerr << ": skipped (unrecognized)." << std::endl;
            return;
        }

    }
    catch(const std::exception& e) {
        std::cerr << "EXCEPTION on case " << caseCount
                  << " test " << testCount << ": " << e.what()
                  << std::endl;
        actual_result = e.what();
    }
    catch(...) {
        std::cerr << "Unknown EXEPTION on case "
                  << caseCount
                  << std::endl;
        actual_result = "Unknown exception thrown";
    }

    if(success) {
        ++succeeded;
    }
    else {
        ++failed;
    }

    if((!success && verbose) || verbose > 0) {
        printTest(!!success, expected_result, actual_result, profile);
    }

    if(test_predicates && gB && gA) {
        runPredicates(gA, gB);
    }

}

void
XMLTester::runPredicates(const geom::Geometry* p_gA, const geom::Geometry* p_gB)
{
    std::cout << "\t    Equals:\tAB=" << (p_gA->equals(p_gB) ? "T" : "F") << ", BA=" << (p_gB->equals(
                  p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t  Disjoint:\tAB=" << (p_gA->disjoint(p_gB) ? "T" : "F") << ", BA=" << (p_gB->disjoint(
                  p_gA) ? "T" : "F") << std::endl;
    std::cout << "\tIntersects:\tAB=" << (p_gA->intersects(p_gB) ? "T" : "F") << ", BA=" << (p_gB->intersects(
                  p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t   Touches:\tAB=" << (p_gA->touches(p_gB) ? "T" : "F") << ", BA=" << (p_gB->touches(
                  p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t   Crosses:\tAB=" << (p_gA->crosses(p_gB) ? "T" : "F") << ", BA=" << (p_gB->crosses(
                  p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t    Within:\tAB=" << (p_gA->within(p_gB) ? "T" : "F") << ", BA=" << (p_gB->within(
                  p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t  Contains:\tAB=" << (p_gA->contains(p_gB) ? "T" : "F") << ", BA=" << (p_gB->contains(
                  p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t  Overlaps:\tAB=" << (p_gA->overlaps(p_gB) ? "T" : "F") << ", BA=" << (p_gB->overlaps(
                  p_gA) ? "T" : "F") << std::endl;

    std::cout << "\t  Prepared Disjoint:\tAB=" << (prepare(p_gA)->disjoint(p_gB) ? "T" : "F") << ", BA=" << (prepare(
                  p_gB)->disjoint(p_gA) ? "T" : "F") << std::endl;
    std::cout << "\tPrepared Intersects:\tAB=" << (prepare(p_gA)->intersects(p_gB) ? "T" : "F") << ", BA=" << (prepare(
                  p_gB)->intersects(p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t   Prepared Touches:\tAB=" << (prepare(p_gA)->touches(p_gB) ? "T" : "F") << ", BA=" << (prepare(
                  p_gB)->touches(p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t   Prepared Crosses:\tAB=" << (prepare(p_gA)->crosses(p_gB) ? "T" : "F") << ", BA=" << (prepare(
                  p_gB)->crosses(p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t    Prepared Within:\tAB=" << (prepare(p_gA)->within(p_gB) ? "T" : "F") << ", BA=" << (prepare(
                  p_gB)->within(p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t  Prepared Contains:\tAB=" << (prepare(p_gA)->contains(p_gB) ? "T" : "F") << ", BA=" << (prepare(
                  p_gB)->contains(p_gA) ? "T" : "F") << std::endl;
    std::cout << "\t Prepared Overlaps:\tAB=" << (prepare(p_gA)->overlaps(p_gB) ? "T" : "F") << ", BA=" << (prepare(
                  p_gB)->overlaps(p_gA) ? "T" : "F") << std::endl;
}

XMLTester::~XMLTester()
{
}


static void
usage(char* me, int exitcode, std::ostream& os)
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

void
request_interrupt(int)
{
    geos::util::Interrupt::request();
}

int
main(int argC, char* argV[])
{
    int verbose = 0;
    bool sql_output = false;

#if defined(_MSC_VER) && defined(GEOS_TEST_USE_STACKWALKER)
    InitAllocCheck();
    {
#endif

        if(argC < 2) {
            usage(argV[0], 1, std::cerr);
        }

        signal(15, request_interrupt);

        XMLTester tester;
        tester.setVerbosityLevel(verbose);

        for(int i = 1; i < argC; ++i) {
            // increment verbosity level
            if(! std::strcmp(argV[i], "-v")) {
                ++verbose;
                tester.setVerbosityLevel(verbose);
                continue;
            }
            if(! std::strcmp(argV[i], "--test-valid-output")) {
                tester.testOutputValidity(true);
                continue;
            }
            if(! std::strcmp(argV[i], "--sql-output")) {
                sql_output = true;
                tester.setSQLOutput(sql_output);
                continue;
            }
            if(! std::strcmp(argV[i], "--wkb-output")) {
                sql_output = true;
                tester.setHEXWKBOutput(sql_output);
                continue;
            }
            if(! std::strcmp(argV[i], "--test-valid-input")) {
                tester.testInputValidity(true);
                continue;
            }

            std::string source = argV[i];

            try {
                tester.run(source);
            }
            catch(const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }

        if(! sql_output) {
            tester.resultSummary(std::cout);
        }
        else {
            tester.resultSummary(std::cerr);
        }

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
 * XMLTester binary ops invoked using the new HeuristicOverlay template function.
 *
 * Revision 1.31  2006/04/07 13:26:38  strk
 * Use of unique_ptr<> to prevent confusing leaks in tester
 *
 * Revision 1.30  2006/03/22 16:01:33  strk
 * indexBintree.h header split, classes renamed to match JTS
 *
 * Revision 1.29  2006/03/17 14:56:39  strk
 * Fixed filename normalizer for sql output
 **********************************************************************/
