#ifndef GEOS_PLATFORM_H
#define GEOS_PLATFORM_H

#include "config.h"

//Defines NaN for intel platforms
#define DoubleNotANumber 1.7e-308

//Don't forget to define infinities
#define DoubleInfinity 1.7e+308
#define DoubleNegInfinity -1.7e+308

#ifdef HAVE_LONG_LONG_INT_64
 typedef long long int int64;
#else
 typedef long int int64;
#ifndef HAVE_LONG_INT_64
#define INT64_IS_REALLY32 1
#warning "Could not find 64bit integer definition!"
#endif
#endif

#endif
