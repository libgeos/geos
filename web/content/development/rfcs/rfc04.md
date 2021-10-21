---
title: "GEOS RFC 4 - Code Formatting Style"
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

This document proposes and describes desired code formatting style used across C/C++ source code in GEOS.

|         |                         |
| :------ | ------------------------ |
| RFC 4   | Code Formatting Style    |
| Author  | Mateusz Łoskot           |
| Contact | mateusz@loskot.net       |
| Status  | [Dropped](https://lists.osgeo.org/pipermail/geos-devel/2017-April/007752.html) (no agreement)  |

## Summary

The document proposes and describes desired default code formatting style guidelines for GEOS programming in C and C++ languages.

The goal of this document is to initiate process to reach an agreement for the default code formatting style.

## Motivation

There is a need to decide on format of GEOS source code and apply such globally consistent format to GEOS C/C++ codebase.

A uniform, codebase-wide formatting style makes reading and comprehending existing code easier, writing code focused on important aspects of new developments and more pleasant, removes burden during a patch or pull request code reviews and prevents [bikeshedding religious arguments](http://wiki.c2.com/?WhereDoTheBracesGo). Even in small projects, contributing developers discover the problems of working without an agreed upon code format.

The utility of such guidelines has been proven by many open source software projects.

The scope of the proposal is specifically limited to formatting style guidelines. It is not an intention to develop a general coding guide covering other aspects of writing software like naming, etc.

## Proposal

It is important to make effortless for developers to produce properly formatted code.

The proposal suggests to use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) version 3.8 or higher to define C++ code formatting rules for GEOS code.

The `clang-format` is a tool to automatically format C/C++ code, so that developers don't need to worry about style issues.
Unlike other tools which use own parsers, `clang-format` uses the Clang tokenizer and supports the same C++ source code as the Clang compiler.
This guarantees correct output is produced and offers unique features (eg. wrapping long lines whether of code, strings, arrays - something which AStyle has no way of doing).

The style settings are defined in `.clang-format` configuration file for our [style settings](https://clang.llvm.org/docs/ClangFormatStyleOptions.html).

The `clang-format` is straightforward to run and can support development workflow as standalone tool or as one of many editor integrations or other bespoke utilities (eg. `git cl format` [Chromium]).

No automation of code reformatting is proposed. It would be treating the symptomps, no cause: developers not following the code formatting standard.

Although no means to enforce the default formatting style are proposed, currently used CI services (eg. Travis CI) may be employed as a post-commit safety valve - a clang-format lint failure as a compile break (eg. [clang_format.py](https://github.com/mongodb/mongo/blob/master/buildscripts/clang_format.py) build script used by MongoDB). Alternatively, a gatekeeper may be installed in SVN/Git, rejecting commits with code not conforming to the code formatting style.

## Code Formatting Rules

What code formatting rules to use?

> "A mature engineers know that a standard is more important than which standard." ~ MongoDB

`clang-format` offers several defaults (eg. LLVM, Mozilla, Linux, Google C++ Style).

The proposal recommends to use one of the base styles, if necessary, fine-tuning as an easier way to get started than deciding on each option one by one.

The reasons are two-fold:

* make GEOS code unified with the wide spectrum of well-established C/C++ projects
* long arguments and religious wars prevention.

### `.clang-format`

Below is complete set of settings suggested, sufficient to maintain the clean code formatting style.

**NOTE:** It remains open for refinements, use different `BasedOnStyle` as base style, etc.

```
---
BasedOnStyle:   Mozilla
Language:       Cpp
Standard:       Cpp03
ColumnLimit:    80
IndentWidth:    4
TabWidth:       4
UseTab:         Never

BraceWrapping:
  AfterClass:      true
  AfterControlStatement: true
  AfterEnum:       true
  AfterFunction:   true
  AfterNamespace:  true
  AfterObjCDeclaration: true
  AfterStruct:     true
  AfterUnion:      true
  BeforeCatch:     true
  BeforeElse:      true
  IndentBraces:    false
BreakBeforeBinaryOperators: None
BreakBeforeBraces: Allman
BreakBeforeTernaryOperators: true
```

For brevity, the settings above are limited to the base style and points of customizations.

For actual implementation, full version of `.clang-format` should be generated using `clang-format -dump-config` option and the `BasedOnStyle:   Mozilla` setting commented with `#`.

### `.editorconfig`

[http://editorconfig.org/ EditorConfig] is currently in use and `.editorconfig` file is provided to automatically tell popular code editors about the basic style settings like indentation, whitespaces and end-of-line markers for distinguished types of plain text files.

The `.editorconfig` file will have to be updated to match the chosen `.clang-format` settings.

### EOL

`clang-format` does not enforce line endings.

The EOL marker is considered to be [http://lists.llvm.org/pipermail/cfe-commits/Week-of-Mon-20130930/090200.html a part of a file encoding decision] and not part of any coding style.

The EOL marker can be enforced as project-wide setting controlled with `.gitattributes` and `.editorconfig`.

However, it shall still be left as configurable setting in developer's environment of choice (eg. `git config`) independently from the project-wide setting.

### Big Reformat

What to do about the existing code?

The proposal recommends to just do one big reformat of the codebase.

While it may seem causing clutter in the repository log (eg. `svn blame`), if it occurs infrequently (eg. yearly) and is applied to the entire codebase at that time, it should not be very disruptive to the source code history. One way to cope with skewed history is to use `git blame -w` which ignores whitespace when comparing commits.

Partial application of the code formatting rules would create more work without delivering the full benefit [MongoDB] leading to codebase with different styles mixed.

#### Implementation

Branches to run the big reformat in are:

* `trunk`
* `branches/3.6`
* `branches/3.5`
* `branches/3.4`

## After Big Reformat

How to work against the natural entropy in a codebase:

* It is highly recommended to use `clang-format` integration while writing a code.
* Format changed code before committing or opening pull requests.
* If you have to commit change in code formatting, do it in separate commit. Avoid commits with a mixture of code and formatting changes.
  * There is downside of history clutter in repository, but this proposal states that a codebase with different styles across is even worse.

> "After all, every moment of time wasted on code formatting or discussion thereof is eliminated." ~ MongoDB

#### Implementation

Set up Travis CI "style safety valve" build dedicated to run clang-format lint based on the approach used in `​clang_format.py` script by MongoDB.

## Miscellaneous

Those who build GEOS with GCC 6+ may appreciate consistent code format style as it will help to avoid some dozens of the [new compiler warnings](https://developers.redhat.com/blog/2016/02/26/gcc-6-wmisleading-indentation-vs-goto-fail/):

```
src/geom/Polygon.cpp: In member function ‘virtual int geos::geom::Polygon::getCoordinateDimension() const’:
src/geom/Polygon.cpp:154:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
     if( shell != NULL )
     ^~
src/geom/Polygon.cpp:157:2: note: ...this statement, but the latter is misleadingly indented as if it is guarded by the ‘if’
  size_t nholes=holes->size();
  ^~~~~~
```

## References

* MongoDB Succeeding With ClangFormat:
  * https://engineering.mongodb.com/post/succeeding-with-clangformat-part-1-pitfalls-and-planning/
  * https://engineering.mongodb.com/post/succeeding-with-clangformat-part-2-the-big-reformat/
  * https://engineering.mongodb.com/post/succeeding-with-clangformat-part-3-persisting-the-change/
* [Chromium](https://chromium.googlesource.com/chromium/src/+/master/docs/clang_format.md) Using clang-format on Chromium C++ Code
* https://clangformat.com - `clang-format` interactive guide and builder
* https://zed0.co.uk/clang-format-configurator/
