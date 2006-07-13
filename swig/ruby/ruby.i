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
