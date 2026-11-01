"""
Bring GEOS curves into ArcGIS Pro as true curves instead of linearized lines.

GEOSCurveToLine turns a curve into a LINESTRING, so ``arcpy.FromWKT`` shows
straight pieces. GEOSEsriJSONWriter instead returns Esri JSON with
circular arcs, elliptic arcs and cubic Beziers, which ``arcpy.AsShape(json,
True)`` loads as real curve segments.

What is exact and what is approximated (see GEOSEsriJSONWriter in
geos_c.h for the full rules):

* exact: CircularString, EllipticalCurve, and the parts of a NURBSCurve that
  are lines, cubic or lower polynomials, parabolas, ellipses or circles;
* approximated by cubic Beziers within ``tolerance``: Clothoid, SpiralCurve,
  and the other NURBS parts. ``tolerance`` is the largest distance between an
  approximating Bezier and the true curve, in coordinate units. It is
  measured by dense sampling (a sampled Hausdorff distance), not proven.

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
    """Declare the argument and result types of the Esri JSON writer.

    ``geos`` is the ``ctypes.CDLL`` of geos_c. Safe to call more than once.
    Raises AttributeError with a hint when the DLL is too old to have it.
    """
    try:
        geos.GEOSEsriJSONWriter_create
    except AttributeError as exc:  # pragma: no cover - depends on the DLL
        raise AttributeError(
            "this geos_c.dll has no GEOSEsriJSONWriter: rebuild GEOS from the "
            "arcgis true-curves branch, copy geos_c.dll and geos.dll next to the "
            "notebook, and restart ArcGIS Pro (a loaded DLL cannot be replaced)"
        ) from exc
    geos.GEOSEsriJSONWriter_create.argtypes = []
    geos.GEOSEsriJSONWriter_create.restype = _c_void_p
    geos.GEOSEsriJSONWriter_destroy.argtypes = [_c_void_p]
    geos.GEOSEsriJSONWriter_destroy.restype = None
    geos.GEOSEsriJSONWriter_setTolerance.argtypes = [_c_void_p, _c_double]
    geos.GEOSEsriJSONWriter_setTolerance.restype = ctypes.c_int
    geos.GEOSEsriJSONWriter_setMatchCurvature.argtypes = [_c_void_p, ctypes.c_int]
    geos.GEOSEsriJSONWriter_setMatchCurvature.restype = None
    # The text is kept as a void pointer so that it can be freed with GEOSFree.
    geos.GEOSEsriJSONWriter_writeGeometry.argtypes = [_c_void_p, _c_void_p]
    geos.GEOSEsriJSONWriter_writeGeometry.restype = _c_void_p
    geos.GEOSEsriJSONWriter_getNumSegments.argtypes = [_c_void_p]
    geos.GEOSEsriJSONWriter_getNumSegments.restype = _c_size_t
    geos.GEOSEsriJSONWriter_getNumApproximated.argtypes = [_c_void_p]
    geos.GEOSEsriJSONWriter_getNumApproximated.restype = _c_size_t
    geos.GEOSEsriJSONWriter_getMaxDeviation.argtypes = [_c_void_p]
    geos.GEOSEsriJSONWriter_getMaxDeviation.restype = _c_double
    geos.GEOSFree.argtypes = [_c_void_p]
    geos.GEOSFree.restype = None


class CurveStats(object):
    """What the conversion wrote: segment counts and the largest deviation.

    ``max_deviation`` is a sampled Hausdorff distance, a measurement and not a
    proof, and it is 0 when every segment is exact.
    """

    def __init__(self, segments, approximated, max_deviation):
        self.segments = segments
        self.approximated = approximated
        self.max_deviation = max_deviation

    def __repr__(self):
        return "CurveStats(segments=%d, approximated=%d, max_deviation=%.3g)" % (
            self.segments, self.approximated, self.max_deviation)


def esri_curve_json(geos, geom, tolerance, with_stats=False, match_curvature=True):
    """Esri JSON text of a GEOS geometry, curves kept as true curves.

    ``tolerance`` has no default, since a sensible value depends on the
    coordinate system. Returns the JSON string, or ``(json, CurveStats)``
    with ``with_stats``. Raises ValueError when GEOS reports an error (a
    non-positive tolerance, a GeometryCollection, a singular placement); the
    GEOS error handler has printed the reason.
    """
    writer = geos.GEOSEsriJSONWriter_create()
    if not writer:
        raise MemoryError("GEOSEsriJSONWriter_create failed")
    try:
        if not geos.GEOSEsriJSONWriter_setTolerance(writer, float(tolerance)):
            raise ValueError("tolerance must be positive and finite")
        geos.GEOSEsriJSONWriter_setMatchCurvature(writer, 1 if match_curvature else 0)
        ptr = geos.GEOSEsriJSONWriter_writeGeometry(writer, geom)
        if not ptr:
            raise ValueError("GEOSEsriJSONWriter_writeGeometry failed (see the GEOS error message)")
        try:
            text = ctypes.string_at(ptr).decode("utf-8")
        finally:
            geos.GEOSFree(ptr)
        stats = CurveStats(
            geos.GEOSEsriJSONWriter_getNumSegments(writer),
            geos.GEOSEsriJSONWriter_getNumApproximated(writer),
            geos.GEOSEsriJSONWriter_getMaxDeviation(writer))
    finally:
        geos.GEOSEsriJSONWriter_destroy(writer)
    return (text, stats) if with_stats else text


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
