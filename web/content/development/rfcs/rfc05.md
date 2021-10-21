---
title: "GEOS RFC 5 - C++11 Compilation Mode"
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

This document proposes and describes desired code formatting style used across C/C++ source code in GEOS.

|         |                          |
| :------ | ------------------------ |
| RFC 5   | C++11 Compilation Mode   |
| Author  | Mateusz Łoskot           |
| Contact | mateusz@loskot.net       |
| Status  | [Accepted](https://lists.osgeo.org/pipermail/geos-devel/2017-April/007735.html) (no agreement)  |


## Summary

The document proposes to switch to [C++11](https://en.wikipedia.org/wiki/C%2B%2B11) compilation mode as default throughout the whole C++ source code of GEOS.

The goal of the document is to request and achieve agreement on using C++11 as the minimum required version  of the C++ programming language standard.

## Motivation

The C++11 is the first major update of the C++ standard  since 1998. (C++03 was a bug fix release.)

The C++11 features aim to promote writing clean, compact, type-safe and fast code. It also delivers better feature-wise compatibility with C language (C99).

The Wikipedia article at https://en.wikipedia.org/wiki/C++11 does a great job describing all changes in  C++11 extensively.

The `std::auto_ptr` smart pointer, together with a bunch of other features, has been deprecated and will be removed from C++17.

The new compilers provide better diagnostics.

Enabling C++11 compilation mode will improve the programming environment making it much friendlier than C++98.

A social factor: since (many) C++ programmers no longer enjoy C++98, allowing C++11 mode may increase potential for new contributions.

## Compilers Landscape

Summary of compilers supported by GEOS with their minimal versions required to compile source code based on [http://en.cppreference.com/w/cpp/compiler_support C++11 features].

### C++11

| Compiler | Version | Status | Support |
| -------- | ------- | ------ | ------- |
| GCC  | 4.8.1+      | [C++11 status](https://gcc.gnu.org/projects/cxx-status.html#cxx11)    | Debian 8 (stable), Ubuntu 15.04+, Ubuntu 14.04 `ppa:ubuntu-toolchain-r/test`, Fedora 19+, RHEL7  |
| Clang| 3.3+        | [C++11 status](https://clang.llvm.org/cxx_status.html#cxx11)          | Debian 8 (stable), Ubuntu 14.04+, Fedora 19+, CentOS 6(?) |
| MSVC | 12.0+ (2013)| [C++11 status](https://msdn.microsoft.com/en-us/library/hh567368.aspx)  | n/a |

### C++14

The C++14 compilers are listed for comparison only:

| Compiler | Version |
| -------- | ------- |
| GCC      | 4.9+    |
| Clang    | 3.4+    |
| MSVC     | 14.0+ (2015) |

## Plan

This proposal only requests agreement for the C++11 compilation mode switch in the current `trunk` branch only.

If accepted, currently available build configurations (Autotools, CMake, NMake) will be updated to switch the compilation mode to C++11.

This proposal does not suggest any detailed roadmap of big refactoring of the GEOS C++ codebase.

The GEOS codebase is around 150 KLOC and given the available man-power to LOCs ratio, such one-step refactoring would not be feasible.

Instead, the task will be tackled with the baby step approach gradually transforming the codebase according to priorities set along the way. Any disruptive refactoring, changes in interfaces of C++ classes, breaking changes in C++ API must be announced and discussed on the mailing list or the bug tracker.

**IMPORTANT**: C++11 refactoring must not change the C API or break C API compatibility, unless agreed upon based on prior RFC proposed.

However, if the proposal is accepted, any new C++ code written for GEOS can be C++11-compliant.

Prior acceptance of this proposal is necessary in order to start any source code refactoring using C++11 features.

Once accepted, first step will be to update the build configurations to require C++11-compliant compiler.

## Issues

This section outlines issues potentially caused by upgrade to C++11 language.

* C++11 destructors, by default, have now the new exception specification of `nothrow(true)`. Destructors of GEOS classes should be reviewed and any that are allowed/expected to throw exceptions must be marked with `nothrow(false)`. Otherwise, any user of the existing GEOS codebase would find the program terminating whenever GEOS destructor throws an exception. Such review would be beneficial anyway.

## Release

First release of GEOS with C++11 compiler requirement could be 3.7.0 or, perhaps, 4.0.0.

### C++14

This section clarifies status of C++14 support in GEOS.

* Once C++11 is adopted as default compilation mode, GEOS developers and maintainers must ensure it also successfully compiles in C++14 and C++17 modes.
* Are contributors allowed to add `ifdef`'s for C++14 and C++17? No.
* Is there a plan to upgrade to C++14 or C++17 to allow use of the C++ latest features? No, there is no plan. It is, however, recognized, such motion may be put to the vote around 2020.

## References

* [C++ compiler support](http://en.cppreference.com/w/cpp/compiler_support)
