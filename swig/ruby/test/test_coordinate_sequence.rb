#!/usr/bin/env ruby

require 'test/unit'
require 'geos'


class TestWkbReader < Test::Unit::TestCase
  def test_create
    cs = Geos::CoordinateSequence.new(5,2)
    assert_instance_of(Geos::CoordinateSequence, cs)
  end
  
  def test_size
    cs = Geos::CoordinateSequence.new(7,2)
    assert_equal(7, cs.length)
  end
 
  def test_getter_setter
    cs = Geos::CoordinateSequence.new(2,3)
    cs.set_x(0, 3)
    cs.set_y(0, 4)
    cs.set_z(0, 5)
    cs.set_x(1, 6)
    cs.set_y(1, 7)
    cs.set_z(1, 8)
    assert_equal(3, cs.get_x(0))
    assert_equal(4, cs.get_y(0))
    assert_equal(5, cs.get_z(0))
    assert_equal(6, cs.get_x(1))
    assert_equal(7, cs.get_y(1))
    assert_equal(8, cs.get_z(1))
  end

  def test_index_out_of_bounds
    cs = Geos::CoordinateSequence.new(2,3)
    assert_raise(RuntimeError) do 
      cs.set_x(2, 6)
    end
    assert_raise(RuntimeError) do 
      cs.set_y(2, 6)
    end
    assert_raise(RuntimeError) do 
      cs.set_z(2, 6)
    end
  end

  def test_ordinate
    cs = Geos::CoordinateSequence.new(1,3)
    cs.set_ordinate(0, 0, 2)
    cs.set_ordinate(0, 1, 3)
    cs.set_ordinate(0, 2, 4)
    
    assert_equal(2, cs.get_ordinate(0, 0))
    assert_equal(3, cs.get_ordinate(0, 1))
    assert_equal(4, cs.get_ordinate(0, 2))
  end
 
  def test_clone
    cs1 = Geos::CoordinateSequence.new(1,3)
    cs1.set_x(0, 2)
    cs1.set_y(0, 3)
    cs1.set_y(0, 4)
    
    cs2 = cs1.clone
    
    assert(!cs1.equal?(cs2))
    assert(2, cs2.get_x(0))
    assert(3, cs2.get_y(0))
    assert(4, cs2.get_z(0))
    
    cs2.set_x(0, 5)
    cs2.set_y(0, 6)
    cs2.set_y(0, 7)
    
    assert(2, cs1.get_x(0))
    assert(3, cs1.get_y(0))
    assert(4, cs1.get_z(0))

    assert(5, cs2.get_x(0))
    assert(6, cs2.get_y(0))
    assert(7, cs2.get_z(0))
  end

  def test_dimensions
    cs = Geos::CoordinateSequence.new(1,3)
    assert_equal(3, cs.dimensions)
  end
end