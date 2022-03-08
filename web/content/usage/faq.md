---
title: "FAQ"
draft: false
weight: 200
---

{{< toc >}}

## Robustness

### Why doesn't a computed point lie exactly on a line?

GEOS represents geometry coordinates using IEEE-754 double-precision floating point numbers.
This is a finite representation, whereas the implicit line connecting two points has infinite precision.  In general it is highly unlikely that a coordinate computed via an arithmetic operation
(such as a point interpolated at a distance along the line) is reported by an `intersects` test as lying exactly on the (theoretical) line.
