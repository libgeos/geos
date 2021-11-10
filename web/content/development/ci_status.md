---
title: "CI Status"
date: 2021-10-04T14:21:00-07:00
draft: false
---

| Branch | GitHub | Debbie | Winnie | Dronie | GitLab | Bessie | Bessie32 |
| ------ | ------ | ------ | ------ | ------ | ------ | -------- | ------ |
| **main** | [![GitHub](https://github.com/libgeos/geos/workflows/CI/badge.svg?branch=main)](https://github.com/libgeos/geos/actions?query=workflow%3ACI+branch%3Amain) | [![Debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Master)](https://debbie.postgis.net/view/GEOS/job/GEOS_Master/) | [![Winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Master/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Master/) | [![Dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?ref=refs/heads/main)](https://dronie.osgeo.org/geos/geos?ref=refs/heads/main) | [![GitLab](https://gitlab.com/geos/libgeos/badges/main/pipeline.svg)](https://gitlab.com/geos/libgeos/commits/main) | [![Bessie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie&BRANCH=main)](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie) | [![Bessie32](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie32&BRANCH=main)](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie32) |
| **3.10** | [![GitHub](https://github.com/libgeos/geos/workflows/CI/badge.svg?branch=3.10)](https://github.com/libgeos/geos/actions?query=branch%3A3.10) | [![Debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Branch_3.10)](https://debbie.postgis.net/view/job/GEOS_Branch_3.10/) | [![Winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.10/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.10/) | [![Dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?ref=refs/heads/3.10)](https://dronie.osgeo.org/geos/geos?refs/heads/3.10) | [![GitLab](https://gitlab.com/geos/libgeos/badges/3.10/pipeline.svg)](https://gitlab.com/geos/libgeos/commits/3.10) |  |  |
| **3.9** | [![GitHub](https://github.com/libgeos/geos/workflows/CI/badge.svg?branch=3.9)](https://github.com/libgeos/geos/actions?query=branch%3A3.9)  | [![Debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Branch_3.9)](https://debbie.postgis.net/view/job/GEOS_Branch_3.9/) | [![Winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.9/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.9/) | [![Dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?ref=refs/heads/3.9)](https://dronie.osgeo.org/geos/geos?refs/heads/3.9) | [![GitLab](https://gitlab.com/geos/libgeos/badges/3.9/pipeline.svg)](https://gitlab.com/geos/libgeos/commits/3.9) |  | |
| **3.8** | [![GitHub](https://github.com/libgeos/geos/workflows/CI/badge.svg?branch=3.8)](https://github.com/libgeos/geos/actions?query=branch%3A3.8)  | [![Debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Branch_3.8)](https://debbie.postgis.net/view/job/GEOS_Branch_3.8/) | [![Winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.8/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.8/) | [![Dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?ref=refs/heads/3.8)](https://dronie.osgeo.org/geos/geos?refs/heads/3.8) | [![GitLab](https://gitlab.com/geos/libgeos/badges/3.8/pipeline.svg)](https://gitlab.com/geos/libgeos/commits/3.8) |  | |
| **3.7** | [![GitHub](https://github.com/libgeos/geos/workflows/CI/badge.svg?branch=3.7)](https://github.com/libgeos/geos/actions?query=branch%3A3.7)  | [![Debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Branch_3.7)](https://debbie.postgis.net/view/job/GEOS_Branch_3.7/) | [![Winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.7/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.7/) | [![Dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?ref=refs/heads/3.7)](https://dronie.osgeo.org/geos/geos?refs/heads/3.7) | [![GitLab](https://gitlab.com/geos/libgeos/badges/3.7/pipeline.svg)](https://gitlab.com/geos/libgeos/commits/3.7) |  |  |


## Runners

* **Debbie** - Debian 8.2, GNU/Linux 64bit, GCC Debian 8.2.0-3, automake
* **Winnie** - Windows Mingw64, 32bit GCC 8.1.0, 64bit GCC 8.1.0, MSys CMake
* **Dronie** - Alpine Linux 3.4, 64bit, GCC 5.3.0, automake
* **GitLab** - Debian 8.2, GNU/Linux 64bit, gcc, automake
* **Bessie** - FreeBSD 12.2, 64-bit clang 6.0, 64bit gcc, CMake (3.11) >=3.8, autotools < 3.8
* **Bessie32** - FreeBSD 12.2, 32-bit clang 6.0, 64bit gcc, CMake (3.11) >=3.8, autotools < 3.8
* **GitHub** - Ubuntu various versions, Windows various, CMake
