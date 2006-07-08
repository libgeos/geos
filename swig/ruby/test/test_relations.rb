#!/usr/bin/env ruby

# These tests evaluate the geos relational operators


require 'test/unit'
require 'geos'
require 'test_helper'

class TestGeosRelations < Test::Unit::TestCase
  X = Geos::IllegalArgumentException
  
  def setup
    @test_helper = GeosTestHelper.new()
  end

  def get_expected_result(results, row, column)
    value = results[row][column]

    # 0 is false and 1 is true
    case
      when value == 0
        return false
      when value == 1
        return true
      when value == X
        return X
      else
        raise ArgumentError, "Unknown result value"
    end
  end
  
  def run_relational_operation(operation, results, &block)
    STDOUT << "\n" << "------ #{operation} RELATION  ------" << "\n"
    STDOUT.flush()  

    # Create the geometries
    geoms = @test_helper.create_geoms()

    # Loop through the the geometries the first time
    row = 0
    geoms.each do |geom1|
      # Loop through the the geometries the second time
      column = 0
      geoms.each do |geom2|
        # Get the expected result
        expected = self.get_expected_result(results, row, column)

        begin
          # Get the actual result from the provided block
          result = yield(geom1, geom2)

          if result
            STDOUT << "1\t"
          else
            STDOUT << "0\t"
          end

          # Did this work?
          assert_equal(expected, result, "#{operation} failure")

        # Geometry Collection is not a valid argument
        rescue Geos::IllegalArgumentException => error
          # This is probably ok but check
          assert_equal(expected, Geos::IllegalArgumentException)
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
    # Setup the expected results

    # CONTAINS Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[1] =  [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[2] =  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    results[3] =  [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    results[4] =  [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]
    results[5] =  [X, X, X, X, X, X, 0, 0, 0, 0, X]
    results[6] =  [0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]
    results[7] =  [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    results[8] =  [0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0]
    results[9] =  [0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0]
    results[10] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("CONTAINS", results) do |geom1, geom2|
      geom1.contains(geom2)
    end
  end
  
  def test_crosses()
    # Setup the expected results

    # CROSSES Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1] =  [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[2] =  [0, 1, 0, 0, 0, X, 1, 1, 1, 1, 0]
    results[3] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[4] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[5] =  [X, X, X, X, X, X, X, X, X, X, X]
    results[6] =  [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[7] =  [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[8] =  [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[9] =  [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[10] = [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    
    self.run_relational_operation("CROSSES", results) do |geom1, geom2|
      geom1.crosses(geom2)
    end
  end
  
  def test_disjoint()
    # Setup the expected results

    # DISJOINT Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [0, 1, 1, 1, 0, X, 1, 1, 1, 1, 1]
    results[1] =  [1, 0, 0, 1, 1, X, 1, 1, 1, 1, 1]
    results[2] =  [1, 0, 0, 1, 1, X, 0, 0, 0, 0, 0]
    results[3] =  [1, 1, 1, 0, 0, X, 1, 1, 1, 1, 1]
    results[4] =  [0, 1, 1, 0, 0, X, 1, 1, 1, 1, 1]
    results[5] =  [X, X, X, X, X, X, X, X, X, X, X]
    results[6] =  [1, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[7] =  [1, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[8] =  [1, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[9] =  [1, 1, 0, 1, 1, X, 0, 0, 0, 0, 1]
    results[10] = [1, 1, 0, 1, 1, X, 1, 1, 1, 1, 0]
    
    self.run_relational_operation("DISJOINT", results) do |geom1, geom2|
      geom1.disjoint(geom2)
    end
  end

  def test_equals()
    # Setup the expected results

    # EQUALS Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[1] =  [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[2] =  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    results[3] =  [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    results[4] =  [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]
    results[5] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[6] =  [0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]
    results[7] =  [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    results[8] =  [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0]
    results[9] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0]
    results[10] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("EQUALS", results) do |geom1, geom2|
      geom1.equals(geom2)
    end
  end

  def test_equals_exact()
    # Setup the expected results

    # EQUALS_EXACT Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[1] =  [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[2] =  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    results[3] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[4] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[5] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[6] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[7] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[8] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[9] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    results[10] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("EQUALS_EXACT", results) do |geom1, geom2|
      geom1.equals_exact(geom2)
    end
  end
  
  def test_intersects()
    # Setup the expected results

    # INTERSECTS Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [1, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[1] =  [0, 1, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[2] =  [0, 1, 1, 0, 0, X, 1, 1, 1, 1, 1]
    results[3] =  [0, 0, 0, 1, 1, X, 0, 0, 0, 0, 0]
    results[4] =  [1, 0, 0, 1, 1, X, 0, 0, 0, 0, 0]
    results[5] =  [X, X, X, X, X, X, X, X, X, X, X]
    results[6] =  [0, 0, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[7] =  [0, 0, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[8] =  [0, 0, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[9] =  [0, 0, 1, 0, 0, X, 1, 1, 1, 1, 0]
    results[10] = [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("INTERSECTS", results) do |geom1, geom2|
      geom1.intersects(geom2)
    end
  end

  def test_is_within_distance()
    # Setup the expected results

    # IS_WITHIN_DISTANCE Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0]
    results[1] =  [0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0]
    results[2] =  [0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    results[3] =  [0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0]
    results[4] =  [1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0]
    results[5] =  [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0]
    results[6] =  [0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    results[7] =  [0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    results[8] =  [0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    results[9] =  [0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0]
    results[10] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("IS_WITHIN_DISTANCE", results) do |geom1, geom2|
      geom1.is_within_distance(geom2, 2)
    end
  end
  
  def test_overlaps()
    # Setup the expected results

    # OVERLAPS Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[2] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 1]
    results[3] =  [0, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[4] =  [0, 0, 0, 1, 0, X, 0, 0, 0, 0, 0]
    results[5] =  [X, X, X, X, X, X, X, X, X, X, X]
    results[6] =  [0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[7] =  [0, 0, 0, 0, 0, X, 1, 0, 1, 1, 0]
    results[8] =  [0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[9] =  [0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[10] = [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    
    self.run_relational_operation("OVERLAPS", results) do |geom1, geom2|
      geom1.overlaps(geom2)
    end
  end

  def test_relate()
    # Setup the expected results

    # RELATE Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[2] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[3] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[4] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[5] =  [X, X, X, X, X, X, X, X, X, X, X]
    results[6] =  [0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[7] =  [0, 0, 0, 0, 0, X, 1, 0, 1, 1, 0]
    results[8] =  [0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[9] =  [0, 0, 0, 0, 0, X, 0, 1, 0, 0, 0]
    results[10] = [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    
    self.run_relational_operation("RELATE", results) do |geom1, geom2|
      geom1.relate(geom2, "212101212")
    end
  end

  def test_touches()
    # Setup the expected results

    # TOUCHES Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [0, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[1] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[2] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[3] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[4] =  [1, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[5] =  [X, X, X, X, X, X, X, X, X, X, X]
    results[6] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[7] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[8] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[9] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[10] = [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    
    self.run_relational_operation("TOUCHES", results) do |geom1, geom2|
      geom1.touches(geom2)
    end
  end

  def test_within()
    # Setup the expected results

    # WITHIN Results
    #             [0][1][2][3][4][5][6][7][8][9][10]
    results = Array.new()
    results[0] =  [1, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[1] =  [0, 1, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[2] =  [0, 0, 1, 0, 0, X, 0, 0, 0, 0, 0]
    results[3] =  [0, 0, 0, 1, 0, X, 0, 0, 0, 0, 0]
    results[4] =  [0, 0, 0, 0, 1, X, 0, 0, 0, 0, 0]
    results[5] =  [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0]
    results[6] =  [0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0]
    results[7] =  [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    results[8] =  [0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0]
    results[9] =  [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0]
    results[10] = [0, 0, 0, 0, 0, X, 0, 0, 0, 0, 1]
    
    self.run_relational_operation("WITHIN", results) do |geom1, geom2|
      geom1.within(geom2)
    end
  end
end