# ArcGIS Pro: GEOS curves as true curves

* `geos_true_curves.py`: ctypes binding for `GEOSGeomToEsriCurveJSON` and
  `to_arcpy_true_curve(geos, geom, tolerance)`, which loads the Esri JSON with
  `arcpy.AsShape(json, True)`.
* `GEOS_True_Curves_demo.ipynb`: a demo notebook (a reconstruction; it is not
  the original test notebook).
* `measure_true_curves.py`: prints segment counts and measured deviations
  without ArcGIS (`python measure_true_curves.py /path/to/libgeos_c.so 0.01`).

## What becomes which segment

| GEOS type | ArcGIS segment | |
|---|---|---|
| CircularString | circular arc | exact |
| EllipticalCurve | elliptic arc (circular arc if a circle), pieces of at most 120 degrees | exact |
| NURBS span, rational quadratic ellipse/circle | elliptic / circular arc | exact |
| NURBS span, polynomial of degree <= 3 (or exactly cubic), parabola | line / cubic Bezier | exact |
| Clothoid, SpiralCurve (all five types) | cubic Beziers, G1, curvature matched at joints | approximation within `tolerance` |
| other NURBS spans (hyperbola, rational degree > 2, polynomial degree > 3) | cubic Beziers | approximation within `tolerance` |

`tolerance` is the largest distance between an approximating Bezier and the
curve, in coordinate units. It is checked by dense sampling (256 points per
Bezier, projected both ways), not proven.

## Windows

1. Build GEOS from this branch with MSVC (x64, Release):

       cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=OFF
       cmake --build build --config Release --target geos_c

2. Copy `build\bin\Release\geos_c.dll` and `geos.dll` next to the notebook.
3. Close the notebook and restart ArcGIS Pro (or restart the Python kernel):
   a loaded DLL cannot be replaced, and ctypes keeps the old one until then.
4. Run the notebook.

Esri JSON note: Z and M are not written, polygon shells are written clockwise
and holes counterclockwise, and a positive SRID is written as `spatialReference`.
