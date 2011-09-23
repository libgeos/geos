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
from test_geometry import TestGeometry

# Create a test suite
suite = unittest.TestSuite()

# Add tests to the suite
suite.addTest(TestGeometry)

# If module is run as a script, execute every test case in the suite
if __name__ == '__main__':
    unittest.main()

