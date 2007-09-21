#!/usr/bin/env ruby

require 'test/unit'
require 'geos'

class SridTest < Test::Unit::TestCase

  def test_srid
    cs = Geos::CoordinateSequence.new(1,2)
    cs.set_x(0, 7)
    cs.set_y(0, 8)
    
    geom1 = Geos.create_point(cs)
    geom1.srid = 4326
  end

  def test_write_srid
    reader = Geos::WktReader.new
    geom = reader.read("POINT(7 8)")
    geom.srid = 4326

    # without srid
    writer = Geos::WkbWriter.new
    writer.include_srid = false
    hex = writer.write_hex(geom)
    assert_equal("01010000000000000000001C400000000000002040", hex)   
    
    # with srid
    writer.include_srid = true
    hex = writer.write_hex(geom)
    assert_equal("0101000020E61000000000000000001C400000000000002040", hex)        
  end
  
  def test_roundtrip_srid
    reader = Geos::WktReader.new
    geom1 = reader.read("POINT(7 8)")
    geom1.srid = 4326
    assert_equal(4326, geom1.srid)
    
    writer = Geos::WkbWriter.new
    writer.include_srid = true
    hex = writer.write_hex(geom1)

    reader = Geos::WkbReader.new
    geom2 = reader.read_hex(hex)
    assert_equal(4326, geom2.srid)
  end
  
  def test_read_srid
    # srid=4326;POINT(7 8)
    xdr = "0101000020E61000000000000000001C400000000000002040"
    
    reader = Geos::WkbReader.new
    geom = reader.read_hex(xdr)
    
    assert_equal(7, geom.coord_seq.get_x(0))
    assert_equal(8,geom.coord_seq.get_y(0))
    assert_equal(4326, geom.srid)
  end
end
