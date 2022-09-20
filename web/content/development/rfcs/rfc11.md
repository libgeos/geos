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

This document describes the GEOS project's policy for Versiong and EOLing GEOS minor releases.

# Versioning
The GEOS project generally releases a new minor release every 1-2 years.
Each minor release has a git repo dedicated branch for it named after the minor version.

These minor releases contain new features and have version numbers starting with
3.9, 3.10, 3.11, and so on with an associated git repo branch with the same name.
The first release of a minor version will have a micro of .0 and be tagged.
e.g. The first release of 3.11 was 3.11.0 and in git is tagged as 3.11.0.

On an as needed basis, each supported minor version will have a new micro release that is then tagged and is considered to be a patched version for that minor.

# Patch Policy
These subsequent micro releases may have bug fixes, performance improvements, and security fixes in them
but will never offer new features.

* The newer the minor, the more likely performance improvements will be added. Performance improvements that could destabilize a minor release (e.g. requiring too much code) or possibly change behavior will generally not be applied.
* Bug fixes that are not trivial changes of code or major security or major crash issues, will likely not be applied to older releases.
* The latest stable (meaning the minor with highest number that is not main branch) will have all bug fixes and enhancements deemed not new features.
* Minor versions marked EOL will never have further micro or tagged releases. We may on occasion apply fixes to that minor git branch if an entity is willing to fund such an effort, but we will never tag such a change.

# End-of-Life Policy
Our policy for EOL'ing a particular minor version is as follows:

* If the first micro of a minor was released more than 3-4 years ago, we may consider EOL'ing it.
* We will likely EOL any minor version that has reached the 5 year mark.
* Once a minor release has reached EOL, it will either no longer be listed on the [download](/usage/download/) or will have an **EOL** marker next to it signaling this is the last micro for this minor version.


