<?php 

# Run with:
# php -n -d enable_dl=On -d extension_dir=.. test.php

dl("geos.so");

require_once 'PHPUnit/Framework.php';


class test extends PHPUnit_Framework_TestCase
{
    public function testGEOSVersion()
    {
        $this->assertContains('-CAPI-', GEOSVersion());
    }

    public function testWKTReader__construct()
    {
        $reader = new GEOSWKTReader();
        $this->assertNotNull($reader);
    }

    public function testWKTReader_read()
    {
        $reader = new GEOSWKTReader();

        /* Good WKT */
        $geom = $reader->read('POINT(0 0)');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOINT(0 0 1, 2 3 4)');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOINT((0 0), (2 3))');
        $this->assertNotNull($geom);
        $geom = $reader->read('LINESTRING(0 0 1, 2 3 4)');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTILINESTRING((0 0 1, 2 3 4),
                                               (10 10 2, 3 4 5))');
        $this->assertNotNull($geom);
        $geom = $reader->read('POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOLYGON(
                                ((0 0, 1 0, 1 1, 0 1, 0 0)),
                                ((10 10, 10 14, 14 14, 14 10, 10 10),
                                    (11 11, 11 12, 12 12, 12 11, 11 11))
                               )');
        $this->assertNotNull($geom);
        $geom = $reader->read('GEOMETRYCOLLECTION(
                MULTIPOLYGON(
                 ((0 0, 1 0, 1 1, 0 1, 0 0)),
                 ((10 10, 10 14, 14 14, 14 10, 10 10),
                  (11 11, 11 12, 12 12, 12 11, 11 11))
                ),
                POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)),
                MULTILINESTRING((0 0, 2 3), (10 10, 3 4)),
                LINESTRING(0 0, 2 3),
                MULTIPOINT(0 0, 2 3),
                POINT(9 0)
        )');
        $this->assertNotNull($geom);

        /* BOGUS WKT */
        try {
            $reader->read("MULTIDOT(0 1 2 3)");
            $this->assertTrue(FALSE); # this is just to fail if we get here
        } catch (Exception $e) {
            $this->assertContains('ParseException', $e->getMessage());
        }
    }

    public function testWKTWriter__construct()
    {
        $writer = new GEOSWKTWriter();
        $this->assertNotNull($writer);
    }

    public function testWKTWriter_write()
    {
        $writer = new GEOSWKTWriter();
        $reader = new GEOSWKTReader();

        try {
            $writer->write(1);
            $this->assertTrue(FALSE); # this is just to fail if we get here
        } catch (Exception $e) {
            $this->assertContains('expects parameter 1', $e->getMessage());
        }

        $g = $reader->read('POINT(6 7)');

        $this->assertEquals('POINT (6.0000000000000000 7.0000000000000000)',
            $writer->write($g));
    }

    public function testWKTWriter_setTrim()
    {
        $writer = new GEOSWKTWriter();
        $reader = new GEOSWKTReader();

        $g = $reader->read('POINT(6 7)');
        $this->assertNotNull($g);

        $writer->setTrim(TRUE);
        $this->assertEquals('POINT (6 7)',
            $writer->write($g));

        $writer->setTrim(FALSE);
        $this->assertEquals('POINT (6.0000000000000000 7.0000000000000000)',
            $writer->write($g));

    }

    public function testWKT_roundTrip()
    {
        $r = new GEOSWKTReader();
        $w = new GEOSWKTWriter();
        $w->setTrim(TRUE);

        $in[] = 'POINT (0 0)';
        $in[] = 'MULTIPOINT (0 1, 2 3)';
        $in[] = 'LINESTRING (0 0, 2 3)';
        $in[] = 'MULTILINESTRING ((0 1, 2 3), (10 10, 3 4))';
        $in[] = 'POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))';
        $in[] = 'MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((10 10, 10 14, 14 14, 14 10, 10 10), (11 11, 11 12, 12 12, 12 11, 11 11)))';
        $in[] = 'GEOMETRYCOLLECTION (MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((10 10, 10 14, 14 14, 14 10, 10 10), (11 11, 11 12, 12 12, 12 11, 11 11))), POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0)), MULTILINESTRING ((0 0, 2 3), (10 10, 3 4)), LINESTRING (0 0, 2 3), MULTIPOINT (0 0, 2 3), POINT (9 0))';

        foreach ($in as $i) {
            $this->assertEquals($i, $w->write($r->read($i)));
        }

    }

    public function testWKTWriter_setRoundingPrecision()
    {
        $writer = new GEOSWKTWriter();
        $reader = new GEOSWKTReader();

        $g = $reader->read('POINT(6.123456 7.123456)');

        $this->assertEquals('POINT (6.1234560000000000 7.1234560000000000)',
            $writer->write($g));

        $writer->setRoundingPrecision(2);
        $this->assertEquals('POINT (6.12 7.12)', $writer->write($g));

        $writer->setRoundingPrecision(5); /* rounds */
        $this->assertEquals('POINT (6.12346 7.12346)', $writer->write($g));

        $writer->setRoundingPrecision(1);
        $this->assertEquals('POINT (6.1 7.1)', $writer->write($g));

        $writer->setRoundingPrecision(0);
        $this->assertEquals('POINT (6 7)', $writer->write($g));

    }

    public function testWKTWriter_setOutputDimension()
    {
        $reader = new GEOSWKTReader();
        $g3d = $reader->read('POINT(1 2 3)');
        $g2d = $reader->read('POINT(3 2)');

        $writer = new GEOSWKTWriter();
        $writer->setTrim(TRUE);

        # Only 2d by default
        $this->assertEquals('POINT (1 2)', $writer->write($g3d));

        # 3d if requested _and_ available
        $writer->setOutputDimension(3);
        $this->assertEquals('POINT Z (1 2 3)', $writer->write($g3d));
        $this->assertEquals('POINT (3 2)', $writer->write($g2d));

    }

    public function testWKTWriter_setOld3D()
    {
        $reader = new GEOSWKTReader();
        $g3d = $reader->read('POINT(1 2 3)');

        $writer = new GEOSWKTWriter();
        $writer->setTrim(TRUE);

        # New 3d WKT by default
        $writer->setOutputDimension(3);
        $this->assertEquals('POINT Z (1 2 3)', $writer->write($g3d));

        # Switch to old
        $writer->setOld3D(TRUE);
        $this->assertEquals('POINT (1 2 3)', $writer->write($g3d));

        # Old3d flag is not reset when changing dimensions
        $writer->setOutputDimension(2);
        $this->assertEquals('POINT (1 2)', $writer->write($g3d));
        $writer->setOutputDimension(3);
        $this->assertEquals('POINT (1 2 3)', $writer->write($g3d));

        # Likewise, dimensions spec is not reset when changing old3d flag
        $writer->setOld3D(FALSE);
        $this->assertEquals('POINT Z (1 2 3)', $writer->write($g3d));

    }

    public function testGeometry_project()
    {
        $reader = new GEOSWKTReader();

        $g = $reader->read('POINT(1 2)');
        $g2 = $reader->read('POINT(3 4)');

        /* The method only accept lineal geometries */
        try {
            $prj = $g->project($g2);
            $this->assertTrue(FALSE); # this is just to fail if we get here
        } catch (Exception $e) {
            $this->assertContains('lineal', $e->getMessage());
        }

        $g = $reader->read('LINESTRING(0 0, 10 0)');

        $g2 = $reader->read('POINT(0 0)');
        $prj = $g->project($g2);
        $this->assertEquals(0, $prj);

        $g2 = $reader->read('POINT(10 0)');
        $prj = $g->project($g2);
        $this->assertEquals(10, $prj);

        $g2 = $reader->read('POINT(5 0)');
        $prj = $g->project($g2);
        $this->assertEquals(5, $prj);

        $g = $reader->read('MULTILINESTRING((0 0, 10 0),(20 10, 20 30))');

        $g2 = $reader->read('POINT(20 0)');
        $prj = $g->project($g2);
        $this->assertEquals(10, $prj);

        $g2 = $reader->read('POINT(20 5)');
        $prj = $g->project($g2);
        $this->assertEquals(10, $prj);


    }
}
