"""
Measure GEOSEsriJSONWriter on curves like the ones in the ArcGIS test
notebook, without ArcGIS: segment counts, exact versus approximated, and the
largest measured deviation.

    python measure_true_curves.py /path/to/libgeos_c.so [tolerance]

Needs only a geos_c built from the true-curves branch. The curve parameters
are the ones named in the notebook description; anything the description did
not give is marked "assumed".
"""

import ctypes
import math
import sys

import geos_true_curves as gtc


def load(path):
    geos = ctypes.CDLL(path)
    geos.GEOSGeomFromWKT.argtypes = [ctypes.c_char_p]
    geos.GEOSGeomFromWKT.restype = ctypes.c_void_p
    geos.GEOSGeom_destroy.argtypes = [ctypes.c_void_p]
    geos.GEOSGeomGetEndPoint.argtypes = [ctypes.c_void_p]
    geos.GEOSGeomGetEndPoint.restype = ctypes.c_void_p
    geos.GEOSGeomGetX.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_double)]
    geos.GEOSGeomGetY.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_double)]
    geos.GEOSGeomToWKT.argtypes = [ctypes.c_void_p]
    geos.GEOSGeomToWKT.restype = ctypes.c_void_p
    gtc.bind(geos)
    geos.initGEOS.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    geos.initGEOS(None, None)
    return geos


PLACE = ("REFERENCELOCATION AFFINEPLACEMENT (LOCATION (%r %r), REFERENCEDIRECTIONS "
         "(VECTOR (%r %r), VECTOR (%r %r)))")


def placement(x, y, heading):
    c, s = math.cos(heading), math.sin(heading)
    return PLACE % (x, y, c, s, -s, c)


def clothoid(x, y, heading, scale, t0, t1):
    return "CLOTHOID (%s, SCALEFACTOR %r, STARTDISTANCE %r, ENDDISTANCE %r)" % (
        placement(x, y, heading), scale, t0, t1)


def spiral(x, y, heading, length, k0, k1, kind):
    return "SPIRALCURVE (%s, LENGTH %r, STARTCURVATURE %r, ENDCURVATURE %r, SPIRALTYPE %s)" % (
        placement(x, y, heading), length, k0, k1, kind)


def end_point(geos, wkt):
    """The exact end point GEOS evaluates for a curve, so that the next
    section of a CompoundCurve starts exactly there."""
    g = geos.GEOSGeomFromWKT(wkt.encode())
    p = geos.GEOSGeomGetEndPoint(g)
    x, y = ctypes.c_double(), ctypes.c_double()
    geos.GEOSGeomGetX(p, ctypes.byref(x))
    geos.GEOSGeomGetY(p, ctypes.byref(y))
    geos.GEOSGeom_destroy(p)
    geos.GEOSGeom_destroy(g)
    return x.value, y.value


def railway(geos):
    """Straight, bloss transition, circular arc, bloss transition, straight.

    Assumed: 400 m straights, 150 m transitions, R = 800 m, 400 m of arc.
    """
    R, Lt, Larc, Ls = 800.0, 150.0, 400.0, 400.0
    k = 1.0 / R
    x, y, h = 0.0, 0.0, 0.0
    parts = ["(%r %r, %r %r)" % (x, y, x + Ls, y)]
    x += Ls
    entry = spiral(x, y, h, Lt, 0.0, k, "bloss")
    parts.append(entry)
    x, y = end_point(geos, entry)
    h += Lt * k / 2.0            # a bloss transition turns (k0 + k1) L / 2
    # Circular arc: left turn about the centre.
    cx, cy = x - R * math.sin(h), y + R * math.cos(h)
    sweep = Larc / R
    pts = ["%r %r" % (x, y)]
    for f in (0.5, 1.0):
        a = h - math.pi / 2 + f * sweep
        pts.append("%r %r" % (cx + R * math.cos(a), cy + R * math.sin(a)))
    parts.append("CIRCULARSTRING (%s)" % ", ".join(pts))
    x, y = [float(v) for v in pts[-1].split()]
    h += sweep
    exit_ = spiral(x, y, h, Lt, k, 0.0, "bloss")
    parts.append(exit_)
    x, y = end_point(geos, exit_)
    h += Lt * k / 2.0
    parts.append("(%r %r, %r %r)" % (x, y, x + Ls * math.cos(h), y + Ls * math.sin(h)))
    return "COMPOUNDCURVE (%s)" % ", ".join(parts)


def cases(geos):
    out = [
        ("clothoid 25 m, SCALEFACTOR 50", clothoid(0, 0, 0, 50, 0, 25)),
        ("clothoid 100 m, SCALEFACTOR 50", clothoid(0, 0, 0, 50, 0, 100)),
    ]
    # Assumed: five spiral types fanned 15 degrees apart, 1200 m long, R = 400 m at the end.
    for i, kind in enumerate(["clothoid", "bloss", "biquadratic", "sine", "cosine"]):
        out.append(("spiral %s 1200 m (fan member)" % kind,
                    spiral(0, 0, math.radians(15.0 * i), 1200.0, 0.0, 1.0 / 400.0, kind)))
    out.append(("railway alignment (bloss transitions)", railway(geos)))
    return out


def main():
    path = sys.argv[1]
    tols = [float(a) for a in sys.argv[2:]] or [0.01]
    geos = load(path)
    for tol in tols:
        print("tolerance %g" % tol)
        print("%-44s %9s %12s %14s" % ("curve", "segments", "approximated", "max deviation"))
        for name, wkt in cases(geos):
            g = geos.GEOSGeomFromWKT(wkt.encode())
            if not g:
                print("%-44s parse error" % name)
                continue
            _, stats = gtc.esri_curve_json(geos, g, tol, with_stats=True)
            print("%-44s %9d %12d %14.3g" % (name, stats.segments, stats.approximated, stats.max_deviation))
            geos.GEOSGeom_destroy(g)


if __name__ == "__main__":
    main()
