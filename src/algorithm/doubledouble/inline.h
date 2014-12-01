// inline.h 
// KMB 97 Dec 29

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

// 97 Aug 04 KMB added ldexp
// 97 Jul 11 Moved this stuff out of quad.h, created inline.h so it can
//	be #included even if we're not inlining, by just "#define inline"
// 97 Jul 12 Added all combos of doubledouble/double/int +-*/.  Only a couple actually
//	written; most just call the others by swapping arguments.  Generates
//	equivalent code with a good inlining compiler (tested with g++ 2.7.2).
//	- e.g., all subtraction is now done by addition of unary minus.
//	- removed if's from doubledouble*int. Zero-branch code is 2.5 faster (tested)
//	- generally cleaned up and re-organized the order of the functions,
//	  now they're grouped nicely by function.
//	- tested Newton's division.  Works but is terribly slow, because
//	  it needs to do several doubledouble + and * operations, and doubledouble /
//	  without it is about the same speed at doubledouble * anyway, so no win.
//	- moved recip here as an inline.
//	- checked and tested doubledouble/double (BUG?), seems fine.

// Absolute value
inline doubledouble fabs(const doubledouble& x) { if (x.h()>=0.0) return x; else return -x; }

// Addition
/*      (C) W. Kahan 1989
*       NOTICE:
*       Copyrighted programs may not be translated, used, nor
*       reproduced without the author's permission.  Normally that
*       permission is granted freely for academic and scientific
*       purposes subject to the following three requirements:
*       1. This NOTICE and the copyright notices must remain attached
*          to the programs and their translations.
*       2. Users of such a program should regard themselves as voluntary
*          participants in the author's researches and so are obliged
*          to report their experience with the program back to the author.
*       3. Neither the author nor the institution that employs him will
*          be held responsible for the consequences of using a program
*          for which neither has received payment.
*       Would-be commercial users of these programs must correspond
*       with the author to arrange terms of payment and warranty.
*/


///////////////////////////////////////////////////////////////
///////////////////  Addition and Subtraction /////////////////
///////////////////////////////////////////////////////////////

// Binary addition
inline doubledouble operator +(const doubledouble& x, const doubledouble& y) {
  x86_FIX
  double H, h, T, t, S, s, e, f;
  S = x.hi+y.hi; T = x.lo+y.lo; e = S-x.hi; f = T-x.lo; s = S-e; t = T-f; 
  s = (y.hi-e)+(x.hi-s); t = (y.lo-f)+(x.lo-t); 
  e = s+T; H = S+e; h = e+(S-H); e = t+h;
  doubledouble z; z.hi = H+e; z.lo = e+double(H-z.hi); 
  END_x86_FIX
  return z;
}

inline doubledouble operator +(const double& x, const doubledouble& y) {
  x86_FIX
  double H, h, S, s, e;
  S = x+y.hi; e = S-x; s = S-e;
  s = (y.hi-e)+(x-s); H = S+(s+y.lo); h = (s+y.lo)+(S-H);
  doubledouble z; z.hi = H+h; z.lo = h+double(H-z.hi); 
  END_x86_FIX
  return z;
}
inline doubledouble operator +(const doubledouble& x,const double& y ) { return y+x; }
inline doubledouble operator +(const int x, const doubledouble& y) { return double(x)+y; }
inline doubledouble operator +(const doubledouble& x, const int y) { return y+x; }

// Subtraction
inline doubledouble operator -(const doubledouble& x, const doubledouble& y)   { return x+(-y); }
inline doubledouble operator -(const double& x, const doubledouble& y) { return x+(-y); }
inline doubledouble operator -(const int x, const doubledouble& y)     { return x+(-y); }
inline doubledouble operator -(const doubledouble& x, const double& y) { return x+(-y); }
inline doubledouble operator -(const doubledouble& x, const int y)     { return x+(-y); }


//////////////////////////  Self-Addition  ///////////////////////
inline doubledouble& doubledouble::operator +=(const doubledouble& y) {
  x86_FIX
  double H, h, T, t, S, s, e, f;
  S = hi+y.hi; T = lo+y.lo; e = S-hi; f = T-lo; s = S-e; t = T-f; 
  s = (y.hi-e)+(hi-s); t = (y.lo-f)+(lo-t); f = s+T; H = S+f; h = f+(S-H);
  hi = H+(t+h); lo = (t+h)+double(H-hi);
  END_x86_FIX
  return *this;
}

inline doubledouble& doubledouble::operator +=(const double& y) {
  x86_FIX
  double H, h, S, s, e, f;
  S = hi+y; e = S-hi; s = S-e;
  s = (y-e)+(hi-s); f = s+lo; H = S+f; h = f+(S-H);
  hi = H+h; lo = h+double(H-hi); 
  END_x86_FIX
  return *this;
}
inline doubledouble& doubledouble::operator +=(const int y) { return *this += double(y); }

// Self-Subtraction
inline doubledouble& doubledouble::operator -=(const doubledouble& y)   { return *this += -y; }
inline doubledouble& doubledouble::operator -=(const double& y) { return *this += -y; }
inline doubledouble& doubledouble::operator -=(const int y)     { return *this += -y; }


/////////////////////////////////////////////////////////////
////////////////////  Multiplication  ///////////////////////
/////////////////////////////////////////////////////////////

// Binary Multiplication
inline doubledouble operator *(const doubledouble& x, const doubledouble& y) {
  x86_FIX
  double hx, tx, hy, ty, C, c;
  C = doubledouble::Split*x.hi; hx = C-x.hi; c = doubledouble::Split*y.hi;
  hx = C-hx; tx = x.hi-hx; hy = c-y.hi; 
  C = x.hi*y.hi; hy = c-hy; ty = y.hi-hy;
  c = ((((hx*hy-C)+hx*ty)+tx*hy)+tx*ty)+(x.hi*y.lo+x.lo*y.hi);
  doubledouble z; z.hi = C+c; hx=C-z.hi; z.lo = c+hx; 
  END_x86_FIX
  return z;
}

// double*doubledouble
inline doubledouble operator *(const double& x, const doubledouble& y) {
  x86_FIX
  double hx, tx, hy, ty, C, c;
  C = doubledouble::Split*x; hx = C-x; c = doubledouble::Split*y.hi; hx = C-hx ; 
  tx = x-hx; hy = c-y.hi; C = x*y.hi; hy = c-hy; ty = y.hi - hy;
  c = ((((hx*hy-C)+hx*ty)+tx*hy)+tx*ty)+x*y.lo;
  doubledouble z; z.hi = C+c; z.lo = c+double(C-z.hi); 
  END_x86_FIX
  return z;
}

inline doubledouble operator *(const int x, const doubledouble& y ) 	{ return double(x)*y; }
inline doubledouble operator *(const doubledouble& x, const double& y ) { return y*x; }
inline doubledouble operator *(const doubledouble& x, const int y )     { return y*x; }

// Self-multiplication
inline doubledouble& doubledouble::operator *=(const doubledouble& y ) {
  x86_FIX
  double hx, tx, hy, ty, C, c;
  C = Split*hi; hx = C-hi; c = Split*y.hi;
  hx = C-hx; tx = hi-hx; hy = c-y.hi; 
  C = hi*y.hi; hy = c-hy; ty = y.hi-hy;
  c = ((((hx*hy-C)+hx*ty)+tx*hy)+tx*ty)+(hi*y.lo+lo*y.hi);
  hx = C+c; hi = hx; lo = c+double(C-hx);
  END_x86_FIX
  return *this;
}

inline doubledouble& doubledouble::operator *=(const double& y ) {
  x86_FIX
  double hx, tx, hy, ty, C, c;
  C = Split*hi; hx = C-hi; c = Split*y;
  hx = C-hx; tx = hi-hx; hy = c-y; 
  C = hi*y; hy = c-hy; ty = y-hy;
  c = ((((hx*hy-C)+hx*ty)+tx*hy)+tx*ty)+(lo*y);
  hx = C+c; hi = hx; lo = c+double(C-hx);
  END_x86_FIX
  return *this;
}
inline doubledouble& doubledouble::operator *=(const int y ) { return *this *= double(y); }


////////////////////////////////////////////////////////////////
//////////////////////////  Division  //////////////////////////
////////////////////////////////////////////////////////////////

// Reciprocal
inline doubledouble recip(const doubledouble& y) {
  x86_FIX
  double  hc, tc, hy, ty, C, c, U, u;
  C = 1.0/y.h(); 
  c = doubledouble::Split*C; 
  hc =c-C;  
  u = doubledouble::Split*y.h();
  hc = c-hc; tc = C-hc; hy = u-y.h(); U = C*y.h(); hy = u-hy; ty = y.h()-hy;
  u = (((hc*hy-U)+hc*ty)+tc*hy)+tc*ty;
  c = ((((1.0-U)-u))-C*y.l())/y.h();
  doubledouble z; z.hi = C+c; z.lo = double(C-z.hi)+c; 
  END_x86_FIX
  return z;
}

inline doubledouble operator /(const doubledouble& x,const doubledouble& y ) {
  x86_FIX
  double hc, tc, hy, ty, C, c, U, u;
  C = x.hi/y.hi; c = doubledouble::Split*C; hc =c-C;  u = doubledouble::Split*y.hi; hc = c-hc;
  tc = C-hc; hy = u-y.hi; U = C * y.hi; hy = u-hy; ty = y.hi-hy;
  u = (((hc*hy-U)+hc*ty)+tc*hy)+tc*ty;
  c = ((((x.hi-U)-u)+x.lo)-C*y.lo)/y.hi;
  doubledouble z; z.hi = C+c; z.lo = double(C-z.hi)+c; 
  END_x86_FIX
  return z;
}

// double/doubledouble:
inline doubledouble operator /(const double& x,const doubledouble& y ) {
  x86_FIX
  double  hc, tc, hy, ty, C, c, U, u;
  C = x/y.hi; c = doubledouble::Split*C; hc =c-C;  u = doubledouble::Split*y.hi; hc = c-hc; 
  tc = C-hc; hy = u-y.hi; U = C*y.hi; hy = u-hy; ty = y.hi-hy;
  u = (((hc*hy-U)+hc*ty)+tc*hy)+tc*ty;
  c = ((((x-U)-u))-C*y.lo)/y.hi;
  doubledouble z; z.hi = C+c; z.lo = double(C-z.hi)+c; 
  END_x86_FIX
  return z;
}

inline doubledouble operator /(const doubledouble& x,const double& y ) {
  x86_FIX
  double hc, tc, hy, ty, C, c, U, u;
  C = x.hi/y; c = doubledouble::Split*C; hc = c-C;  u = doubledouble::Split*y; hc = c-hc; 
  tc = C-hc; hy = u-y; U = C*y; hy = u-hy; ty = y-hy;
  u = (((hc*hy-U)+hc*ty)+tc*hy)+tc*ty;
  c = (((x.hi-U)-u)+x.lo)/y;
  doubledouble z;  z.hi = C+c; z.lo = double(C-z.hi)+c; 
  END_x86_FIX
  return z;
}

// doubledouble/int
inline doubledouble operator /(const doubledouble& x, const int y) { return x/double(y); }
inline doubledouble operator /(const int x, const doubledouble& y) { return double(x)/y; }

// Self-division.
inline doubledouble& doubledouble::operator /=(const doubledouble& y) {
  x86_FIX
  double hc, tc, hy, ty, C, c, U, u;
  C = hi/y.hi; c = Split*C; hc =c-C;  u = Split*y.hi; hc = c-hc;
  tc = C-hc; hy = u-y.hi; U = C * y.hi; hy = u-hy; ty = y.hi-hy;
  u = (((hc*hy-U)+hc*ty)+tc*hy)+tc*ty;
  c = ((((hi-U)-u)+lo)-C*y.lo)/y.hi;
  u = C+c; hi = u; lo = double(C-u)+c; 
  END_x86_FIX
  return *this;
}

inline doubledouble& doubledouble::operator /=(const double& y) {
  x86_FIX
  double hc, tc, hy, ty, C, c, U, u;
  C = hi/y; c = Split*C; hc =c-C;  u = Split*y; hc = c-hc;
  tc = C-hc; hy = u-y; U = C * y; hy = u-hy; ty = y-hy;
  u = (((hc*hy-U)+hc*ty)+tc*hy)+tc*ty;
  c = (((hi-U)-u)+lo)/y;
  u = C+c; hi = u; lo = double(C-u)+c; 
  END_x86_FIX
  return *this;
}

inline doubledouble& doubledouble::operator /=(const int y) { return *this/=double(y); }
