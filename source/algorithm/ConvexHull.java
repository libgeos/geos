/*
 * The Java Topology Suite (JTS) is a collection of Java classes that
 * implement the fundamental operations required to validate a given
 * geo-spatial data set to a known topological specification.
 *
 * Copyright (C) 2001 Vivid Solutions
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more information, contact:
 *
 *     Vivid Solutions
 *     Suite #1A
 *     2328 Government Street
 *     Victoria BC  V8T 5G5
 *     Canada
 *
 *     (250)385-6040
 *     www.vividsolutions.com
 */
package com.vividsolutions.jts.algorithm;
import com.vividsolutions.jts.geom.*;
import com.vividsolutions.jts.util.Assert;

import java.util.*;
import com.vividsolutions.jts.util.UniqueCoordinateArrayFilter;

/**
 *@version 1.2
 */
public class ConvexHull {
  private PointLocator pointLocator = new PointLocator();
  private CGAlgorithms cgAlgorithms;
  private Geometry geometry;

  public ConvexHull(CGAlgorithms cgAlgorithms) {
    this.cgAlgorithms = cgAlgorithms;
  }

  public Geometry getConvexHull(Geometry geometry) {
    this.geometry = geometry;
    UniqueCoordinateArrayFilter filter = new UniqueCoordinateArrayFilter();
    geometry.apply(filter);
    Coordinate[] pts = filter.getCoordinates();

    if (pts.length == 0) {
      return new GeometryCollection(new Geometry[]{},
            geometry.getPrecisionModel(), geometry.getSRID());
    }
    if (pts.length == 1) {
      return new Point(pts[0], geometry.getPrecisionModel(), geometry.getSRID());
    }
    if (pts.length == 2) {
      return new LineString(pts, geometry.getPrecisionModel(), geometry.getSRID());
    }

    // sort points for Graham scan.
    Coordinate[] pspts;
    if (pts.length > 10) {
      //Probably should be somewhere between 50 and 100?
      Coordinate[] rpts = reduce(pts);
      pspts = preSort(rpts);
    }
    else {
      pspts = preSort(pts);
    }

    // Use Graham scan to find convex hull.
    Stack cHS = grahamScan(pspts);

    // Convert stack to an array.
    Coordinate[] cH = toCoordinateArray(cHS);

    // Convert array to linear ring.
    return lineOrPolygon(cH);
  }

  /**
   * An alternative to Stack.toArray, which is not present in earlier versions
   * of Java.
   */
  protected Coordinate[] toCoordinateArray(Stack stack) {
    Coordinate[] coordinates = new Coordinate[stack.size()];
    for (int i = 0; i < stack.size(); i++) {
      Coordinate coordinate = (Coordinate) stack.get(i);
      coordinates[i] = coordinate;
    }
    return coordinates;
  }

  private Coordinate[] reduce(Coordinate[] pts) {
    BigQuad bigQuad = bigQuad(pts);

    // Build a linear ring defining a big poly.
    ArrayList bigPoly = new ArrayList();
    bigPoly.add(bigQuad.westmost);
    if (!bigPoly.contains(bigQuad.northmost)) {
      bigPoly.add(bigQuad.northmost);
    }
    if (!bigPoly.contains(bigQuad.eastmost)) {
      bigPoly.add(bigQuad.eastmost);
    }
    if (!bigPoly.contains(bigQuad.southmost)) {
      bigPoly.add(bigQuad.southmost);
    }
    if (bigPoly.size() < 3) {
      return pts;
    }
    bigPoly.add(bigQuad.westmost);
    Coordinate[] bigPolyArray = new Coordinate[bigPoly.size()];
    LinearRing bQ = new LinearRing((Coordinate[]) bigPoly.toArray(bigPolyArray),
        geometry.getPrecisionModel(), geometry.getSRID());

    // load an array with all points not in the big poly
    // and the defining points.
    TreeSet reducedSet = new TreeSet(bigPoly);
    for (int i = 0; i < pts.length; i++) {
      if (pointLocator.locate(pts[i], bQ) == Location.EXTERIOR) {
        reducedSet.add(pts[i]);
      }
    }
    Coordinate[] rP = (Coordinate[]) reducedSet.toArray(new Coordinate[0]);

    // Return this array as the reduced problem.
    return rP;
  }

  private Coordinate[] preSort(Coordinate[] pts) {
    Coordinate t;

    // find the lowest point in the set. If two or more points have
    // the same minimum y coordinate choose the one with the minimu x.
    // This focal point is put in array location pts[0].
    for (int i = 1; i < pts.length; i++) {
      if ((pts[i].y < pts[0].y) || ((pts[i].y == pts[0].y) && (pts[i].x < pts[0].x))) {
        t = pts[0];
        pts[0] = pts[i];
        pts[i] = t;
      }
    }

    // sort the points radially around the focal point.
    radialSort(pts);
    return pts;
  }

  private Stack grahamScan(Coordinate[] c) {
    Coordinate p;
    Coordinate p1;
    Coordinate p2;
    Stack ps = new Stack();
    p = (Coordinate) ps.push(c[0]);
    p = (Coordinate) ps.push(c[1]);
    p = (Coordinate) ps.push(c[2]);
    for (int i = 3; i < c.length; i++) {
      p = (Coordinate) ps.pop();
      while (cgAlgorithms.computeOrientation((Coordinate) ps.peek(), p, c[i]) > 0) {
        p = (Coordinate) ps.pop();
      }
      p = (Coordinate) ps.push(p);
      p = (Coordinate) ps.push(c[i]);
    }
    p = (Coordinate) ps.push(c[0]);
    return ps;
  }

  private void radialSort(Coordinate[] p) {

    // A selection sort routine, assumes the pivot point is
    // the first point (i.e., p[0]).
    Coordinate t;
    for (int i = 1; i < (p.length - 1); i++) {
      int min = i;
      for (int j = i + 1; j < p.length; j++) {
        if (polarCompare(p[0], p[j], p[min]) < 0) {
          min = j;
        }
      }
      t = p[i];
      p[i] = p[min];
      p[min] = t;
    }
  }

  private int polarCompare(Coordinate o, Coordinate p, Coordinate q) {

    // Given two points p and q compare them with respect to their radial
    // ordering about point o. -1, 0 or 1 depending on whether p is less than,
    // equal to or greater than q. First checks radial ordering then if both
    // points lie on the same line, check distance to o.
    double dxp = p.x - o.x;
    double dyp = p.y - o.y;
    double dxq = q.x - o.x;
    double dyq = q.y - o.y;
    double alph = Math.atan2(dxp, dyp);
    double beta = Math.atan2(dxq, dyq);
    if (alph < beta) {
      return -1;
    }
    if (alph > beta) {
      return 1;
    }
    double op = dxp * dxp + dyp * dyp;
    double oq = dxq * dxq + dyq * dyq;
    if (op < oq) {
      return -1;
    }
    if (op > oq) {
      return 1;
    }
    return 0;
  }

  /**
   *@return    whether the three coordinates are collinear and c2 lies between
   *      c1 and c3 inclusive
   */
  private boolean isBetween(Coordinate c1, Coordinate c2, Coordinate c3) {
    if (cgAlgorithms.computeOrientation(c1, c2, c3) != 0) {
      return false;
    }
    if (c1.x != c3.x) {
      if (c1.x <= c2.x && c2.x <= c3.x) {
        return true;
      }
      if (c3.x <= c2.x && c2.x <= c1.x) {
        return true;
      }
    }
    if (c1.y != c3.y) {
      if (c1.y <= c2.y && c2.y <= c3.y) {
        return true;
      }
      if (c3.y <= c2.y && c2.y <= c1.y) {
        return true;
      }
    }
    return false;
  }

  private BigQuad bigQuad(Coordinate[] pts) {
    BigQuad bigQuad = new BigQuad();
    bigQuad.northmost = pts[0];
    bigQuad.southmost = pts[0];
    bigQuad.westmost = pts[0];
    bigQuad.eastmost = pts[0];
    for (int i = 1; i < pts.length; i++) {
      if (pts[i].x < bigQuad.westmost.x) {
        bigQuad.westmost = pts[i];
      }
      if (pts[i].x > bigQuad.eastmost.x) {
        bigQuad.eastmost = pts[i];
      }
      if (pts[i].y < bigQuad.southmost.y) {
        bigQuad.southmost = pts[i];
      }
      if (pts[i].y > bigQuad.northmost.y) {
        bigQuad.northmost = pts[i];
      }
    }
    return bigQuad;
  }

  /**
   *@param  vertices  the vertices of a linear ring, which may or may not be
   *      flattened (i.e. vertices collinear)
   *@return           a 2-vertex <code>LineString</code> if the vertices are
   *      collinear; otherwise, a <code>Polygon</code> with unnecessary
   *      (collinear) vertices removed
   */
  private Geometry lineOrPolygon(Coordinate[] coordinates) {

    coordinates = cleanRing(coordinates);
    if (coordinates.length == 3) {
      return new LineString(new Coordinate[]{coordinates[0], coordinates[1]},
          geometry.getPrecisionModel(), geometry.getSRID());
    }

    LinearRing linearRing = new LinearRing(coordinates,
          geometry.getPrecisionModel(), geometry.getSRID());
    return new Polygon(linearRing, geometry.getPrecisionModel(), geometry.getSRID());
  }

  /**
   *@param  vertices  the vertices of a linear ring, which may or may not be
   *      flattened (i.e. vertices collinear)
   *@return           the coordinates with unnecessary (collinear) vertices
   *      removed
   */
  private Coordinate[] cleanRing(Coordinate[] original) {
    Assert.equals(original[0], original[original.length - 1]);
    ArrayList cleanedRing = new ArrayList();
    Coordinate previousDistinctCoordinate = null;
    for (int i = 0; i <= original.length - 2; i++) {
      Coordinate currentCoordinate = original[i];
      Coordinate nextCoordinate = original[i+1];
      if (currentCoordinate.equals(nextCoordinate)) {
        continue;
      }
      if (previousDistinctCoordinate != null
          && isBetween(previousDistinctCoordinate, currentCoordinate, nextCoordinate)) {
        continue;
      }
      cleanedRing.add(currentCoordinate);
      previousDistinctCoordinate = currentCoordinate;
    }
    cleanedRing.add(original[original.length - 1]);
    Coordinate[] cleanedRingCoordinates = new Coordinate[cleanedRing.size()];
    return (Coordinate[]) cleanedRing.toArray(cleanedRingCoordinates);
  }

  private static class BigQuad {
    public Coordinate northmost;
    public Coordinate southmost;
    public Coordinate westmost;
    public Coordinate eastmost;
  }

}
