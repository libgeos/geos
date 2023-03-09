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

This document describes the GEOS project policy for versioning and EOLing GEOS minor versions.

# Versioning
The GEOS project generally produces a new minor version every 1-2 years.
Minor versions typically contain new features, as well as bug fixes and performance improvements.
Minor versions have version numbers in the form `major.minor` (e.g. `3.9`, `3.10`, `3.11`, etc).

Each minor version has a named git branch (e.g. branch `3.11`).
The first release of a minor version has a micro version of `x.x.0` and is git tagged.
For example, the first release of version `3.11` was `3.11.0` and has git tag `3.11.0`.

# Patch Policy
On an as-needed basis, each supported minor version will have additional micro releases (e.g. `3.10.1`,`3.10.2`).
These subsequent micro releases may have bug fixes, performance improvements, and security fixes
but will not include new features.
They are patch releases for the parent minor version.
Micro releases are git tagged with the version number (e.g. tag `3.10.1`).

* The newer the minor version, the more likely performance improvements will be added. Performance improvements that could destabilize a release (e.g. requiring too much code) or change behavior will generally not be applied.
* Bug fixes that are not trivial changes of code or major security or crash issues, will likely not be applied to older versions.
* The latest stable release (meaning the minor release with highest number that is not the main branch) will have all bug fixes and enhancements which are not new features.

# End-of-Life Policy

Minor versions marked EOL will not have further releases. We may apply fixes to a minor branch if an entity is willing to fund such an effort, but the change will not be tagged.

The policy for making a minor version EOL is:

* If the first release of the minor version is more than 3-5 years old, it may become EOL.

When a minor version is EOL, it will either be removed from [Downloads](/usage/download/) or will have an **EOL** marker showing it is the last release for the minor version.
