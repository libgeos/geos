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


// ===  Alias ===
%alias GeosGeometry::getSRID "srid";
%alias GeosGeometry::setSRID "srid=";

// Some manual renames
%rename("get_wkb_output_dimensions") getWKBOutputDims;
%rename("set_wkb_output_dimensions") setWKBOutputDims;
%rename("get_wkb_byte_order") getWKBByteOrder;
%rename("set_wkb_byte_order") setWKBByteOrder;
%rename("__len__")  GeosCoordinateSequence::getSize;


%predicate GeosGeometry::disjoint;
%predicate GeosGeometry::touches;
%predicate GeosGeometry::intersects;
%predicate GeosGeometry::crosses;
%predicate GeosGeometry::within;
%predicate GeosGeometry::contains;
%predicate GeosGeometry::overlaps;
//%predicate GeosGeometry::equals;

%predicate GeosGeometry::isEmpty;
%predicate GeosGeometry::isValid;
%predicate GeosGeometry::isSimple;
%predicate GeosGeometry::isRing;
%predicate GeosGeometry::hasZ;

/* Convert a Ruby array of GeosLinearRings to a C array. */
%typemap(in,numinputs=1) (GeosLinearRing **holes, size_t nholes)
{
    if (NIL_P($input))
    {
        $1 = NULL;
        $2 = 0;
    }
    else
    {
        /* Make sure the input can be treated as an array. */
        Check_Type($input, T_ARRAY);

        /* Get the length */
        $2 = RARRAY($input)->len;
      
        /* Allocate space for the C array. */
        $1 = (GeosLinearRing**) malloc($2*sizeof(GeosLinearRing*));
      
        for(int i = 0; i<$2; i++)
        {
            /* Get the Ruby Object */
            VALUE item = rb_ary_entry($input,i);

            /* Get the underlying pointer and give up ownership of it. */
            GeosLinearRing *ring = NULL;
            int convertResult = SWIG_ConvertPtr(item, (void**)&ring, $descriptor(GeosLinearRing*), SWIG_POINTER_DISOWN);
            if (!SWIG_IsOK(convertResult)) {
                SWIG_exception_fail(SWIG_ArgError(convertResult), "in method '" "createPolygon" "', argument " "1"" of type '" "GeosLinearRing *""'");
            }

            /* Put the pointer in the array */
            $1[i] = ring;
        }    
    }
}

%typemap(freearg)  (GeosLinearRing **holes, size_t nholes)
{
  if ($1) {
    free((void*) $1);
  }
}


// Manually rename methods on vector to get around SWIG 1.3.29 bug
/*%rename(__len__) std::vector<geos::geom::Geometry *>::size;
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
*/

/*
	%extend Geometry {
		std::string to_s()
		{
			std::ostringstream os;
			os << "<Geometry ";
			os << self->toString();
			os << ">";
			return os.str();
		}
	};*/

	/* Geos uses vectors of pointers to pass around geometries.  These will be 
		 wrapped by SWIG - but we have to be careful.  The problem is
		 if we put a C++ object into the vector that a Ruby object owns, when
		 that Ruby object goes out of scope the C++ object will be freed.  Thus
		 the pointer inside the vector becomes corrupt.  To stop this from happening
		 we have to implement a Ruby mark function, which in turn requires turning
		 on SWIG Ruby Object tracking for these containers.*/

	// First declare mark functions for the containers
	/*%markfunc std::vector<Geometry *> "mark_GeometryVector";
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
} */

/* End geom namespace */
//} /* End geos namespace */


// Last define the mark functions
/*%header %{
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
}*/
/* End geom namespace */
//} /* End geos namespace */
//%}


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