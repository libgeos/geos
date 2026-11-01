"""
Bring GEOS curves into ArcGIS Pro as true curves instead of linearized lines.

GEOSCurveToLine turns a curve into a LINESTRING, so ``arcpy.FromWKT`` shows
straight pieces. GEOSGeomToEsriCurveJSON instead returns Esri JSON with
circular arcs, elliptic arcs and cubic Beziers, which ``arcpy.AsShape(json,
True)`` loads as real curve segments.

What is exact and what is approximated (see GEOSGeomToEsriCurveJSON in
geos_c.h for the full rules):

* exact: CircularString, EllipticalCurve, and the parts of a NURBSCurve that
  are lines, cubic or lower polynomials, parabolas, ellipses or circles;
* approximated by cubic Beziers within ``tolerance``: Clothoid, SpiralCurve,
  and the other NURBS parts. ``tolerance`` is the largest distance between an
  approximating Bezier and the true curve, in coordinate units. It is
  measured by dense sampling, not proven.

Typical use in a notebook that already loaded geos_c.dll through ctypes::

    import geos_true_curves as gtc
    gtc.bind(geos)                      # geos = ctypes.CDLL(r"...\\geos_c.dll")
    shape = gtc.to_arcpy_true_curve(geos, geom, 0.01)   # geom: GEOSGeometry*
"""

import ctypes
import json

_c_void_p = ctypes.c_void_p
_c_double = ctypes.c_double
_c_size_t = ctypes.c_size_t


def bind(geos):
    """Declare the argument and result types of the Esri JSON functions.

    ``geos`` is the ``ctypes.CDLL`` of geos_c. Safe to call more than once.
    Raises AttributeError with a hint when the DLL is too old to have them.
    """
    try:
        fn = geos.GEOSGeomToEsriCurveJSON
        fn_stats = geos.GEOSGeomToEsriCurveJSONWithStats
    except AttributeError as exc:  # pragma: no cover - depends on the DLL
        raise AttributeError(
            "this geos_c.dll has no GEOSGeomToEsriCurveJSON: rebuild GEOS from the "
            "arcgis true-curves branch, copy geos_c.dll and geos.dll next to the "
            "notebook, and restart ArcGIS Pro (a loaded DLL cannot be replaced)"
        ) from exc
    # The result is kept as a void pointer so that it can be freed with GEOSFree.
    fn.argtypes = [_c_void_p, _c_double]
    fn.restype = _c_void_p
    fn_stats.argtypes = [
        _c_void_p, _c_double,
        ctypes.POINTER(_c_size_t), ctypes.POINTER(_c_size_t), ctypes.POINTER(_c_double),
    ]
    fn_stats.restype = _c_void_p
    geos.GEOSFree.argtypes = [_c_void_p]
    geos.GEOSFree.restype = None


class CurveStats(object):
    """What the conversion wrote: segment counts and the largest deviation."""

    def __init__(self, segments, approximated, max_deviation):
        self.segments = segments
        self.approximated = approximated
        self.max_deviation = max_deviation

    def __repr__(self):
        return "CurveStats(segments=%d, approximated=%d, max_deviation=%.3g)" % (
            self.segments, self.approximated, self.max_deviation)


def esri_curve_json(geos, geom, tolerance, with_stats=False):
    """Esri JSON text of a GEOS geometry, curves kept as true curves.

    Returns the JSON string, or ``(json, CurveStats)`` with ``with_stats``.
    Raises ValueError when GEOS reports an error (for example a non-positive
    tolerance or a GeometryCollection); the GEOS error handler has printed
    the reason.
    """
    segments = _c_size_t(0)
    approximated = _c_size_t(0)
    deviation = _c_double(0.0)
    ptr = geos.GEOSGeomToEsriCurveJSONWithStats(
        geom, float(tolerance),
        ctypes.byref(segments), ctypes.byref(approximated), ctypes.byref(deviation))
    if not ptr:
        raise ValueError("GEOSGeomToEsriCurveJSON failed (see the GEOS error message)")
    try:
        text = ctypes.string_at(ptr).decode("utf-8")
    finally:
        geos.GEOSFree(ptr)
    if with_stats:
        return text, CurveStats(segments.value, approximated.value, deviation.value)
    return text


def to_arcpy_true_curve(geos, geom, tolerance, spatial_reference=None, with_stats=False):
    """An arcpy geometry with true curve segments.

    ``geom`` is a ``GEOSGeometry*``, ``tolerance`` the largest deviation of
    an approximating Bezier (coordinate units). ``spatial_reference`` is an
    ``arcpy.SpatialReference``, a WKID or None; when None the SRID of the
    GEOS geometry (if any) is used. Uses ``arcpy.AsShape(esri_json, True)``.
    """
    import arcpy  # only available inside ArcGIS Pro

    text, stats = esri_curve_json(geos, geom, tolerance, with_stats=True)
    if spatial_reference is not None:
        document = json.loads(text)
        wkid = getattr(spatial_reference, "factoryCode", spatial_reference)
        document["spatialReference"] = {"wkid": int(wkid)}
        text = json.dumps(document)
    shape = arcpy.AsShape(text, True)
    return (shape, stats) if with_stats else shape


def to_arcpy_linearized(geos, geom, wkt_reader_fn, spatial_reference=None):
    """The old path, for comparison: GEOSCurveToLine then ``arcpy.FromWKT``.

    ``wkt_reader_fn(geom)`` must return the WKT of the linearized geometry,
    as the notebook's existing GEOSCurveToLine helper does.
    """
    import arcpy

    return arcpy.FromWKT(wkt_reader_fn(geom), spatial_reference)
