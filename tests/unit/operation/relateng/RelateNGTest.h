
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/operation/relateng/RelateNG.h>
#include <geos/operation/relateng/RelatePredicate.h>
#include <geos/operation/relateng/RelateMatrixPredicate.h>
#include <geos/operation/relateng/IntersectionMatrixPattern.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_relateng_support {

    WKTReader r;
    WKTWriter w;

    void checkPrepared(const std::string& wkta, const std::string& wktb)
    {
        std::unique_ptr<Geometry> a = r.read(wkta);
        std::unique_ptr<Geometry> b = r.read(wktb);
        return checkPrepared(a.get(), b.get());
    }

    void checkPrepared(const Geometry* a, const Geometry* b)
    {
        auto prep_a = RelateNG::prepare(a);
        auto prep_b = RelateNG::prepare(b);
        ensure_equals("preparedEqualsTopo", prep_a->equalsTopo(b), a->equals(b));
        ensure_equals("preparedIntersects", prep_a->intersects(b), a->intersects(b));
        ensure_equals("preparedDisjoint",   prep_a->disjoint(b),   a->disjoint(b));
        ensure_equals("preparedCovers",     prep_a->covers(b),     a->covers(b));
        ensure_equals("preparedCoveredby",  prep_a->coveredBy(b),  a->coveredBy(b));
        ensure_equals("preparedWithin",     prep_a->within(b),     a->within(b));
        ensure_equals("preparedContains",   prep_a->contains(b),   a->contains(b));
        ensure_equals("preparedCrosses",    prep_a->crosses(b),    a->crosses(b));
        ensure_equals("preparedTouches",    prep_a->touches(b),    a->touches(b));
        ensure_equals("preparedRelate", prep_a->relate(b)->toString(), a->relate(b)->toString());
    }

    void checkIntersectsDisjoint(const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        checkPredicate(*RelatePredicate::intersects(), wkta, wktb, expectedValue);
        checkPredicate(*RelatePredicate::intersects(), wktb, wkta, expectedValue);
        checkPredicate(*RelatePredicate::disjoint(), wkta, wktb, ! expectedValue);
        checkPredicate(*RelatePredicate::disjoint(), wktb, wkta, ! expectedValue);
    }

    void checkContainsWithin(const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        checkPredicate(*RelatePredicate::contains(), wkta, wktb, expectedValue);
        checkPredicate(*RelatePredicate::within(),   wktb, wkta, expectedValue);
    }

    void checkCoversCoveredBy(const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        checkPredicate(*RelatePredicate::covers(),    wkta, wktb, expectedValue);
        checkPredicate(*RelatePredicate::coveredBy(), wktb, wkta, expectedValue);
    }

    void checkCrosses(const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        checkPredicate(*RelatePredicate::crosses(), wkta, wktb, expectedValue);
        checkPredicate(*RelatePredicate::crosses(), wktb, wkta, expectedValue);
    }

    void checkOverlaps(const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        checkPredicate(*RelatePredicate::overlaps(), wkta, wktb, expectedValue);
        checkPredicate(*RelatePredicate::overlaps(), wktb, wkta, expectedValue);
    }

    void checkTouches(const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        checkPredicate(*RelatePredicate::touches(), wkta, wktb, expectedValue);
        checkPredicate(*RelatePredicate::touches(), wktb, wkta, expectedValue);
    }

    void checkEquals(const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        checkPredicate(*RelatePredicate::equalsTopo(), wkta, wktb, expectedValue);
        checkPredicate(*RelatePredicate::equalsTopo(), wktb, wkta, expectedValue);
    }

    void checkRelate(const std::string& wkta, const std::string& wktb, const std::string expectedValue)
    {
        std::unique_ptr<Geometry> a = r.read(wkta);
        std::unique_ptr<Geometry> b = r.read(wktb);
        RelateMatrixPredicate pred;
        // TopologyPredicate predTrace = trace(pred);
        RelateNG::relate(a.get(), b.get(), pred);
        std::string actualVal = pred.getIM()->toString();
        if (actualVal != expectedValue) {
            std::cerr << std::endl << w.write(*a) << " relate " << w.write(*b) << " = " << actualVal << std::endl;
        }
        ensure_equals("checkRelate", actualVal, expectedValue);
        checkPrepared(a.get(), b.get());
    }

    void checkRelateMatches(const std::string& wkta, const std::string& wktb, const std::string pattern, bool expectedValue)
    {
        auto pred = RelatePredicate::matches(pattern);
        checkPredicate(*pred, wkta, wktb, expectedValue);
    }

    void checkPredicate(TopologyPredicate& pred, const std::string& wkta, const std::string& wktb, bool expectedValue)
    {
        std::unique_ptr<Geometry> a = r.read(wkta);
        std::unique_ptr<Geometry> b = r.read(wktb);
        // TopologyPredicate predTrace = trace(pred);
        bool actualVal = RelateNG::relate(a.get(), b.get(), pred);
        if (actualVal != expectedValue) {
            std::cerr << std::endl << w.write(*a) << " " << pred << " " << w.write(*b) << " = " << actualVal << std::endl;
        }
        ensure_equals("checkPredicate", actualVal, expectedValue);
        checkPrepared(a.get(), b.get());
    }

};


} // namespace tut
