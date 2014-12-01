// doubledouble.h
// Keith Briggs.   Last revised 98 Feb 09

/*
Copyright (C) 1997 Keith Martin Briggs

Except where otherwise indicated,
this program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// Change log:  (See doubledouble.cc also)
//  97 Dec 22 KMB added x86_FIX
//            Eliminates -ffloat-store requirement
//  97 Aug 04 KMB added ldexp
//  97Jul25 (WH) - fixed Qrand48 to return drand48 + (2^-47)*drand48
//  97Jul11 (WH) added qtoa declaration.
//	- made all constants (Pi etc) member constants, moved to quad.cc
//	- added doubledoubleRand48().
//	- cleaned up constructors (fewer, with default arguments).
//	- added some code for integer exponent, but commented it out.  It
//	  looks hard.  eg, adding numbers with different exponents.  Almost
//	  always the smaller would be essentially zero, but near boundaries...
//  96Nov20 added normalizing constructor doubledouble(double,double) (needed by floor)

#ifndef __QUAD_H__
#define __QUAD_H__

#define DEBUG_QUAD 0

#ifdef SGI_CC
#define bool int
#undefine _G_HAVE_BOOL
#endif

#ifdef __GNUC__
  #if (__GNUC_MAJOR__<=2 && __GNUC_MINOR__<=6)
    #define bool int
    #define explicit
    #ifndef __isinf
      #define __isinf(x) ((x)!=(x)) // gcc 2.7.2 defines __isinf as a function
    #endif
  #endif
#endif

#ifdef _MSC_VER
#include <float.h>
#include "drand48.h"
#else
#include <nan.h>  // defines NAN, at least in gcc
#endif
#include <assert.h>

// DOMAIN_ERROR=action to take on function domain errors
#ifdef NAN
#define DOMAIN_ERROR return(doubledouble(NAN,NAN))
#else
#define DOMAIN_ERROR assert(0)
#endif

#ifdef x86
#define x86_FIX \
  unsigned short __old_cw, __new_cw; \
  asm volatile ("fnstcw %0":"=m" (__old_cw)); \
  __new_cw = (__old_cw & ~0x300) | 0x200; \
  asm volatile ("fldcw %0": :"m" (__new_cw));
#define END_x86_FIX  asm volatile ("fldcw %0": :"m" (__old_cw));
#else
#define x86_FIX
#define END_x86_FIX
#endif

/*
C++ functions for doubledouble (i.e. double+double) precision.

These functions use techniques due to Dekker, Linnainmaa, Kahan, Knuth 
and Priest.  I credit Kahan with the addition algorithm (the simplification
which permits the elimination of the tests and branches is due to Knuth);
Dekker and Linnainmaa with the multiply, divide, and square root routines,
and Priest for the initial transcription into C++.

A doubledouble x is represented as a pair of doubles, x.hi and x.lo,
such that the number represented by x is x.hi + x.lo, where

   |x.lo| <= 0.5*ulp(x.hi),  (*)

and ulp(y) means `unit in the last place of y'.  For the software to
work correctly, IEEE Standard Arithmetic is sufficient.  That includes
just about every modern workstation.  Also sufficient is any platform
that implements arithmetic with correct rounding, i.e., given double
floating point numbers a and b, a op b is computed exactly and then
rounded to the nearest double.  The tie-breaking rule is not
important.

See:

T. J. Dekker
   Point Technique for Extending the Available Precision,
   Numer. Math. 18 (1971), pp. 224-242.
S. Linnainmaa
   Software for doubled-precision floating point computations
   ACM TOMS 7, 172-283 (1081).
D. Priest 
  On properties of floating point arithmetics: numerical stability
  and the cost of accurate computations.   Ph.D. Diss, Berkeley 1992.

and more references in http://www.cs.wisc.edu/~shoup/ntl/quad_float.txt.
*/

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include <float.h>

class doubledouble {
protected:
  double hi, lo;
public:
  // Public data members, initialized in doubledouble.cc
  static const double Split; // cannot be initialized here; see ARM 9.4
  static const doubledouble Log2, Log10, Pi, TwoPi, Pion2, Pion4, _Pi;

  // Public access to hi and lo
  inline double h() const { return hi; }
  inline double l() const { return lo; }

  // Constructors
  inline doubledouble():hi(0.0),lo(0.0) {}
  inline doubledouble(const int n) { hi=double(n); lo=0.0; }
  inline doubledouble(const double x, const double y);
  inline doubledouble(const doubledouble&);
  doubledouble(const char*);

  // Operators
  doubledouble& operator +=(const doubledouble&);
  doubledouble& operator +=(const double&);
  doubledouble& operator +=(const int);
  doubledouble& operator -=(const doubledouble&);
  doubledouble& operator -=(const double&);
  doubledouble& operator -=(const int);
  doubledouble& operator *=(const doubledouble&);
  doubledouble& operator *=(const double&);
  doubledouble& operator *=(const int);
  doubledouble& operator /=(const doubledouble&);
  doubledouble& operator /=(const double&);
  doubledouble& operator /=(const int);
  doubledouble& operator=(const doubledouble&);
  doubledouble& operator=(const double&);
  doubledouble& operator=(const int);  // Get funny errors without this
  doubledouble& operator=(const char*);

  // Type conversion operator.  Not really necessary...
  operator double() const { return hi+lo; }
  operator int() const { return (int)(hi+lo); }

  inline doubledouble normalize(void) { 
    double h=hi+lo; lo=lo+(hi-h); hi=h;
    return *this;
  }
  void dump(char*) const;  // debugging use only

  // Friends
  inline friend doubledouble operator -(const doubledouble& x);  // Unary -
  friend doubledouble operator +(const doubledouble&, const doubledouble& );
  friend doubledouble operator +(const double&, const doubledouble& );
  friend doubledouble operator +(const int, const doubledouble& );
  friend doubledouble operator +(const doubledouble&, const double& );
  friend doubledouble operator +(const doubledouble&, const int );
  friend doubledouble operator -(const doubledouble&, const doubledouble& );
  friend doubledouble operator -(const double&, const doubledouble& );
  friend doubledouble operator -(const int, const doubledouble& );
  friend doubledouble operator -(const doubledouble&, const double& );
  friend doubledouble operator -(const doubledouble&, const int );
  friend doubledouble operator *(const doubledouble&, const doubledouble& );
  friend doubledouble operator *(const double&, const doubledouble& );
  friend doubledouble operator *(const int, const doubledouble& );
  friend doubledouble operator *(const doubledouble&, const double& );
  friend doubledouble operator *(const doubledouble&, const int );
  friend doubledouble operator /(const doubledouble&, const doubledouble& );
  friend doubledouble operator /(const doubledouble&, const double& );
  friend doubledouble operator /(const doubledouble&, const int );
  friend doubledouble operator /(const double&, const doubledouble& );
  friend doubledouble operator /(const int, const doubledouble& );
  friend doubledouble recip(const doubledouble &);
  friend doubledouble operator |(const doubledouble&, const doubledouble& );
  friend double dnorm(const doubledouble&);
  friend int intq(const doubledouble&);
  friend doubledouble ldexp(const doubledouble x, const int exp);
  // member functions
  bool operator!=(const doubledouble& y) { return hi!=y.h() || lo!=y.l(); };
};  // end class doubledouble

void base_and_prec(void);
doubledouble atodd(const char *);  // string to doubledouble conversion
// doubledouble to string conversion.  W must be long enough.  Returns W.
char *qtoa(char *Word, int prec, int fmtch, doubledouble q);
bool operator> (const doubledouble&, const doubledouble&);
bool operator>=(const doubledouble&, const doubledouble&);
bool operator< (const doubledouble&, const doubledouble&);
bool operator<=(const doubledouble&, const doubledouble&);
bool operator==(const doubledouble&, const doubledouble&);
//bool operator!=(const doubledouble&, const doubledouble&);

// inline members

inline doubledouble::doubledouble(const double x, const double y = 0.0) {
  x86_FIX
  hi=x+y; lo=y+(x-hi); // normalize
  END_x86_FIX
}
inline doubledouble::doubledouble(const doubledouble& x):hi(x.hi),lo(x.lo) {}
inline doubledouble& doubledouble::operator=(const doubledouble& x){ hi=x.hi; lo=x.lo; return *this;}
inline doubledouble& doubledouble::operator=(const double& x){ hi=x; lo=0.0; return *this;}
inline doubledouble& doubledouble::operator=(const int x){ hi=x; lo=0.0; return *this;}

// inline functions
inline doubledouble operator-(const doubledouble& x) { return doubledouble(-x.hi, -x.lo); }
inline doubledouble normalize(const doubledouble& x) { return doubledouble(x.h(), x.l()); }
inline double dnorm(const doubledouble& x) { return fabs(x.h());}
inline int intq(const doubledouble& x) { // explicit type conversion doubledouble -> int
  return int(x.h()); 
} 
inline doubledouble doubledoubleRand48(void) {
  return doubledouble(drand48(), ldexp(drand48(), -47)); 
}

// inline functions (defined in doubledouble.cc)
doubledouble Qcopysign(const doubledouble&, const double);

// non inline functions (defined in doubledouble.cc and math.cc)

std::istream& operator >> (std::istream&, doubledouble&);
std::ostream& operator << (std::ostream&, const doubledouble&);
int sign(const doubledouble&);
doubledouble hypot(const doubledouble, const doubledouble);
doubledouble recip(const doubledouble&);
doubledouble sqrt(const doubledouble&);
doubledouble sqr(const doubledouble&);
doubledouble cub(const doubledouble&);
doubledouble sqr_double(const double&);
doubledouble rint(const doubledouble&);
doubledouble floor(const doubledouble&);
doubledouble trunc(const doubledouble&);
doubledouble fmod(const doubledouble&, const int);
doubledouble modf(const doubledouble&, doubledouble *ip);
doubledouble fabs(const doubledouble&);
doubledouble exp(const doubledouble&); 
doubledouble log(const doubledouble&);  
doubledouble log10(const doubledouble&);  
doubledouble powint(const doubledouble&, const int);
doubledouble pow(const doubledouble&, const doubledouble&);
doubledouble sin(const doubledouble&);
void sincos(const doubledouble x, doubledouble& sinx, doubledouble& cosx);
doubledouble cos(const doubledouble&);
doubledouble atan(const doubledouble&);
doubledouble atan2(const doubledouble&, const doubledouble&);
doubledouble asin(const doubledouble&);
doubledouble sinh(const doubledouble&);
doubledouble cosh(const doubledouble&);
doubledouble tanh(const doubledouble&);
doubledouble erf(const doubledouble);
doubledouble erfc(const doubledouble);
doubledouble gamma(const doubledouble);
int  digits(const doubledouble&,const doubledouble&);
doubledouble modr(const doubledouble a, const doubledouble b, int& n, doubledouble& rem);

#ifdef DD_INLINE
#include "inline.h"
#endif	// DD_INLINE

#endif	// __QUAD_H__
