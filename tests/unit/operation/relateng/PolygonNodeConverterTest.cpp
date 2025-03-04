//
// Test Suite for geos::operation::relateng::PolygonNodeConverter class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/geom/Dimension.h>
#include <geos/operation/relateng/PolygonNodeConverter.h>
#include <geos/algorithm/PolygonNodeTopology.h>


// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::algorithm::PolygonNodeTopology;
using geos::io::WKTReader;
// using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_polygonnodeconverter_data {

    std::deque<std::unique_ptr<Coordinate>> coordQue;
    std::deque<std::unique_ptr<NodeSection>> nsQue;

    Coordinate* newCoordinate(double x, double y)
    {
        coordQue.emplace_back(new Coordinate(x, y));
        return coordQue.back().get();
    }

    NodeSection* section(int ringId, double v0x, double v0y, double nx, double ny, double v1x, double v1y)
    {
        NodeSection *ns = new NodeSection(
            true, Dimension::A, 1, ringId,
            nullptr, false,
            newCoordinate(v0x, v0y),
            Coordinate(nx, ny),
            newCoordinate(v1x, v1y)
            );
        nsQue.emplace_back(ns);
        return nsQue.back().get();
    }

    NodeSection* sectionShell(double v0x, double v0y, double nx, double ny, double v1x, double v1y) {
        return section(0, v0x, v0y, nx, ny, v1x, v1y);
    }

    NodeSection* sectionHole(double v0x, double v0y, double nx, double ny, double v1x, double v1y) {
        return section(1, v0x, v0y, nx, ny, v1x, v1y);
    }

    std::vector<const NodeSection*> 
    toPtrVector(const std::vector<std::unique_ptr<NodeSection>>& input)
    {
        std::vector<const NodeSection*> vec;
        for(std::size_t i = 0, n = input.size(); i < n; ++i) {
            vec.push_back(input[i].get());
        }
        return vec;
    }

    bool checkSectionsEqual(std::vector<const NodeSection*>& ns1, 
                            std::vector<const NodeSection*>& ns2) {
        if (ns1.size() != ns2.size())
            return false;
        sort(ns1);
        sort(ns2);
        for (std::size_t i = 0; i < ns1.size(); i++) {
            int comp = ns1[i]->compareTo(ns2[i]);
            if (comp != 0)
                return false;
        }
        return true;
    }

    void sort(std::vector<const NodeSection*>& ns) {

        // Comparator lambda for sort support
        auto comparator = [](
            const NodeSection* a,
            const NodeSection* b)
        {
            return a->compareTo(b) < 0;
        };
            
        std::sort(ns.begin(), ns.end(), comparator);
    }

    void
    checkConversion(std::vector<const NodeSection *>& input, 
                    std::vector<const NodeSection *>& expected)
    {
        auto actual = PolygonNodeConverter::convert( input );
        auto actualPtr = toPtrVector(actual);
        bool isEqual = checkSectionsEqual(actualPtr, expected);
        ensure("checkConversion", isEqual);
    }

};

typedef test_group<test_polygonnodeconverter_data> group;
typedef group::object object;

group test_polygonnodeconverter_group("geos::operation::relateng::PolygonNodeConverter");

//
// Test Cases
//


// testShells
template<>
template<>
void object::test<1> ()
{
    std::vector<const NodeSection *> input{
        sectionShell( 1,1, 5,5, 9,9 ),
        sectionShell( 8,9, 5,5, 6,9 ),
        sectionShell( 4,9, 5,5, 2,9 )
    };
    std::vector<const NodeSection *> expected{
        sectionShell( 1,1, 5,5, 9,9 ),
        sectionShell( 8,9, 5,5, 6,9 ),
        sectionShell( 4,9, 5,5, 2,9 )
    };
    checkConversion(input, expected);
}

// testShellAndHole
template<>
template<>
void object::test<2> ()
{
    std::vector<const NodeSection *> input{
            sectionShell( 1,1, 5,5, 9,9 ),
            sectionHole(  6,0, 5,5, 4,0 )
    };
    std::vector<const NodeSection *> expected{
            sectionShell( 1,1, 5,5, 4,0 ),
            sectionShell( 6,0, 5,5, 9,9 )
   };
    checkConversion(input, expected);
}

// testShellsAndHoles
template<>
template<>
void object::test<3> ()
{
    std::vector<const NodeSection *> input{
        sectionShell( 1,1, 5,5, 9,9 ),
        sectionHole(  6,0, 5,5, 4,0 ),
        
        sectionShell( 8,8, 5,5, 1,8 ),
        sectionHole(  4,8, 5,5, 6,8 )
    };
    std::vector<const NodeSection *> expected{
        sectionShell( 1,1, 5,5, 4,0 ),
        sectionShell( 6,0, 5,5, 9,9 ),
        
        sectionShell( 4,8, 5,5, 1,8 ),
        sectionShell( 8,8, 5,5, 6,8 )
   };
    checkConversion(input, expected);
}

// testShellAnd2Holes
template<>
template<>
void object::test<5> ()
{
    std::vector<const NodeSection *> input{
        sectionShell( 1,1, 5,5, 9,9 ),
        sectionHole(  7,0, 5,5, 6,0 ),
        sectionHole(  4,0, 5,5, 3,0 )
    };
    std::vector<const NodeSection *> expected{
        sectionShell( 1,1, 5,5, 3,0 ),
        sectionShell( 4,0, 5,5, 6,0 ),
        sectionShell( 7,0, 5,5, 9,9 )
    };
    checkConversion(input, expected);
}

// testHoles
template<>
template<>
void object::test<6> ()
{
    std::vector<const NodeSection *> input{
        sectionHole(  7,0, 5,5, 6,0 ),
        sectionHole(  4,0, 5,5, 3,0 )
    };
    std::vector<const NodeSection *> expected{
        sectionShell( 4,0, 5,5, 6,0 ),
        sectionShell( 7,0, 5,5, 3,0 )
    };
    checkConversion(input, expected);
}

} // namespace tut
