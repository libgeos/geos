#!/usr/bin/env ruby

require 'geos'
require "rexml/document"

XmlTest = Struct.new(:case_id, :test_id,
                     :operation, :expected,
                     :geom1, :geom2, 
                     :arg1, :arg2, :arg3)

class GeosTestHelper

  def create_point(x, y)
    cs = Geos::CoordinateSequence.new(1,2)
    cs.set_x(0, x)
    cs.set_y(0, y)
    Geos.create_point(cs)
  end
  
  def create_ushaped_linestring(xoffset, yoffset, side)
    cs = Geos::CoordinateSequence.new(4,2)
    cs.set_x(0, xoffset)
    cs.set_y(0, yoffset)
    cs.set_x(1, xoffset)
    cs.set_y(1, yoffset + side)
    cs.set_x(2, xoffset + side)
    cs.set_y(2, yoffset + side)
    cs.set_x(3, xoffset + side)
    cs.set_y(3, yoffset)
    Geos.create_line_string(cs)
  end
    
  # This function will create a LinearRing geometry
  # representing a square with the given origin and side 
  def create_square_linear_ring(xoffset, yoffset, side)
    cs = Geos::CoordinateSequence.new(5,2)
    cs.set_x(0, xoffset)
    cs.set_y(0, yoffset)
    cs.set_x(1, xoffset)
    cs.set_y(1, yoffset + side)
    cs.set_x(2, xoffset + side)
    cs.set_y(2, yoffset + side)
    cs.set_x(3, xoffset + side)
    cs.set_y(3, yoffset)
    cs.set_x(4, xoffset)
    cs.set_y(4, yoffset)
    Geos.create_linear_ring(cs)
  end

  # This function will create a Polygon
  # geometry representing a square with the given origin 
  # and side and with a central hole 1/3 sided.
  def create_square_polygon(xoffset, yoffset, side)
    # We need a LinearRing for the polygon shell 
    outer = create_square_linear_ring(xoffset,yoffset,side);

    # And another for the hole 
    inner = create_square_linear_ring(xoffset+(side/3), yoffset+(side/3),(side/3));
  
    Geos.create_polygon(outer, [inner])
  end
  
  def create_simple_collection(geoms)
    # This function creates a GeometryCollection
    # containing copies of all Geometries in given vector.
    #@geom_factory.create_geometry_collection(geoms)
    wkt = "GEOMETRYCOLLECTION (POLYGON ((0.00 250.00, 0.00 550.00, 300.00 550.00, 300.00 250.00, 0.00 250.00), (100.00 350.00, 200.00 350.00, 200.00 450.00, 100.00 450.00, 100.00 350.00)), POLYGON ((0.00 200.00, 0.00 500.00, 300.00 500.00, 300.00 200.00, 0.00 200.00), (100.00 300.00, 200.00 300.00, 200.00 400.00, 100.00 400.00, 100.00 300.00)), LINEARRING (0.00 0.00, 0.00 100.00, 100.00 100.00, 100.00 0.00, 0.00 0.00), LINEARRING (0.00 0.00, 0.00 100.00, 100.00 100.00, 100.00 0.00, 0.00 0.00), LINESTRING (60.00 60.00, 60.00 160.00, 160.00 160.00, 160.00 60.00), POINT (150.00 350.00))"
    reader = Geos::WktReader.new
    reader.read(wkt)
  end

  def create_circle(centerX, centerY, radius)
    # Use a GeometricShapeFactory to render
    # a circle having the specified center and radius

    #shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    #shapefactory.set_centre(Geos::Coordinate.new(centerX, centerY))
    #shapefactory.set_size(radius);
    # same as:
    #   shapefactory.set_height(radius)
    #   shapefactory.set_width(radius)
    
    #shapefactory.create_circle()
    wkt = "POLYGON ((-5.00 0.00, -4.99 0.31, -4.96 0.63, -4.91 0.94, -4.84 1.24, -4.76 1.55, -4.65 1.84, -4.52 2.13, -4.38 2.41, -4.22 2.68, -4.05 2.94, -3.85 3.19, -3.64 3.42, -3.42 3.64, -3.19 3.85, -2.94 4.05, -2.68 4.22, -2.41 4.38, -2.13 4.52, -1.84 4.65, -1.55 4.76, -1.24 4.84, -0.94 4.91, -0.63 4.96, -0.31 4.99, 0.00 5.00, 0.31 4.99, 0.63 4.96, 0.94 4.91, 1.24 4.84, 1.55 4.76, 1.84 4.65, 2.13 4.52, 2.41 4.38, 2.68 4.22, 2.94 4.05, 3.19 3.85, 3.42 3.64, 3.64 3.42, 3.85 3.19, 4.05 2.94, 4.22 2.68, 4.38 2.41, 4.52 2.13, 4.65 1.84, 4.76 1.55, 4.84 1.24, 4.91 0.94, 4.96 0.63, 4.99 0.31, 5.00 0.00, 4.99 -0.31, 4.96 -0.63, 4.91 -0.94, 4.84 -1.24, 4.76 -1.55, 4.65 -1.84, 4.52 -2.13, 4.38 -2.41, 4.22 -2.68, 4.05 -2.94, 3.85 -3.19, 3.64 -3.42, 3.42 -3.64, 3.19 -3.85, 2.94 -4.05, 2.68 -4.22, 2.41 -4.38, 2.13 -4.52, 1.84 -4.65, 1.55 -4.76, 1.24 -4.84, 0.94 -4.91, 0.63 -4.96, 0.31 -4.99, -0.00 -5.00, -0.31 -4.99, -0.63 -4.96, -0.94 -4.91, -1.24 -4.84, -1.55 -4.76, -1.84 -4.65, -2.13 -4.52, -2.41 -4.38, -2.68 -4.22, -2.94 -4.05, -3.19 -3.85, -3.42 -3.64, -3.64 -3.42, -3.85 -3.19, -4.05 -2.94, -4.22 -2.68, -4.38 -2.41, -4.52 -2.13, -4.65 -1.84, -4.76 -1.55, -4.84 -1.24, -4.91 -0.94, -4.96 -0.63, -4.99 -0.31, -5.00 0.00))"
    reader = Geos::WktReader.new
    reader.read(wkt)
  end

  def create_ellipse(centerX, centerY, width, height)
    # Use a GeometricShapeFactory to render
    # a circle having the specified center and radius

    #shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    #shapefactory.set_centre(Geos::Coordinate.new(centerX, centerY))
    #shapefactory.set_height(width)
    #shapefactory.set_width(height)
    
    #shapefactory.create_circle()
    wkt = "POLYGON ((-4.00 0.00, -3.99 0.38, -3.97 0.75, -3.93 1.12, -3.87 1.49, -3.80 1.85, -3.72 2.21, -3.62 2.55, -3.51 2.89, -3.38 3.21, -3.24 3.53, -3.08 3.82, -2.92 4.11, -2.74 4.37, -2.55 4.62, -2.35 4.85, -2.14 5.07, -1.93 5.26, -1.70 5.43, -1.47 5.58, -1.24 5.71, -0.99 5.81, -0.75 5.89, -0.50 5.95, -0.25 5.99, 0.00 6.00, 0.25 5.99, 0.50 5.95, 0.75 5.89, 0.99 5.81, 1.24 5.71, 1.47 5.58, 1.70 5.43, 1.93 5.26, 2.14 5.07, 2.35 4.85, 2.55 4.62, 2.74 4.37, 2.92 4.11, 3.08 3.82, 3.24 3.53, 3.38 3.21, 3.51 2.89, 3.62 2.55, 3.72 2.21, 3.80 1.85, 3.87 1.49, 3.93 1.12, 3.97 0.75, 3.99 0.38, 4.00 0.00, 3.99 -0.38, 3.97 -0.75, 3.93 -1.12, 3.87 -1.49, 3.80 -1.85, 3.72 -2.21, 3.62 -2.55, 3.51 -2.89, 3.38 -3.21, 3.24 -3.53, 3.08 -3.82, 2.92 -4.11, 2.74 -4.37, 2.55 -4.62, 2.35 -4.85, 2.14 -5.07, 1.93 -5.26, 1.70 -5.43, 1.47 -5.58, 1.24 -5.71, 0.99 -5.81, 0.75 -5.89, 0.50 -5.95, 0.25 -5.99, -0.00 -6.00, -0.25 -5.99, -0.50 -5.95, -0.75 -5.89, -0.99 -5.81, -1.24 -5.71, -1.47 -5.58, -1.70 -5.43, -1.93 -5.26, -2.14 -5.07, -2.35 -4.85, -2.55 -4.62, -2.74 -4.37, -2.92 -4.11, -3.08 -3.82, -3.24 -3.53, -3.38 -3.21, -3.51 -2.89, -3.62 -2.55, -3.72 -2.21, -3.80 -1.85, -3.87 -1.49, -3.93 -1.12, -3.97 -0.75, -3.99 -0.38, -4.00 0.00))"
    reader = Geos::WktReader.new
    reader.read(wkt)
  end

  def create_rectangle(llX, llY, width, height)
    # This function uses GeometricShapeFactory to render
    # a rectangle having lower-left corner at given coordinates
    # and given sizes.

    #shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    #shapefactory.set_base(Geos::Coordinate.new(llX, llY))
    #shapefactory.set_height(height)
    #shapefactory.set_width(width)
    
    # we don't need more then 4 points for a rectangle...
    #shapefactory.set_num_points(4)
    # can use setSize for a square
    #shapefactory.create_rectangle()
    wkt = "POLYGON ((#{llX} #{llY}, #{llX} #{llY + height}, #{llX+width} #{llY+height}, #{llX+width} #{llY}, #{llX} #{llY}))"
    reader = Geos::WktReader.new
    reader.read(wkt)
  end
  
  def create_arc(llX, llY, width, height, startang, endang)
    # This function uses GeometricShapeFactory to render
    # an arc having lower-left corner at given coordinates,
    # given sizes and given angles. 

    #shapefactory = Geos::GeometricShapeFactory.new(@geom_factory)
    #shapefactory.set_base(Geos::Coordinate.new(llX, llY))
    #shapefactory.set_height(height)
    #shapefactory.set_width(width)
    # the default (100 pts)
    #shapefactory.set_num_points(100)
    
    #shapefactory.create_arc(startang, endang)
    wkt = "LINESTRING (5.00 20.00, 5.00 20.00, 5.00 20.00, 5.50 20.00, 5.50 20.00, 5.50 20.00, 5.50 20.00, 5.50 20.00, 5.50 20.00, 6.00 20.00, 6.00 20.00, 6.00 20.00, 6.00 20.00, 6.00 20.00, 6.00 19.50, 6.00 19.50, 6.50 19.50, 6.50 19.50, 6.50 19.50, 6.50 19.50, 6.50 19.50, 6.50 19.50, 7.00 19.50, 7.00 19.50, 7.00 19.00, 7.00 19.00, 7.00 19.00, 7.00 19.00, 7.00 19.00, 7.50 19.00, 7.50 19.00, 7.50 19.00, 7.50 18.50, 7.50 18.50, 7.50 18.50, 7.50 18.50, 7.50 18.50, 8.00 18.50, 8.00 18.00, 8.00 18.00, 8.00 18.00, 8.00 18.00, 8.00 18.00, 8.00 17.50, 8.00 17.50, 8.50 17.50, 8.50 17.50, 8.50 17.50, 8.50 17.00, 8.50 17.00, 8.50 17.00, 8.50 17.00, 8.50 16.50, 9.00 16.50, 9.00 16.50, 9.00 16.50, 9.00 16.50, 9.00 16.00, 9.00 16.00, 9.00 16.00, 9.00 16.00, 9.00 15.50, 9.00 15.50, 9.00 15.50, 9.50 15.00, 9.50 15.00, 9.50 15.00, 9.50 15.00, 9.50 14.50, 9.50 14.50, 9.50 14.50, 9.50 14.50, 9.50 14.00, 9.50 14.00, 9.50 14.00, 9.50 13.50, 9.50 13.50, 9.50 13.50, 9.50 13.00, 10.00 13.00, 10.00 13.00, 10.00 13.00, 10.00 12.50, 10.00 12.50, 10.00 12.50, 10.00 12.00, 10.00 12.00, 10.00 12.00, 10.00 11.50, 10.00 11.50, 10.00 11.50, 10.00 11.50, 10.00 11.00, 10.00 11.00, 10.00 11.00, 10.00 10.50, 10.00 10.50, 10.00 10.50, 10.00 10.00, 10.00 10.00)"
    reader = Geos::WktReader.new
    reader.read(wkt)
  end
  
  def create_geoms
    geoms = Array.new

    geoms.push(create_point(150, 350))
    geoms.push(create_square_linear_ring(0,0,100))
    geoms.push(create_ushaped_linestring(60,60,100))
    geoms.push(create_square_linear_ring(0,0,100))
    geoms.push(create_square_polygon(0,200,300))
    geoms.push(create_square_polygon(0,250,300))
    geoms.push(create_simple_collection(geoms))

    # These ones use a GeometricShapeFactory
    geoms.push(create_circle(0, 0, 10))
    geoms.push(create_ellipse(0, 0, 8, 12))
    # A square
    geoms.push(create_rectangle(-5, -5, 10, 10))
    # A rectangle
    geoms.push(create_rectangle(-5, -5, 10, 20))
    # The upper-right quarter of a vertical ellipse
    geoms.push(create_arc(0, 0, 10, 20, 0, Math::PI/2))

    return geoms
  end
  
  def print_geoms(geoms)
    geoms.each do |geom|
      wkt = Geos.geom_to_wkt(geom)
      STDOUT << wkt << "\n"
    end
  end 
  
  # ----- xml support ---------
  def load_tests(file_name) 
    current_path = File.expand_path(__FILE__)
    xml_path = File.join(current_path, "..", "..", "..", "..", "tests", "xmltester")
    file_path = File.expand_path(File.join(xml_path, file_name))
    
    file = File.new(file_path)
    result = nil
    begin
      doc = REXML::Document.new(file)
      result = load_cases(doc)
    ensure
      file.close
    end
    result 
  end
  
  def parse_geometry(value)
    value.strip!
    
    if value.match(/\A[0-9A-F]/)
      reader = Geos::WkbReader.new
      reader.read_hex(value)
    else
      reader = Geos::WktReader.new
      reader.read(value)
    end
  end
  
  def load_cases(doc)
    case_id = 1
    result = Array.new
    
    # get the run element
    run_element = doc.elements["run"]
    
    # Iterate over each case
    run_element.elements.each("case") do |case_element|
      test_id = 1
      # Get geometry a
      a_element = case_element.elements["a"]
      geom1 = parse_geometry(a_element.text) if a_element

      # Get geometry b
      b_element = case_element.elements["b"]
      geom2 = parse_geometry(b_element.text) if b_element

      # Iterate over each test in this case
      case_element.elements.each("test") do |test_element|
        operation_element = test_element.elements["op"]

        # Get operation name and result
        operation = operation_element.attributes['name'].strip!
        expected = operation_element.text.strip!
        
        # Figure arguments
        arg1 = operation_element.attributes['arg1']
        arg2 = operation_element.attributes['arg2']
        arg3 = operation_element.attributes['arg3']
        
        arg1.strip! if arg1
        arg2.strip! if arg2
        arg3.strip! if arg3
        
        # Create test
        test = XmlTest.new(case_id, test_id,
                           operation, expected,
                           geom1, geom2,
                           arg1, arg2, arg3)
        result.push(test)
        test_id += 1
      end      
      case_id += 1
    end  
    result
  end
end 