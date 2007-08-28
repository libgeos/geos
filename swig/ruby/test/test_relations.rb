#!/usr/bin/env ruby

# These tests evaluate the geos relational operators


require 'test/unit'
require 'geos'
require 'test_helper'

class TestGeosRelations < Test::Unit::TestCase
  X = RuntimeError
  
  def setup
    @test_helper = GeosTestHelper.new()
  end

  def get_expected_result(value)

    # 0 is false and 1 is true
    case
      when value == 0
        return false
      when value == 1
        return true
      when value == X
        return X
      else
        return value
    end
  end
  
  def run_relational_operation(operation, results, &block)
    STDOUT << "\n" << "------ #{operation} RELATION  ------" << "\n"
    STDOUT.flush()  

    # Create the geometries
    geoms = @test_helper.create_geoms()
    #@test_helper.print_geoms(geoms)

    # Loop through the the geometries the first time
    row = 0
    geoms.each do |geom1|
      # Loop through the the geometries the second time
      column = 0
      geoms.each do |geom2|
        # get value
        value = results[row][column]
        expected = self.get_expected_result(value)

        begin
          # Get the actual result from the provided block
          result = yield(geom1, geom2)
          
          STDOUT << "#{value}\t"
          
          # Did this work?
          assert_equal(expected, result, "#{operation} failure.  Row: #{row}.  Column: #{column}")

        # Geometry Collection is not a valid argument
        rescue RuntimeError => error
          assert_equal(expected, RuntimeError)
          STDOUT << "X\t"
        end
        STDOUT.flush()
        column += 1
      end
      STDOUT << "\n"
      STDOUT.flush()
      row += 1
    end
  end

  def test_contains()
    results = Array.new()

    # CONTAINS      [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[1]    = [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    results[2]    = [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[3]    = [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    results[4]    = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    results[5]    = [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]
    results[6]    = [X, X, X, X, X, X, X, 0, 0, 0, 0, X]
    results[7]    = [0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0]
    results[8]    = [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    results[9]    = [0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0]
    results[10]   = [0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("CONTAINS", results) do |geom1, geom2|
      geom1.contains?(geom2)
    end
  end
  
  def test_crosses()
    results = Array.new()

    # CROSSES       [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1]    = [0, 0, 1, 0, 0, 0, X, 1, 1, 1, 1, 0]
    results[2]    = [0, 1, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[3]    = [0, 0, 1, 0, 0, 0, X, 1, 1, 1, 1, 0]
    results[4]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[5]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[6]    = [X, X, X, X, X, X, X, X, X, X, X, X]
    results[7]    = [0, 1, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[8]    = [0, 1, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[9]    = [0, 1, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[10]   = [0, 1, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
        
    self.run_relational_operation("CROSSES", results) do |geom1, geom2|
      geom1.crosses?(geom2)
    end
  end
  
  def test_disjoint()
    results = Array.new()

    # DISJOINT      [0][1][2][3][4][5][6][7][8][9][10][11] 
    results[0]    = [0, 1, 1, 1, 1, 0, X, 1, 1, 1, 1, 1]
    results[1]    = [1, 0, 0, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[2]    = [1, 0, 0, 0, 1, 1, X, 1, 1, 1, 1, 1]
    results[3]    = [1, 0, 0, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[4]    = [1, 1, 1, 1, 0, 0, X, 1, 1, 1, 1, 1]
    results[5]    = [0, 1, 1, 1, 0, 0, X, 1, 1, 1, 1, 1]
    results[6]    = [X, X, X, X, X, X, X, X, X, X, X, X]
    results[7]    = [1, 0, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[8]    = [1, 0, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[9]    = [1, 0, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[10]   = [1, 0, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[11]   = [1, 1, 1, 1, 1, 1, X, 1, 1, 1, 1, 0]
        
    self.run_relational_operation("DISJOINT", results) do |geom1, geom2|
      geom1.disjoint?(geom2)
    end
  end

  def test_equals()
    results = Array.new()

    # EQUALS        [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[1]    = [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    results[2]    = [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[3]    = [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    results[4]    = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    results[5]    = [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]
    results[6]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[7]    = [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]
    results[8]    = [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    results[9]    = [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0]
    results[10]   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
        
    self.run_relational_operation("EQUALS", results) do |geom1, geom2|
      geom1.eql?(geom2)
    end
  end

  #def test_equals_exact()
    ## Setup the expected results

    ## EQUALS_EXACT [0][1][2][3][4][5][6][7][8][9][10][11]
    #results[0]    = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    #results[1]    = [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    #results[2]    = [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    #results[3]    = [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    #results[4]    = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    #results[5]    = [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]
    #results[6]    = [0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0]
    #results[7]    = [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]
    #results[8]    = [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    #results[9]    = [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0]
    #results[10]   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0]
    #results[11]   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
    
    #self.run_relational_operation("EQUALS_EXACT", results) do |geom1, geom2|
      #geom1.equals_exact?(geom2)
    #end
  #end
  
  def test_intersects()
    results = Array.new()
    
    # INTERSECTS    [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [1, 0, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[1]    = [0, 1, 1, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[2]    = [0, 1, 1, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[3]    = [0, 1, 1, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[4]    = [0, 0, 0, 0, 1, 1, X, 0, 0, 0, 0, 0]
    results[5]    = [1, 0, 0, 0, 1, 1, X, 0, 0, 0, 0, 0]
    results[6]    = [X, X, X, X, X, X, X, X, X, 1, 1, X]
    results[7]    = [0, 1, 0, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[8]    = [0, 1, 0, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[9]    = [0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    results[10]   = [0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("INTERSECTS", results) do |geom1, geom2|
      geom1.intersects?(geom2)
    end
  end

  #def test_is_within_distance()
    #results = Array.new()

    ## IS_WITHIN_DIST[0][1][2][3][4][5][6][7][8][9][10][11]
    #results[0]    = [1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0]
    #results[1]    = [0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    #results[2]    = [0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0]
    #results[3]    = [0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    #results[4]    = [0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0]
    #results[5]    = [1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0]
    #results[6]    = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0]
    #results[7]    = [0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    #results[8]    = [0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    #results[9]    = [0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    #results[10]   = [0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    #results[11]   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
    
    #self.run_relational_operation("IS_WITHIN_DISTANCE", results) do |geom1, geom2|
      #geom1.is_within_distance?(geom2, 2)
    #end
  #end
  
  def test_overlaps()
    results = Array.new()

    # OVERLAPS      [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[2]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[3]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[4]    = [0, 0, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[5]    = [0, 0, 0, 0, 1, 0, X, 0, 0, 0, 0, 0]
    results[6]    = [X, X, X, X, X, X, X, X, X, X, X, X]
    results[7]    = [0, 0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[8]    = [0, 0, 0, 0, 0, 0, X, 1, 0, 1, 1, 0]
    results[9]    = [0, 0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[10]   = [0, 0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    
    self.run_relational_operation("OVERLAPS", results) do |geom1, geom2|
      geom1.overlaps?(geom2)
    end
  end

  def test_relate_pattern()
    results = Array.new()

    # RELATE        [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[2]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[3]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[4]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[5]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[6]    = [X, X, X, X, X, X, X, X, X, X, X, X]
    results[7]    = [0, 0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[8]    = [0, 0, 0, 0, 0, 0, X, 1, 0, 1, 1, 0]
    results[9]    = [0, 0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[10]   = [0, 0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    
    self.run_relational_operation("RELATE PATTERN", results) do |geom1, geom2|
      geom1.relate_pattern(geom2, "212101212")
    end
  end

  def test_relate()
    results = Array.new()

    # ------ RELATE RELATION  ------
    results[0]   = ["0FFFFFFF2", "FF0FFF1F2", "FF0FFF102", "FF0FFF1F2", "FF0FFF212", "F0FFFF212", nil, "FF0FFF212", "FF0FFF212", "FF0FFF212", "FF0FFF212", "FF0FFF102"]
    results[1]   = ["FF1FFF0F2", "1FFFFFFF2", "0F1FFF102", "1FFFFFFF2", "FF1FFF212", "FF1FFF212", nil, "101FFF212", "101FFF212", "101FFF212", "101FFF212", "FF1FFF102"]
    results[2]   = ["FF1FF00F2", "0F1FF01F2", "1FFF0FFF2", "0F1FF01F2", "FF1FF0212", "FF1FF0212", nil, "FF1FF0212", "FF1FF0212", "FF1FF0212", "FF1FF0212", "FF1FF0102"]
    results[3]   = ["FF1FFF0F2", "1FFFFFFF2", "0F1FFF102", "1FFFFFFF2", "FF1FFF212", "FF1FFF212", nil, "101FFF212", "101FFF212", "101FFF212", "101FFF212", "FF1FFF102"]
    results[4]   = ["FF2FF10F2", "FF2FF11F2", "FF2FF1102", "FF2FF11F2", "2FFF1FFF2", "212111212", nil, "FF2FF1212", "FF2FF1212", "FF2FF1212", "FF2FF1212", "FF2FF1102"]
    results[5]   = ["FF20F1FF2", "FF2FF11F2", "FF2FF1102", "FF2FF11F2", "212111212", "2FFF1FFF2", nil, "FF2FF1212", "FF2FF1212", "FF2FF1212", "FF2FF1212", "FF2FF1102"]
    results[6]   = [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
    results[7]   = ["FF2FF10F2", "1F20F11F2", "FF2FF1102", "1F20F11F2", "FF2FF1212", "FF2FF1212", nil, "2FFF1FFF2", "212101212", "2FF10F212", "2FF10F212", "FF2FF1102"]
    results[8]   = ["FF2FF10F2", "1F20F11F2", "FF2FF1102", "1F20F11F2", "FF2FF1212", "FF2FF1212", nil, "212101212", "2FFF1FFF2", "212101212", "212101212", "FF2FF1102"]
    results[9]   = ["FF2FF10F2", "1F20F11F2", "FF2FF1102", "1F20F11F2", "FF2FF1212", "FF2FF1212", nil, "212F01FF2", "212101212", "2FFF1FFF2", "2FF11F212", "FF2FF1102"]
    results[10]  = ["FF2FF10F2", "1F20F11F2", "FF2FF1102", "1F20F11F2", "FF2FF1212", "FF2FF1212", nil, "212F01FF2", "212101212", "212F11FF2", "2FFF1FFF2", "FF2FF1102"]
    results[11]  = ["FF1FF00F2", "FF1FF01F2", "FF1FF0102", "FF1FF01F2", "FF1FF0212", "FF1FF0212", nil, "FF1FF0212", "FF1FF0212", "FF1FF0212", "FF1FF0212", "1FFF0FFF2"]
    
    self.run_relational_operation("RELATE", results) do |geom1, geom2|
      geom1.relate(geom2)
    end
  end
  
  def test_touches()
    results = Array.new()

    # TOUCHES       [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [0, 0, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[1]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[2]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[3]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[4]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[5]    = [1, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[6]    = [X, X, X, X, X, X, X, X, X, X, X, X]
    results[7]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[8]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[9]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[10]   = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    
    self.run_relational_operation("TOUCHES", results) do |geom1, geom2|
      geom1.touches?(geom2)
    end
  end

  def test_within()
    results = Array.new()

    # WITHIN        [0][1][2][3][4][5][6][7][8][9][10][11]
    results[0]    = [1, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1]    = [0, 1, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[2]    = [0, 0, 1, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[3]    = [0, 1, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[4]    = [0, 0, 0, 0, 1, 0, X, 0, 0, 0, 0, 0]
    results[5]    = [0, 0, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[6]    = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[7]    = [0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0]
    results[8]    = [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    results[9]    = [0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0]
    results[10]   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0]
    results[11]   = [0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("WITHIN", results) do |geom1, geom2|
      geom1.within?(geom2)
    end
  end
end