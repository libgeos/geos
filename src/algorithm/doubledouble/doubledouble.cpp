// doubledouble.cc
// KMB 98 Jan 19

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

#include "doubledouble.h"
#include <math.h>
#include <stdlib.h>

// member constants
const double doubledouble::Split = 134217729.0L; // 2^27+1, for IEEE double
const doubledouble doubledouble::Log2 ="0.6931471805599453094172321214581765680755";
const doubledouble doubledouble::Log10="2.302585092994045684017991454684364207601";
const doubledouble doubledouble::Pi   ="3.1415926535897932384626433832795028841972";
const doubledouble doubledouble::TwoPi="6.2831853071795864769252867665590057683943";
const doubledouble doubledouble::Pion2="1.5707963267948966192313216916397514420985";
const doubledouble doubledouble::Pion4="0.7853981633974483096156608458198757210493";
const doubledouble doubledouble::_Pi  ="0.3183098861837906715377675267450287240689";

void base_and_prec(void) { // Linnainmaa ACM TOMS 7, 272 Thm 3
  int p;
  x86_FIX
  std::cerr<<"Base and precision determination by Linnainmaa's method:"<<std::endl;
  {
    double U,R,u,r,beta;
    U=4.0/3.0;
    U-=1; U*=3; U-=1; U=fabs(U);
    R=U/2+1; R-=1;
    if (R!=0.0) U=R;
    u=2; u/=3; u-=0.5; u*=3; u-=0.5; 
    u=fabs(u); r=u/2+0.5; r-=0.5;
    if (r!=0.0) u=r;
    beta=U/u; p=int(-log(u)/log(beta)+0.5);
    std::cout<<"Type double: base is "<<beta<<",  precision is "<<p<<std::endl;
  } {
    doubledouble U,R,u,r,beta;
    U=4; U/=3;
    U-=1; U*=3; U-=1; U=fabs(U);
    R=U/2+1; R-=1;
    if (R.h()!=0.0) U=R;
    u=2; u/=3; u-=0.5; u*=3; u-=0.5; 
    u=fabs(u); r=u/2+0.5; r-=0.5;
    if (r.h()!=0.0) u=r;
    beta=U/u; p=int(-log(u)/log(beta)+0.5);
    std::cout<<"Type doubledouble:   base is "<<beta<<",  precision is "<<p<<std::endl;
  }
  END_x86_FIX
  return;
}

// number of decimal digits to which x and y agree
int digits(const doubledouble& x, const doubledouble& y){ 
 doubledouble diff=fabs(x-y);
 if (diff.h()==0.0) return 32;
 int d=-intq(floor((0.4*log((diff/fabs(x))))).h());
 return d<32?d:32;
}

doubledouble Qcopysign(const doubledouble& x, const double y){
  if (y>=0.0) return fabs(x); else return -fabs(x); }

doubledouble atodd(const char *s) {
  doubledouble result = 0; int n, sign, ex = 0;
  /* eat whitespace */ while (*s == ' ' || *s == '\t' || *s == '\n') s++;
  switch (*s) { // get sign of mantissa
    case '-': { sign = -1;  s++; break; }
    case '+': s++;  // no break 
    default: sign = 1;
  }
  /* get digits before decimal point */
  while (n=(*s++)-'0', n>=0 && n<10) result = 10.0*result+n;
  s--;
  if (*s == '.')   /* get digits after decimal point */ {
    s++;
    while (n=(*s++)-'0', n>=0 && n<10) { result = 10.0*result+n; --ex; }
    s--;
  }
  if (*s == 'e' || *s == 'E') /* get exponent */ ex+=atoi(++s);
  /* exponent adjustment */
  // std::cerr<<"atodd: result="<<std::endl; result.dump(""); std::cerr<<"atodd: ex="<<ex<<std::endl; 
  while (ex-- > 0) result *= 10;
  while (++ex < 0) result /= 10;
  return (sign>=0) ? result : -result;
}

void doubledouble::dump(char* s="") const {  // debugging use only
  std::cerr<<s<<std::setprecision(16)<<"doubledouble("<<hi<<","<<lo<<")";
}

// Constructor from string
doubledouble::doubledouble(const char* s){ *this=atodd(s); }

// doubledouble = string
doubledouble& doubledouble::operator=(const char* s){ return *this=atodd(s); }

std::istream& operator >> (std::istream& s, doubledouble& x){
  doubledouble result=0.0; int n, sign=1, ex=0; char ws; char ch;
  s>>ws; // eat whitespace
  s>>ch;
  if (ch=='-') { sign = -1;  s>>ch; } else if (ch=='+') { s>>ch; }
  // get digits before decimal point
  n=ch-'0';
  while (n>=0 && n<10) { 
    result = 10*result+n; 
    s.get(ch);  // cannot use s>>ch; as that will eat whitespace
    if (ch == ' ' || ch == '\t' || ch == '\n') goto fixup;
    n=ch-'0'; 
  }
  if (ch=='.') {  // get digits after decimal point
     s>>ch; n=ch-'0';
     while (n>=0 && n<10) { 
       result = 10*result+n; 
       s.get(ch);
       n=ch-'0'; --ex; 
       if (ch == ' ' || ch == '\t' || ch == '\n') goto fixup;
     }
  }
  n=0;
  if (ch == 'e' || ch == 'E') { s>>n; ex+=n; } // get exponent
    else s.putback(ch);
  fixup:
  if (sign<0) result = -result;
  // exponent adjustment
  while (ex-- > 0) result *= 10;
  while (++ex < 0) result /= 10;
  x = result;
  return s;
}

// output
std::ostream& operator << (std::ostream& s, const doubledouble& x){
  if (x.h()==0.0)   { s << "0.0 "; return s; }
  if (x.h()!=x.h()) { s << "NaN "; return s; }
  int Digits=s.precision();
  doubledouble ten=10.0,y=fabs(x); double q=log10(y.h());
  int m,n=int(floor(q)); 
  if (n<0) n++;
  doubledouble l = powint(ten,n);
  y = y/l;
  if (sign(x)<0) s<<"-"; //else s<<" ";
  int d = Digits>34 ? 34 : Digits;
  d = d<3 ? 3 : d;
  for (int i=1; i<=d; i++) {
    if (2==i) s<<".";
    m = int(floor(y));
    if (m<0 || m>9) { std::cerr<<"Internal error in doubledouble.cc: digit="<<m<<
                            " in ostream& operator <<\n"; 
		      std::cerr<<"Argument to << was "; x.dump(""); std::cerr<<std::endl; 
		      assert(0); }
    s<<m;
    y = (y-doubledouble(m))*ten;
    if (y.h()<=0.0) break; // x must be an integer
    //if (y.h()==0.0) break; // ???????????
  }
  if (n!=0) s<<"e"<<n; else s<<"";
  s << "";
  return s;
}

// rint (round to nearest int)
doubledouble rint(const doubledouble& x){ return floor(x+doubledouble(0.5)); }

// Another floor. V. Shoup 97 Sep 23...
doubledouble floor_s(const doubledouble& x) { 
  double fhi=floor(x.h());
  if (fhi!=x.h())
     return doubledouble(fhi);
  else 
     return doubledouble(fhi,floor(x.l()));
}

// Floor.  See Graham, Knuth and Patashnik `Concrete Mathematics' page 70.
// Greatest integer <= x
// maybe floor_s is better?
doubledouble floor(const doubledouble& x) { 
  double fh=floor(x.h()), fl=floor(x.l()); 
  double t1=x.h()-fh;
  double t2=x.l()-fl;
  double t3=t1+t2;
  t3 = x.h()-fh+x.l()-fl;
  int t=int(floor(t3));
  switch (t) {
    case 0: return doubledouble(fh)+doubledouble(fl);
    case 1: return doubledouble(fh)+doubledouble(fl+1);
    // case 2 can only arise when fp({hi})<=1, fp({lo})<=1, but fp({hi}+{lo})=2
    case 2: return doubledouble(fh)+doubledouble(fl+1);
  }
  // never get here
}

doubledouble ceil(const doubledouble& x) { 
  return -floor(-x);
}

doubledouble trunc(const doubledouble& x) { 
  if (x.h()>=0.0) return floor(x); else return -floor(-x);
}

doubledouble fmod(const doubledouble& x, const int n) { return x-n*floor(x/n); }

/* Same as the ANSI modf for doubles.
** BEWARE: contains ugly, magical code
*/
doubledouble modf(const doubledouble &D, doubledouble *id) {
    int sign=1;
    doubledouble qFrac, d = D;
    double lowFrac, highFrac, lowInt, highInt;
    if (d < doubledouble(0)) { sign = -1; d = -d; }
    if (d < doubledouble(1)) { /* it's all fraction, no integer */
	*id = 0; return sign*d; }
    if (d + 1 == d) { /* It's all integer, no fraction */
	*id = sign*d; return 0.0; }
    highFrac = modf(d.h(), &highInt);
    lowFrac = modf(d.l(), &lowInt);
    /* special case: if d.l is opposite in sign to d.h, then adding them
    ** together changes the integer part.
    */
    if (highInt == d.h() && d.l() < 0) {
#if 1
	if (lowFrac != 0) {
	    *id = doubledouble(highInt) + doubledouble(lowInt) - 1;
	    qFrac = 1 + doubledouble(lowFrac);
	} else {
	    qFrac = lowFrac;
	    *id = doubledouble(highInt) + doubledouble(lowInt);
	}
#else
    /* loop method, slow but should work */
    *id = 0;
    top = QUAD_BITS;
    while (1) {
	/* Invariant: *id <= d */
	if (!(*id <= d)) printf("Oops, !(*id <= d): %f %f\n\n", *id, d);
	if (d < *id + 1) break;
	for(i = top-1; i >= 0 && (*id + ldexp(1.,i) > d); i--) ;
	*id += ldexp(1.,i);
	top = i;
    }
    *id *= sign;
    qFrac = d - sign*(*id);
    printf("loop Int = %.16g %.16g, frac = %.16g %.16g\n",
	id->h, id->l, qFrac.h, qFrac.l);
#endif
    }
    else {
	*id = doubledouble(highInt) + doubledouble(lowInt);
	qFrac = doubledouble(highFrac) + doubledouble(lowFrac);
    }
    if (sign == -1) { *id = -*id; return -qFrac; } 
    else return qFrac;
}

// Signum
int sign(const doubledouble& x){
  if (x.h()>0.0) return 1; else if (x.h()<0.0) return -1; else return 0;
}

// Comparison
bool operator> (const doubledouble& x, const doubledouble& y) { 
   return (x.h()> y.h()) || (x.h()==y.h() && x.l()> y.l()); }
bool operator>=(const doubledouble& x, const doubledouble& y) { 
   return (x.h()>y.h()) || (x.h()==y.h() && x.l()>=y.l()); }
bool operator< (const doubledouble& x, const doubledouble& y) { 
   return (x.h()< y.h()) || (x.h()==y.h() && x.l()< y.l()); }
bool operator<=(const doubledouble& x, const doubledouble& y) { 
   return (x.h()<y.h()) || (x.h()==y.h() && x.l()<=y.l()); }
bool operator==(const doubledouble& x, const doubledouble& y) {
   return x.h()==y.h() && x.l()==y.l(); }
bool operator!=(const doubledouble& x, const doubledouble& y) {
   return x.h()!=y.h() || x.l()!=y.l(); }

#ifndef DD_INLINE
  #define inline	/* nothing */
  #include "inline.h"
  #undef inline
#endif // ifndef DD_INLINE

// Square  (faster than x*x)
doubledouble sqr(const doubledouble& x) {
  x86_FIX
  double hx,tx,C,c;
  C=doubledouble::Split*x.h(); hx=C-x.h(); hx=C-hx; tx=x.h()-hx;
  C=x.h()*x.h();
  c=((((hx*hx-C)+2.0*hx*tx))+tx*tx)+2.0*x.h()*x.l();
  hx=C+c;
  doubledouble r=doubledouble(hx,c+(C-hx));
  END_x86_FIX
  return r;
}

// cube
doubledouble cub(const doubledouble& x) { doubledouble z=x*sqr(x); return z; }

doubledouble ldexp(const doubledouble x, const int exp) { // x*2^exp
  return doubledouble(ldexp(x.hi,exp),ldexp(x.lo,exp));
}

#include <sys/types.h>
#include <string.h>
#include <stdarg.h>

#define MAX_STRING 512	/* reasonable size for string to hold a doubledouble */
/*
 * Define FLOATING_POINT to get floating point.
 */
#define FLOATING_POINT

/* end of configuration stuff */

#ifdef FLOATING_POINT
#define BUF     (MAXEXP+MAXFRACT+1) /* + decimal point */
#define DEFPREC     6
#else /* no FLOATING_POINT */
#define BUF     40
#endif /* FLOATING_POINT */

/* 11-bit exponent (VAX G floating point) is 308 decimal digits */
#define MAXEXP      308
/* 128 bit fraction takes up 39 decimal digits; max reasonable precision */
#define MAXFRACT    39

/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
 * Actual printf innards.
 *
 * This code is large and complicated...
 */

/*
 * Macros for converting digits to letters and vice versa
 */
#define to_digit(c) ((c) - '0')
#define is_digit(c) ((unsigned)to_digit(c) <= 9)
#define to_char(n)  ((n) + '0')
inline int dd_to_char(doubledouble n)    { return '0' + (int)n;}

/*
 * Flags used during conversion.
 */
#define LONGINT     0x01        /* long integer */
#define LONGDBL     0x02        /* long doubledouble; unimplemented */
#define SHORTINT    0x04        /* short integer */
#define ALT     0x08        /* alternate form */
#define LADJUST     0x10        /* left adjustment */
#define ZEROPAD     0x20        /* zero (as opposed to blank) pad */
#define HEXPREFIX   0x40        /* add 0x or 0X prefix */

static char *exponent(register char *p, register int exp, int fmtch)
{
    register char *t;
    char expbuf[MAXEXP];

    *p++ = fmtch;
    if (exp < 0) {
	exp = -exp;
	*p++ = '-';
    }
    else
	*p++ = '+';
    t = expbuf + MAXEXP;
    if (exp > 9) {
	do {
	    *--t = to_char(exp % 10);
	} while ((exp /= 10) > 9);
	*--t = to_char(exp);
	for (; t < expbuf + MAXEXP; *p++ = *t++);
    }
    else {
	*p++ = '0';
	*p++ = to_char(exp);
    }
    return (p);
}

static char * round(doubledouble fract, int *exp,
	    register char *start, register char *end,
	    char ch, int *signp) {
    doubledouble tmp;
    if (fract != doubledouble(0))
    (void)modf(fract * 10, &tmp);
    else
	tmp = to_digit(ch);
    if (tmp > doubledouble(4))
	for (;; --end) {
	    if (*end == '.')
		--end;
	    if (++*end <= '9')
		break;
	    *end = '0';
	    if (end == start) {
		if (exp) {  /* e/E; increment exponent */
		    *end = '1';
		    ++*exp;
		}
		else {      /* f; add extra digit */
		*--end = '1';
		--start;
		}
		break;
	    }
	}
    /* ``"%.3f", (doubledouble)-0.0004'' gives you a negative 0. */
    else if (*signp == '-')
	for (;; --end) {
	    if (*end == '.')
		--end;
	    if (*end != '0')
		break;
	    if (end == start)
		*signp = 0;
	}
    return (start);
}

int __cvt_doubledouble(doubledouble number, register int prec, int flags, int *signp,
	 int fmtch, char *startp, char *endp) {
    register char *p, *t;
    doubledouble fract;
    int dotrim = 0, expcnt, gformat = 0;
    doubledouble integer, tmp;
    expcnt = 0;
    if (number < doubledouble(0)) {
	number = -number;
	*signp = '-';
    } else
	*signp = 0;

    fract = modf(number, &integer);

    /* get an extra slot for rounding. */
    t = ++startp;
    /*
     * get integer portion of number; put into the end of the buffer; the
     * .01 is added for modf(356.0 / 10, &integer) returning .59999999...
     */
    for (p = endp - 1; integer != doubledouble(0); ++expcnt) {
	tmp = modf(integer / 10, &integer);
	*p-- = to_char((int)((tmp + .01) * 10));
    }
    switch (fmtch) {
    case 'f':
    case 'F':
	/* reverse integer into beginning of buffer */
	if (expcnt)
	    for (; ++p < endp; *t++ = *p);
	else
	    *t++ = '0';
	/*
	 * if precision required or alternate flag set, add in a
	 * decimal point.
	 */
	if (prec || flags&ALT)
	    *t++ = '.';
	/* if requires more precision and some fraction left */
	if (fract != doubledouble(0)) {
	    if (prec)
		do {
		    fract = modf(fract * 10, &tmp);
		    *t++ = dd_to_char(tmp);
		} while (--prec && fract != doubledouble(0));
	    if (fract != doubledouble(0))
		startp = round(fract, (int *)NULL, startp,
		    t - 1, (char)0, signp);
	}
	for (; prec--; *t++ = '0');
	break;
    case 'e':
    case 'E':
eformat:    if (expcnt) {
	    *t++ = *++p;
	    if (prec || flags&ALT)
		*t++ = '.';
	    /* if requires more precision and some integer left */
	    for (; prec && ++p < endp; --prec)
		*t++ = *p;
	    /*
	     * if done precision and more of the integer component,
	     * round using it; adjust fract so we don't re-round
	     * later.
	     */
	    if (!prec && ++p < endp) {
		fract = 0;
		startp = round((doubledouble)0, &expcnt, startp,
		    t - 1, *p, signp);
	    }
	    /* adjust expcnt for digit in front of decimal */
	    --expcnt;
	}
	/* until first fractional digit, decrement exponent */
	else if (fract != doubledouble(0)) {
	    /* adjust expcnt for digit in front of decimal */
	    for (expcnt = -1;; --expcnt) {
		fract = modf(fract * 10, &tmp);
		if (tmp != doubledouble(0))
		    break;
	    }
	    *t++ = dd_to_char(tmp);
	    if (prec || flags&ALT)
		*t++ = '.';
	}
	else {
	    *t++ = '0';
	    if (prec || flags&ALT)
		*t++ = '.';
	}
	/* if requires more precision and some fraction left */
	if (fract != doubledouble(0)) {
	    if (prec)
		do {
		    fract = modf(fract * 10, &tmp);
		    *t++ = dd_to_char(tmp);
		} while (--prec && fract != doubledouble(0));
	    if (fract != doubledouble(0))
		startp = round(fract, &expcnt, startp,
		    t - 1, (char)0, signp);
	}
	/* if requires more precision */
	for (; prec--; *t++ = '0');

	/* unless alternate flag, trim any g/G format trailing 0's */
	if (gformat && !(flags&ALT)) {
	    while (t > startp && *--t == '0');
	    if (*t == '.')
		--t;
	    ++t;
	}
	t = exponent(t, expcnt, fmtch);
	break;
    case 'g':
    case 'G':
	/* a precision of 0 is treated as a precision of 1. */
	if (!prec)
	    ++prec;
	/*
	 * ``The style used depends on the value converted; style e
	 * will be used only if the exponent resulting from the
	 * conversion is less than -4 or greater than the precision.''
	 *  -- ANSI X3J11
	 */
	if (expcnt > prec || (!expcnt && fract != doubledouble(0) && fract < doubledouble(0.0001))) {
	    /*
	     * g/G format counts "significant digits, not digits of
	     * precision; for the e/E format, this just causes an
	     * off-by-one problem, i.e. g/G considers the digit
	     * before the decimal point significant and e/E doesn't
	     * count it as precision.
	     */
	    --prec;
	    fmtch -= 2;     /* G->E, g->e */
	    gformat = 1;
	    goto eformat;
	}
	/*
	 * reverse integer into beginning of buffer,
	 * note, decrement precision
	 */
	if (expcnt)
	    for (; ++p < endp; *t++ = *p, --prec);
	else
	    *t++ = '0';
	/*
	 * if precision required or alternate flag set, add in a
	 * decimal point.  If no digits yet, add in leading 0.
	 */
	if (prec || flags&ALT) {
	    dotrim = 1;
	    *t++ = '.';
	}
	else
	    dotrim = 0;
	/* if requires more precision and some fraction left */
	if (fract != doubledouble(0)) {
	    if (prec) {
		/* If no integer part, don't count initial
		 * zeros as significant digits. */
		do {
		    fract = modf(fract * 10, &tmp);
		    *t++ = dd_to_char(tmp);
		} while(tmp == doubledouble(0) && !expcnt);
		while (--prec && fract != doubledouble(0)) {
		    fract = modf(fract * 10, &tmp);
		    *t++ = dd_to_char(tmp);
		}
	    }
	    if (fract != doubledouble(0))
		startp = round(fract, (int *)NULL, startp,
		    t - 1, (char)0, signp);
	}
	/* alternate format, adds 0's for precision, else trim 0's */
	if (flags&ALT)
	    for (; prec--; *t++ = '0');
	else if (dotrim) {
	    while (t > startp && *--t == '0');
	    if (*t != '.')
		++t;
	}
    }
    return (t - startp);
}


/*
** From here down is not UCB code, it's Wayne Hayes code.
**
** Convert doubledouble to ASCII.  'word' should be no less than prec+8 long.
*/
char *qtoa(char *Word, int prec, int fmtch, doubledouble q) {
    int flags = 0, sign_char;
    static char buf[MAX_STRING], *word;
    word = buf;
    *word = '\0';
    prec = __cvt_doubledouble(q, prec, flags, &sign_char, fmtch, buf, buf+sizeof(buf));
    if (sign_char)
	word[0] = sign_char, prec++;
    if (word[0] == '\0')
	word++;
    word[prec] = '\0';
    if (Word)
	return strcpy(Word, word);
    else
	return word;
}
