---
title: "GEOS RFC 1 - Project Steering Committee"
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

This document describes how the PSC GEOS Project Steering Committee determines membership, and makes decisions on all aspects of the GEOS project - both technical and non-technical.

|          |                              |
| :------- | ---------------------------- |
| RFC 1    | Project Steering Committee   |
| Author   | Paul Ramsey                  |
| Contact  | pramsey@cleverelephant.ca    |
| Status   | Approved, April 10, 2008     |

## Summary

This document describes how the GEOS Project Steering Committee (PSC) determines membership, and makes decisions on all aspects of the GEOS project - both technical and non-technical.

Examples of PSC management responsibilities:

 * setting the overall development road map
 * developing technical standards and policies (e.g. coding standards, file naming conventions, etc...)
 * ensuring regular releases (major and maintenance) of GEOS software
 * reviewing RFC for technical enhancements to the software
 * project infrastructure (e.g. SVN, bug tracking, hosting options, etc...)
 * formalization of affiliation with external entities such as OSGeo
 * setting project priorities, especially with respect to project sponsorship
 * creation and oversight of specialized sub-committees (e.g. project infrastructure, training)

In brief the project team votes on proposals on [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel). Proposals are available for review for at least two days, and a single veto is sufficient delay progress though ultimately a majority of members can pass a proposal.

## Detailed Process

 1. Proposals are announced on the [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) mailing list for discussion and voting, by any interested party, not just committee members.
 1. Proposals need to be available for review for at least two business days before a final decision can be made.
 1. Respondents may vote "+1" to indicate support for the proposal and a willingness to support implementation.
 1. Respondents may vote "-1" to veto a proposal, but must provide clear reasoning and alternate approaches to resolving the problem within the two days.
 1. A vote of -0 indicates mild disagreement, but has no effect. A 0 indicates no opinion. A +0 indicate mild support, but has no effect.
 1. Anyone may comment on proposals on the list, but only members of the Project Steering Committee's votes will be counted.
 1. A proposal will be accepted if it receives +2 (including the author) and no vetoes (-1).
 1. If a proposal is vetoed, and it cannot be revised to satisfy all parties, then it can be resubmitted for an override vote in which a majority of all eligible voters indicating +1 is sufficient to pass it. Note that this is a majority of all committee members, not just those who actively vote.
 1. Upon completion of discussion and voting the author should announce whether they are proceeding (proposal accepted) or are withdrawing their proposal (vetoed).
 1. The Chair gets a vote.
 1. The Chair is responsible for keeping track of who is a member of the Project Steering Committee.
 1. Addition and removal of members from the committee, as well as selection of a Chair should be handled as a proposal to the committee.
 1. The Chair adjudicates in cases of disputes about voting.

## When is Vote Required?

 * Any change to committee membership (new members, removing inactive members)
 * Changes to project infrastructure (e.g. tool, location or substantive configuration)
 * Anything that could cause backward compatibility issues.
 * Adding substantial amounts of new code.
 * Changing inter-subsystem API or objects.
 * Issues of procedure.
 * When releases should take place.
 * Anything dealing with relationships with external entities such as OSGeo
 * Anything that might be controversial.

## Observations

 * The Chair is the ultimate adjudicator if things break down.
 * The absolute majority rule can be used to override an obstructionist veto, but it is intended that in normal circumstances voters need to be convinced to withdraw their veto. We are trying to reach consensus.
 * It is anticipated that separate "committees" will exist to manage conferences, documentation and web sites. That said, it is expected that the PSC will be the entity largely responsible for creating any such committees.

## Committee Membership

The PSC is made up of individuals consisting of technical contributors (e.g. developers) and prominent members of the GEOS user community.

### Adding Members

Any member of the [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) mailing list may nominate someone for committee membership at any time. Only existing PSC committee members may vote on new members. Nominees must receive a majority vote from existing members to be added to the PSC.

### Stepping Down

If for any reason a PSC member is not able to fully participate then they certainly are free to step down. If a member is not active (e.g. no voting, no IRC or email participation) for a period of two months then the committee reserves the right to seek nominations to fill that position.

Should that person become active again (hey, it happens) then they would certainly be welcome, but would require a nomination.

## Membership Responsibilities

### Guiding Development

Members should take an active role guiding the development of new features they feel passionate about. Once a change request has been accepted and given a green light to proceed does not mean the members are free of their obligation. PSC members voting "+1" for a change request are expected to stay engaged and ensure the change is implemented and documented in a way that is most beneficial to users. Note that this applies not only to change requests that affect code, but also those that affect the web site, technical infrastructure, policies and standards.

### IRC Meeting Attendance

PSC members are expected to participate in pre-scheduled IRC development meetings. If known in advance that a member cannot attend a meeting, the member should let the meeting organizer know via e-mail.

### Mailing List Participation

PSC members are expected to be active on both the [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) mailing lists, subject to open source mailing list etiquette. Non-developer members of the PSC are not expected to respond to coding level questions on the developer mailing list, however they are expected to provide their thoughts and opinions on user level requirements and compatibility issues when RFC discussions take place.

## Bootstrapping

**Note:** The following is for historical interest only. The current PSC is available [here]({{< ref "/project/psc" >}}).

Prior to announcing itself, the PSC must distribute this RFC to the GEOS community via [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel) for comment. Any and all substantive comments must be discussed (and hopefully, but not necessarily, addressed via [geos-devel](http://lists.osgeo.org/mailman/listinfo/geos-devel).

Initial members are:

 * Dale Lutz
 * Gary Crawford
 * Martin Davis
 * [Howard Butler](mailto:hobu.inc@gmail.com)
