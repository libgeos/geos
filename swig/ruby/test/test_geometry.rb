#!/usr/bin/env ruby

require 'test/unit'
require 'geos'


class TestGeometry < Test::Unit::TestCase
  def test_create_point
    cs = Geos::CoordinateSequence.new(1,3)
    cs.set_x(0, 7)
    cs.set_y(0, 8)
    cs.set_z(0, 9)
    
    geom = Geos::create_point(cs)
    assert_instance_of(Geos::Point, geom)
    assert_equal('Point', geom.geom_type)
    assert_equal(Geos::GEOS_POINT, geom.type_id)
    
    assert(!geom.empty?)
    assert(geom.valid?)
    assert(geom.simple?)
    assert(!geom.ring?)
    assert(geom.has_z?)
    
    assert_equal(1, geom.num_geometries)

    coords = geom.coord_seq
    assert_instance_of(Geos::CoordinateSequence, coords)
    assert_equal(7, coords.get_x(0))
    assert_equal(8, coords.get_y(0))
    assert_equal(9, coords.get_z(0))
    
    assert_in_delta(0, geom.area, 0.01)
    assert_in_delta(0, geom.length, 0.01)
  end
  
  def test_create_point_illegal
    cs = Geos::CoordinateSequence.new(0, 0)
    
    assert_raise(RuntimeError) do
      geom = Geos::create_point(cs)
    end
  end
  
  def test_create_line_string
    cs = Geos::CoordinateSequence.new(2,3)
    cs.set_x(0, 7)
    cs.set_y(0, 8)
    cs.set_z(0, 9)
    cs.set_x(1, 3)
    cs.set_y(1, 3)
    cs.set_z(1, 3)
    
    geom = Geos::create_line_string(cs)
    assert_instance_of(Geos::LineString, geom)
    assert_equal('LineString', geom.geom_type)
    assert_equal(Geos::GEOS_LINESTRING, geom.type_id)
    
    assert(!geom.empty?)
    assert(geom.valid?)
    assert(geom.simple?)
    assert(!geom.ring?)
    assert(geom.has_z?)
    
    assert_equal(1, geom.num_geometries)
    
    coords = geom.coord_seq
    assert_instance_of(Geos::CoordinateSequence, coords)
    assert_equal(7, coords.get_x(0))
    assert_equal(8, coords.get_y(0))
    assert_equal(9, coords.get_z(0))
    assert_equal(3, coords.get_x(1))
    assert_equal(3, coords.get_y(1))
    assert_equal(3, coords.get_z(1))
    
    assert_in_delta(0, geom.area, 0.01)
    assert_in_delta(6.40312423743285, geom.length, 0.01)
  end
  
  def test_create_line_string_illegal
    cs = Geos::CoordinateSequence.new(1,0)
    
    assert_raise(RuntimeError) do
      geom = Geos::create_line_string(cs)
    end
  end
  
  def test_create_linear_ring
    cs = Geos::CoordinateSequence.new(4,3)
    cs.set_x(0, 7)
    cs.set_y(0, 8)
    cs.set_z(0, 9)
    cs.set_x(1, 3)
    cs.set_y(1, 3)
    cs.set_z(1, 3)
    cs.set_x(2, 11)
    cs.set_y(2, 15.2)
    cs.set_z(2, 2)
    cs.set_x(3, 7)
    cs.set_y(3, 8)
    cs.set_z(3, 9)
    
    geom = Geos::create_linear_ring(cs)
    assert_instance_of(Geos::LinearRing, geom)
    assert_equal('LinearRing', geom.geom_type)
    assert_equal(Geos::GEOS_LINEARRING, geom.type_id)
    
    assert(!geom.empty?)
    assert(geom.valid?)
    assert(geom.simple?)
    assert(geom.ring?)
    assert(geom.has_z?)
    
    assert_equal(1, geom.num_geometries)

    coords = geom.coord_seq
    assert_instance_of(Geos::CoordinateSequence, coords)
    assert_equal(7, coords.get_x(0))
    assert_equal(8, coords.get_y(0))
    assert_equal(9, coords.get_z(0))
    assert_equal(3, coords.get_x(1))
    assert_equal(3, coords.get_y(1))
    assert_equal(3, coords.get_z(1))
    assert_equal(11, coords.get_x(2))
    assert_equal(15.2, coords.get_y(2))
    assert_equal(2, coords.get_z(2))
    assert_equal(7, coords.get_x(3))
    assert_equal(8, coords.get_y(3))
    assert_equal(9, coords.get_z(3))
    
    assert_in_delta(0, geom.area, 0.01)
    assert_in_delta(29.228665330092, geom.length, 0.01)
  end
  
  def test_create_linear_ring_illegal
    cs = Geos::CoordinateSequence.new(1,0)
    
    assert_raise(RuntimeError) do
      geom = Geos::create_linear_ring(cs)
    end
  end
  
  def test_create_polygon
    # Polygon shell
    cs = Geos::CoordinateSequence.new(5,2)
    cs.set_x(0, 0)
    cs.set_y(0, 0)

    cs.set_x(1, 0)
    cs.set_y(1, 10)
    
    cs.set_x(2, 10)
    cs.set_y(2, 10)
    
    cs.set_x(3, 10)
    cs.set_y(3, 0)
    
    cs.set_x(4, 0)
    cs.set_y(4, 0)
    shell = Geos::create_linear_ring(cs)
    
    geom = Geos::create_polygon(shell)
    assert_instance_of(Geos::Polygon, geom)
    assert_equal('Polygon', geom.geom_type)
    assert_equal(Geos::GEOS_POLYGON, geom.type_id)
    
    assert(!geom.empty?)
    assert(geom.valid?)
    assert(geom.simple?)
    assert(!geom.ring?)
    assert(geom.has_z?)
    
    assert_equal(1, geom.num_geometries)
    
    exterior_ring = geom.exterior_ring()
    assert(shell.eql?(exterior_ring))
    assert_equal(0, geom.num_interior_rings)
    
    assert_raise(RuntimeError) do
      assert_equal(0, geom.interior_ring_n(1))
    end
    
    assert_in_delta(100, geom.area, 0.01)
    assert_in_delta(40, geom.length, 0.01)
  end
  
  def test_create_polygon_with_holes
    # Polygon shell
    cs = Geos::CoordinateSequence.new(5,2)
    cs.set_x(0, 0)
    cs.set_y(0, 0)

    cs.set_x(1, 0)
    cs.set_y(1, 10)
    
    cs.set_x(2, 10)
    cs.set_y(2, 10)
    
    cs.set_x(3, 10)
    cs.set_y(3, 0)
    
    cs.set_x(4, 0)
    cs.set_y(4, 0)
    shell = Geos::create_linear_ring(cs)
    
    # Hole 1
    cs = Geos::CoordinateSequence.new(5,2)
    cs.set_x(0, 2)
    cs.set_y(0, 2)

    cs.set_x(1, 2)
    cs.set_y(1, 4)
    
    cs.set_x(2, 4)
    cs.set_y(2, 4)
    
    cs.set_x(3, 4)
    cs.set_y(3, 2)
    
    cs.set_x(4, 2)
    cs.set_y(4, 2)
    hole1 = Geos::create_linear_ring(cs)
    
    # Hole 2
    cs = Geos::CoordinateSequence.new(5,2)
    cs.set_x(0, 6)
    cs.set_y(0, 6)

    cs.set_x(1, 6)
    cs.set_y(1, 8)
    
    cs.set_x(2, 8)
    cs.set_y(2, 8)
    
    cs.set_x(3, 8)
    cs.set_y(3, 6)
    
    cs.set_x(4, 6)
    cs.set_y(4, 6)
    hole2 = Geos::create_linear_ring(cs)
    
    geom = Geos::create_polygon(shell, [hole1, hole2])
    assert_instance_of(Geos::Polygon, geom)
    assert_equal('Polygon', geom.geom_type)
    assert_equal(Geos::GEOS_POLYGON, geom.type_id)
    
    assert(!geom.empty?)
    assert(geom.valid?)
    assert(geom.simple?)
    assert(!geom.ring?)
    assert(geom.has_z?)
    
    assert_equal(1, geom.num_geometries)
    
    exterior_ring = geom.exterior_ring()
    assert(shell.eql?(exterior_ring))
    
    assert_equal(2, geom.num_interior_rings)
    assert(hole1.eql?(geom.interior_ring_n(0)))
    assert(hole2.eql?(geom.interior_ring_n(1)))
    
    assert_in_delta(92, geom.area, 0.01)
    assert_in_delta(56, geom.length, 0.01)
  end
  
  
  def test_distance
    cs = Geos::CoordinateSequence.new(1,2)
    cs.set_x(0, 0)
    cs.set_y(0, 0)
    geom1 = Geos::create_point(cs)

    cs = Geos::CoordinateSequence.new(1,2)
    cs.set_x(0, 3)
    cs.set_y(0, 4)
    geom2 = Geos::create_point(cs)
    
    assert_equal(5, geom1.distance(geom2))
  end
end  
