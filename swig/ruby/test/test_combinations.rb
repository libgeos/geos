#!/usr/bin/env ruby

require 'test/unit'
require 'geos'
require 'test_helper'

class TestGeosCombinations < Test::Unit::TestCase
  def setup
    @test_helper = GeosTestHelper.new()
  end
  
  def run_combination(message, &block)
    STDOUT << "\n" << "-------- #{message} COMBINATION  ------" << "\n"
    STDOUT.flush()

    geoms = @test_helper.create_geoms()
    new_geoms = Array.new()
    
    geoms.each do |geom1|
      geoms.each do |geom2|
        begin
          # Get the actual result from the provided block
          result = yield(geom1, geom2)
          new_geoms.push(result)
     #     puts result
        # Geometry Collection is not a valid argument
        rescue RuntimeError => error
          # This is ok
        end
      end
    end
  end

  def test_union()
    run_combination("UNION") do |geom1, geom2|
      geom1.union(geom2)
    end
  end

  def test_intersection()
    run_combination("INTERSECTION") do |geom1, geom2|
      geom1.intersection(geom2)
    end
  end
  
  def test_difference()
    run_combination("DIFFERENCE") do |geom1, geom2|
      geom1.difference(geom2)
    end
  end
  
  def test_symdifference()
    run_combination("SYMDIFFERENCE") do |geom1, geom2|
      geom1.sym_difference(geom2)
    end
  end
end