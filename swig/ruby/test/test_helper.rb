#!/usr/bin/env ruby

require 'geos'

class GeosTestHelper
  attr_reader :geom_factory
  
  def initialize
    # Define a precision model using 0,0 as the reference origin
    # and 2.0 as coordinates scale.
    pm = Geos::PrecisionModel.new(2.0, 0, 0)
    @geom_factory = Geos::GeometryFactory.new(pm, -1)
  end
  
  def create_coord(x, y)
    Geos::Coordinate.new(x, y)
  end

  def create_point(x, y)
    coord = create_coord(x, y)
    @geom_factory.create_point(coord)
  end
  
  def create_ushaped_linestring(xoffset, yoffset, side)
    # We will use a coordinate list to build the linestring
    cl = Geos::CoordinateArraySequence.new()
    cl.add(Geos::Coordinate.new(xoffset, yoffset))
    cl.add(Geos::Coordinate.new(xoffset, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset))

    # Now that we have a CoordinateSequence we can create the linestring.
    # The newly created LineString will take ownership of the CoordinateSequence.
    @geom_factory.create_line_string!(cl)

    # This is what you do if you want the new LineString
    # to make a copy of your CoordinateSequence:
    @geom_factory.create_line_string(cl)
  end
    
  # This function will create a LinearRing geometry
  # representing a square with the given origin and side 
  def create_square_linearring(xoffset, yoffset, side)
    # We will use a coordinate list to build the linearring
    cl = Geos::CoordinateArraySequence.new()
    cl.add(Geos::Coordinate.new(xoffset, yoffset))
    cl.add(Geos::Coordinate.new(xoffset, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset+side))
    cl.add(Geos::Coordinate.new(xoffset+side, yoffset))
    cl.add(Geos::Coordinate.new(xoffset, yoffset))

    # Create the line string The newly created LinearRing will
    # take ownership of the CoordinateSequence.
    @geom_factory.create_linear_ring!(cl)

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
    # @geom_factory.create_geometry_collection!(geoms)

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

    geoms.push(create_point(150, 350))
    geoms.push(create_ushaped_linestring(60,60,100))
    geoms.push(create_square_linearring(0,0,100))
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
end