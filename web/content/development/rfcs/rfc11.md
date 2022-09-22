---
title: "GEOS RFC 11 - GEOS Version and EOL Policy"
date: 2022-09-12T00:00:00-04:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
weight: 10
---

|         |                                 |
| :------ | ------------------------------- |
| RFC 11  | End-of-Life (EOL) Policy        |
| Author  | Regina Obe                      |
| Contact | lr@pcorp.us                     |
| Status  |  [Accepted](https://lists.osgeo.org/pipermail/geos-devel/2022-September/010784.html), September 20, 2022       |

This document describes the GEOS project policy for Versioning and EOLing GEOS minor releases.

# Versioning
The GEOS project generally produces a minor release for a version every 1-2 years.
Minor releases typically contain new features, as well as bug fixes and performance improvements.
Minor releases have version numbers in the form `major.minor` (e.g. `3.9`, `3.10`, `3.11`, etc).

Each minor release has a git branch named for the minor version (e.g. `3.11`).
The first release of a minor version has a micro version of `.0` and is git tagged.
(e.g. the first release of `3.11` was `3.11.0` and has git tag as `3.11.0`).

# Patch Policy
On an as needed basis, each supported minor version will have a new micro release (e.g. `3.10.2`).
These subsequent micro releases may have bug fixes, performance improvements, and security fixes in them
but will never offer new features.
It is the patched version for the parent minor release.
Micro releases are git tagged with the version number.

* The newer the minor, the more likely performance improvements will be added. Performance improvements that could destabilize a minor release (e.g. requiring too much code) or  change behavior will generally not be applied.
* Bug fixes that are not trivial changes of code or major security or crash issues, will likely not be applied to older releases.
* The latest stable (meaning the minor with highest number that is not main branch) will have all bug fixes and enhancements which are not new features.

# End-of-Life Policy

Minor versions marked EOL will not have further micro or tagged releases. We may apply fixes to a minor release branch if an entity is willing to fund such an effort, but the change will not be tagged.

The policy for making a minor version EOL is:

* If the first release of the minor version is more than 3-5 years old, it may become EOL.

When a minor release is EOL, it will either be removed from [Downloads](/usage/download/) or will have an **EOL** marker showing it is the last release for the minor version.
