---
title: "FAQ"
draft: false
weight: 200
---

{{< toc >}}

## Robustness

### Why is `GEOSIntersects(GEOSIntersection(A, B), A) == false`?

GEOS represents geometry coordinates using IEEE-754 double-precision floating point numbers.
This is a finite representation, whereas the implicit lines between vertices have infinite precision.  In general it is highly unlikely that a coordinate computed via an arithmetic operation
(such as a line intersection) is reported by an `intersects` test as lying exactly on the (theoretical) lines.

For example, the diagram below shows how the computed intersection point of two lines in general does not lie exactly on either line (scale exaggerrated for clarity):

![GEOS computed intersection point for two lines](/geos-line-intersect-precision.png)

### Why doesn't a computed point lie exactly on a line?

As explained in the previous question, GEOS uses finite-precision floating point arithmetic.  In general coordinates computed via arithmetic operations
(such as interpolation along a line) is reported by an `intersects` test as lying exactly on the (theoretical) line.

![GEOS computed points interpolated along line](/geos-line-interpolated-precision.png)
