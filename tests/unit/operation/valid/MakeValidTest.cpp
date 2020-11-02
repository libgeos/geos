#include <tut/tut.hpp>
// geos
#include <geos/constants.h> // for std::isnan
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
// std
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

using namespace geos::geom;
using namespace geos::operation::valid;

namespace tut {
//
// Test Group
//

struct test_makevalid_data {
    test_makevalid_data() {}
};

typedef test_group<test_makevalid_data> group;
typedef group::object object;

group test_makevalid_group("geos::operation::valid::MakeValid");

//
// Test Cases
//

// https://github.com/libgeos/geos/issues/265
template<>
template<>
void object::test<1>
()
{
    std::vector<Coordinate> v;
    v.emplace_back(2.22, 2.28);
    v.emplace_back(7.67, 2.06);
    v.emplace_back(10.98, 7.70);
    v.emplace_back(9.39, 5.00);
    v.emplace_back(7.96, 7.12);
    v.emplace_back(6.77, 5.16);
    v.emplace_back(7.43, 6.24);
    v.emplace_back(3.70, 7.22);
    v.emplace_back(5.72, 5.77);
    v.emplace_back(4.18, 10.74);
    v.emplace_back(2.20, 6.83);
    v.emplace_back(2.22, 2.28);

    auto gf = GeometryFactory::getDefaultInstance();

    auto cs = gf->getCoordinateSequenceFactory()->create(std::move(v));
    auto lr = gf->createLinearRing(std::move(cs));
    auto errplyg = gf->createPolygon(std::move(lr));

    ensure(!errplyg->isValid());

    MakeValid mkvalid;
    auto validGeom = mkvalid.build(errplyg.get());

    ensure(validGeom->isValid());
}

// template<>
// template<>
// void object::test<2>
// ()
// {


//     std::ifstream ifs("GoesBathymetryBug.txt");
//     std::string content((std::istreambuf_iterator<char>(ifs)),
//                        (std::istreambuf_iterator<char>()));

//     geos::io::WKTReader reader;
//     auto geom(reader.read(content));

//     // auto gf = GeometryFactory::getDefaultInstance();

//     // auto cs = gf->getCoordinateSequenceFactory()->create(std::move(v));
//     // auto lr = gf->createLinearRing(std::move(cs));
//     // auto errplyg = gf->createPolygon(std::move(lr));

//     // ensure(!errplyg->isValid());

//     MakeValid mkvalid;
//     auto validGeom = mkvalid.build(geom.get());
//     ensure("MakeValid output is not valid", validGeom->isValid());

//     geos::io::WKTWriter writer;
//     writer.setOutputDimension(2);
//     writer.setTrim(true);
//     std::string result = writer.write(validGeom.get());
//     std::cout << result << std::endl;

// }



} // namespace tut
