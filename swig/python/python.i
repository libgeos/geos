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

/* Convert a Ruby array of GeosLinearRings to a C array. */
%typemap(in,numinputs=1) (GeosLinearRing **holes, size_t nholes)
{
    if ($input == Py_None)
    {
        $1 = NULL;
        $2 = 0;
    }
    else
    {
        /* Make sure the input can be treated as an array. */
        if (!PySequence_Check($input))
            SWIG_exception_fail(SWIG_RuntimeError, "in method '" "createPolygon" "', argument " "1"" of type '" "GeosLinearRing *""'");

        /* Get the length */
        $2 = PySequence_Size($input);

        /* Allocate space for the C array. */
        $1 = (GeosLinearRing**) malloc($2*sizeof(GeosLinearRing*));
      
        for(size_t i = 0; i<$2; i++)
        {
            /* Get the Python Object */
            PyObject *item = PySequence_GetItem($input,i);

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
