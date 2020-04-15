#include <geos/profiler.h>
#include <string>
#include <sstream>
#include <utility>

using namespace std;

#include <cmath>
#include <geos/math/DD.h>

namespace geos {
namespace math { // geos.util


/* private */
DD
DD::parse(std::string &str)
{
    DD d(0.0, 0.0);
    return d;
}

/* private */
int
DD::magnitude(double x)
{
    double xAbs = std::fabs(x);
    double xLog10 = std::log(xAbs) / std::log(10);
    int xMag = (int) std::floor(xLog10);
    /**
     * Since log computation is inexact, there may be an off-by-one error
     * in the computed magnitude.
     * Following tests that magnitude is correct, and adjusts it if not
     */
    double xApprox = std::pow(10, xMag);
    if (xApprox * 10 <= xAbs)
      xMag += 1;

    return xMag;
}

/* public */
bool DD::isNaN()
{
    return std::isnan(hi);
}
/* public */
bool DD::isNegative()
{
    return hi < 0.0 || (hi == 0.0 && lo < 0.0);
}
/* public */
bool DD::isPositive()
{
    return hi > 0.0 || (hi == 0.0 && lo > 0.0);
}
/* public */
bool DD::isZero()
{
    return hi == 0.0 && lo == 0.0;
}

/* public */
double DD::doubleValue()
{
    return hi + lo;
}

/* public */
int DD::intValue()
{
    return (int) hi;
}

/* public */
void DD::selfAdd(DD const &y)
{
    return selfAdd(y.hi, y.lo);
}

/* public */
void DD::selfAdd(double yhi, double ylo)
{
    double H, h, T, t, S, s, e, f;
    S = hi + yhi;
    T = lo + ylo;
    e = S - hi;
    f = T - lo;
    s = S-e;
    t = T-f;
    s = (yhi-e)+(hi-s);
    t = (ylo-f)+(lo-t);
    e = s+T; H = S+e; h = e+(S-H); e = t+h;

    double zhi = H + e;
    double zlo = e + (H - zhi);
    hi = zhi;
    lo = zlo;
    return;
}

/* public */
void DD::selfAdd(double y)
{
    double H, h, S, s, e, f;
    S = hi + y;
    e = S - hi;
    s = S - e;
    s = (y - e) + (hi - s);
    f = s + lo;
    H = S + f;
    h = f + (S - H);
    hi = H + h;
    lo = h + (H - hi);
    return;
}

/* public */
DD DD::operator+(DD const &rhs) const
{
    DD lhs(hi, lo);
    lhs.selfAdd(rhs);
    return lhs;
}

/* public */
DD DD::operator+(double rhs) const
{
    DD lhs(hi, lo);
    lhs.selfAdd(rhs);
    return lhs;
}


}
}
