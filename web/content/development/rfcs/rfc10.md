---
title: "GEOS RFC 10 - Move Project to GitHub"
date: 2021-10-29T00:00:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
weight: 10
---

|         |                                 |
| :------ | ------------------------------- |
| RFC 10  | Move Project to GitHub          |
| Author  | Paul Ramsey                     |
| Contact | pramsey@cleverelephant.ca       |
| Status  | [Accepted](https://lists.osgeo.org/pipermail/geos-devel/2021-November/010580.html), November 9, 2021             |

GitHub has been the largest source of 3rd party code contribution via pull-requests for some time now.

Moving to Github has the following components:

* Move the canonical (writeable) repository to GitHub
* Migrate the (current, useful) contents of the Trac wiki to the new web framework
* Deleting the migrated and out-of-date contents of the Trac wiki
* Switching the Trac tickets to read-only
* Web scraping the Trac ticket contents and placing in a geos-old-tickets repo

At that point:

* New code is pushed to GitHub
* New issues are filed at GitHub
* New documentation is committed to the repository

This should unlock:

* Easier path for new contributors to discover and assist with the project
* Easier collaboration with downstream projects
* Far easier story on "how to we manage the project" and "where the important things happen"
* Far less dependence on individual contributors for infrastructure work that only they can do


