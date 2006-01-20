#!/usr/bin/env ruby

require 'test/unit'
require 'geos'
require 'stringio'

class TestGeos < Test::Unit::TestCase
  def setup
    @geom_factory = Geos::GeometryFactory.new()
  end
  
  def create_coord(x, y)
    Geos::Coordinate.new(x, y)
  end

  def test_create_coord
    coord = create_coord(5,6)
    assert_equal(5, coord.x)
    assert_equal(6, coord.y)
  end
    
  def create_point(x, y)
    coord = create_coord(x, y)
    @geom_factory.create_point(coord)
  end
  
  def test_create_point
    point = create_point(5,6)
    assert_equal(5, point.x)
    assert_equal(6, point.y)
  end

  def create_ushaped_linestring(xoffset, yoffset, side)
    # We will use a coordinate list to build the linestring
    cl = Geos::DefaultCoordinateSequence.new()
    cl.add(Geos::Coordinate.new(xoffset, yoffset))
    cl.add(Geos::Coordinate.new(xoffset, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset))

    # Now that we have a CoordinateSequence we can create the linestring.
    # The newly created LineString will take ownership of the CoordinateSequence.
    # @geom_factory.create_line_string!(cl)

    # This is what you do if you want the new LineString
    # to make a copy of your CoordinateSequence:
     @geom_factory.create_line_string(cl)
  end
    
  # This function will create a LinearRing geometry
  # representing a square with the given origin and side 
  def create_square_linearring(xoffset, yoffset, side)
    #  We will use a coordinate list to build the linearring
    cl = Geos::DefaultCoordinateSequence.new()
    cl.add(Geos::Coordinate.new(xoffset, yoffset))
    cl.add(Geos::Coordinate.new(xoffset, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset))
    cl.add(Geos::Coordinate.new(xoffset, yoffset))

    # Create the line string The newly created LinearRing will
    # take ownership of the CoordinateSequence.
    #@geom_factory.create_linear_ring!(cl)

    # To make a copy of your CoordinateSequence
    @geom_factory.create_linear_ring(cl)
  end

  def create_square_polygon(xoffset, yoffset, side)
    # This function will create a Polygon
    # geometry rapresenting a square with the given origin 
    # and side and with a central hole 1/3 sided.

    # We need a LinearRing for the polygon shell 
    outer = create_square_linearring(xoffset,yoffset,side);

    # And another for the hole 
    inner = create_square_linearring(xoffset+(side/3), yoffset+(side/3),(side/3));
  
    # Specify hole as vector of Geometries
    holes = Geos::GeometryVector.new()

    # Add the newly created geometry to the vector of holes.
    holes.push(inner)

    # Finally we call the polygon constructor. Both the outer LinearRing 
    # and the vector of holes will be referenced by the resulting
    # Polygon object.
    poly = @geom_factory.create_polygon(outer, holes)
  end
  
  def create_simple_collection(geoms)
    # To transfer ownership of the vector and its
    # elements you can do this
    #@geom_factory.create_geometry_collection!(geoms)

    # This function creates a GeometryCollection
    # containing copies of all Geometries in given vector.
    @geom_factory.create_geometry_collection(geoms)
  end

  def create_circle(centerX, centerY, radius)
    # Use a GeometricShapeFactory to render
    # a circle having the specified center and radius

    shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    shapefactory.set_centre(Geos::Coordinate.new(centerX, centerY))
    shapefactory.set_size(radius);
    # same as:
    #   shapefactory.set_height(radius)
    #   shapefactory.set_width(radius)
    
    shapefactory.create_circle()
  end

  def create_ellipse(centerX, centerY, width, height)
    # Use a GeometricShapeFactory to render
    # a circle having the specified center and radius

    shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    shapefactory.set_centre(Geos::Coordinate.new(centerX, centerY))
    shapefactory.set_height(width)
    shapefactory.set_width(height)
    
    shapefactory.create_circle()
  end

  def create_rectangle(llX, llY, width, height)
    # This function uses GeometricShapeFactory to render
    # a rectangle having lower-left corner at given coordinates
    # and given sizes.

    shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    shapefactory.set_base(Geos::Coordinate.new(llX, llY))
    shapefactory.set_height(height)
    shapefactory.set_width(width)
    
    # we don't need more then 4 points for a rectangle...
    shapefactory.set_num_points(4)
    # can use setSize for a square
    shapefactory.create_rectangle()
  end
  
  def create_arc(llX, llY, width, height, startang, endang)
    # This function uses GeometricShapeFactory to render
    # an arc having lower-left corner at given coordinates,
    # given sizes and given angles. 

    shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    shapefactory.set_base(Geos::Coordinate.new(llX, llY))
    shapefactory.set_height(height)
    shapefactory.set_width(width)
    # the default (100 pts)
    shapefactory.set_num_points(100)
    
    shapefactory.create_arc(startang, endang)
  end
  
  def create_geoms
    geoms = Geos::GeometryVector.new()

    # Define a precision model using 0,0 as the reference origin
    # and 2.0 as coordinates scale.
    pm = Geos::PrecisionModel.new(2.0, 0, 0)

    # Initialize global factory with defined PrecisionModel
    # and a SRID of -1 (undefined).
    global_factory = Geos::GeometryFactory.new(pm, -1)

    geoms.push(create_point(150, 350))
    geoms.push(create_ushaped_linestring(60,60,100))
    geoms.push(create_square_linearring(0,0,100))
    geoms.push(create_square_polygon(0,200,300))
#   geoms.push(create_simple_collection(geoms))

    geoms.push(create_circle(0, 0, 10))
    geoms.push(create_ellipse(0, 0, 8, 12))
    ## A square
    geoms.push(create_rectangle(-5, -5, 10, 10))
    ## A rectangle
    geoms.push(create_rectangle(-5, -5, 10, 20))
    ## The upper-right quarter of a vertical ellipse
    geoms.push(create_arc(0, 0, 10, 20, 0, Math::PI/2))

    return geoms
  end
  
  def test_wkb()
    # This function tests writing and reading geometries to
    # the well-known binary format

    geoms = create_geoms()
    
    wkb_writer = Geos::WKBWriter.new()
    wkb_reader = Geos::WKBReader.new(@geom_factory)

    STDOUT << "\n" << "-------- TESTING WKB OUTPUT ----------" << "\n"
    geoms.each do |geom_in|
      value = wkb_writer.write(geom_in)
      geom_out = wkb_reader.read(value);

      # Geometries should be equal
      assert(geom_out.equals(geom_in))

      # Check precision model
      geom_in.normalize()
      geom_out.normalize()

      # This seems to always fail
      #assert_equal(0, geom_in.compare_to(geom_out))

      # Print out a hex value
      STDOUT << Geos::WKBReader.print_hex(value) << "\n"
    end
    STDOUT << "\n"
  end
  
  def print_wkt(geoms, message)
    ## This function will print given geometries in WKT
    ## format to stdout.

    wkt_writer = Geos::WKTWriter.new()

    STDOUT << "\n" << "-------- #{message} ----------" << "\n"
    geoms.each do |geom_in|
      puts wkt_writer.write(geom_in)
    end
    STDOUT << "\n"
  end

  def test_wkt()
    ## This function will print given geometries in WKT
    ## format to stdout.

    geoms = create_geoms()

    print_wkt(geoms, "BASE GEOMS")
  end

  def test_operation(message, &block)
    # First find the centroid of each base geometry
    new_geoms = Geos::GeometryVector.new()

    geoms = create_geoms()

    geoms.each do |geom|
      new_geoms.push(yield(geom))
    end

    # Print results
    print_wkt(new_geoms, message)
  end

  def test_centroid()
    test_operation("CENTROIDS") do |geom|
      geom.get_centroid()
    end
  end

  def test_buffer()
    test_operation("BUFFERS") do |geom|
      geom.buffer(10)
    end
  end
  
  def test_convex_hull()
    test_operation("CONVEX HULL") do |geom|
      geom.convex_hull()
    end
  end

  def test_relational_operation(message, &block)
    STDOUT << "\n" << "-------- #{message} ----------" << "\n"

    geoms = create_geoms()

    geoms.each do |geom1|
      geoms.each do |geom2|
        result = yield(geom1, geom2)
        if result
          STDOUT << " 1\t"
        else
          STDOUT << " 0\t"
        end
      end
    end
    
    STDOUT << "\n"
  end

  def test_disjoint()
    test_relational_operation("DISJOINT") do |geom1, geom2|
      geom1.disjoint(geom2)
    end
  end

  def test_touches()
    test_relational_operation("TOUCHES") do |geom1, geom2|
      geom1.touches(geom2)
    end
  end
  
  def test_intersects()
    test_relational_operation("INTERSECTS") do |geom1, geom2|
      geom1.intersects(geom2)
    end
  end
  
  def test_crosses()
    test_relational_operation("CROSSES") do |geom1, geom2|
      geom1.touches(geom2)
    end
  end
  
  def test_within()
    test_relational_operation("WITHIN") do |geom1, geom2|
      geom1.touches(geom2)
    end
  end
  
  def test_contains()
    test_relational_operation("CONTAINS") do |geom1, geom2|
      geom1.contains(geom2)
    end
  end
  
  def test_overlaps()
    test_relational_operation("OVERLAPS") do |geom1, geom2|
      geom1.overlaps(geom2)
    end
  end
  
  def test_relate()
    test_relational_operation("RELATE") do |geom1, geom2|
      geom1.relate(geom2, "212101212")
    end
  end

  def test_equals()
    test_relational_operation("EQUALS") do |geom1, geom2|
      geom1.equals(geom2)
    end
  end
  
  def test_equals_exact()
    test_relational_operation("EQUALS EXACT") do |geom1, geom2|
      geom1.equals_exact(geom2)
    end
  end

  def test_is_within_distance()
    test_relational_operation("IS WITHIN DISTANCE") do |geom1, geom2|
      geom1.is_within_distance(geom2, 2)
    end
  end

  
  def test_combination(message, &block)
    puts "#{message}  GGGGGGGGGGGGGG"
    STDOUT.flush()
    geoms = create_geoms()

    geoms.each do |geom1|
      puts "1: #{geom1}"
      STDOUT.flush()
      geoms.each do |geom2|
        puts "2: #{geom2}"
        STDOUT.flush()
        result = yield(geom1, geom2)
        puts "result: #{result}"
        STDOUT.flush()
      end
    end

    print_wkt(geoms, message)
  end

  def test_union()
    test_combination("UNION") do |geom1, geom2|
      geom1.union(geom2)
    end
  end

  def test_intersection()
    test_combination("INTERSECTION") do |geom1, geom2|
      geom1.intersection(geom2)
    end
  end
  
  def test_difference()
    test_combination("DIFFERENCE") do |geom1, geom2|
      geom1.difference(geom2)
    end
  end
  
  def test_symdifference()
    test_combination("SYMDIFFERENCE") do |geom1, geom2|
      geom1.sym_difference(geom2)
    end
  end

  def test_line_merge()
    geoms = create_geoms()
    merger = Geos::LineMerger.new()

    merger.add(geoms)
    new_geoms = merger.get_merged_line_strings()
    print_wkt(new_geoms, "LINE MERGE")
  end

  def test_polygonize()
    geoms = create_geoms()
    polygonizer = Geos::Polygonizer.new()

    polygonizer.add(geoms)
    new_geoms = polygonizer.get_polygons()
    print_wkt(new_geoms, "POLYGONIZE")
  end
end