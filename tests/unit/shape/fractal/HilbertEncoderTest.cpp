// Test Suite for HilbertEncoder class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/shape/fractal/HilbertEncoder.h>
// std
#include <vector>

using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::shape::fractal::HilbertEncoder;

namespace tut {

// Common data used by tests
struct test_hilbertencoder_data {
    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;

    test_hilbertencoder_data()
        : factory_(geos::geom::GeometryFactory::create(&pm_, 0)),
          reader_(factory_.get()) {}

    ~test_hilbertencoder_data() {}
};

typedef test_group<test_hilbertencoder_data> group;
typedef group::object object;

group test_hilbertencoder_group("geos::shape::fractal::HilbertEncoder");

// test various points along the curve
template <>
template <>
void object::test<1>() {
    auto point1 = reader_.read("POINT (0 0)");
    auto point2 = reader_.read("POINT (1 1)");
    Envelope extent(0, 1, 0, 1);

    HilbertEncoder encoder(2, extent);

    ensure_equals(encoder.encode(point1->getEnvelopeInternal()), 0u);
    ensure_equals(encoder.encode(point2->getEnvelopeInternal()), 10u);

    // midpoint is used for Hilbert code
    ensure_equals(encoder.encode(&extent), 2u);
}

// if envelope has no width or height, all codes will be 0
template <>
template <>
void object::test<2>() {
    auto point1 = reader_.read("POINT (0 0)");
    auto point2 = reader_.read("POINT (1 1)");
    Envelope extent = *point1->getEnvelopeInternal();

    HilbertEncoder encoder(2, extent);

    ensure_equals(encoder.encode(point1->getEnvelopeInternal()), 0u);
    ensure_equals(encoder.encode(point2->getEnvelopeInternal()), 0u);
}

// test that geometries are sorted in descending Hilbert order
template <>
template <>
void object::test<3>() {
    auto point1 = reader_.read("POINT (0 0)");
    auto point2 = reader_.read("POINT (1 1)");
    auto point3 = reader_.read("POINT (0.5 0.5)");
    Envelope extent(0, 1, 0, 1);

    HilbertEncoder encoder(2, extent);

    std::vector<Geometry*> geoms;
    geoms.push_back(point2.release());
    geoms.push_back(point1.release());
    geoms.push_back(point3.release());

    encoder.sort(geoms);

    uint32_t expected[3] = {10, 2, 0};
    for (size_t i = 0; i < 3; i++) {
        ensure_equals(encoder.encode(geoms[i]->getEnvelopeInternal()), expected[i]);

        // cleanup geom
        factory_->destroyGeometry(geoms[i]);
    }
}

}  // namespace tut
