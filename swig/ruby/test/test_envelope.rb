#!/usr/bin/env ruby

require 'test/unit'
require 'geos'

# NOTE - These tests are no longer valid with the C-API

class TestGeosEnvelope < Test::Unit::TestCase
  def check(envelope)
    assert_equal(5, envelope.get_min_x())
    assert_equal(6, envelope.get_min_y())
    assert_equal(10, envelope.get_max_x())
    assert_equal(11, envelope.get_max_y())
  end
  
  def test_from_coords
    min_coord = Geos::Coordinate.new(5,6)
    max_coord = Geos::Coordinate.new(10, 11)

    envelope = Geos::Envelope.new(min_coord, max_coord)
    check(envelope)
  end
  
  def test_from_values
    envelope = Geos::Envelope.new(5, 10, 6, 11)
    check(envelope)
  end
  
  def test_serialize
    out_envelope = Geos::Envelope.new(5, 10, 6, 11)
    str = out_envelope.to_string()

    in_envelope = Geos::Envelope.new(str)
    check(in_envelope)
    assert(out_envelope.equals(in_envelope))
  end

  def test_to_s
    envelope = Geos::Envelope.new(5, 10, 6, 11)
    string = envelope.to_s

    assert_equal("<Geos::Envelope Env[5:10,6:11]>", string)
  end
end