// 
// Test Suite for geos::operation::linemerge::LineSequencer class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/linemerge/LineSequencer.h>
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
  struct test_linesequencer_data
  {
    typedef geos::operation::linemerge::LineSequencer LineSequencer;
    typedef std::vector<geos::geom::Geometry*> GeomVect;
    typedef std::vector<geos::geom::LineString*> LineVect;

    geos::geom::GeometryFactory gf;
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    typedef geos::geom::Geometry Geom;
    typedef geos::geom::Geometry::AutoPtr GeomPtr;

    GeomVect inpGeoms;

    test_linesequencer_data()
      : gf(), wktreader(&gf), wktwriter()
    {
      wktwriter.setTrim(true);
    }

    ~test_linesequencer_data()
    {
      delAll(inpGeoms);
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

    void runLineSequencer(const char * const * inputWKT,
                const std::string& expectedWKT)
    {
      readWKT(inputWKT, inpGeoms);

      LineSequencer sequencer;
      sequencer.add(inpGeoms);

      if ( ! sequencer.isSequenceable() ) {
      	ensure( expectedWKT.empty() );
      } else {  
        GeomPtr expected = readWKT(expectedWKT);
        GeomPtr result ( sequencer.getSequencedLineStrings() );
        ensure( expected->equalsExact( result.get() ) );

        bool isSequenced = LineSequencer::isSequenced(result.get());
        ensure( isSequenced );
      }
    }

    void runIsSequenced(const std::string& inputWKT, bool exp)
    {
      GeomPtr g = readWKT(inputWKT);
      bool isSequenced = LineSequencer::isSequenced(g.get());
      ensure_equals( isSequenced, exp );
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


  private:
    // noncopyable
    test_linesequencer_data(test_linesequencer_data const& other);
    test_linesequencer_data& operator=(test_linesequencer_data const& rhs);
  };

  typedef test_group<test_linesequencer_data> group;
  typedef group::object object;

  group test_linesequencer_group("geos::operation::linemerge::LineSequencer");

  //
  // Test Cases
  //

  // testSimple
  template<> template<>
  void object::test<1>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 20, 0 30 )",
      "LINESTRING ( 0 10, 0 20 )",
      NULL };
    const char* expWKT = 
      "MULTILINESTRING ((0 0, 0 10), (0 10, 0 20), (0 20, 0 30))";

    runLineSequencer(inpWKT, expWKT);
  }

  // testSimpleLoop
  template<> template<>
  void object::test<2>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 10, 0 0 )",
      NULL };
    const char* expWKT = 
      "MULTILINESTRING ((0 0, 0 10), (0 10, 0 0))";
    runLineSequencer(inpWKT, expWKT);
  }

  // testSimpleBigLoop
  template<> template<>
  void object::test<3>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 20, 0 30 )",
      "LINESTRING ( 0 30, 0 00 )",
      "LINESTRING ( 0 10, 0 20 )",
      NULL };
    const char* expWKT = 
      "MULTILINESTRING ((0 0, 0 10), (0 10, 0 20), (0 20, 0 30), (0 30, 0 0))";
    runLineSequencer(inpWKT, expWKT);
  }

  // test2SimpleLoops
  template<> template<>
  void object::test<4>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 10, 0 0 )",
      "LINESTRING ( 0 0, 0 20 )",
      "LINESTRING ( 0 20, 0 0 )",
      NULL };
    const char* expWKT = 
      "MULTILINESTRING ((0 10, 0 0), (0 0, 0 20), (0 20, 0 0), (0 0, 0 10))";
    runLineSequencer(inpWKT, expWKT);
  }

  // testWide8WithTail
  template<> template<>
  void object::test<5>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 10 0, 10 10 )",
      "LINESTRING ( 0 0, 10 0 )",
      "LINESTRING ( 0 10, 10 10 )",
      "LINESTRING ( 0 10, 0 20 )",
      "LINESTRING ( 10 10, 10 20 )",
      "LINESTRING ( 0 20, 10 20 )",
 
      "LINESTRING ( 10 20, 30 30 )",
      NULL };
    const char* expWKT = "";
    runLineSequencer(inpWKT, expWKT);
  }

  // testSimpleLoopWithTail
  template<> template<>
  void object::test<6>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 10, 10 10 )",
      "LINESTRING ( 10 10, 10 20, 0 10 )",
      NULL };
    const char* expWKT = 
"MULTILINESTRING ((0 0, 0 10), (0 10, 10 10), (10 10, 10 20, 0 10))";
    runLineSequencer(inpWKT, expWKT);
  }

  // testLineWithRing
  template<> template<>
  void object::test<7>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 10, 10 10, 10 20, 0 10 )",
      "LINESTRING ( 0 30, 0 20 )",
      "LINESTRING ( 0 20, 0 10 )",
      NULL };
    const char* expWKT = 
        "MULTILINESTRING ((0 0, 0 10), (0 10, 10 10, 10 20, 0 10), (0 10, 0 20), (0 20, 0 30))";
    runLineSequencer(inpWKT, expWKT);
  }

  // testMultipleGraphsWithRing
  template<> template<>
  void object::test<8>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 10, 10 10, 10 20, 0 10 )",
      "LINESTRING ( 0 30, 0 20 )",
      "LINESTRING ( 0 20, 0 10 )",
      "LINESTRING ( 0 60, 0 50 )",
      "LINESTRING ( 0 40, 0 50 )",
      NULL };
    const char* expWKT = 
"MULTILINESTRING ((0 0, 0 10), (0 10, 10 10, 10 20, 0 10), (0 10, 0 20), (0 20, 0 30), (0 40, 0 50), (0 50, 0 60))";
    runLineSequencer(inpWKT, expWKT);
  }

  // testMultipleGraphsWithMultipeRings
  template<> template<>
  void object::test<9>()
  {
    const char* inpWKT[] = {
      "LINESTRING ( 0 0, 0 10 )",
      "LINESTRING ( 0 10, 10 10, 10 20, 0 10 )",
      "LINESTRING ( 0 10, 40 40, 40 20, 0 10 )",
      "LINESTRING ( 0 30, 0 20 )",
      "LINESTRING ( 0 20, 0 10 )",
      "LINESTRING ( 0 60, 0 50 )",
      "LINESTRING ( 0 40, 0 50 )",
      NULL };
    const char* expWKT = 
        "MULTILINESTRING ((0 0, 0 10), (0 10, 40 40, 40 20, 0 10), (0 10, 10 10, 10 20, 0 10), (0 10, 0 20), (0 20, 0 30), (0 40, 0 50), (0 50, 0 60))";

    runLineSequencer(inpWKT, expWKT);
  }

  // testLineSequence
  template<> template<>
  void object::test<10>()
  {
    const char* expWKT = "LINESTRING ( 0 0, 0 10 )";
    runIsSequenced(expWKT, true);
  }

  // testSplitLineSequence
  template<> template<>
  void object::test<11>()
  {
    const char* expWKT =
      "MULTILINESTRING ((0 0, 0 1), (0 2, 0 3), (0 3, 0 4) )";
    runIsSequenced(expWKT, true);
  }

  // testBadLineSequence
  template<> template<>
  void object::test<12>()
  {
    const char* expWKT =
      "MULTILINESTRING ((0 0, 0 1), (0 2, 0 3), (0 1, 0 4) )";
    runIsSequenced(expWKT, false);
  }


} // namespace tut

