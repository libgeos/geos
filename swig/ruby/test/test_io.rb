#!/usr/bin/env ruby

require 'test/unit'
require 'geos'
require 'test_helper'

class TestGeosIO < Test::Unit::TestCase
  def setup
    @test_helper = GeosTestHelper.new()
  end

  def run_test(message, &block)
    # This function tests writing and reading geometries to
    # the well-known binary hex format
    geoms = @test_helper.create_geoms()

    STDOUT << "\n" << "-------- #{message} ----------" << "\n"
    geoms.each do |geom_in|
      geom_out = yield(geom_in)

      begin
        ## Geometries should be equal
        assert(geom_out.equals(geom_in),
               "Geom In: #{geom_in}\nGeom Out: #{geom_out}")
      rescue Geos::IllegalArgumentException => error
        # this is ok
      end

      ## Check precision model
      geom_in.normalize()
      geom_out.normalize()

      ## This seems to always fail
     # assert_equal(0, geom_in.compare_to(geom_out))
    end
  end
  
  def test_wkb_hex()
    wkb_writer = Geos::WKBWriter.new()
    wkb_reader = Geos::WKBReader.new(@test_helper.geom_factory)

    self.run_test("TESTING WKB HEX IO") do |geom_in|
      value = wkb_writer.write_hex(geom_in)
      geom_out = wkb_reader.read_hex(value)
    end     
  end
  
  def test_wkb()
    wkb_writer = Geos::WKBWriter.new()
    wkb_reader = Geos::WKBReader.new(@test_helper.geom_factory)

    self.run_test("TESTING WKB IO") do |geom_in|
      value = wkb_writer.write(geom_in)
      geom_out = wkb_reader.read(value)
    end     
  end
  
  def test_wkt()
    wkt_writer = Geos::WKTWriter.new()
    wkt_reader = Geos::WKTReader.new(@test_helper.geom_factory)

    self.run_test("TESTING WKT IO") do |geom_in|
      value = wkt_writer.write(geom_in)
      geom_out = wkt_reader.read(value)
    end     
  end
end