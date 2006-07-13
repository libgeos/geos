#!/usr/bin/env python

import unittest
import geos

class TestGeometry(unittest.TestCase):
	def testCreatePoint(self):
		cs = geos.CoordinateSequence(1,3)
		cs.setX(0, 7)
		cs.setY(0, 8)
		cs.setZ(0, 9)
		
		geom = geos.createPoint(cs)
		assert isinstance(geom, geos.Point)
		self.assertEqual('Point', geom.geomType())
		self.assertEqual(geos.GEOS_POINT, geom.typeId())
		
		self.assertEqual(False, geom.isEmpty())
		self.assertEqual(True, geom.isValid())
		self.assertEqual(True, geom.isSimple())
		self.assertEqual(False, geom.isRing())
		self.assertEqual(True, geom.hasZ())
		
		self.assertEqual(1, geom.getNumGeometries())

		coords = geom.getCoordSeq()
		assert isinstance(coords, geos.CoordinateSequence)
		self.assertEqual(7, coords.getX(0))
		self.assertEqual(8, coords.getY(0))
		self.assertEqual(9, coords.getZ(0))
    
		self.assertEqual(0, geom.area())
		self.assertEqual(0, geom.length())


	def testCreatePointIllegal(self):
		cs = geos.CoordinateSequence(0, 0)
		self.assertRaises(RuntimeError, geos.createPoint, cs)


	def testCreateLineString(self):
		cs = geos.CoordinateSequence(2,3)
		cs.setX(0, 7)
		cs.setY(0, 8)
		cs.setZ(0, 9)
		cs.setX(1, 3)
		cs.setY(1, 3)
		cs.setZ(1, 3)
		
		geom = geos.createLineString(cs)
		assert isinstance(geom, geos.LineString)
		self.assertEqual('LineString', geom.geomType())
		self.assertEqual(geos.GEOS_LINESTRING, geom.typeId())
		
		self.assertEqual(False, geom.isEmpty())
		self.assertEqual(True, geom.isValid())
		self.assertEqual(True, geom.isSimple())
		self.assertEqual(False, geom.isRing())
		self.assertEqual(True, geom.hasZ())
		
		self.assertEqual(1, geom.getNumGeometries())
		
		coords = geom.getCoordSeq()
		assert isinstance(coords, geos.CoordinateSequence)
		self.assertEqual(7, coords.getX(0))
		self.assertEqual(8, coords.getY(0))
		self.assertEqual(9, coords.getZ(0))
		self.assertEqual(3, coords.getX(1))
		self.assertEqual(3, coords.getY(1))
		self.assertEqual(3, coords.getZ(1))
		
		self.assertEqual(0, geom.area())
		self.assertEqual(6.4031242374328485, geom.length())
	
	
	def testCreateLineStringIllegal(self):
		cs = geos.CoordinateSequence(1,0)
		self.assertRaises(RuntimeError, geos.createLineString, cs)

	def testCreateLinearRing(self):
		cs = geos.CoordinateSequence(4,3)
		cs.setX(0, 7)
		cs.setY(0, 8)
		cs.setZ(0, 9)
		cs.setX(1, 3)
		cs.setY(1, 3)
		cs.setZ(1, 3)
		cs.setX(2, 11)
		cs.setY(2, 15.2)
		cs.setZ(2, 2)
		cs.setX(3, 7)
		cs.setY(3, 8)
		cs.setZ(3, 9)
		
		geom = geos.createLinearRing(cs)
		assert isinstance(geom, geos.LinearRing)
		self.assertEqual('LinearRing', geom.geomType())
		self.assertEqual(geos.GEOS_LINEARRING, geom.typeId())
		
		self.assertEqual(False, geom.isEmpty())
		self.assertEqual(True, geom.isValid())
		self.assertEqual(True, geom.isSimple())
		self.assertEqual(True, geom.isRing())
		self.assertEqual(True, geom.hasZ())
		
		self.assertEqual(1, geom.getNumGeometries())

		coords = geom.getCoordSeq()
		assert isinstance(coords, geos.CoordinateSequence)
		self.assertEqual(7, coords.getX(0))
		self.assertEqual(8, coords.getY(0))
		self.assertEqual(9, coords.getZ(0))
		self.assertEqual(3, coords.getX(1))
		self.assertEqual(3, coords.getY(1))
		self.assertEqual(3, coords.getZ(1))
		self.assertEqual(11, coords.getX(2))
		self.assertEqual(15.2, coords.getY(2))
		self.assertEqual(2, coords.getZ(2))
		self.assertEqual(7, coords.getX(3))
		self.assertEqual(8, coords.getY(3))
		self.assertEqual(9, coords.getZ(3))
		
		self.assertEqual(0, geom.area())
		self.assertEqual(29.228665330091953, geom.length())
	
	def testCreateLinearRingIllegal(self):
		cs = geos.CoordinateSequence(1,0)
		self.assertRaises(RuntimeError, geos.createLinearRing, cs)

	def testCreatePolygon(self):
		# Polygon shell
		cs = geos.CoordinateSequence(5,2)
		cs.setX(0, 0)
		cs.setY(0, 0)

		cs.setX(1, 0)
		cs.setY(1, 10)
		
		cs.setX(2, 10)
		cs.setY(2, 10)
		
		cs.setX(3, 10)
		cs.setY(3, 0)
		
		cs.setX(4, 0)
		cs.setY(4, 0)
		shell = geos.createLinearRing(cs)
		
		geom = geos.createPolygon(shell)
		assert isinstance(geom, geos.Polygon)
		self.assertEqual('Polygon', geom.geomType())
		self.assertEqual(geos.GEOS_POLYGON, geom.typeId())
		
		self.assertEqual(False, geom.isEmpty())
		self.assertEqual(True, geom.isValid())
		self.assertEqual(True, geom.isSimple())
		self.assertEqual(False, geom.isRing())
		self.assertEqual(True, geom.hasZ())
		
		self.assertEqual(1, geom.getNumGeometries())
		
		exteriorRing = geom.getExteriorRing()
		assert(shell.equals(exteriorRing))
		self.assertEqual(0, geom.getNumInteriorRings())
		
		self.assertRaises(RuntimeError, geom.getInteriorRingN, 1)
		
		self.assertEqual(100, geom.area())
		self.assertEqual(40, geom.length())
	
	def testCreatePolygonWithHoles(self):
		# Polygon shell
		cs = geos.CoordinateSequence(5,2)
		cs.setX(0, 0)
		cs.setY(0, 0)

		cs.setX(1, 0)
		cs.setY(1, 10)
		
		cs.setX(2, 10)
		cs.setY(2, 10)
		
		cs.setX(3, 10)
		cs.setY(3, 0)
		
		cs.setX(4, 0)
		cs.setY(4, 0)
		shell = geos.createLinearRing(cs)
		
		# Hole 1
		cs = geos.CoordinateSequence(5,2)
		cs.setX(0, 2)
		cs.setY(0, 2)

		cs.setX(1, 2)
		cs.setY(1, 4)
		
		cs.setX(2, 4)
		cs.setY(2, 4)
		
		cs.setX(3, 4)
		cs.setY(3, 2)
		
		cs.setX(4, 2)
		cs.setY(4, 2)
		hole1 = geos.createLinearRing(cs)
		
		# Hole 2
		cs = geos.CoordinateSequence(5,2)
		cs.setX(0, 6)
		cs.setY(0, 6)

		cs.setX(1, 6)
		cs.setY(1, 8)
		
		cs.setX(2, 8)
		cs.setY(2, 8)
		
		cs.setX(3, 8)
		cs.setY(3, 6)
		
		cs.setX(4, 6)
		cs.setY(4, 6)
		hole2 = geos.createLinearRing(cs)
		
		geom = geos.createPolygon(shell, [hole1, hole2])
		assert isinstance(geom, geos.Polygon)
		self.assertEqual('Polygon', geom.geomType())
		self.assertEqual(geos.GEOS_POLYGON, geom.typeId())
		
		self.assertEqual(False, geom.isEmpty())
		self.assertEqual(True, geom.isValid())
		self.assertEqual(True, geom.isSimple())
		self.assertEqual(False, geom.isRing())
		self.assertEqual(True, geom.hasZ())
		
		self.assertEqual(1, geom.getNumGeometries())
		
		exteriorRing = geom.getExteriorRing()
		assert(shell.equals(exteriorRing))
		
		self.assertEqual(2, geom.getNumInteriorRings())
		assert(hole1.equals(geom.getInteriorRingN(0)))
		assert(hole2.equals(geom.getInteriorRingN(1)))
		
		self.assertEqual(92, geom.area())
		self.assertEqual(56, geom.length())
	

	def testDistance(self):
		cs = geos.CoordinateSequence(1,2)
		cs.setX(0, 0)
		cs.setY(0, 0)
		geom1 = geos.createPoint(cs)

		cs = geos.CoordinateSequence(1,2)
		cs.setX(0, 3)
		cs.setY(0, 4)
		geom2 = geos.createPoint(cs)
		
		self.assertEqual(5, geom1.distance(geom2))

if __name__ == "__main__":
	unittest.main()
