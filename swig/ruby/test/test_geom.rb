#!/usr/bin/env ruby

require 'test/unit'
require 'geos'
require 'test_helper'

class TestGeosSimple < Test::Unit::TestCase
  def setup
    @test_helper = GeosTestHelper.new()
  end
  
  def test_create_coord
    coord = @test_helper.create_coord(5,6)
    assert_equal(5, coord.x)
    assert_equal(6, coord.y)
  end
  
  def test_create_point
    point = @test_helper.create_point(5,6)
    assert_equal(5, point.x)
    assert_equal(6, point.y)
  end

  def test_srid
    point = @test_helper.create_point(5,6)
    point.srid = 4326
    assert_equal(4326, point.srid)
  end
      
  def test_clone
    point1 = @test_helper.create_point(5,6)
    point1.srid = 4326
    point2 = point1.clone
    assert(point1.equals(point2))
    assert(point1.srid, point2.srid)
  end
  
  #def test_marshal
    #point1 = @test_helper.create_point(5,6)
    #dump = Marshal.dump(point1)
    
    #point2 = Marshal.load(dump)
    #assert(point1.equals(point2))
  #end    
end