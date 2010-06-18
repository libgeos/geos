<?php

# 
# This file contains code attempting to segfault the extension
#
# Run with:
# php -n -d enable_dl=On -d extension_dir=.. test.php
#

dl("geos.so");

class Point extends GEOSWKTReader {
    public function __construct()
    {
        parent::__construct();
    }
    public function test() {    
        return GEOSGeometry::numGeometries();
    }
};

$p = new Point();
$p->test();


