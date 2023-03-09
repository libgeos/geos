---
title: "GEOS RFC 7 - Use CMake for Build System"
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

|         |                                 |
| :------ | ------------------------------- |
| RFC 7   | Use CMake for build system |
| Author  | Daniel Baston                    |
| Contact | dbaston@gmail.com    |
| Status  | [Accepted](https://lists.osgeo.org/pipermail/geos-devel/2021-January/010098.html), January 15, 2021                   |

This document proposes to use CMake as the build system for GEOS and discontinue use of autotools and NMake.

Since version 3.5, we have officially supported building GEOS with CMake: https://trac.osgeo.org/geos/wiki/BuildingOnUnixWithCMake

GEOS is also required to build with autotools and NMake.

Supporting three build systems:

* Decreases ease of contribution, because any change must be tested against all three build systems. This results in more developer effort devoted to the build system rather than the library itself (see for example, commit history of this PR: https://github.com/libgeos/geos/pull/125)
* Increases the risk that differences between build systems cause the library to be compiled with different behavior, or with different version information (for example, see https://trac.osgeo.org/geos/ticket/882)
* Increases the length of the commit-testing feedback cycle, since multiple build systems must be tested before giving a pull request a "green light" and insufficient workers are available to test all build systems in parallel.

This RFC proposes that CMake be used as the exclusive build system because:

* It is used by the majority of active committers
* It is the only cross-platform (OS/compiler) build system
