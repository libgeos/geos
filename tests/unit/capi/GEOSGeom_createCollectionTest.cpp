//
// Test Suite for C-API GEOSGeom_createCollection

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosgeom_createcollection_data {
    GEOSContextHandle_t handle_;
    GEOSWKTReader * reader_;
    GEOSGeometry * geom_;
    GEOSGeometry ** geoms_;
    unsigned int ngeoms_;

    enum { geom_size = 3 };

    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    GEOSGeometry*
    read(const char* wkt)
    {
        return GEOSWKTReader_read_r(handle_, reader_, wkt);
    }

    test_capigeosgeom_createcollection_data()
        : handle_(initGEOS_r(notice, notice))
        , reader_(GEOSWKTReader_create_r(handle_))
        , geom_(nullptr)
        , geoms_(nullptr)
        , ngeoms_(0)
    {
    }

    ~test_capigeosgeom_createcollection_data()
    {
        if (reader_) GEOSWKTReader_destroy_r(handle_, reader_);
        if (geom_)   GEOSGeom_destroy_r(handle_, geom_);
        if (geoms_)  GEOSFree_r(handle_, geoms_);
        finishGEOS_r(handle_);
        handle_ = nullptr;
        reader_ = nullptr;
        geom_ = nullptr;
        geoms_ = nullptr;
    }
};

typedef test_group<test_capigeosgeom_createcollection_data> group;
typedef group::object object;

group test_capigeosgeom_createcollection_group("capi::GEOSGeom_createCollection");

//
// Test Cases
//

// Create collection from constant length C-array
template<>
template<>
void object::test<1>
()
{
    GEOSGeom geoms[geom_size];
    geoms[0] = GEOSGeom_createEmptyPoint_r(handle_);
    geoms[1] = GEOSGeom_createEmptyPoint_r(handle_);
    geoms[2] = GEOSGeom_createEmptyPoint_r(handle_);
    // takes ownership of individual geometries
    geom_ = GEOSGeom_createCollection_r(handle_, GEOS_MULTIPOINT, geoms, geom_size);
    ensure_equals(GEOSGetNumGeometries_r(handle_, geom_), (int)geom_size);
}

// Create collection from constant length std::array
template<>
template<>
void object::test<2>
()
{
    std::array<GEOSGeom, geom_size> geoms = {{
        GEOSGeom_createEmptyLineString_r(handle_),
        GEOSGeom_createEmptyLineString_r(handle_),
        GEOSGeom_createEmptyLineString_r(handle_)
    }};
    // takes ownership of individual geometries
    geom_ = GEOSGeom_createCollection_r(handle_, GEOS_MULTILINESTRING,
                                        geoms.data(), static_cast<unsigned int>(geoms.size()));
    ensure_equals(GEOSGetNumGeometries_r(handle_, geom_), geom_size);
}

// Create collection from dynamic length std::vector of geometries
template<>
template<>
void object::test<3>
()
{
    std::vector<GEOSGeom> geoms;
    geoms.push_back(GEOSGeom_createEmptyPolygon_r(handle_));
    geoms.push_back(GEOSGeom_createEmptyPolygon_r(handle_));
    geoms.push_back(GEOSGeom_createEmptyPolygon_r(handle_));
    geoms.push_back(GEOSGeom_createEmptyPolygon_r(handle_));
    geoms.push_back(GEOSGeom_createEmptyPolygon_r(handle_));
    // takes ownership of individual geometries
    geom_ = GEOSGeom_createCollection_r(handle_, GEOS_MULTIPOLYGON,
                                        geoms.data(), static_cast<unsigned int>(geoms.size()));
    ensure_equals(static_cast<size_t>(GEOSGetNumGeometries_r(handle_, geom_)), geoms.size());
}

// Error on invalid collection type, ownership is still transferred
template<>
template<>
void object::test<4>
()
{
    std::vector<GEOSGeom> geoms;
    geoms.push_back(GEOSGeom_createEmptyPolygon_r(handle_));
    // takes ownership of individual geometries
    geom_ = GEOSGeom_createCollection_r(handle_, 12345,
                                        geoms.data(), static_cast<unsigned int>(geoms.size()));
    ensure(geom_ == nullptr);

    geom_ = GEOSGeom_createEmptyCollection_r(handle_, 12345);
    ensure(geom_ == nullptr);
}

// Release empty collection
template<>
template<>
void object::test<5>
()
{
    const char *wkt = "MULTIPOLYGON EMPTY";
    geom_ = read(wkt);
    ensure(geom_ != nullptr);

    geoms_ = GEOSGeom_releaseCollection_r(handle_, geom_, &ngeoms_);
    ensure(geoms_ == nullptr);
    ensure(ngeoms_ == 0);
}


// Release generic collection
template<>
template<>
void object::test<6>
()
{
    const char *wkt = "GEOMETRYCOLLECTION(POINT(0 0), POINT(1 1))";
    geom_ = read(wkt);
    ensure(geom_ != nullptr);

    geoms_ = GEOSGeom_releaseCollection_r(handle_, geom_, &ngeoms_);
    ensure(geoms_ != nullptr);
    ensure(ngeoms_ == 2);

    for (size_t i = 0 ; i < ngeoms_; i++) {
        ensure(GEOSGeomTypeId_r(handle_, geoms_[i]) == GEOS_POINT);
        GEOSGeom_destroy_r(handle_, geoms_[i]);
    }

}

// Release typed collection
template<>
template<>
void object::test<7>
()
{
    const char *wkt = "MULTIPOINT((0 0), (1 1))";
    geom_ = read(wkt);
    ensure(geom_ != nullptr);

    geoms_ = GEOSGeom_releaseCollection_r(handle_, geom_, &ngeoms_);
    ensure(geoms_ != nullptr);
    ensure(ngeoms_ == 2);

    for (size_t i = 0 ; i < ngeoms_; i++) {
        ensure(GEOSGeomTypeId_r(handle_, geoms_[i]) == GEOS_POINT);
        GEOSGeom_destroy_r(handle_, geoms_[i]);
    }

}

} // namespace tut

