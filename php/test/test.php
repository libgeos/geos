<?php 

# Run with:
# php -n -d enable_dl=On -d extension_dir=.. test.php

dl("geos.so");

echo "\n";
echo "Geos version is: " . GEOSVersion() . "\n";
echo "\n";

$reader = new GEOSWKTReader();
$writer = new GEOSWKTWriter();


/* BOGUS WKT */
try {
    $geom = $reader->read("ciao");
} catch (Exception $e) {
    echo 'EXPECTED [WKTReader::read("ciao")]: ' . $e->getMessage() . "\n";
}

$geom = $reader->read("POINT(0 0)");
$num = $geom->numGeometries();
echo 'Geom has ' . $num . " geometries, WKT follows:\n";
echo "[DEFAULT UNTRIMMED]: ".$writer->write($geom) . "\n";
$writer->setTrim(TRUE);
echo "[TRIMMED]: ".$writer->write($geom) . "\n";
$writer->setTrim(FALSE);
echo "[UNTRIMMED]: ".$writer->write($geom) . "\n";

$writer->setTrim(TRUE);

echo "We'll do trimmed WKT from now on";

$geom = $reader->read("MULTIPOINT(0 0, 1 1)");
$num = $geom->numGeometries();
echo 'Geom has ' . $num . " geometries. WKT follows:\n";
echo $writer->write($geom) . "\n";

echo "\n";
echo "\n";

