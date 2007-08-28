#!/usr/bin/env ruby

require 'test/unit'
require 'geos'
require 'test_helper'

class TestBuffer < Test::Unit::TestCase
  def setup
    @test_helper = GeosTestHelper.new()
  end
  
  def test_xml_cases
    cases = @test_helper.load_tests("buffer.xml")
    
    cases.each do |test_case|
      geom = test_case.geom1
      tolerance = test_case.arg3.to_f
      expected = @test_helper.parse_geometry(test_case.expected)
      expected.normalize
      
      result = geom.buffer(tolerance, 8)
      result.normalize
      
      assert(expected.eql_exact?(result, 0.00000000001),
             "Buffer:\n" + 
             "  Case: #{test_case.case_id}\n" + 
             "  Test: #{test_case.test_id}\n" + 
             "  Geom: #{Geos.geom_to_wkt(geom)}\n" + 
             "  Tolerance: #{tolerance}\n" + 
             "  Expected: #{Geos.geom_to_wkt(expected)}.\n" + 
             "    Actual: #{Geos.geom_to_wkt(result)}")
    end
  end    
end



