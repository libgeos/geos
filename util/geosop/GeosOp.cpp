/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/profiler.h>

#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "WKTStreamReader.h"
#include "WKBStreamReader.h"
#include "GeomFunction.h"
#include "GeosOp.h"
#include "cxxopts.hpp"

using namespace geos;
using namespace geos::geom;
using namespace geos::io;

std::string const GeosOp::opNames[] = {
    "area",
    "boundary",
    "buffer D",
    "centroid",
    "convexHull",
    "envelope",
    "interiorPoint",
    "isValid",
    "length",
    "makeValid",
    "union",
};


void showHelp() {
    std::cout << "geosop executes GEOS geometry operations on inputs." << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: geosop [wktfile] opname args..." << std::endl;
}

static bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

int main(int argc, char** argv) {
    GeomFunction::init();

    GeosOpArgs cmdArgs;

    cxxopts::Options options("geosop", "Executes GEOS geometry operations");
    options.add_options()
        ("a", "source for A geometries (WKT, WKB, file, stdin, stdin.wkb)", cxxopts::value<std::string>( cmdArgs.srcA ))
        ("b", "source for B geometries (WKT, WKB, file, stdin, stdin.wkb)", cxxopts::value<std::string>( cmdArgs.srcB ))
        ("alimit", "Limit nunber of A geometries read", cxxopts::value<int>( cmdArgs.limitA ))
        ("c,collect", "Collect input into single geometry", cxxopts::value<bool>( cmdArgs.isCollect ))
        ("e,explode", "Explode results into component geometries", cxxopts::value<bool>( cmdArgs.isExplode))
        ("f,format", "Output format (wkt, wkb or txt)", cxxopts::value<std::string>( ))
        ("h,help", "Print help")
        ("p,precision", "Sets number of decimal places in output coordinates", cxxopts::value<int>( cmdArgs.precision ) )
        ("r,repeat", "Repeat operation N times", cxxopts::value<int>( cmdArgs.repeatNum ) )
        ("t,time", "Print execution time", cxxopts::value<bool>( cmdArgs.isShowTime ) )
        ("v,verbose", "Verbose output", cxxopts::value<bool>( cmdArgs.isVerbose )->default_value("false"))

        ("opName", "Operation name", cxxopts::value<std::string>()->default_value("no-op"))
        ("opArgs", "Operation arguments ", cxxopts::value<std::vector<std::string>>())
    ;

    options.parse_positional({"opName", "opArgs"});
    auto result = options.parse(argc, argv);

    if (argc <= 1 || result.count("help")) {
        std::cout << "geosop - GEOS " << geosversion() << std::endl;
        options.positional_help("opName opArg");
        std::cout << options.help() << std::endl;
        //showHelp();
        if (result.count("help")) {
            std::cout << "Notes:" << std::endl;
            std::cout << "- Negative numeric op arguments can be specified with leading N:  e.g. N-0.1" << std::endl;
            std::cout << std::endl;
            std::cout << "Operations:" << std::endl;
            std::vector<std::string> ops = GeomFunction::list();
            for (auto opName : ops) {
               std::cout << "  " << opName << std::endl;
            }

        }
        return 0;
    }

    if (result.count("format")) {
        auto fmt = result["format"].as<std::string>();
        if (fmt == "txt" || fmt == "wkt" ) {
            cmdArgs.format = GeosOpArgs::fmtText;
        }
        else if (fmt == "wkb") {
            cmdArgs.format = GeosOpArgs::fmtWKB;
        }
        else {
            std::cerr << "Invalid format value: " << fmt << std::endl;
            exit(1);
        }
    }

    if (result.count("opName")) {
        cmdArgs.opName = result["opName"].as<std::string>();
    }
    //--- parse positional op arg (only one supported for now)
    if (result.count("opArgs"))
    {
        auto& v = result["opArgs"].as<std::vector<std::string>>();
        if (v.size() >= 1) {
            auto val = v[0];
            /**
             * To get around cmdline parset limitation for parsing neg numbers,
             * allow syntax "Nnum" as well
             */
            if (startsWith(val, "N")) {
                val = val.substr(1, val.size()-1);
            }
            cmdArgs.opArg1 = std::stod(val);
        }
    }

    GeosOp geosop(cmdArgs);
    geosop.run();
}

GeosOp::GeosOp(GeosOpArgs& arg)
    : args(arg)
{
}

GeosOp::~GeosOp() {
}

std::string formatNum(int n)
{
    auto fmt = std::to_string(n);
    int insertPosition = static_cast<int>(fmt.length()) - 3;
    while (insertPosition > 0) {
        fmt.insert(insertPosition, ",");
        insertPosition-=3;
    }
    return fmt ;
}

std::vector<std::unique_ptr<Geometry>> collect( std::vector<std::unique_ptr<Geometry>>& geoms ) {
    auto gf = GeometryFactory::create();
    auto gc = gf->createGeometryCollection( std::move(geoms) );
    std::vector<std::unique_ptr<Geometry>> geomsColl;
    geomsColl.push_back( std::move( gc ) );
    return geomsColl;
}

bool isWKTLiteral(std::string s) {
    // check for empty geoms (which do not have parens)
    if (endsWith(s, " EMPTY")) return true;

    // assume if string contains a ( it is WKT
    int numLParen = std::count(s.begin(), s.end(), '(');
    return numLParen > 0;
}

bool isWKBLiteral(std::string s) {
    // assume WKB if only chars are [0-9] and [a-fA-F]
    const std::string hexChars = "0123456789abcdefABCDEF";
    return s.find_first_not_of(hexChars) == std::string::npos;
}

std::vector<std::unique_ptr<Geometry>>
readWKTFile(std::istream& in, int limit) {

    WKTStreamReader rdr( in );
    std::vector<std::unique_ptr<Geometry>> geoms;
    int count = 0;
    while (limit < 0 || count < limit) {
        auto geom = rdr.next();
        if (geom == nullptr)
            break;
        geoms.push_back( std::unique_ptr<Geometry>(geom) );
        count++;
    }
    return geoms;
}

std::vector<std::unique_ptr<Geometry>>
readWKTFile(std::string src, int limit) {
    if (src == "-" || src == "-.wkt" || src == "stdin" || src == "stdin.wkt") {
        return readWKTFile( std::cin, limit );
    }
    std::ifstream f( src );
    auto geoms = readWKTFile( f, limit );
    f.close();
    return geoms;
}

std::vector<std::unique_ptr<Geometry>>
readWKBFile(std::istream& in, int limit) {
    WKBStreamReader rdr( in );
    std::vector<std::unique_ptr<Geometry>> geoms;
    int count = 0;
    while (limit < 0 || count < limit) {
        auto geom = rdr.next();
        if (geom == nullptr)
            break;
        geoms.push_back( std::unique_ptr<Geometry>(geom) );
        count++;
    }
    return geoms;
}

std::vector<std::unique_ptr<Geometry>>
readWKBFile(std::string src, int limit) {
    if (src == "-.wkb" || src == "stdin.wkb" ) {
        return readWKBFile( std::cin, limit );
    }
    std::ifstream f( src );
    auto geoms = readWKBFile( f, limit );
    f.close();
    return geoms;
}

void GeosOp::log(std::string s) {
    if (args.isVerbose) {
        std::cout << s << std::endl;
    }

}

std::vector<std::unique_ptr<Geometry>>
GeosOp::readInput(std::string name, std::string src, int limit) {
    std::vector<std::unique_ptr<Geometry>> geoms;
    std::string srcDesc = "Input " + name + ": ";
    if ( isWKTLiteral(src) ) {
        log(srcDesc + "WKT literal");

        geos::io::WKTReader rdr;
        auto geom = rdr.read( src );
        geoms.push_back( std::move(geom) );
    }
    else if ( isWKBLiteral(src) ) {
        log(srcDesc + "WKB literal");

        geos::io::WKBReader rdr;
        std::istringstream hex(src);
        auto geom = rdr.readHEX( hex );
        geoms.push_back( std::move(geom) );
    }
    else if (endsWith(src, ".wkb")) {
        log(srcDesc + "WKB file " + src);
        geoms = readWKBFile( src, limit );
    }
    else {
        log(srcDesc + "WKT file " + src);
        geoms = readWKTFile( src, limit );
    }
    return geoms;
}

std::string geomStats(int geomCount, int geomVertices) {
    return formatNum( geomCount) + " geometries, "
        + formatNum( geomVertices) + " vertices";

}
std::string summaryStats(std::vector<std::unique_ptr<Geometry>>& geoms) {
    int geomCount = 0;
    int geomPts = 0;
    for (const  auto& geom : geoms) {
        geomCount++;
        geomPts += geom->getNumPoints();
    }
    return geomStats(geomCount, geomPts);
}

std::vector<std::unique_ptr<Geometry>>
GeosOp::loadInput(std::string name, std::string src, int limit) {
    if (src.length() == 0) {
        std::vector<std::unique_ptr<Geometry>> geoms;
        return geoms;
    }
    geos::util::Profile sw( "Read" );
    sw.start();
    auto geoms = readInput( name, src, limit );
    sw.stop();
    auto stats = summaryStats(geoms);
    log("Read " + stats  + "  -- " + formatNum( sw.getTot() ) + " usec");
    return geoms;
}

void GeosOp::run() {
    // ensure at least one op processed
    if (args.repeatNum < 1) args.repeatNum = 1;

    auto geomsLoadA = loadInput("A", args.srcA, args.limitA);

    //--- collect input into single geometry collection if specified
    if (args.isCollect && geomsLoadA.size() > 1) {
        geomA = collect( geomsLoadA );
    }
    else {
        geomA = std::move(geomsLoadA);
    }

    geomB = loadInput("B", args.srcB, -1);

    //------------------------

    execute();

    if (args.isShowTime || args.isVerbose) {
        std::cout
            << "Ran " <<  formatNum( opCount ) << " " << args.opName << " ops ( "
            << formatNum( vertexCount ) << " vertices)"
            << "  -- " << formatNum( totalTime ) <<  " usec"
            << "    (GEOS " << geosversion() << ")"
            << std::endl;
    }
}

void GeosOp::execute() {
    std::string op = args.opName;

    GeomFunction * fun;
    if (op == "" || op == "no-op") {
        op = "copy";
    }
    fun = GeomFunction::find(op);

    if (fun == nullptr) {
        std::cerr << "Unknown operation: " << op << std::endl;
        exit(1);
    }

    if (fun->isBinary()) {
        executeBinary(fun);
    }
    else {
        executeUnary(fun);
    }
}

void GeosOp::executeUnary(GeomFunction * fun) {
    for (unsigned i = 0; i < geomA.size(); i++) {
        vertexCount += geomA[i]->getNumPoints();
        Result* result = executeOpRepeat(fun, i, geomA[i], 0, nullptr);

        output(result);
        delete result;
    }
}

void GeosOp::executeBinary(GeomFunction * fun) {
    for (unsigned ia = 0; ia < geomA.size(); ia++) {
        for (unsigned ib = 0; ib < geomB.size(); ib++) {
            vertexCount += geomA[ia]->getNumPoints();
            vertexCount += geomB[ib]->getNumPoints();
            Result* result = executeOpRepeat(fun, ia, geomA[ia], ib, geomB[ib]);

            output(result);
            delete result;
        }
    }
}

std::string inputDesc(std::string name, int index, const std::unique_ptr<Geometry>& geom)
{
    if (geom == nullptr) {
        return "";
    }
    std::string desc = name + "[" + std::to_string(index+1) + "] " + geom->getGeometryType()
        + "( " + std::to_string(geom->getNumPoints()) + " )";
    return desc;
}

Result* GeosOp::executeOpRepeat(GeomFunction * fun,
    int indexA,
    const std::unique_ptr<Geometry>& geomA,
    int indexB,
    const std::unique_ptr<Geometry>& geomB)
{
    Result * res;
    for (int i = 0; i < args.repeatNum; i++) {
        res = executeOp(fun, indexA, geomA, indexB, geomB);
    }
    return res;
}

Result* GeosOp::executeOp(GeomFunction * fun,
    int indexA,
    const std::unique_ptr<Geometry>& geomA,
    int indexB,
    const std::unique_ptr<Geometry>& geomB) {

    opCount++;
    geos::util::Profile sw( "op" );
    sw.start();

    Result* result = fun->execute( geomA, geomB, args.opArg1  );
    sw.stop();
    double time = sw.getTot();
    totalTime += time;

    // avoid cost of logging if not verbose
    if (args.isVerbose) {
        log(
            "[ " + std::to_string(opCount) + "] " + fun->name() + ": "
            + inputDesc("A", indexA, geomA) + " "
            + inputDesc("B", indexB, geomB)
            + " -> " + result->metadata()
            + "  --  " + formatNum( time ) + " usec"
        );
    }

    return result;
}

void GeosOp::output(Result* result) {
    //---- print result if format specified
    if (args.format == GeosOpArgs::fmtNone )
        return;

    if (result->isGeometry() ) {
        if (args.isExplode) {
            outputExplode( result->valGeom );
        }
        else {
            outputGeometry( result->valGeom.get() );
        }
    }
    else if (result->isGeometryList() ) {
        outputGeometryList( result->valGeomList );
    }
    else {
        // output as text/WKT
        std::cout << result->toString() << std::endl;
    }
}

void GeosOp::outputExplode(std::unique_ptr<Geometry>& geom) {
    for (size_t i = 0; i < geom->getNumGeometries(); i++) {
        auto g = geom->getGeometryN(i);
        outputGeometry( g );
    }
}

void GeosOp::outputGeometry(const Geometry * geom) {
    if (geom == nullptr) {
        std::cout << "null" << std::endl;
        return;
    }

    if (args.format == GeosOpArgs::fmtWKB ) {
        // output as hex-encoded WKB
        WKBWriter writer;
        writer.setOutputDimension(3);
        writer.writeHEX(*geom, std::cout);
        std::cout << std::endl;
    }
    else {
        // output as text/WKT
        WKTWriter writer;
        writer.setOutputDimension(3);
        if (args.precision >= 0) {
             writer.setRoundingPrecision(args.precision);
        }
        else {
            // turn off stoopid fixed precision
            writer.setTrim(true);
        }
        std::cout << writer.write(geom) << std::endl;
    }
}

void GeosOp::outputGeometryList(std::vector<std::unique_ptr<const Geometry>> & list) {
    for (size_t i = 0; i < list.size(); i++) {
        outputGeometry( list[i].get() );
    }
}
