#!/usr/bin/env ruby

require 'geos'
require 'test/unit'
require 'test_helper'

# Some methods are marked as factory methods in SWIG since
# they can return Geometry* or subclasses.  Check to
# see if they work

class TestGeosFactoryMethods < Test::Unit::TestCase
  def setup
    @test_helper = GeosTestHelper.new()
  end

  def test_reader
    point = @test_helper.create_point(9,11)
    
    wkt_writer = Geos::WKTWriter.new()
    wkt_reader = Geos::WKTReader.new(@test_helper.geom_factory)
  
    wkt = wkt_writer.write(point)
    point2 = wkt_reader.read(wkt)
    puts point2.class
    assert(point2.instance_of?(Geos::Point))
  end
end
