---
title: "GEOS RFC 2 - Committer Guidelines "
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

This document describes the technical and legal responsibilities of [wiki:PSC GEOS committers].

|          |                                        |
| :------- | -------------------------------------- |
| RFC 2    | Committer Guidelines                   |
| Author   | Paul Ramsey, Regina Obe                |
| Contact  | pramsey@cleverelephant.ca, lr@pcorp.us |
| Status   | Draft                                  |

## Summary

This document describes the technical and legal responsibilities of [wiki:PSC GEOS committers].

## Election to GIT Commit Access

Permission for GIT commit access shall be provided to new developers only if accepted by the [wiki:PSC GEOS Project Steering Committee]. A proposal should be written to the PSC for new committers and voted on normally. It is not necessary to write an [RFC](http://en.wikipedia.org/wiki/Request_for_Comments) document for these votes, a proposal to [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) is sufficient.

Every developer position in the project is represented by an individual, and commit access will be granted to individuals only, group accounts are not permitted.

Removal of GIT commit access should be handled by the same process.

The new committer should have demonstrated commitment to GEOS and knowledge of the GEOS source code and processes to the committee's satisfaction, usually by reporting bugs, submitting patches, and/or actively participating in the GEOS mailing list(s).

The new committer should also be prepared to support any new feature or changes that he/she commits to the GEOS source tree in future releases, or to find someone to which to delegate responsibility for them if he/she stops being available to support the portions of code that he/she is responsible for.

All committers should also be a member of the [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) mailing list so they can stay informed on policies, technical developments and release preparation.

**Before being approved, new committers must send an email to the [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) mailing list confirming that they have read, understand, and agree to follow the terms of this document.**

## Committer Tracking

A list of all project committers will be kept in the GEOS source tree in `AUTHORS` , listing for each committer:

 * Userid: the id that will appear in the SVN logs for this person.
 * Full name: the users actual name.
 * Email address: A current email address at which the committer can be reached. It may be altered in normal ways to make it harder to auto-harvest.
 * A brief indication of areas of responsibility.

## GIT Administrator

One member of the Project Steering Committee will be designated the [GEOS GIT](https://git.osgeo.org/gitea/geos/) repository Administrator. That person will be responsible for giving GIT commit access to folks, updating the committers list, and other GIT related management.

## Legal Responsibilities

Committers are the front line gatekeepers to keep the code base clear of improperly contributed code. It is important to the GEOS users, developers and the OSGeo foundation to avoid contributing any code to the project without it being clearly licensed under the project license.

Every GEOS code contribution should meet the following tests:

 1. The person or organization providing code understands that the code will be released under the [LGPL license](http://www.opensource.org/licenses/lgpl-2.1.php).

 1. The person or organization providing the code has the legal right to contribute the code.

 1. If the contribution was developed on behalf of an employer (on work time, as part of a work project, etc) then it is important that employees have permission from a supervisor or manager to contribute the code.

 1. The code should be developed by the contributor, or the code should be from a source which can be rightfully contributed such as from the public domain, or from an open source project under a compatible license.

All unusual situations need to be discussed, preferably on the public [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) mailing list, and/or documented.

Committers should adhere to the following guidelines, and may be personally legally liable for improperly contributing code to the source repository:

 * Make sure the contributor (and possibly employer) is aware of the contribution terms.

 * Code coming from a source other than the contributor (such as adapted from another project) should be clearly marked as to the original source, copyright holders, license terms and so forth. This information can be in the file headers, but should also be added to the project licensing file if not exactly matching normal project licensing (see [source:trunk/COPYING COPYING] file).

 * Existing copyright headers and license text should never be stripped from a file. If a copyright holder wishes to give up copyright they must do so in writing to the [OSGeo Foundation](http://www.osgeo.org) before copyright messages are removed. If license terms are changed it has to be by agreement (written in email is OK) of the copyright holders.

 * When substantial contributions are added to a file (such as substantial patches) the author/contributor should be added to the list of copyright holders for the file.

 * If there is uncertainty about whether a change is proper to contribute to the code base, please seek more information from the project steering committee, or the foundation legal counsel.

## Technical Responsibilities

The following are considered good SVN commit practices for the GEOS project.

 * Use meaningful descriptions for commit log entries.

 * Add a bug reference like "references #1232" or "closes #1232" at the end of the commit log entries when committing changes related to an existing [wiki:TracTickets Ticket] in the GEOS Trac database, so it's properly linked on the Trac pages (see [wiki:TracLinks])

 * Changes should not be committed in stable branches without a corresponding [wiki:TracTickets Ticket] number. Any change worth pushing into the stable version is worth a [wiki:TracTickets Ticket] entry.

 * Never commit new features to a stable branch without permission of the [wiki:PSC PSC] or release manager. Normally only fixes should go into stable branches. New features go in the main development trunk.

 * Only bug fixes should be committed to the code during pre-release code freeze, without permission from the [wiki:PSC PSC] or release manager.

 * Significant changes to the main development version should be discussed on the [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) list before you make them, and larger changes will require a RFC approved by the [wiki:PSC PSC].

 * All source code in GIT should be in Unix [text format](http://en.wikipedia.org/wiki/Newline) (LF) as opposed to DOS (CR+LF) or Mac OS text mode (CR).

 * When committing new features or significant changes to existing source code, the committer should take reasonable measures to insure that the source code continues to build and work on the most commonly supported platforms (currently Linux, Windows and Mac OS), either by testing on those platforms directly, running [wiki:Buildbot] tests, or by getting help from other developers working on those platforms. If new files or library dependencies are added, then the configure.in, Makefile.in, Makefile.vc and related documentations should be kept up to date.

 * Every commit introducing new feature **should*() be covered with corresponding test case included to the GEOS set of unit tests.


