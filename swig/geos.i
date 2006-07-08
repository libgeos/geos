/* =========================================================================
 * $Id$
 *
 * geos.i
 * 
 * Copyright 2004 Sean Gillies, sgillies@frii.com
 * Updated 2005 Charlie Savage, cfis@interserv.com
 *
 * Interface for a SWIG generated geos module.  Depends on 
 * SWIG 1.3.29 and higher.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */

%module geos
%include "exception.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_except.i"
%include "factory.i"

%{ 
#include "geos.h"
#include "geos/version.h"
#include "geos/planargraph.h"
#include "geos/opLinemerge.h"
#include "geos/opPolygonize.h"
#include <sstream>
%}

/* ================= Shadowed Methods ============== */

/* The overloaded C++ methods cannot be disambiguated by
   SWIG.  For example:
   
	geos::GeometryFactory::createPoint(CoordinateSequence *) const;
	geos::GeometryFactory::createPoint(const CoordinateSequence&) const;
   
   For each pair of methods, one will take ownership
   of the parameter (thus it is passed as a pointer) while
   the other one will not (thus is passed as a const reference).
   To be on the safe-side, we tell SWIG to ignore the method
   that takes over ownership.  To expose these ignored methods
   to a scripting language requires renaming the method to
   something else.  This can be done in each language module
   as it is loaded below.*/

%ignore geos::GeometryFactory::buildGeometry(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createGeometryCollection(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createLinearRing(CoordinateSequence *) const;
%ignore geos::GeometryFactory::createLineString(CoordinateSequence *) const;
%ignore geos::GeometryFactory::createMultiLineString(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createMultiPoint(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createMultiPolygon(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createPoint(CoordinateSequence *) const;
%ignore geos::GeometryFactory::createPolygon(LinearRing *,vector<Geometry * > *) const;

/* For scripting languages the CoordinateSequence::add method is ambigious
   since there are two overloaded versions that are the same except
   a const declaration.  However, one of them is available only for 
   backwards compatibility so hide that one.*/
%ignore geos::CoordinateSequence::add(CoordinateSequence *cl,bool allowRepeated,bool direction);


/* ================= Typemaps ============== */

/* These typemaps allows scripting languages to call methods that take
   iostreams and ostreams, such as WKTWriter.printHEX and WKBWriter.write. 
   The results are returned from the method as a string object in the
   scripting language.*/
   
/* Get rid of the ostream parameter by setting numinputs to 0 */   
%typemap(in,numinputs=0) ostream& (std::ostringstream stream)
{
  $1 = &stream;
}

/* Return the results as a string object in the scripting language */
%typemap(argout) ostream& 
{
	std::string str = stream$argnum.str();	
	$result = SWIG_FromCharPtrAndSize(str.data(), str.size());
}

/* This typemap allows the scripting language to pass in a string
   which will be converted to an istringstream for GEOS */
%typemap(in) istream& (char *buf = 0, size_t size = 0, int alloc = 0, std::stringstream stream)
{
  /* Convert from scripting language string to char* */
  if (SWIG_AsCharPtrAndSize($input, &buf, &size, &alloc) != SWIG_OK)
  {
    %argument_fail(SWIG_TypeError, "(TYPEMAP, SIZE)", $symname, $argnum);
  }
  
  /* Write data to the stream.  Note that the returned size includes
     the last character, which is a null character.  We do not want
     to write that to the stream so subtract one from its size. */
  stream.write(buf, size - 1);

  $1 = &stream;
}

/* Free allocated buffer created in the (in) typemap */
%typemap(freearg) istream&
{
  if (alloc$argnum == SWIG_NEWOBJ) %delete_array(buf$argnum);
}


/* ==============  Language Specific Files ============ */

/* Import language specific SWIG files.  This allows each language
   to define its own renames as well as any special functionality
   such as language specific iterators for collections. Note 
   that %include allows the included files to generate interface
   wrapper code while %import does not.  Thus use %include since
   this is an important feature (for example, Ruby needs it to #undef
   the select macro) */


#ifdef SWIGPYTHON
%include ../python/python.i
#endif

#ifdef SWIGRUBY
%include ../ruby/ruby.i
#endif


/* ================= Ownership Rules ============== */

/* These disown definitions are not correct in all cases - 
   needs to be fixed */
%apply SWIGTYPE *DISOWN { geos::CoordinateSequence * };
%apply SWIGTYPE *DISOWN { geos::LinearRing * };
%apply SWIGTYPE *DISOWN { std::vector<geos::Geometry * > * };
%apply SWIGTYPE *DISOWN { std::vector<geos::Coordinate> * };


// These methods create new objects
namespace geos {

%newobject *::clone;
%newobject *::getCoordinates;

%newobject CoordinateArraySequence::create;
%newobject GeometryFactory::createPointFromInternalCoord;
%newobject GeometryFactory::toGeometry;
%newobject GeometryFactory::createPoint;
%newobject GeometryFactory::createGeometryCollection;
%newobject GeometryFactory::createMultiLineString;
%newobject GeometryFactory::createMultiPolygon;
%newobject GeometryFactory::createLinearRing;
%newobject GeometryFactory::createMultiPoint;
%newobject GeometryFactory::createPolygon;
%newobject GeometryFactory::createLineString;
%newobject GeometryFactory::buildGeometry;
%newobject GeometryFactory::createGeometry;
%newobject GeometricShapeFactory::getEnvelope;

%newobject WKBReader::read;
%newobject WKBReader::readHEX;
%newobject WKTReader::read;
}

/* Surface methods that return Geometry* so that they
	 return the actual geometry object (point, linestring, etc.)
	 instead of a generic geometry object.  Make sure the checks
	 go from child class to parent class so we get the right one.*/
%factory(geos::Geometry * geos::WKTReader::read, geos::Point, 
				 geos::LinearRing, geos::LineString,
				 geos::Polygon, 
				 geos::MultiPoint, geos::MultiLineString,
				 geos::MultiPolygon, geos::GeometryCollection);

%factory(geos::Geometry * geos::WKBReader::read, geos::Point, 
				 geos::LinearRing, geos::LineString,
				 geos::Polygon, 
				 geos::MultiPoint, geos::MultiLineString,
				 geos::MultiPolygon, geos::GeometryCollection);

%factory(geos::Geometry * geos::WKBReader::readHEX, geos::Point, 
				 geos::LinearRing, geos::LineString,
				 geos::Polygon, 
				 geos::MultiPoint, geos::MultiLineString,
				 geos::MultiPolygon, geos::GeometryCollection);

%factory(geos::Geometry * geos::Geometry::downcast, geos::Point, 
				 geos::LinearRing, geos::LineString,
				 geos::Polygon, 
				 geos::MultiPoint, geos::MultiLineString,
				 geos::MultiPolygon, geos::GeometryCollection);

%newobject geos::Geometry::fromWKT;
%factory(geos::Geometry * geos::Geometry::fromWKT, geos::Point, 
				 geos::LinearRing, geos::LineString,
				 geos::Polygon, 
				 geos::MultiPoint, geos::MultiLineString,
				 geos::MultiPolygon, geos::GeometryCollection);

%newobject geos::Geometry::fromHEX;
%factory(geos::Geometry * geos::Geometry::fromHEX, geos::Point, 
				 geos::LinearRing, geos::LineString,
				 geos::Polygon, 
				 geos::MultiPoint, geos::MultiLineString,
				 geos::MultiPolygon, geos::GeometryCollection);

/* ================= Operators  ============== */
/* Ignore these for now - if we don't SWIG will create
   names with the same methods, which doesn't work out very well.

	 std::ostream& operator<<(std::ostream& os, const planarNode& n); 
   std::ostream& operator<<(std::ostream& os, const planarEdge& n); 
   std::ostream& operator<<(std::ostream& os, const planarNode& n); 
   std::ostream& operator<< (std::ostream& os, const Coordinate& c);
   std::ostream& operator<< (std::ostream& o, const LineSegment& l);
   std::ostream& operator<< (std::ostream& os, const Coordinate& c); */
%ignore geos::operator<<;


/* ================= Exception Handling  ============== */

/* Mark these classes as exception classes */
%exceptionclass geos::GEOSException;

/* These are all subclasses of GEOSException */
%exceptionclass geos::AssertionFailedException;
%exceptionclass geos::IllegalArgumentException;
%exceptionclass geos::ParseException;
%exceptionclass geos::TopologyException;
%exceptionclass geos::UnsupportedOperationException;

/* This exception class is not surfaced to SWIG 
%exceptionclass geos::NotRepresentableException;*/

/* Setup up generalized exception handling.  Note that GEOS
   2 series throws classes that are allocated on the heap 
	 so we need to catch them by pointer and are responsible
	 for freeing them.  To do this we'll just pass the object 
	 along the scripting language (wrapped of course) and make
	 it responsbile for freeing the object via the
   use of the SWIG_POINTER_OWN flag.*/

%exception {
    try {
        $action
    }
  catch (geos::AssertionFailedException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__AssertionFailedException, SWIG_POINTER_OWN), "geos::AssertionFailedException", SWIGTYPE_p_geos__AssertionFailedException);
  }
  catch (geos::IllegalArgumentException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__IllegalArgumentException, SWIG_POINTER_OWN), "geos::IllegalArgumentException", SWIGTYPE_p_geos__IllegalArgumentException);
  }
  catch (geos::ParseException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__ParseException, SWIG_POINTER_OWN), "geos::ParseException", SWIGTYPE_p_geos__ParseException);
  }
  catch (geos::TopologyException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__TopologyException, SWIG_POINTER_OWN), "geos::TopologyException", SWIGTYPE_p_geos__TopologyException);
  }
  catch (geos::UnsupportedOperationException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__UnsupportedOperationException, SWIG_POINTER_OWN), "geos::UnsupportedOperationException", SWIGTYPE_p_geos__UnsupportedOperationException);
  }
    catch (geos::GEOSException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__GEOSException, SWIG_POINTER_OWN), "geos::GEOSException", SWIGTYPE_p_geos__GEOSException);
  }
  catch (...) {
      SWIG_exception(SWIG_RuntimeError, "Unknown exception took place in the method: $symname.");
    }
}


/* =============  Define Attributes ============ */
%include <attribute.i>
%attribute(geos::Point, double, x, getX);
%attribute(geos::Point, double, y, getY);
%attribute(geos::Geometry, double, srid, getSRID, setSRID);


/* ===========  Headers To Wrap  ================ */
%include "../../source/headers/geos/geom.h"
%include "../../source/headers/geos/version.h"
%include "../../source/headers/geos/util.h"
%include "../../source/headers/geos/io.h"

/* ===========  Global Factory  ================ */
/* When creating geometries GEOS needs to use a factory.
   The created geometries have back references to the
	 factory and thus the factory should not be deleted.
	 However, this becomes hard when unmarshalling serialized
	 objects.  So create a global factory.*/

/* Add some helper methods to geometries */
%extend geos::Geometry {
	static geos::GeometryFactory& factory()
	{
		static geos::GeometryFactory factory;
		return factory;
	}

	static geos::WKBReader& reader()
	{
		#if GEOS_VERSION_MAJOR == 2
		static geos::WKBReader reader(geos_Geometry_factory());
		#else
		static geos::io::WKBReader reader(geos_Geometry_factory());
		#endif
		return reader;
	}

	static Geometry* fromWKT(std::string wkt)
	{
		std::stringstream in(wkt);
		return geos_Geometry_reader().read(in);
	}
	
	static Geometry* fromHEX(std::string hex)
	{
		std::stringstream in(hex);
		return geos_Geometry_reader().readHEX(in);
	}
};
