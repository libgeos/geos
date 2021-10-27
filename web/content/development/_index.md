---
title: "Development"
date: 2021-10-04T14:21:00-07:00
draft: false
---

## Relationship to JTS

GEOS started as a direct port to C++ of the [JTS Topology Suite](http://tsusiatsoftware.net/jts/main.html) (JTS), and remains tightly bound to that project. Most core algorithms have been prototyped in JTS and ported to GEOS when complete.

The projects attempt to share testing data, and to ascertain when failures are caused by differences in implementation (GEOS fails and JTS does not) and when they are caused by algorithm (both libraries fail).

## Governance

The GEOS project is run by a [Project Steering Committee](({{< ref "psc" >}})) made up of developers and contributors to the project and is a project of [OSGeo](https://www.osgeo.org/projects/geos/).

