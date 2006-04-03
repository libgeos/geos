/* =========================================================================
 * $Id$
 *
 * ruby.i
 * 
 * Copyright 2005 Charlie Savage, cfis@interserv.com
 *
 * Interface for a SWIG generated geos module.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */


// Manually rename methods on vector to get around SWIG 1.3.29 bug
%rename(__len__) std::vector<geos::geom::Geometry *>::size;
%rename("empty?") std::vector<geos::geom::Geometry *>::empty;
%rename(push) std::vector<geos::geom::Geometry *>::push_back;

%rename(__len__) std::vector<geos::geom::LineString *>::size;
%rename("empty?") std::vector<geos::geom::LineString *>::empty;
%rename(push) std::vector<geos::geom::LineString *>::push_back;

%rename(__len__) std::vector<geos::geom::Polygon *>::size;
%rename("empty?") std::vector<geos::geom::Polygon *>::empty;
%rename(push) std::vector<geos::geom::Polygon *>::push_back;

// Now define the containers
%template("GeometryVector") std::vector<geos::geom::Geometry *>;
%template("LineStringVector") std::vector<geos::geom::LineString *>;
%template("PolygonVector") std::vector<geos::geom::Polygon *>;


namespace geos {
namespace geom {
	/* Next conflicts with a Ruby keyword */
	%rename(next_) GeometryCollectionIterator::next();

	/* Need to deal with these ignores by renaming them */
	%rename("build_geometry!") GeometryFactory::buildGeometry(vector<Geometry * > *) const;
	%rename("create_geometry_collection!") GeometryFactory::createGeometryCollection(vector<Geometry * > *) const;
	%rename("create_linear_ring!") GeometryFactory::createLinearRing(CoordinateSequence *) const;
	%rename("create_line_string!") GeometryFactory::createLineString(CoordinateSequence *) const;
	%rename("create_multi_line_string!") GeometryFactory::createMultiLineString(vector<Geometry * > *) const;
	%rename("create_multi_point!") GeometryFactory::createMultiPoint(vector<Geometry * > *) const;
	%rename("create_multi_polygon!") GeometryFactory::createMultiPolygon(vector<Geometry * > *) const;
	%rename("create_point!") GeometryFactory::createPoint(CoordinateSequence *) const;
	%rename("create_polygon!") GeometryFactory::createPolygon(LinearRing *,vector<Geometry * > *) const;

	%extend Coordinate {
		std::string to_s()
		{
			std::ostringstream os;
			os << "<Coordinate ";
			os << self->toString();
			os << ">";
			return os.str();
		}
	};

	%extend CoordinateSequence {
		std::string to_s()
		{
			std::ostringstream os;
			os << "<CoordinateSequence ";
			os << self->toString();
			os << ">";
			return os.str();
		}
	};

	%extend Envelope {
		std::string to_s()
		{
			std::ostringstream os;
			os << "<Envelope ";
			os << self->toString();
			os << ">";
			return os.str();
		}
	};


	%extend Geometry {
		std::string to_s()
		{
			std::ostringstream os;
			os << "<Geometry ";
			os << self->toString();
			os << ">";
			return os.str();
		}
	};

	/* Geos uses vectors of pointers to pass around geometries.  These will be 
		 wrapped by SWIG - but we have to be careful.  The problem is
		 if we put a C++ object into the vector that a Ruby object owns, when
		 that Ruby object goes out of scope the C++ object will be freed.  Thus
		 the pointer inside the vector becomes corrupt.  To stop this from happening
		 we have to implement a Ruby mark function, which in turn requires turning
		 on SWIG Ruby Object tracking for these containers.*/

	// First declare mark functions for the containers
	%markfunc std::vector<Geometry *> "mark_GeometryVector";
	%markfunc std::vector<LineString *> "mark_LineStringVector";
	%markfunc std::vector<Polygon *> "mark_PolygonVector";

	// Now track the objects that go go into the containers, which
	// is Geometry or any class inherited from Geometry
	%trackobjects Geometry;
	%trackobjects Point;
	%trackobjects LineString;
	%trackobjects LinearRing;
	%trackobjects Polygon;
	%trackobjects GeometryCollection;
	%trackobjects MultiPoint;
	%trackobjects MultiLineString;
	%trackobjects MultiPolygon;
} /* End geom namespace */
} /* End geos namespace */


// Last define the mark functions
%header %{
namespace geos {
namespace geom {
	static void mark_GeometryVector(void* ptr)
	{
		typedef std::vector<Geometry *> GeometryVector;
		typedef GeometryVector::iterator GeometryVectorIter;
	  
		GeometryVector *vec = reinterpret_cast<GeometryVector*>(ptr);
		GeometryVectorIter iter = vec->begin();
		GeometryVectorIter last = vec->end();
	  
		for(; iter != last; ++iter)
		{
			Geometry *geometry = *iter;
			VALUE object = SWIG_RubyInstanceFor(geometry);
			if (object != Qnil)
			{
				rb_gc_mark(object);
			}
			else
			{
				// This should not happen
				rb_raise(rb_eRuntimeError, "Unknown object stored in vector");
			}
		}
	}

	static void mark_LineStringVector(void* ptr)
	{
		typedef std::vector<LineString *> LineStringVector;
		typedef LineStringVector::iterator LineStringVectorIter;
	  
		LineStringVector *vec = reinterpret_cast<LineStringVector*>(ptr);
		LineStringVectorIter iter = vec->begin();
		LineStringVectorIter last = vec->end();
	  
		for(; iter != last; ++iter)
		{
			LineString *geometry = *iter;
			VALUE object = SWIG_RubyInstanceFor(geometry);
			if (object != Qnil)
			{
				rb_gc_mark(object);
			}
			else
			{
				// This should not happen
				rb_raise(rb_eRuntimeError, "Unknown object stored in vector");
			}
		}
	}

	static void mark_PolygonVector(void* ptr)
	{
		typedef std::vector<Polygon *> PolygonVector;
		typedef PolygonVector::iterator PolygonVectorIter;
	  
		PolygonVector *vec = reinterpret_cast<PolygonVector*>(ptr);
		PolygonVectorIter iter = vec->begin();
		PolygonVectorIter last = vec->end();
	  
		for(; iter != last; ++iter)
		{
			Polygon *geometry = *iter;
			VALUE object = SWIG_RubyInstanceFor(geometry);
			if (object != Qnil)
			{
				rb_gc_mark(object);
			}
			else
			{
				// This should not happen
				rb_raise(rb_eRuntimeError, "Unknown object stored in vector");
			}
		}
	}
} /* End geom namespace */
} /* End geos namespace */
%}


/* ======== Marshalling Support ========== */
/*%extend Geometry {
	std::string marshal_dump()
	{
		std::stringstream os(std::ios_base::binary|std::ios_base::in|std::ios_base::out);
		WKBWriter wkbWriter;
		const Geometry& geom = *self;
		wkbWriter.write(geom, os);
		return os.str();
	}
};

%insert("wrapper")
{
	VALUE geometry_marshal_load(int argc, VALUE *argv, VALUE self)
	{
		std::string *ptr = (std::string *)0;
	  
		if ((argc < 1) || (argc > 1)) {
			rb_raise(rb_eArgError, "wrong # of arguments(%d for 1)",argc);
		}

		int res = SWIG_AsPtr_std_string(argv[0], &ptr);

		if (!SWIG_IsOK(res) || !ptr) {
			rb_raise(rb_eArgError, "Invalid string sent to marhsal_load");
		}

		std::stringstream in(*ptr);
		Geometry* geom = geos_geom_Geometry_reader().read(in);

		DATA_PTR(self) = geom;
	  
		return self;
	}
}
*/