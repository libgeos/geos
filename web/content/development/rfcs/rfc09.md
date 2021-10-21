---
title: "GEOS RFC 9 - Restore the C++ API (WIP)"
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

|         |                          |
| :------ | ------------------------ |
| RFC 9   | Restore the C++ API   |
| Author  | Mateusz Łoskot           |
| Contact | mateusz@loskot.net       |
| Status  | [https://lists.osgeo.org/pipermail/geos-devel/2019-May/008972.html Proposed]  |

The GEOS library is a C++ library offering two kinds of public API: C++ API and C API.

The GEOS library started as a C++ library. The C API was introduced in version 2.2.

The GEOS library has never promised any stability of the C++ API and this fact has always been documented and clearly stated:

on the wiki:

> C++ API (will likely change across versions)
> C API (provides long-term ABI stability)

on the front page of the API reference:

> The recommended low-level interface to the GEOS library is the simplified C wrapper interface.
> This will ensure stability of the API and the ABI of the library during performance improvements
> that will likely change classes definitions.
>
> If you don't care about adapting/rebuilding your client code you can still use the C++ interface.

in the NEWS file:

> Changes in 2.2.0
>    - NEW Simplified and stabler C API

The GEOS library as always been deployed as two **distinct** binaries:

* `geos` accompanied with the C++ headers.
* `geos_c` accompanied with the C header.

Removing the C++ API from the public scope and asking developers to opt-in to use the C++ API,fundamentally breaks the original concept of the library.

If there are developers surprised by any breaking changes in the C++ API, it means they have not read the documentation and it is not role of the GEOS developers to make them read it.

Any user of the GEOS C++ API is expected to be aware of its volatile state and be prepared to update in order to use any newer version of GEOS. These implicit usage terms of the contract, which have always been clear and consistent with nearly any other C++ API, remain unchanged.

Considering these issues, there is very little value in the `#ifdef USE_UNSTABLE_GEOS_CPP_API` and related guards.

Let's revert the implementation of the RFC6.
