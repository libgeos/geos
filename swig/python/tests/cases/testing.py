# $Id$
# ===========================================================================
# Author:   Sean Gillies, <sgillies@frii.com>
# ===========================================================================
# Copyright (c) 2004, Sean Gillies
# ===========================================================================

import os
import sys
import distutils.util
import unittest

# Put local build directory on head of python path
platformdir = '-'.join((distutils.util.get_platform(), 
                        '.'.join(map(str, sys.version_info[0:2]))))
sys.path.insert(0, os.path.join('build', 'lib.' + platformdir))

# import geos from the local build directory
import geos

