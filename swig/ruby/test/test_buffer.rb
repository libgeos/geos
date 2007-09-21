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
      
      writer = Geos::WktWriter.new
      geom_wkt = writer.write(geom)
      expected_wkt = writer.write(expected)
      result_wkt = writer.write(result)
      
      assert(expected.eql_exact?(result, 0.00000000001),
             "Buffer:\n" + 
             "  Case: #{test_case.case_id}\n" + 
             "  Test: #{test_case.test_id}\n" + 
             "  Geom: #{geom_wkt}\n" + 
             "  Tolerance: #{tolerance}\n" + 
             "  Expected: #{expected_wkt}.\n" + 
             "    Actual: #{result_wkt}")
    end
  end    
end



