# $Id$
# ===========================================================================
# Copyright 2004 Sean Gillies, sgillies@frii.com
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
# ===========================================================================
#
# Execute this module as a script from PyGEOS
#
#     python tests/runtests.py -v
#
# ===========================================================================

import unittest

# Import test cases
from cases.pointtest import PointTestCase
from cases.wkttest import WKTReaderTestCase, WKTWriterTestCase

# Create a test suite
suite = unittest.TestSuite()

# Add tests to the suite
suite.addTest(PointTestCase)
suite.addTest(WKTReaderTestCase)
suite.addTest(WKTWriterTestCase)

# If module is run as a script, execute every test case in the suite
if __name__ == '__main__':
    unittest.main()

