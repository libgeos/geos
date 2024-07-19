---
title: "GEOS RFC 6 - Require explicit configure to use the C++ API"
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

|         |                          |
| :------ | ------------------------ |
| RFC 9   | Require explicit configure to use the C++ API   |
| Author  | Regina Obe           |
| Contact | lr@pcorp.us       |
| Status  | [Not Passed](https://lists.osgeo.org/pipermail/geos-devel/2017-October/thread.html#8050)  |

## Past discussions

[Trac ticket to deprecate](https://trac.osgeo.org/geos/ticket/553)
[another request to deprecate and osm2pgsql mess](https://lists.osgeo.org/pipermail/geos-devel/2017-January/007652.html)
[more examples about how apps linking directly to GEOS C++ causing problems for other applications](https://lists.osgeo.org/pipermail/geos-devel/2012-June/005861.html)
[Pointing out removing ability to use GEOS C++ reduces users freedoms](https://lists.osgeo.org/pipermail/geos-devel/2017-January/007653.html)

## Summary

This document proposes to change the `./configure` and CMake to by default only allow use of the C-API.

The C++ API headers and library will only be installed if explicitly asked for
as detailed in https://lists.osgeo.org/pipermail/geos-devel/2017-October/008054.html

Any developers who want to use the C++ API will have to build with

```
#autoconf users
   ./configure --with-cplusplus-sdk-install

#cmake users
   cmake -DINSTALL_CPLUSPLUS_SDK
```

If cplusplus sdk install is not expressly requested only the C headers will be included and the C++ headers will not be installed. In addition, when the C++ headers are used by any project, if users choose to build with the cplusplus-sdk, a warning will be shown before start of compile stating:

> The GEOS project does not guarantee ABI stability of the C++ API during minor updates.
> This means your code may need recompilation or changes to upgrade to next minor version of GEOS.
> If you want ABI stability from minor version to minor version, you should use the C-API instead.

In addition the GEOS C++API Headers `Geometry.h` will by default include a warning as noted in https://git.osgeo.org/gogs/geos/geos/pulls/14 as proposed in https://lists.osgeo.org/pipermail/geos-devel/2017-October/008071.html

> The GEOS C++ API is unstable, please use the C API instead
> HINT: #include geos_c.h

Which will show during compile time if the following variable is not defined

> WE_ACKNOWLEDGE_THAT_THE_GEOS_CPLUSPLUS_API_IS_UNSTABLE

This message will continue to be shown in every project that a user tries to compile using GEOS C++ API headers.

I propose doing this in GEOS 3.7.0.

The main purpose is to discourage the use of the C++ API because we do not have the manpower to guarantee ABI or API compatibility from minor version to minor version and thus using it in an environment where the GEOS library is shared across many software is unsupported.  We are also planning significant refactoring in GEOS 3.8 which will very likely break the C++ API.

Currently [osm2pgsql](https://github.com/openstreetmap/osm2pgsql/issues/634) and OSSIM are the only ones that used the GEOS C++ API and largely distributed in shared environment.  We want to discourage future projects that plan to be used in a shared environment from using the GEOS C++ API and to stick with the GEOS C API.

So the purpose of the above is to affect the following change:

1. Package Distributions do not need compile with these flags, though they may choose to at their own risk to support C++ API users.
1. The end effect being, no C++ headers installed  and just the C header file installed.
1. People building their own binaries or their own projects that utilize the C++ API may not be able to build using a packaged libgeosdev-++ if packagers don't compile with c++ support.
1. If a package distributtion does choose to offer the headers and take the default of not defining the WE_... users building with the package will get the above warning at compile time of their project.

That way new projects will be clear about what compromise they are making using the C++ API and if they are not users will ask

> What is this warning I keep on getting about you using an unstable API?

