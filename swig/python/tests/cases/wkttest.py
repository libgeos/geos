# $Id$
# ===========================================================================
# Author:   Sean Gillies, sgillies@frii.com
# ===========================================================================
# Copyright (c) 2004, Sean Gillies
# ===========================================================================
#
# Execute this module as a script from PyGEOS
#
#     python tests/cases/wkttest.py -v
#
# ===========================================================================

import os, sys
import unittest

# the testing module helps us import the pre-installed geos
from testing import geos

# ===========================================================================

class WKTWriterTestCase(unittest.TestCase):

    def testWritePoint(self):
        """dump a point to WKT"""
        gf = geos.GeometryFactory()
        x = 492187.0
        y = 4493034.0
        z = 1500.0
        c = geos.Coordinate(x, y, z)  # Me!
        p = gf.createPoint(c)
        writer = geos.WKTWriter()
        wkt = writer.write(p)
        assert wkt == \
               'POINT (492187.0000000000000000 4493034.0000000000000000)', \
               wkt

class WKTReaderTestCase(unittest.TestCase):

    def testReadPoint(self):
        """read a point from WKT"""
        gf = geos.GeometryFactory()
        reader = geos.WKTReader(gf)
        wkt = 'POINT (492187.0 4493034.0)'
        geometry = reader.read(wkt)
        c = geometry.getCoordinate()
        assert c.x == 492187.0, c.x
        assert c.y == 4493034.0, c.y

    def testReadInvalidPoint(self):
        """expect a RuntimeError when reading invalid WKT string"""
        gf = geos.GeometryFactory()
        reader = geos.WKTReader(gf)
        wkt = 'POINT (492187.0, 4493034.0)'
        self.assertRaises(RuntimeError, reader.read, wkt)
        

# ===========================================================================
# Run the tests outside of the main suite

if __name__ == '__main__':
    unittest.main()
    
