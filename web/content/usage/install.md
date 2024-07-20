---
title: "Install Packages"
date: 2021-10-04T13:44:59-07:00
draft: false
---

{{< toc >}}

### Red Hat

There is a GEOS package in the EPEL (Extra Packages for Enterprise Linux) repository.

```bash
# Add the EPEL repository
yum -y install epel-release

# Install the GEOS runtime and development packages
rpm -Uvh geos geos-devel

```

### Ubuntu

The [Ubuntu GIS](https://wiki.ubuntu.com/UbuntuGIS) project maintains a collection of repositories with builds of common open source geospatial projects, including GEOS.

```bash
# Add the Ubuntu GIS PPA
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:ubuntugis/ppa

# Install the packages
sudo apt-get install geos
```

### Debian

The [Debian GIS](https://wiki.debian.org/DebianGis) project maintains [GEOS packages](https://tracker.debian.org/pkg/geos) and pushes them into the appropriate Debian repositories.

```bash
sudo apt-get install geos
```

### Amazon Linux

Amazon Linux is based on RH7, and can read from the EPEL repository. To enable using Amazon tools, use the `amazon-linux-extras` utility.

```bash
sudo yum install -y amazon-linux-extras
sudo amazon-linux-extras enable epel
sudo yum search geos
sudo yum install geos geos-devel
```

### Homebrew

For MacOS, GEOS can be installed using the [Homebrew](https://brew.sh/) package repository, which downloads source packages and builds them in place using a recipe to ensure all packages integrate with each other nicely.

First [install Homebrew](https://brew.sh/). Then:

```bash
brew install geos
```

### Macports

For MacOS, GEOS can be installed using the [MacPorts](https://www.macports.org/) package repository, which downloads source packages and builds them in place using a recipe to ensure all packages integrate with each other nicely.

First [install MacPorts](https://www.macports.org/install.php). Then:

```bash
port install geos
```
