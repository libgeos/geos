# $Id$
# ===========================================================================
# Author:   Sean Gillies, sgillies@frii.com
# ===========================================================================
# Copyright (c) 2004, Sean Gillies
# ===========================================================================
#
# Execute this module as a script from PyGEOS
#
#     python tests/cases/pointtest.py -v
#
# ===========================================================================

import os, sys
import unittest

# the testing module helps us import the pre-installed geos
from testing import geos

# ===========================================================================

class PointTestCase(unittest.TestCase):

    def testConstructor(self):
        gf = geos.GeometryFactory()
        x = 492187.0
        y = 4493034.0
        z = 1500.0
        c = geos.Coordinate(x, y, z)  # Me!
        p = gf.createPoint(c)
        assert p.getX() == x
        assert p.getY() == y
     

# ===========================================================================
# Run the tests outside of the main suite

if __name__ == '__main__':
    unittest.main()
    
