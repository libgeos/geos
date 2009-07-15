/* =========================================================================
 * Copyright 2005-2007 Charlie Savage, cfis@interserv.com
 *
 * Interface for a SWIG generated geos module.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */

%rename("dimensions") GeosCoordinateSequence::getDimensions;

%rename("srid") GeosGeometry::getSRID;
%rename("srid=") GeosGeometry::setSRID;
%rename("envelope") GeosGeometry::getEnvelope;

%rename("read_hex") GeosWkbReader::readHEX;
%rename("write_hex") GeosWkbWriter::writeHEX;
%rename("output_dimensions") GeosWkbWriter::getOutputDimension;
%rename("output_dimensions=") GeosWkbWriter::setOutputDimension;
%rename("byte_order") GeosWkbWriter::getByteOrder;
%rename("byte_order=") GeosWkbWriter::setByteOrder;
%rename("include_srid") GeosWkbWriter::getIncludeSRID;
%rename("include_srid=") GeosWkbWriter::setIncludeSRID;

%rename("__len__") GeosCoordinateSequence::getSize;

%rename("coord_seq") GeosPoint::getCoordSeq;
%rename("coord_seq") GeosLineString::getCoordSeq;
%rename("coord_seq") GeosLinearRing::getCoordSeq;

%rename("exterior_ring") GeosPolygon::getExteriorRing;
%rename("num_interior_rings") GeosPolygon::getNumInteriorRings;
%rename("interior_ring_n") GeosPolygon::getInteriorRingN;

// Use predicates to make the ruby code nicer  - so disjoint?
%predicate GeosGeometry::disjoint;
%predicate GeosGeometry::touches;
%predicate GeosGeometry::intersects;
%predicate GeosGeometry::crosses;
%predicate GeosGeometry::within;
%predicate GeosGeometry::contains;
%predicate GeosGeometry::overlaps;

// Use ruby naming conventions for equals
%rename("eql?") GeosGeometry::equals;
%alias GeosGeometry::equals "==";
%rename("eql_exact?") GeosGeometry::equalsExact;
%rename("dimensions") GeosGeometry::getDimensions;
%rename("num_geometries") GeosGeometry::getNumGeometries;


%rename("centroid") GeosGeometry::getCentroid;
%alias GeosGeometry::getCentroid "center"

%rename("empty?") GeosGeometry::isEmpty;
%rename("valid?") GeosGeometry::isValid;
%rename("simple?") GeosGeometry::isSimple;
%rename("ring?") GeosGeometry::isRing;
%rename("has_z?") GeosGeometry::hasZ;

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
      
        for(size_t i = 0; i<$2; i++)
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

// GeosPreparedGeometry
// Use predicates to make the ruby code nicer  - so disjoint?
%rename("contains_properly?") GeosPreparedGeometry::containsProperly;
%predicate GeosPreparedGeometry::contains;
%predicate GeosPreparedGeometry::intersects;
%predicate GeosPreparedGeometry::covers;

// GeosSTRtree

%rename("each") GeosSTRtree::iterate;

%typemap(in) GeosIndexItem
{
    $1 = (GeosIndexItem) $input;
}

/* accumulator will be blissfully unused because
 * Ruby supports closures, more or less */
%typemap(in,numinputs=0) (GeosIndexItem accumulator)
{
    $1 = (GeosIndexItem) Qnil;
}

/* typecheck GeosIndexItem and make sure it's really a VALUE */
%typemap(typecheck) GeosIndexItem
{
    $1 = (TYPE($input) & T_MASK) ? 1 : 0;
}

/* always call the provided block as the query callback */
%typemap(in,numinputs=0) GeosQueryCallback
{
    $1 = GeosSTRtree_query_callback;
}

%typemap(typecheck) GeosQueryCallback
{
    /* SWIG throws a warning if we don't do this */
    $1 = 1;
}

%{
    /* this callback yields the data item to the block */
    static void GeosSTRtree_query_callback (void *data, void *nothing) {
        if (rb_block_given_p()) {
            rb_yield((VALUE) data);
        }
    }
%}

/* assuming that GeosIndexItems are all VALUEs (since this gets tested
 * on typemap(in)), mark them during the mark phase of GC to hang on to them */
%{
    static void GeosSTRtree_mark_item (void *data, void *nothing)
    {
        if ((VALUE) data != Qnil) {
            rb_gc_mark((VALUE)data);
        }
    }

    static void mark_GeosSTRtree(void *self)
    {
        GEOSSTRtree *tree = (GEOSSTRtree *) self;
        GEOSSTRtree_iterate(tree, GeosSTRtree_mark_item, NULL);
    }
%}

%markfunc GeosSTRtree "mark_GeosSTRtree";

