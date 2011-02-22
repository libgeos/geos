// 
// Test Suite for geos::operation::linemerge::LineMerger class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/platform.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/util/IllegalArgumentException.h>
// std
#include <memory>
#include <string>
#include <vector>

namespace tut
{
  //
  // Test Group
  //

  // Common data used by tests
  struct test_linemerger_data
  {
    typedef geos::operation::linemerge::LineMerger LineMerger;
    typedef std::vector<geos::geom::Geometry*> GeomVect;
    typedef std::vector<geos::geom::LineString*> LineVect;

    geos::geom::GeometryFactory gf;
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    typedef geos::geom::Geometry Geom;
    typedef geos::geom::Geometry::AutoPtr GeomPtr;

    GeomVect inpGeoms;
    GeomVect expGeoms;
    LineVect* mrgGeoms;

    test_linemerger_data()
      : gf(), wktreader(&gf), wktwriter(), mrgGeoms(0)
    {
      wktwriter.setTrim(true);
    }

    ~test_linemerger_data()
    {
      delAll(inpGeoms);
      delAll(expGeoms);
      if ( mrgGeoms ) {
        delAll(*mrgGeoms);
        delete mrgGeoms;
      }
    }

    GeomPtr readWKT(const std::string& inputWKT)
    {
        return GeomPtr(wktreader.read(inputWKT));
    }

    void readWKT(const char* const* inputWKT, std::vector<Geom*>& geoms)
    {
      for (const char* const* ptr=inputWKT; *ptr; ++ptr) {
        geoms.push_back(readWKT(*ptr).release());
      }
    }

    void doTest(const char * const * inputWKT,
                const char * const * expectedWKT,
                bool compareDirections=true)
    {
      LineMerger lineMerger;

      readWKT(inputWKT, inpGeoms);
      readWKT(expectedWKT, expGeoms);

      lineMerger.add(&inpGeoms);
      mrgGeoms = lineMerger.getMergedLineStrings();
      compare(expGeoms, *mrgGeoms, compareDirections);

    }

    template <class TargetContainer>
    void delAll(TargetContainer& geoms)
    {
      for (typename TargetContainer::const_iterator i = geoms.begin(),
           e = geoms.end(); i != e; ++i)
      {
        Geom* g = dynamic_cast<Geom*>(*i);
        delete g;
      }
    }

    template <class TargetContainer1, class TargetContainer2>
    static void compare(TargetContainer1& expectedGeometries,
                        TargetContainer2& actualGeometries,
                        bool compareDirections)
    {
      ensure_equals(actualGeometries.size(), expectedGeometries.size());
      for (typename TargetContainer1::const_iterator
          i = expectedGeometries.begin(),
          e = expectedGeometries.end(); i != e; ++i)
      {
        Geom* g = dynamic_cast<Geom*>(*i);
        ensure( contains(actualGeometries, g, compareDirections) );
      }
    }

    template <class TargetContainer>
    static bool contains(TargetContainer& actualGeometries,
                         const Geom* g, bool exact)
    {
      for (typename TargetContainer::const_iterator
           i = actualGeometries.begin(),
           e = actualGeometries.end(); i != e; ++i)
      {
        Geom* element = dynamic_cast<Geom*>(*i);
        if (exact && element->equalsExact(g)) {
          return true;
        }
        if (!exact && element->equals(g)) {
          return true;
        }
      }

      return false;
    }
 


  private:
    // noncopyable
    test_linemerger_data(test_linemerger_data const& other);
    test_linemerger_data& operator=(test_linemerger_data const& rhs);
  };

  typedef test_group<test_linemerger_data> group;
  typedef group::object object;

  group test_linemerger_group("geos::operation::linemerge::LineMerger");

  //
  // Test Cases
  //

  template<> template<>
  void object::test<1>()
  {
    const char* inpWKT[] = {
      "LINESTRING (120 120, 180 140)", "LINESTRING (200 180, 180 140)",
      "LINESTRING (200 180, 240 180)",
      NULL };
    const char* expWKT[] = {
      "LINESTRING (120 120, 180 140, 200 180, 240 180)",
      NULL };

    doTest(inpWKT, expWKT);
  }

  template<> template<>
  void object::test<2>()
  {
    const char* inpWKT[] = {
      "LINESTRING (120 300, 80 340)",
      "LINESTRING (120 300, 140 320, 160 320)",
      "LINESTRING (40 320, 20 340, 0 320)",
      "LINESTRING (0 320, 20 300, 40 320)",
      "LINESTRING (40 320, 60 320, 80 340)",
      "LINESTRING (160 320, 180 340, 200 320)",
      "LINESTRING (200 320, 180 300, 160 320)",
      NULL };
    const char* expWKT[] = {
      "LINESTRING (160 320, 180 340, 200 320, 180 300, 160 320)",
      "LINESTRING (40 320, 20 340, 0 320, 20 300, 40 320)",
      "LINESTRING (40 320, 60 320, 80 340, 120 300, 140 320, 160 320)",
      NULL };

    doTest(inpWKT, expWKT);
  }

  template<> template<>
  void object::test<3>()
  {
    const char* inpWKT[] = {
      "LINESTRING (0 0, 100 100)", "LINESTRING (0 100, 100 0)",
      NULL };
    const char* expWKT[] = {
      "LINESTRING (0 0, 100 100)", "LINESTRING (0 100, 100 0)",
      NULL };

    doTest(inpWKT, expWKT);
  }

  template<> template<>
  void object::test<4>()
  {
    const char* inpWKT[] = {
      "LINESTRING EMPTY",
      "LINESTRING EMPTY",
      NULL };
    const char* expWKT[] = {
      NULL };

    doTest(inpWKT, expWKT);
  }

  // Don't blame me for this, I'm just copying JTS tests :P
  template<> template<>
  void object::test<5>()
  {
    const char* inpWKT[] = {
      NULL };
    const char* expWKT[] = {
      NULL };

    doTest(inpWKT, expWKT);
  }

  // testSingleUniquePoint()
  template<> template<>
  void object::test<6>()
  {
    const char* inpWKT[] = {
      "LINESTRING (10642 31441, 10642 31441)",
      NULL };
    const char* expWKT[] = {
      NULL };

    doTest(inpWKT, expWKT);
  }


} // namespace tut

