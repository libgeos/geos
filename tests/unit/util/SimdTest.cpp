//
// Test Suite for the runtime-dispatched SIMD (AVX-512) code paths.
//
// Each test compares the results of the SIMD path with the scalar path
// on randomized inputs designed to exercise degenerate cases (repeated
// points, collinear and horizontal segments, points on clip edges, signed
// zeros, NaN). If AVX-512 is not supported, both runs use the scalar path.

#include <tut/tut.hpp>

#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/RingClipper.h>
#include <geos/operation/overlayng/RobustClipEnvelopeComputer.h>
#include <geos/util/GEOSException.h>
#include <geos/util/Simd.h>

#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <vector>

using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYZM;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::util::simd::setAvx512Enabled;

namespace tut {

struct test_simd_data {
    std::mt19937_64 rng{20261024};
    geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create();

    const double NaN = std::numeric_limits<double>::quiet_NaN();

    bool initialState = geos::util::simd::isAvx512Enabled();

    ~test_simd_data()
    {
        setAvx512Enabled(initialState);
    }

    std::size_t randomIndex(std::size_t n)
    {
        return std::uniform_int_distribution<std::size_t>(0, n - 1)(rng);
    }

    bool chance(double p)
    {
        return std::uniform_real_distribution<double>(0, 1)(rng) < p;
    }

    // An ordinate on a coarse grid (to create degeneracies), or continuous,
    // occasionally a signed zero or NaN
    double ordinate(bool allowNaN)
    {
        if (allowNaN && chance(0.01)) {
            return NaN;
        }
        if (chance(0.03)) {
            return chance(0.5) ? 0.0 : -0.0;
        }
        if (chance(0.6)) {
            return static_cast<double>(std::uniform_int_distribution<int>(-5, 5)(rng));
        }
        return std::uniform_real_distribution<double>(-5, 5)(rng);
    }

    CoordinateXYZM coordinate(bool allowNaN)
    {
        return CoordinateXYZM(ordinate(allowNaN), ordinate(allowNaN),
                              std::uniform_real_distribution<double>(0, 1)(rng),
                              std::uniform_real_distribution<double>(0, 1)(rng));
    }

    // A sequence of n points with random dimension; with repeated points,
    // and optionally closed
    std::unique_ptr<CoordinateSequence> sequence(std::size_t n, bool closed, bool allowNaN)
    {
        bool hasZ = chance(0.5);
        bool hasM = chance(0.5);
        auto seq = std::make_unique<CoordinateSequence>(0, hasZ, hasM);
        for (std::size_t i = 0; i < n; i++) {
            if (i > 0 && chance(0.1)) {
                CoordinateXYZM prev;
                seq->getAt(i - 1, prev);
                seq->add(prev);
            }
            else {
                seq->add(coordinate(allowNaN));
            }
        }
        if (closed && n > 0) {
            CoordinateXYZM first;
            seq->getAt(0, first);
            seq->add(first);
        }
        return seq;
    }

    Envelope envelope()
    {
        double x1 = ordinate(false), x2 = ordinate(false);
        double y1 = ordinate(false), y2 = ordinate(false);
        return Envelope(x1, x2, y1, y2);
    }

    // Identical, including signed zeros and NaN
    static bool same(double a, double b)
    {
        return std::memcmp(&a, &b, sizeof(double)) == 0 || (std::isnan(a) && std::isnan(b));
    }

    static bool sameSequence(const CoordinateSequence& a, const CoordinateSequence& b)
    {
        if (a.size() != b.size() || a.hasZ() != b.hasZ() || a.hasM() != b.hasM()) {
            return false;
        }
        for (std::size_t i = 0; i < a.size(); i++) {
            CoordinateXYZM ca, cb;
            a.getAt(i, ca);
            b.getAt(i, cb);
            if (!same(ca.x, cb.x) || !same(ca.y, cb.y) || !same(ca.z, cb.z) || !same(ca.m, cb.m)) {
                return false;
            }
        }
        return true;
    }

    static bool sameEnvelope(const Envelope& a, const Envelope& b)
    {
        return same(a.getMinX(), b.getMinX()) && same(a.getMaxX(), b.getMaxX()) &&
               same(a.getMinY(), b.getMinY()) && same(a.getMaxY(), b.getMaxY());
    }

    // A random star-shaped polygon with grid-snapped vertices
    std::unique_ptr<geos::geom::Polygon> starPolygon(double cx, double cy, std::size_t n)
    {
        auto seq = std::make_unique<CoordinateSequence>(0, false, false);
        for (std::size_t i = 0; i < n; i++) {
            const double pi = 3.14159265358979323846;
            double angle = 2 * pi * static_cast<double>(i) / static_cast<double>(n);
            double r = std::uniform_real_distribution<double>(1, 5)(rng);
            double x = cx + r * std::cos(angle);
            double y = cy + r * std::sin(angle);
            if (chance(0.5)) {
                x = std::round(x * 4) / 4;
                y = std::round(y * 4) / 4;
            }
            seq->add(CoordinateXY(x, y), false);
        }
        seq->closeRing();
        if (seq->size() < 4) {
            return factory->createPolygon(2);
        }
        return factory->createPolygon(factory->createLinearRing(std::move(seq)));
    }

    /*
     * The original (GEOS 3.14) RingClipper::clipToBoxEdge, used as a reference
     * for the rewritten scalar and SIMD implementations.
     */
    static bool referenceIsInside(const Envelope& env, const CoordinateXY& p, int edgeIndex)
    {
        if (env.isNull()) {
            return false;
        }
        switch (edgeIndex) {
            case 0: return p.y > env.getMinY();
            case 1: return p.x < env.getMaxX();
            case 2: return p.y < env.getMaxY();
            default: return p.x > env.getMinX();
        }
    }

    static CoordinateXY referenceIntersection(const Envelope& env, const CoordinateXY& a, const CoordinateXY& b, int edgeIndex)
    {
        auto lineY = [](const CoordinateXY& p, const CoordinateXY& q, double y) {
            double m = (q.x - p.x) / (q.y - p.y);
            double intercept = (y - p.y) * m;
            return p.x + intercept;
        };
        auto lineX = [](const CoordinateXY& p, const CoordinateXY& q, double x) {
            double m = (q.y - p.y) / (q.x - p.x);
            double intercept = (x - p.x) * m;
            return p.y + intercept;
        };
        switch (edgeIndex) {
            case 0: return CoordinateXY(lineY(a, b, env.getMinY()), env.getMinY());
            case 1: return CoordinateXY(env.getMaxX(), lineX(a, b, env.getMaxX()));
            case 2: return CoordinateXY(lineY(a, b, env.getMaxY()), env.getMaxY());
            default: return CoordinateXY(env.getMinX(), lineX(a, b, env.getMinX()));
        }
    }

    static std::unique_ptr<CoordinateSequence> referenceClip(const Envelope& env, const CoordinateSequence* cs)
    {
        std::unique_ptr<CoordinateSequence> pts;
        for (int edgeIndex = 0; edgeIndex < 4; edgeIndex++) {
            auto ptsClip = std::make_unique<CoordinateSequence>(0, cs->hasZ(), cs->hasM());
            CoordinateXYZM p0;
            cs->getAt(cs->size() - 1, p0);
            for (std::size_t i = 0; i < cs->size(); i++) {
                CoordinateXYZM p1;
                cs->getAt(i, p1);
                if (referenceIsInside(env, p1, edgeIndex)) {
                    if (!referenceIsInside(env, p0, edgeIndex)) {
                        ptsClip->add(referenceIntersection(env, p0, p1, edgeIndex), false);
                    }
                    ptsClip->add(p1, false);
                }
                else if (referenceIsInside(env, p0, edgeIndex)) {
                    ptsClip->add(referenceIntersection(env, p0, p1, edgeIndex), false);
                }
                p0 = p1;
            }
            if (edgeIndex == 3) {
                ptsClip->closeRing();
            }
            pts = std::move(ptsClip);
            if (pts->size() == 0) {
                return pts;
            }
            cs = pts.get();
        }
        return pts;
    }

    struct SegmentResult {
        std::string error;
        std::size_t num = 0;
        bool proper = false;
        CoordinateXYZM pts[2];
    };

    static SegmentResult intersect(const CoordinateXY& p1, const CoordinateXY& p2,
                                   const CoordinateXY& q1, const CoordinateXY& q2)
    {
        SegmentResult r;
        try {
            geos::algorithm::LineIntersector li;
            li.computeIntersection(p1, p2, q1, q2);
            r.num = li.getIntersectionNum();
            r.proper = li.isProper();
            for (std::size_t i = 0; i < r.num; i++) {
                r.pts[i] = li.getIntersection(i);
            }
        }
        catch (const geos::util::GEOSException& e) {
            r.error = e.what();
        }
        return r;
    }

    static bool sameResult(const SegmentResult& a, const SegmentResult& b)
    {
        if (a.error != b.error || a.num != b.num || a.proper != b.proper) {
            return false;
        }
        for (std::size_t i = 0; i < a.num; i++) {
            if (!same(a.pts[i].x, b.pts[i].x) || !same(a.pts[i].y, b.pts[i].y) ||
                    !same(a.pts[i].z, b.pts[i].z) || !same(a.pts[i].m, b.pts[i].m)) {
                return false;
            }
        }
        return true;
    }
};

typedef test_group<test_simd_data> group;
typedef group::object object;

group test_simd_group("geos::util::Simd");

// Enabling is limited to supported CPUs; disabling always works
template<>
template<>
void object::test<1>
()
{
    setAvx512Enabled(true);
    ensure_equals(geos::util::simd::isAvx512Enabled(), geos::util::simd::isAvx512Supported());
    ensure_equals(setAvx512Enabled(false), geos::util::simd::isAvx512Supported());
    ensure(!geos::util::simd::isAvx512Enabled());
}

// RingClipper matches the original algorithm
template<>
template<>
void object::test<2>
()
{
    for (int iter = 0; iter < 20000; iter++) {
        auto seq = sequence(1 + randomIndex(40), true, true);
        Envelope env = chance(0.01) ? Envelope() : envelope();
        geos::operation::overlayng::RingClipper clipper(&env);

        auto expected = referenceClip(env, seq.get());

        setAvx512Enabled(false);
        auto scalar = clipper.clip(seq.get());
        setAvx512Enabled(true);
        auto simd = clipper.clip(seq.get());

        ensure("scalar clip differs from reference", sameSequence(*expected, *scalar));
        ensure("SIMD clip differs from reference", sameSequence(*expected, *simd));
    }
}

// RayCrossingCounter: location, crossing count and on-segment state
template<>
template<>
void object::test<3>
()
{
    using geos::algorithm::RayCrossingCounter;

    for (int iter = 0; iter < 20000; iter++) {
        auto ring = sequence(1 + randomIndex(40), true, chance(0.2));
        CoordinateXY p;
        if (chance(0.3) && ring->size() > 0) {
            // a ring vertex, or a point on a ring segment
            std::size_t i = randomIndex(ring->size());
            p = ring->getAt<CoordinateXY>(i);
            if (chance(0.5) && i + 1 < ring->size()) {
                const CoordinateXY& q = ring->getAt<CoordinateXY>(i + 1);
                p = CoordinateXY((p.x + q.x) / 2, (p.y + q.y) / 2);
            }
        }
        else {
            p = CoordinateXY(ordinate(false), ordinate(false));
        }
        if (!std::isfinite(p.x) || !std::isfinite(p.y)) {
            // a non-finite point is reported by an exception
            p = CoordinateXY(ordinate(false), ordinate(false));
        }

        // reference: count each segment until the point is found on one
        RayCrossingCounter ref(p);
        for (std::size_t i = 1; i < ring->size() && !ref.isOnSegment(); i++) {
            ref.countSegment(ring->getAt<CoordinateXY>(i - 1), ring->getAt<CoordinateXY>(i));
        }

        for (bool enabled : {false, true}) {
            setAvx512Enabled(enabled);

            ensure_equals("locatePointInRing", RayCrossingCounter::locatePointInRing(p, *ring), ref.getLocation());

            RayCrossingCounter rcc(p);
            rcc.processSequence(*ring, true);
            ensure_equals("count", rcc.getCount(), ref.getCount());
            ensure_equals("onSegment", rcc.isOnSegment(), ref.isOnSegment());
            ensure_equals("location", rcc.getLocation(), ref.getLocation());
        }
    }
}

// MonotoneChainBuilder produces the same chains
template<>
template<>
void object::test<4>
()
{
    using geos::index::chain::MonotoneChain;
    using geos::index::chain::MonotoneChainBuilder;

    for (int iter = 0; iter < 20000; iter++) {
        auto seq = sequence(randomIndex(60), chance(0.5), chance(0.1));

        std::vector<MonotoneChain> scalar, simd;
        setAvx512Enabled(false);
        MonotoneChainBuilder::getChains(seq.get(), nullptr, scalar);
        setAvx512Enabled(true);
        MonotoneChainBuilder::getChains(seq.get(), nullptr, simd);

        ensure_equals("number of chains", simd.size(), scalar.size());
        for (std::size_t i = 0; i < scalar.size(); i++) {
            ensure_equals("chain start", simd[i].getStartIndex(), scalar[i].getStartIndex());
            ensure_equals("chain end", simd[i].getEndIndex(), scalar[i].getEndIndex());
        }
    }
}

// LineIntersector, including near-collinear and non-finite inputs
template<>
template<>
void object::test<5>
()
{
    const double inf = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < 200000; iter++) {
        CoordinateXY pts[4];
        for (auto& pt : pts) {
            pt = CoordinateXY(ordinate(false), ordinate(false));
        }
        if (chance(0.2)) {
            // nearly collinear, requiring extended precision
            double t = std::uniform_real_distribution<double>(-1, 2)(rng);
            pts[2] = CoordinateXY(pts[0].x + t * (pts[1].x - pts[0].x), pts[0].y + t * (pts[1].y - pts[0].y));
            if (chance(0.5)) {
                pts[2].x = std::nextafter(pts[2].x, inf);
            }
        }
        if (chance(0.1)) {
            pts[randomIndex(4)] = pts[randomIndex(4)];
        }
        if (chance(0.01)) {
            double special[] = {NaN, inf, -inf};
            CoordinateXY& pt = pts[randomIndex(4)];
            (chance(0.5) ? pt.x : pt.y) = special[randomIndex(3)];
        }

        setAvx512Enabled(false);
        auto scalar = intersect(pts[0], pts[1], pts[2], pts[3]);
        setAvx512Enabled(true);
        auto simd = intersect(pts[0], pts[1], pts[2], pts[3]);

        ensure(sameResult(scalar, simd));
    }
}

// RobustClipEnvelopeComputer, including signed zeros
template<>
template<>
void object::test<6>
()
{
    using geos::operation::overlayng::RobustClipEnvelopeComputer;

    for (int iter = 0; iter < 20000; iter++) {
        auto shell = sequence(3 + randomIndex(40), true, chance(0.1));
        // LinearRing requires the (finite) first and last points to be equal
        CoordinateXYZM first(1, 2, 3, 4);
        shell->setAt(first, 0);
        shell->setAt(first, shell->size() - 1);
        auto poly = factory->createPolygon(factory->createLinearRing(std::move(shell)));
        auto other = factory->createPolygon(factory->createLinearRing(sequence(3, true, false)));
        Envelope target = envelope();

        setAvx512Enabled(false);
        Envelope scalar = RobustClipEnvelopeComputer::getEnvelope(poly.get(), other.get(), &target);
        setAvx512Enabled(true);
        Envelope simd = RobustClipEnvelopeComputer::getEnvelope(poly.get(), other.get(), &target);

        ensure(sameEnvelope(scalar, simd));
    }
}

// Orientation::isCCW and CoordinateSequence::hasRepeatedPoints
template<>
template<>
void object::test<7>
()
{
    for (int iter = 0; iter < 20000; iter++) {
        auto ring = sequence(randomIndex(40), true, chance(0.1));
        if (chance(0.1)) {
            // flat ring
            for (std::size_t i = 0; i < ring->size(); i++) {
                ring->setOrdinate(i, CoordinateSequence::Y, 1.0);
            }
        }

        // NaN ordinates can make isCCW throw
        auto isCCW = [&ring]() -> std::string {
            try {
                return geos::algorithm::Orientation::isCCW(ring.get()) ? "true" : "false";
            }
            catch (const geos::util::GEOSException& e) {
                return e.what();
            }
        };

        setAvx512Enabled(false);
        std::string ccw = isCCW();
        bool repeated = ring->hasRepeatedPoints();
        setAvx512Enabled(true);
        ensure_equals("isCCW", isCCW(), ccw);
        ensure_equals("hasRepeatedPoints", ring->hasRepeatedPoints(), repeated);
    }
}

// Overlay intersection and relate of random polygons give identical results
template<>
template<>
void object::test<8>
()
{
    using geos::operation::overlayng::OverlayNG;

    for (int iter = 0; iter < 1000; iter++) {
        auto a = starPolygon(0, 0, 3 + randomIndex(200));
        auto b = starPolygon(ordinate(false), ordinate(false), 3 + randomIndex(200));

        std::unique_ptr<Geometry> scalar, simd;
        std::string scalarError, simdError, scalarIM, simdIM;
        bool scalarIntersects = false, simdIntersects = false;

        for (bool enabled : {false, true}) {
            setAvx512Enabled(enabled);
            std::unique_ptr<Geometry> result;
            std::string error, im;
            bool intersects = false;
            try {
                result = OverlayNG::overlay(a.get(), b.get(), OverlayNG::INTERSECTION);
                im = a->relate(b.get())->toString();
                intersects = a->intersects(b.get());
            }
            catch (const geos::util::GEOSException& e) {
                error = e.what();
            }
            if (enabled) {
                simd = std::move(result); simdError = error; simdIM = im; simdIntersects = intersects;
            }
            else {
                scalar = std::move(result); scalarError = error; scalarIM = im; scalarIntersects = intersects;
            }
        }

        ensure_equals("error", simdError, scalarError);
        ensure_equals("relate", simdIM, scalarIM);
        ensure_equals("intersects", simdIntersects, scalarIntersects);
        ensure_equals("result presence", simd == nullptr, scalar == nullptr);
        if (scalar) {
            ensure("intersection", simd->equalsIdentical(scalar.get()));
        }
    }
}

} // namespace tut
