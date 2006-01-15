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

  def test_envelope
    in_envelope = Geos::Envelope.new(3.1,3.3, 7.215, 8.392)
    str = in_envelope.to_string()
    out_envelope = Geos::Envelope.new(str)
    assert(out_envelope.equals(in_envelope))
  end
end