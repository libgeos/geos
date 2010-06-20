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

    public function testConstants()
    {
        $this->assertEquals(1, GEOSBUF_CAP_ROUND);
        $this->assertEquals(2, GEOSBUF_CAP_FLAT);
        $this->assertEquals(3, GEOSBUF_CAP_SQUARE);
        $this->assertEquals(1, GEOSBUF_JOIN_ROUND);
        $this->assertEquals(2, GEOSBUF_JOIN_MITRE);
        $this->assertEquals(3, GEOSBUF_JOIN_BEVEL);
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
        $geom = $reader->read('POINT EMPTY');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOINT(0 0 1, 2 3 4)');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOINT((0 0), (2 3))');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOINT EMPTY');
        $this->assertNotNull($geom);
        $geom = $reader->read('LINESTRING(0 0 1, 2 3 4)');
        $this->assertNotNull($geom);
        $geom = $reader->read('LINESTRING EMPTY');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTILINESTRING((0 0 1, 2 3 4),
                                               (10 10 2, 3 4 5))');
        $this->assertNotNull($geom);
        $geom = $reader->read('POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))');
        $this->assertNotNull($geom);
        $geom = $reader->read('POLYGON EMPTY');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOLYGON(
                                ((0 0, 1 0, 1 1, 0 1, 0 0)),
                                ((10 10, 10 14, 14 14, 14 10, 10 10),
                                    (11 11, 11 12, 12 12, 12 11, 11 11))
                               )');
        $this->assertNotNull($geom);
        $geom = $reader->read('MULTIPOLYGON EMPTY');
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
        $geom = $reader->read('GEOMETRYCOLLECTION EMPTY');
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
        $in[] = 'POINT EMPTY';
        $in[] = 'MULTIPOINT (0 1, 2 3)';
        $in[] = 'MULTIPOINT EMPTY';
        $in[] = 'LINESTRING (0 0, 2 3)';
        $in[] = 'LINESTRING EMPTY';
        $in[] = 'MULTILINESTRING ((0 1, 2 3), (10 10, 3 4))';
        $in[] = 'MULTILINESTRING EMPTY';
        $in[] = 'POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))';
        $in[] = 'POLYGON EMPTY';
        $in[] = 'MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((10 10, 10 14, 14 14, 14 10, 10 10), (11 11, 11 12, 12 12, 12 11, 11 11)))';
        $in[] = 'MULTIPOLYGON EMPTY';
        $in[] = 'GEOMETRYCOLLECTION (MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((10 10, 10 14, 14 14, 14 10, 10 10), (11 11, 11 12, 12 12, 12 11, 11 11))), POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0)), MULTILINESTRING ((0 0, 2 3), (10 10, 3 4)), LINESTRING (0 0, 2 3), MULTIPOINT (0 0, 2 3), POINT (9 0))';
        $in[] = 'GEOMETRYCOLLECTION EMPTY';

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
        $prj = $g->project($g2, TRUE);
        $this->assertEquals(0, $prj);

        $g2 = $reader->read('POINT(10 0)');
        $prj = $g->project($g2);
        $this->assertEquals(10, $prj);
        $prj = $g->project($g2, TRUE);
        $this->assertEquals(1, $prj);

        $g2 = $reader->read('POINT(5 0)');
        $prj = $g->project($g2);
        $this->assertEquals(5, $prj);
        $prj = $g->project($g2, TRUE);
        $this->assertEquals(0.5, $prj);

        $g = $reader->read('MULTILINESTRING((0 0, 10 0),(20 10, 20 20))');

        $g2 = $reader->read('POINT(20 0)');
        $prj = $g->project($g2);
        $this->assertEquals(10, $prj);
        $prj = $g->project($g2, TRUE);
        $this->assertEquals(0.5, $prj);

        $g2 = $reader->read('POINT(20 5)');
        $prj = $g->project($g2);
        $this->assertEquals(10, $prj);
        $prj = $g->project($g2, TRUE);
        $this->assertEquals(0.5, $prj);


    }

    public function testGeometry_interpolate()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setTrim(TRUE);

        /* The method only accept LineString geometries */
        $g = $reader->read('POINT(1 2)');
        try {
            $prj = $g->interpolate(0);
            $this->assertTrue(FALSE); # this is just to fail if we get here
        } catch (Exception $e) {
            $this->assertContains('LineString', $e->getMessage());
        }

        $g = $reader->read('LINESTRING(0 0, 10 0)');

        $prj = $g->interpolate(0);
        $this->assertNotNull($prj);
        $this->assertEquals('POINT (0 0)', $writer->write($prj));
        $prj = $g->interpolate(0, TRUE);
        $this->assertNotNull($prj);
        $this->assertEquals('POINT (0 0)', $writer->write($prj));

        $prj = $g->interpolate(5);
        $this->assertNotNull($prj);
        $this->assertEquals('POINT (5 0)', $writer->write($prj));
        $prj = $g->interpolate(0.5, TRUE);
        $this->assertNotNull($prj);
        $this->assertEquals('POINT (5 0)', $writer->write($prj));

        /* return closest on longer distance */
        $prj = $g->interpolate(20);
        $this->assertNotNull($prj);
        $this->assertEquals('POINT (10 0)', $writer->write($prj));
        $prj = $g->interpolate(2, TRUE);
        $this->assertNotNull($prj);
        $this->assertEquals('POINT (10 0)', $writer->write($prj));
    
    }

    public function testGeometry_buffer()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('POINT(0 0)');
        $b = $g->buffer(0);
        $this->assertEquals('POLYGON EMPTY', $writer->write($b));

        $b = $g->buffer(10);
        $this->assertEquals(
'POLYGON ((10 0, 10 -2, 9 -4, 8 -6, 7 -7, 6 -8, 4 -9, 2 -10, 0 -10, -2 -10, -4 -9, -6 -8, -7 -7, -8 -6, -9 -4, -10 -2, -10 -0, -10 2, -9 4, -8 6, -7 7, -6 8, -4 9, -2 10, -0 10, 2 10, 4 9, 6 8, 7 7, 8 6, 9 4, 10 2, 10 0))'
            , $writer->write($b));

        # One segment per quadrant
        $b = $g->buffer(10, array('quad_segs' => 1));
        $this->assertEquals(
'POLYGON ((10 0, 0 -10, -10 -0, -0 10, 10 0))'
            , $writer->write($b));

        /* End cap styles */

        $g = $reader->read('LINESTRING(0 0, 100 0)');

        $b = $g->buffer(10, array(
            'quad_segs' => 1,
            'endcap' => GEOSBUF_CAP_ROUND
        ));
        $this->assertEquals(
'POLYGON ((100 10, 110 0, 100 -10, 0 -10, -10 0, 0 10, 100 10))'
            , $writer->write($b));

        $b = $g->buffer(10, array(
            'quad_segs' => 1,
            'endcap' => GEOSBUF_CAP_FLAT
        ));
        $this->assertEquals(
'POLYGON ((100 10, 100 -10, 0 -10, 0 10, 100 10))'
            , $writer->write($b));

        $b = $g->buffer(10, array(
            'quad_segs' => 1,
            'endcap' => GEOSBUF_CAP_SQUARE
        ));
        $this->assertEquals(
'POLYGON ((100 10, 110 10, 110 -10, 0 -10, -10 -10, -10 10, 100 10))'
            , $writer->write($b));

        /* Join styles */

        $g = $reader->read('LINESTRING(0 0, 100 0, 100 100)');

        $b = $g->buffer(10, array(
            'quad_segs' => 2,
            'join' => GEOSBUF_JOIN_ROUND
        ));
        $this->assertEquals(
'POLYGON ((90 10, 90 100, 93 107, 100 110, 107 107, 110 100, 110 0, 107 -7, 100 -10, 0 -10, -7 -7, -10 0, -7 7, 0 10, 90 10))'
            , $writer->write($b));

        $b = $g->buffer(10, array(
            'quad_segs' => 2,
            'join' => GEOSBUF_JOIN_BEVEL
        ));
        $this->assertEquals(
'POLYGON ((90 10, 90 100, 93 107, 100 110, 107 107, 110 100, 110 0, 100 -10, 0 -10, -7 -7, -10 0, -7 7, 0 10, 90 10))'
            , $writer->write($b));

        $b = $g->buffer(10, array(
            'quad_segs' => 2,
            'join' => GEOSBUF_JOIN_MITRE
        ));
        $this->assertEquals(
'POLYGON ((90 10, 90 100, 93 107, 100 110, 107 107, 110 100, 110 -10, 0 -10, -7 -7, -10 0, -7 7, 0 10, 90 10))'
            , $writer->write($b));

        $b = $g->buffer(10, array(
            'quad_segs' => 2,
            'join' => GEOSBUF_JOIN_MITRE,
            'mitre_limit' => 1.0
        ));
        $this->assertEquals(
'POLYGON ((90 10, 90 100, 93 107, 100 110, 107 107, 110 100, 109 -5, 105 -9, 0 -10, -7 -7, -10 0, -7 7, 0 10, 90 10))'
            , $writer->write($b));

        /* Check that elements of the passed style array are not
         * type-converted (buffer op will need to type-convert
         * internally)
         */
        $ary = array('a' => 1);

        $myStyle = array(
            'quad_segs' => "a string",
            'join' => "1",
            'endcap' => $ary,
            'mitre_limit' => 2 /* an int.. */
        );
        $this->assertEquals('string', gettype($myStyle['quad_segs']));
        $this->assertEquals('string', gettype($myStyle['join']));
        $this->assertEquals('array', gettype($myStyle['endcap']));
        $this->assertEquals('integer', gettype($myStyle['mitre_limit']));
        $b = $g->buffer(10, $myStyle);
        $this->assertEquals('string', gettype($myStyle['quad_segs']));
        $this->assertEquals('string', gettype($myStyle['join']));
        $this->assertEquals('array', gettype($myStyle['endcap']));
        $this->assertEquals('integer', gettype($myStyle['mitre_limit']));


    }

    public function testGeometry_envelope()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('POINT(0 0)');
        $b = $g->envelope();
        $this->assertEquals(
'POINT (0 0)'
            , $writer->write($b));

        $g = $reader->read('LINESTRING(0 0, 10 10)');
        $b = $g->envelope();
        $this->assertEquals(
'POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))'
            , $writer->write($b));

    }

    public function testGeometry_intersection()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        /* POINT - POINT */
        $g = $reader->read('POINT(0 0)');
        $g2 = $reader->read('POINT(0 0)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'POINT (0 0)'
            , $writer->write($gi));
        $g2 = $reader->read('POINT(1 0)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'GEOMETRYCOLLECTION EMPTY'
            , $writer->write($gi));

        /* POINT - LINE */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('POINT(5 0)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'POINT (5 0)'
            , $writer->write($gi));
        $g2 = $reader->read('POINT(12 0)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'GEOMETRYCOLLECTION EMPTY'
            , $writer->write($gi));

        /* LINE - LINE */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'POINT (5 0)'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(5 0, 20 0)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'LINESTRING (5 0, 10 0)'
            , $writer->write($gi));

        /* LINE - POLY */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'LINESTRING (5 0, 5 10)'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(10 0, 20 0)');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'POINT (10 0)'
            , $writer->write($gi));

        /* POLY - POLY */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('POLYGON((5 -5, 5 5, 15 5, 15 -5, 5 -5))');
        $gi = $g->intersection($g2);
        $this->assertEquals(
'POLYGON ((10 5, 10 0, 5 0, 5 5, 10 5))'
            , $writer->write($gi));
        $g2 = $reader->read('POLYGON((10 0, 20 0, 20 -5, 10 -5, 10 0))');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'POINT (10 0)'
            , $writer->write($gi));
        $g2 = $reader->read('POLYGON((8 0, 20 0, 20 -5, 10 -5, 8 0))');
        $gi = $g->intersection($g2);
        $this->assertEquals( 'LINESTRING (8 0, 10 0)'
            , $writer->write($gi));
    }

    public function testGeometry_convexHull()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('POINT(0 0)');
        $b = $g->convexHull();
        $this->assertEquals(
'POINT (0 0)'
            , $writer->write($b));

        $g = $reader->read('LINESTRING(0 0, 10 10)');
        $b = $g->convexHull();
        $this->assertEquals(
'LINESTRING (0 0, 10 10)'
            , $writer->write($b));

        $g = $reader->read('POLYGON((0 0, 0 10, 5 5, 10 10, 10 0, 0 0))');
        $b = $g->convexHull();
        $this->assertEquals(
'POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))'
            , $writer->write($b));

    }

    public function testGeometry_difference()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        /* POINT - POINT */
        $g = $reader->read('POINT(0 0)');
        $g2 = $reader->read('POINT(0 0)');
        $gi = $g->difference($g2);
        $this->assertEquals( 'GEOMETRYCOLLECTION EMPTY'
            , $writer->write($gi));
        $g2 = $reader->read('POINT(1 0)');
        $gi = $g->difference($g2);
        $this->assertEquals( 'POINT (0 0)'
            , $writer->write($gi));

        /* LINE - POINT */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('POINT(5 0)');
        $gi = $g->difference($g2);
        $this->assertEquals('LINESTRING (0 0, 10 0)'
            , $writer->write($gi));

        /* POINT - LINE */
        $g = $reader->read('POINT(5 0)');
        $g2 = $reader->read('LINESTRING(0 0, 10 0)');
        $gi = $g->difference($g2);
        $this->assertEquals('GEOMETRYCOLLECTION EMPTY'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(0 1, 10 1)');
        $gi = $g->difference($g2);
        $this->assertEquals( 'POINT (5 0)'
            , $writer->write($gi));

        /* LINE - LINE */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->difference($g2);
        $this->assertEquals( 'MULTILINESTRING ((0 0, 5 0), (5 0, 10 0))'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(5 0, 20 0)');
        $gi = $g->difference($g2);
        $this->assertEquals( 'LINESTRING (0 0, 5 0)'
            , $writer->write($gi));

        /* POLY - LINE */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->difference($g2);
        $this->assertEquals(
'POLYGON ((5 0, 0 0, 0 10, 5 10, 10 10, 10 0, 5 0))'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(10 0, 20 0)');
        $gi = $g->difference($g2);
        $this->assertEquals(
'POLYGON ((10 0, 0 0, 0 10, 10 10, 10 0))'
            , $writer->write($gi));

        /* POLY - POLY */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('POLYGON((5 -5, 5 5, 15 5, 15 -5, 5 -5))');
        $gi = $g->difference($g2);
        $this->assertEquals(
'POLYGON ((5 0, 0 0, 0 10, 10 10, 10 5, 5 5, 5 0))'
            , $writer->write($gi));
    }

    public function testGeometry_symdifference()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        /* POINT - POINT */
        $g = $reader->read('POINT(0 0)');
        $g2 = $reader->read('POINT(0 0)');
        $gi = $g->symDifference($g2);
        $this->assertEquals( 'GEOMETRYCOLLECTION EMPTY'
            , $writer->write($gi));
        $g2 = $reader->read('POINT(1 0)');
        $gi = $g->symDifference($g2);
        $this->assertEquals( 'MULTIPOINT (0 0, 1 0)'
            , $writer->write($gi));

        /* LINE - POINT */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('POINT(5 0)');
        $gi = $g->symDifference($g2);
        $this->assertEquals('LINESTRING (0 0, 10 0)'
            , $writer->write($gi));

        /* POINT - LINE */
        $g = $reader->read('POINT(5 0)');
        $g2 = $reader->read('LINESTRING(0 0, 10 0)');
        $gi = $g->symDifference($g2);
        $this->assertEquals( 'LINESTRING (0 0, 10 0)'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(0 1, 10 1)');
        $gi = $g->symDifference($g2);
        $this->assertEquals(
'GEOMETRYCOLLECTION (POINT (5 0), LINESTRING (0 1, 10 1))'
            , $writer->write($gi));

        /* LINE - LINE */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->symDifference($g2);
        $this->assertEquals(
'MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (5 -10, 5 0), (5 0, 5 10))'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(5 0, 20 0)');
        $gi = $g->symDifference($g2);
        $this->assertEquals(
'MULTILINESTRING ((0 0, 5 0), (10 0, 20 0))'
            , $writer->write($gi));

        /* POLY - LINE */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->symDifference($g2);
        $this->assertEquals(
'GEOMETRYCOLLECTION (LINESTRING (5 -10, 5 0), POLYGON ((5 0, 0 0, 0 10, 5 10, 10 10, 10 0, 5 0)))'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(10 0, 20 0)');
        $gi = $g->symDifference($g2);
        $this->assertEquals(
'GEOMETRYCOLLECTION (LINESTRING (10 0, 20 0), POLYGON ((10 0, 0 0, 0 10, 10 10, 10 0)))'
            , $writer->write($gi));

        /* POLY - POLY */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('POLYGON((5 -5, 5 5, 15 5, 15 -5, 5 -5))');
        $gi = $g->symDifference($g2);
        $this->assertEquals(
'MULTIPOLYGON (((5 0, 0 0, 0 10, 10 10, 10 5, 5 5, 5 0)), ((5 0, 10 0, 10 5, 15 5, 15 -5, 5 -5, 5 0)))'
            , $writer->write($gi));
    }

    public function testGeometry_boundary()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('POINT(0 0)');
        $b = $g->boundary();
        $this->assertEquals(
'GEOMETRYCOLLECTION EMPTY'
            , $writer->write($b));

        $g = $reader->read('LINESTRING(0 0, 10 10)');
        $b = $g->boundary();
        $this->assertEquals(
'MULTIPOINT (0 0, 10 10)'
            , $writer->write($b));

        $g = $reader->read(
'POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),( 5 5, 5 6, 6 6, 6 5, 5 5))');
        $b = $g->boundary();
        $this->assertEquals(
'MULTILINESTRING ((0 0, 10 0, 10 10, 0 10, 0 0), (5 5, 5 6, 6 6, 6 5, 5 5))'
            , $writer->write($b));

    }

    public function testGeometry_union()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        /* POINT - POINT */
        $g = $reader->read('POINT(0 0)');
        $g2 = $reader->read('POINT(0 0)');
        $gi = $g->union($g2);
        $this->assertEquals( 'POINT (0 0)'
            , $writer->write($gi));
        $g2 = $reader->read('POINT(1 0)');
        $gi = $g->union($g2);
        $this->assertEquals( 'MULTIPOINT (0 0, 1 0)'
            , $writer->write($gi));

        /* LINE - POINT */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('POINT(5 0)');
        $gi = $g->union($g2);
        $this->assertEquals('LINESTRING (0 0, 10 0)'
            , $writer->write($gi));

        /* POINT - LINE */
        $g = $reader->read('POINT(5 0)');
        $g2 = $reader->read('LINESTRING(0 0, 10 0)');
        $gi = $g->union($g2);
        $this->assertEquals( 'LINESTRING (0 0, 10 0)'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(0 1, 10 1)');
        $gi = $g->union($g2);
        $this->assertEquals(
'GEOMETRYCOLLECTION (POINT (5 0), LINESTRING (0 1, 10 1))'
            , $writer->write($gi));

        /* LINE - LINE */
        $g = $reader->read('LINESTRING(0 0, 10 0)');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->union($g2);
        $this->assertEquals(
'MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (5 -10, 5 0), (5 0, 5 10))'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(5 0, 20 0)');
        $gi = $g->union($g2);
        $this->assertEquals(
'MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (10 0, 20 0))'
            , $writer->write($gi));

        /* POLY - LINE */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('LINESTRING(5 -10, 5 10)');
        $gi = $g->union($g2);
        $this->assertEquals(
'GEOMETRYCOLLECTION (LINESTRING (5 -10, 5 0), POLYGON ((5 0, 0 0, 0 10, 5 10, 10 10, 10 0, 5 0)))'
            , $writer->write($gi));
        $g2 = $reader->read('LINESTRING(10 0, 20 0)');
        $gi = $g->union($g2);
        $this->assertEquals(
'GEOMETRYCOLLECTION (LINESTRING (10 0, 20 0), POLYGON ((10 0, 0 0, 0 10, 10 10, 10 0)))'
            , $writer->write($gi));

        /* POLY - POLY */
        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('POLYGON((5 -5, 5 5, 15 5, 15 -5, 5 -5))');
        $gi = $g->union($g2);
        $this->assertEquals(
'POLYGON ((5 0, 0 0, 0 10, 10 10, 10 5, 15 5, 15 -5, 5 -5, 5 0))'
            , $writer->write($gi));
    }

    public function testGeometry_unionCascaded()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('MULTIPOLYGON(
                 ((0 0, 1 0, 1 1, 0 1, 0 0)),
                 ((10 10, 10 14, 14 14, 14 10, 10 10),
                  (11 11, 11 12, 12 12, 12 11, 11 11)),
                 ((0 0, 11 0, 11 11, 0 11, 0 0))
                ))');

        $gu = $g->union();
        $this->assertEquals(
'POLYGON ((1 0, 0 0, 0 1, 0 11, 10 11, 10 14, 14 14, 14 10, 11 10, 11 0, 1 0), (11 11, 12 11, 12 12, 11 12, 11 11))'
            , $writer->write($gu));

    }

    public function testGeometry_pointOnSurface()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('POINT(0 0)');
        $b = $g->pointOnSurface();
        $this->assertEquals(
'POINT (0 0)'
            , $writer->write($b));

        $g = $reader->read('LINESTRING(0 0, 5 5, 10 10)');
        $b = $g->pointOnSurface();
        $this->assertEquals(
'POINT (5 5)'
            , $writer->write($b));

        $g = $reader->read('POLYGON((0 0, 0 10, 5 5, 10 10, 10 0, 0 0))');
        $b = $g->pointOnSurface();
        $this->assertEquals(
'POINT (2 5)'
            , $writer->write($b));
    }

    public function testGeometry_centroid()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('POINT(0 0)');
        $b = $g->centroid();
        $this->assertEquals(
'POINT (0 0)'
            , $writer->write($b));

        $g = $reader->read('LINESTRING(0 0, 10 10)');
        $b = $g->centroid();
        $this->assertEquals(
'POINT (5 5)'
            , $writer->write($b));

        $g = $reader->read('POLYGON((0 0, 0 10, 5 5, 10 10, 10 0, 0 0))');
        $b = $g->centroid();
        $this->assertEquals(
'POINT (5 4)'
            , $writer->write($b));
    }

    public function testGeometry_relate()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('POINT(0 0)');
        $g2 = $reader->read('POINT(0 0)');
        $ret = $g->relate($g2);
        $this->assertEquals('0FFFFFFF2', $ret);
        $ret = $g->relate($g2, '0FFFFFFF2');
        $this->assertEquals(TRUE, $ret);
        $ret = $g->relate($g2, '0*******T');
        $this->assertEquals(TRUE, $ret);
        $ret = $g->relate($g2, '0*******1');
        $this->assertEquals(FALSE, $ret);

        $g = $reader->read('POINT(0 0)');
        $g2 = $reader->read('POINT(1 0)');
        $ret = $g->relate($g2);
        $this->assertEquals('FF0FFF0F2', $ret);
        $ret = $g->relate($g2, 'FF0FFF0F2');
        $this->assertEquals(TRUE, $ret);
        $ret = $g->relate($g2, 'F*******2');
        $this->assertEquals(TRUE, $ret);
        $ret = $g->relate($g2, 'T*******2');
        $this->assertEquals(FALSE, $ret);

        $g = $reader->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $g2 = $reader->read('POINT(1 0)');
        $ret = $g->relate($g2);
        $this->assertEquals('FF20F1FF2', $ret);
        $ret = $g->relate($g2, 'FF20F1FF2');
        $this->assertEquals(TRUE, $ret);
        $ret = $g->relate($g2, 'F****T**T');
        $this->assertEquals(TRUE, $ret);
        $ret = $g->relate($g2, 'T*******2');
        $this->assertEquals(FALSE, $ret);

    }

    public function testGeometry_polygonize()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('GEOMETRYCOLLECTION(
            LINESTRING(0 0, 10 10),
            LINESTRING(185 221, 100 100),
            LINESTRING(185 221, 88 275, 180 316),
            LINESTRING(185 221, 292 281, 180 316),
            LINESTRING(189 98, 83 187, 185 221),
            LINESTRING(189 98, 325 168, 185 221))
            )');

        $g2 = $reader->read('POINT(0 0)');
        $g = $g->union($g2); /* Make sure linestrings are noded */

        $ret = GEOSPolygonize($g);

        /*
         * NOTE: the following expected results are suspicious
         *       due to the duplicated dangle and lack of a cut edge
         */

        //var_dump($ret);

        $this->assertEquals('array', gettype($ret));
        $this->assertEquals('array', gettype($ret['rings']));
        $this->assertEquals('array', gettype($ret['cut_edges']));
        $this->assertEquals('array', gettype($ret['dangles']));
        $this->assertEquals('array', gettype($ret['invalid_rings']));

        $this->assertEquals(3, count($ret['rings']));
        $this->assertEquals(
'POLYGON ((185 221, 132 146, 83 187, 185 221))'
            , $writer->write($ret['rings'][0]));
        $this->assertEquals(
'POLYGON ((132 146, 185 221, 325 168, 189 98, 132 146))'
            , $writer->write($ret['rings'][1]));
        $this->assertEquals(
'POLYGON ((185 221, 88 275, 180 316, 292 281, 185 221))'
            , $writer->write($ret['rings'][2]));

        $this->assertEquals(0, count($ret['cut_edges']));

        $this->assertEquals(3, count($ret['dangles']));
        $this->assertEquals(
'LINESTRING (132 146, 100 100)'
            , $writer->write($ret['dangles'][0]));
        $this->assertEquals(
'LINESTRING (0 0, 10 10)'
            , $writer->write($ret['dangles'][1]));
        $this->assertEquals(
'LINESTRING (0 0, 10 10)'
            , $writer->write($ret['dangles'][2]));

        $this->assertEquals(0, count($ret['invalid_rings']));

    }

    public function testGeometry_lineMerge()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('MULTILINESTRING(
            (0 0, 10 10),
            (10 10, 10 0),
            (5 0, 10 0),
            (5 -5, 5 0)
            )');

        $ret = GEOSLineMerge($g);

        $this->assertEquals('array', gettype($ret));
        $this->assertEquals('1', count($ret));

        $this->assertEquals(
'LINESTRING (0 0, 10 10, 10 0, 5 0, 5 -5)'
            , $writer->write($ret[0]));

    }

    public function testGeometry_simplify()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read('LINESTRING(0 0, 3 4, 5 10, 10 0, 10 9, 5 11, 0 9)');
        $gs = $g->simplify(2);
        $this->assertEquals( 'LINESTRING (0 0, 5 10, 10 0, 10 9, 0 9)'
            , $writer->write($gs));
        $gs = $g->simplify(2, TRUE);
        $this->assertEquals( 'LINESTRING (0 0, 5 10, 10 0, 10 9, 5 11, 0 9)'
            , $writer->write($gs));
    }

    public function testGeometry_extractUniquePoints()
    {
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        $writer->setRoundingPrecision(0);

        $g = $reader->read(
    'GEOMETRYCOLLECTION (
        MULTIPOLYGON (
            ((0 0, 1 0, 1 1, 0 1, 0 0)),
            ((10 10, 10 14, 14 14, 14 10, 10 10),
                (11 11, 11 12, 12 12, 12 11, 11 11))
        ),
        POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0)),
        MULTILINESTRING ((0 0, 2 3), (10 10, 3 4)),
        LINESTRING (0 0, 2 3),
        MULTIPOINT (0 0, 2 3),
        POINT (9 0),
        POINT(1 0)),
        LINESTRING EMPTY
');

        $gs = $g->extractUniquePoints();
        if ( ! $gs ) RETURN_NULL(); /* should get an exception before */

        $this->assertEquals( 
'MULTIPOINT (0 0, 1 0, 1 1, 0 1, 10 10, 10 14, 14 14, 14 10, 11 11, 11 12, 12 12, 12 11, 2 3, 3 4, 9 0)'
            , $writer->write($gs));
    }
}
