// math.cc
// KMB 98 Feb 03
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
#ifdef _MSC_VER
#include <float.h>
#include "drand48.h"
#else
#include <nan.h>  // defines NAN, at least in gcc
#endif

#include <assert.h>

doubledouble exp(const doubledouble& x) { 
/* Uses some ideas by Alan Miller
   Method:
    x    x.log2(e)    nint[x.log2(e)] + frac[x.log2(e)]
   e  = 2          = 2
 
                      iy    fy
                   = 2   . 2
   Then
    fy    y.loge(2)
   2   = e
 
   Now y.loge(2) will be less than 0.3466 in absolute value.
   This is halved and a Pade' approximant is used to approximate e^x over
   the region (-0.1733, +0.1733).   This approximation is then squared.
*/
  if (x.h()<-744.4400719213812) return 0.0; // exp(x)<1e-300
  int iy;
  doubledouble y,temp,ysq,sum1,sum2;
  y=x/doubledouble::Log2;
  temp=iy=rint(y);
  y=(y-temp)*doubledouble::Log2;
  y=ldexp(y,-1);
  ysq=sqr(y);
  sum1=y*((((ysq+3960.)*ysq+2162160.)*ysq+302702400.)*ysq+8821612800.);
  sum2=(((90.*ysq+110880.)*ysq+30270240.)*ysq+2075673600.)*ysq+17643225600.;
/*
                      sum2 + sum1         2.sum1
  Now approximation = ----------- = 1 + ----------- = 1 + 2.temp
                      sum2 - sum1       sum2 - sum1
 
  Then (1 + 2.temp)^2 = 4.temp.(1 + temp) + 1
*/
  temp=sum1/(sum2-sum1);
  y=temp*(temp+1);
  y=ldexp(y,2);
  return ldexp(y+1,iy);
}

// See Higham "Accuracy and Stability of Numerical Algorithms", p 511
doubledouble hypot(const doubledouble a, const doubledouble b) { 
  doubledouble p,q,r,s,aa,ab,four=4.0;
  aa=fabs(a); ab=fabs(b);
  if (aa>ab) { p=aa; q=ab; } else { q=aa; p=ab; } // now p>=q
  if (0.0==p.h()) return q;
  while (1) {
     r=sqr(q/p);
     if (four==(aa=r+four)) return p;
     s=r/aa; p+=2*s*p; q*=s;
  } // Never get here
}

// square root
doubledouble sqrt(const doubledouble& y) {
  x86_FIX
  double c,p,q,hx,tx,u,uu,cc,hi=y.h();
  if (hi<0.0) {
    std::cerr << "\ndoubledouble: attempt to take sqrt of " << hi << std::endl;
    DOMAIN_ERROR;
  }
  if (0.0==hi) return y;
  c=sqrt(hi);
  p=doubledouble::Split*c; hx=c-p; hx+=p; tx=c-hx;
  p=hx*hx;
  q=2.0*hx*tx;
  u=p+q;
  uu=(p-u)+q+tx*tx;
  cc=(((y.h()-u)-uu)+y.l())/(c+c);
  u=c+cc;
  doubledouble r=doubledouble(u,cc+(c-u));
  END_x86_FIX
  return r;
}

// Natural logarithm
doubledouble log(const doubledouble& x) { // Newton method
  if (x.h()<0.0) {
    std::cerr<<"\ndoubledouble: attempt to take log of negative argument!\n";
    DOMAIN_ERROR;
  }
  if (x.h()==0.0) {
    std::cerr<<"\ndoubledouble: attempt to take log(0)!\n";
    DOMAIN_ERROR;
  }
  doubledouble s=log(x.h()), e=exp(s); // s=double approximation to result
  return s+(x-e)/e;  // Newton correction, good enough
  //doubledouble d=(x-e)/e; return s+d-0.5*sqr(d);  // or 2nd order correction
}

// logarithm base 10
doubledouble log10(const doubledouble& t) { 
  static const doubledouble one_on_log10="0.4342944819032518276511289189166050822944";
  return one_on_log10*log(t);
}

// doubledouble^doubledouble
doubledouble pow(const doubledouble& a, const doubledouble& b) {
  return exp(b*log(a));
}

// doubledouble^int
doubledouble powint(const doubledouble& u, const int c) {
  if (c<0) return recip(powint(u,-c));
  switch (c) {
    case 0: return u.h()==0.0?doubledouble(pow(0.0,0.0)):doubledouble(1.0); // let math.h handle 0^0.
    case 1: return u;  
    case 2: return sqr(u);
    case 3: return sqr(u)*u;
    default: { // binary method
      int n=c, m; doubledouble y=1.0, z=u; if (n<0) n=-n;
      while (1) {
        m=n; n/=2;
        if (n+n!=m) { // if m odd
           y*=z; if (0==n) return y;
        }
        z=sqr(z); 
      }
    }
  }
}

// Like Miller's modr
// a=n*b+rem, |rem|<=b/2, exact result.
doubledouble modr(const doubledouble a, const doubledouble b, int& n, doubledouble& rem) {
  if (0.0==b.h()) {
    std::cerr<<"\ndoubledouble: divisor is zero in modr!\n";
    DOMAIN_ERROR;
  }
  doubledouble temp;
  temp=a/b;
  n=int(rint(temp.h()));
  temp=n*doubledouble(b.h());
  rem=doubledouble(a.h());
  temp=rem-temp;
  rem=doubledouble(a.l());
  temp=rem+temp;
  rem=n*doubledouble(b.l());
  rem=temp-rem;
  return rem;
}

doubledouble sin(const doubledouble& x) {
  static const doubledouble tab[9]={ // tab[b] := sin(b*Pi/16)...
    0.0,
    "0.1950903220161282678482848684770222409277",
    "0.3826834323650897717284599840303988667613",
    "0.5555702330196022247428308139485328743749",
    "0.7071067811865475244008443621048490392850",
    "0.8314696123025452370787883776179057567386",
    "0.9238795325112867561281831893967882868225",
    "0.9807852804032304491261822361342390369739",
    1.0
  };
  static const doubledouble sinsTab[7] = { // Chebyshev coefficients
    "0.9999999999999999999999999999993767021096",
    "-0.1666666666666666666666666602899977158461",
    "8333333333333333333322459353395394180616.0e-42",
    "-1984126984126984056685882073709830240680.0e-43",
    "2755731922396443936999523827282063607870.0e-45",
    "-2505210805220830174499424295197047025509.0e-47",
    "1605649194713006247696761143618673476113.0e-49"
  };
  if (fabs(x.h())<1.0e-7) return x*(1.0-sqr(x)/3);
  int a,b; doubledouble sins, coss, k1, k3, t2, s, s2, sinb, cosb;
  // reduce x: -Pi < x <= Pi
  k1=x/doubledouble::TwoPi; k3=k1-rint(k1);
  // determine integers a and b to minimize |s|, where  s=x-a*Pi/2-b*Pi/16
  t2=4*k3;
  a=int(rint(t2));
  b=int(rint((8*(t2-a))));   // must have |a|<=2 and |b|<=7 now
  s=doubledouble::Pi*(k3+k3-(8*a+b)/16.0); // s is now reduced argument. -Pi/32 < s < Pi/32
  s2=sqr(s); 
  // Chebyshev series on -Pi/32..Pi/32, max abs error 2^-98=3.16e-30, whereas
  // power series has error 6e-28 at Pi/32 with terms up to x^13 included.
  sins=s*(sinsTab[0]+(sinsTab[1]+(sinsTab[2]+(sinsTab[3]+(sinsTab[4]+
         (sinsTab[5]+sinsTab[6]*s2)*s2)*s2)*s2)*s2)*s2);
  coss=sqrt(1.0-sqr(sins));  // ok as -Pi/32 < s < Pi/32
  // here sinb=sin(b*Pi/16) etc.
  sinb= (b>=0) ? tab[b] : -tab[-b]; cosb=tab[8-abs(b)];
  if (0==a) {
    return  sins*cosb+coss*sinb;
  } else if (1==a) {
    return -sins*sinb+coss*cosb;
  } else if (-1==a) {
    return  sins*sinb-coss*cosb;
  } else  { // |a|=2
    return -sins*cosb-coss*sinb;
  }
  // i.e. return sins*(cosa*cosb-sina*sinb)+coss*(sina*cosb+cosa*sinb);
}

// sin and cos.   Faster than separate calls of sin and cos.
void sincos(const doubledouble x, doubledouble& sinx, doubledouble& cosx) { 
  static const doubledouble tab[9]={ // tab[b] := sin(b*Pi/16)...
    0.0,
    "0.1950903220161282678482848684770222409277",
    "0.3826834323650897717284599840303988667613",
    "0.5555702330196022247428308139485328743749",
    "0.7071067811865475244008443621048490392850",
    "0.8314696123025452370787883776179057567386",
    "0.9238795325112867561281831893967882868225",
    "0.9807852804032304491261822361342390369739",
    1.0
  };
  static const doubledouble sinsTab[7] = { // Chebyshev coefficients
    "0.9999999999999999999999999999993767021096",
    "-0.1666666666666666666666666602899977158461",
    "8333333333333333333322459353395394180616.0e-42",
    "-1984126984126984056685882073709830240680.0e-43",
    "2755731922396443936999523827282063607870.0e-45",
    "-2505210805220830174499424295197047025509.0e-47",
    "1605649194713006247696761143618673476113.0e-49"
  };
  if (fabs(x.h())<1.0e-11) { sinx=x; cosx=1.0-0.5*sqr(x); return; }
  int a,b; doubledouble sins, coss, k1, k3, t2, s, s2, sinb, cosb;
  k1=x/doubledouble::TwoPi; k3=k1-rint(k1);
  t2=4*k3;
  a=int(rint(t2));
  b=int(rint((8*(t2-a))));
  s=doubledouble::Pi*(k3+k3-(8*a+b)/16.0);
  s2=sqr(s);
  sins=s*(sinsTab[0]+(sinsTab[1]+(sinsTab[2]+(sinsTab[3]+(sinsTab[4]+
         (sinsTab[5]+sinsTab[6]*s2)*s2)*s2)*s2)*s2)*s2);
  coss=sqrt(1.0-sqr(sins)); // ok, sins is small
  sinb= (b>=0) ? tab[b] : -tab[-b]; cosb=tab[8-abs(b)];
  // sin(x)=
  // sin(s)(cos(1/2 a Pi) cos(1/16 b Pi) - sin(1/2 a Pi) sin(1/16 b Pi))
  // cos(s)(sin(1/2 a Pi) cos(1/16 b Pi) + cos(1/2 a Pi) sin(1/16 b Pi))
  // cos(x)=
  // cos(s)(cos(1/2 a Pi) cos(1/16 b Pi) - sin(1/2 a Pi) sin(1/16 b Pi))
  //-sin(s)(sin(1/2 a Pi) cos(1/16 b Pi) + cos(1/2 a Pi) sin(1/16 b Pi))
  if (0==a) {
    sinx= sins*cosb+coss*sinb;
    cosx= coss*cosb-sins*sinb;
  } else if (1==a) {
    sinx=-sins*sinb+coss*cosb;
    cosx=-coss*sinb-sins*cosb;
  } else if (-1==a) {
    sinx= sins*sinb-coss*cosb;
    cosx= coss*sinb+sins*cosb;
  } else  { // |a|=2
    sinx=-sins*cosb-coss*sinb;
    cosx=-coss*cosb+sins*sinb;
  }
  return;
}

// cos
doubledouble cos(const doubledouble& x) { return sin(doubledouble::Pion2-x); }

// hyperbolic
doubledouble sinh(const doubledouble& x) { 
  if (fabs(x.h())<1.0e-5) { // avoid cancellation in e^x-e^(-x), use Taylor series...
    doubledouble q=sqr(x);  return x*(1+q/6*(1+q/20*(1+q/42))); }
  doubledouble t=exp(x); 
  return 0.5*(t-recip(t)); 
}
doubledouble cosh(const doubledouble& x) { doubledouble t=exp(x); return 0.5*(t+recip(t)); }
doubledouble tanh(const doubledouble& z) {
  doubledouble e;
  if (z.h()>0.0) { e=exp(-2.0*z); return (1.0-e)/(1.0+e); }
            else { e=exp( 2.0*z); return (e-1.0)/(1.0+e); }
}

doubledouble atan(const doubledouble& x) {
  double xh=x.h();
  if (0.0==xh) return doubledouble(0.0);
  // Asymptotic formula for large |x|...
  if (fabs(xh)>1.0e6) 
    { doubledouble r=recip(x), r2=sqr(r); return doubledouble::Pion2-r+r2*r*(1.0-"0.6"*r2)/3; }
  doubledouble s,c,a=atan(xh); // a=double approx to result
  sincos(a,s,c);
  return a+c*(c*x-s);  // Newton step
}

doubledouble atan2(const doubledouble& qy, const doubledouble& qx) { // Based on GNU libc atan2.c
  static const double one=1.0, zero=0.0; double x, y;
  x=qx.h(); y=qy.h();
  double signx, signy;
  if (x!=x) return qx; // x=NaN
  if (y!=y) return qy;
  signy=copysign(one,y);
  signx=copysign(one,x);
  if (y==zero) return signx==one ? qy : Qcopysign(doubledouble::Pi,signy);
  if (x==zero) return Qcopysign(doubledouble::Pion2,signy);
  if (std::isinf(x)) {
    if (std::isinf(y)) return Qcopysign(signx==one ? doubledouble::Pion4 : 3.0*doubledouble::Pion4,signy);
    else            return Qcopysign(signx==one ? doubledouble(0.0) : doubledouble::Pi,signy);
  }
  if (std::isinf(y)) return Qcopysign(doubledouble::Pion2,signy);
  doubledouble aqy=fabs(qy);
  if (x<0.0) // X is negative.
    return Qcopysign(doubledouble::Pi-atan(aqy/(-qx)),signy);
  return Qcopysign(atan(aqy/qx),signy);
}

// arcsin
doubledouble asin(const doubledouble& x) {
  if (fabs(x)>doubledouble(1.0)) 
    { std::cerr<<"\ndoubledouble |Argument|>1 in asin!\n"; DOMAIN_ERROR; }
  return atan2(x,sqrt(1.0-sqr(x)));
}

// KMB 96 Oct 28 version 0.3 97 Dec 22 OK now.
// erfc written 97 Dec 22, NOT CHECKED!!!!!!!
// Based on series and continued fraction for incomplete gamma function.

doubledouble erf(const doubledouble x) {
  if (0.0==x.h()) return 0.0;
  int i; doubledouble y,r;
  // const below is evalf(1/sqrt(Pi),40)
  static const doubledouble oneonrootpi="0.564189583547756286948079451560772585844";
  const double cut=1.5;  // switch to continued fraction here
  y=fabs(x);
  if (y.h()>26.0) { // erf is +or- 1 to 300 dp.
    r=1;
  } else if (y.h()<cut) { // use power series
    doubledouble ap=0.5,s,t,x2;
    s=t=2.0; x2=sqr(x);
    for (i=0; i<200; i++) {
      ap+=1;
      t*=x2/ap;
      s+=t;
      if (fabs(t.h())<1e-35*fabs(s.h())) {
        r=x*oneonrootpi*s/exp(x2);
	break;
      }
    }
    if (i>199) 
      { std::cerr<<"\ndoubledouble: no convergence in erf power series, x="<<x<<std::endl; exit(1); }
  } else { // |x|>=cut, use continued fraction, Lentz method
    double an,small=1e-300;
    doubledouble b,c,d,h,del,x2;
    x2=sqr(x);
    b=x2+0.5;
    c=1.0e300;
    d=recip(b);
    h=d;
    for (i=1; i<300; i++) {
      an=i*(0.5-i);
      b+=2.0;
      d=an*d+b;
      if (fabs(d.h())<small) d=small;
      c=b+an/c;
      if (fabs(c.h())<small) c=small;
      d=recip(d);
      del=d*c;
      h*=del;
      if (del.h()==1.0 && del.l()<1.0e-30) break;
      if (299==i)
        { std::cerr<<"\ndoubledouble: no convergence in erf continued fraction, x="<<x<<std::endl; exit(1); }
    }
    r=1.0-oneonrootpi*sqrt(x2)/exp(x2)*h;
  } 
  if (x.h()>0.0) return r; else return -r;
}

doubledouble erfc(const doubledouble x) {
  if (0.0==x.h()) return 1.0;
  if (x.h()<0.0) return 1.0-erf(x);
  int i; doubledouble y,r;
  // const below is evalf(1/sqrt(Pi),40)
  static const doubledouble oneonrootpi="0.564189583547756286948079451560772585844";
  const double cut=1.5;  // switch to continued fraction here
  y=fabs(x);
  if (y.h()<cut) { // use power series
    doubledouble ap=0.5,s,t,x2;
    s=t=2.0; x2=sqr(x);
    for (i=0; i<200; i++) {
      ap+=1;
      t*=x2/ap;
      s+=t;
      if (fabs(t.h())<1e-35*fabs(s.h())) {
        r=1.0-x*oneonrootpi*s/exp(x2);
	break;
      }
    }
    if (i>199) { std::cerr<<"\ndoubledouble: no convergence in erfc power series, x="<<x<<std::endl; exit(1); }
  } else { // |x|>=cut, use continued fraction
    double an,small=1e-300;
    doubledouble b,c,d,h,del,x2;
    x2=sqr(x);
    b=x2+0.5;
    c=1e300;
    h=d=recip(b);
    for (i=1; i<300; i++) {
      an=i*(0.5-i);
      b+=2.0;
      d=an*d+b;
      if (fabs(d.h())<small) d=small;
      c=b+an/c;
      if (fabs(c.h())<small) c=small;
      d=recip(d);
      del=d*c;
      h*=del;
      if (del.h()==1.0 && del.l()<1.0e-30) break;
      if (299==i) { std::cerr<<"\ndoubledouble: no convergence in erfc continued fraction, x="<<x<<std::endl; exit(1);}
    }
    r=oneonrootpi*sqrt(x2)/exp(x2)*h;
  } 
  return r;
}

doubledouble gamma(const doubledouble x) {
  const int n=43; // don't really need so many!
  static const doubledouble c[n]={ // Taylor coefficients for 1/gamma(1+x)-x...
    "+0.5772156649015328606065120900824024310421593359",
    "-0.6558780715202538810770195151453904812797663805",
    "-0.0420026350340952355290039348754298187113945004",
    "+0.1665386113822914895017007951021052357177815022",
    "-0.0421977345555443367482083012891873913016526841",
    "-0.0096219715278769735621149216723481989753629422",
    "+0.0072189432466630995423950103404465727099048009",
    "-0.0011651675918590651121139710840183886668093337",
    "-0.0002152416741149509728157299630536478064782419",
    "+0.0001280502823881161861531986263281643233948920",
    "-0.0000201348547807882386556893914210218183822948",
    "-0.0000012504934821426706573453594738330922423226",
    "+0.0000011330272319816958823741296203307449433240",
    "-0.0000002056338416977607103450154130020572836512",
    "+0.0000000061160951044814158178624986828553428672",
    "+0.0000000050020076444692229300556650480599913030",
    "-0.0000000011812745704870201445881265654365055777",
    "+1.0434267116911005104915403323122501914007098231E-10",
    "+7.7822634399050712540499373113607772260680861813E-12",
    "-3.6968056186422057081878158780857662365709634513E-12",
    "+5.1003702874544759790154813228632318027268860697E-13",
    "-2.0583260535665067832224295448552374197460910808E-14",
    "-5.3481225394230179823700173187279399489897154781E-15",
    "+1.2267786282382607901588938466224224281654557504E-15",
    "-1.1812593016974587695137645868422978312115572918E-16",
    "+1.1866922547516003325797772429286740710884940796E-18",
    "+1.4123806553180317815558039475667090370863507503E-18",
    "-2.2987456844353702065924785806336992602845059314E-19",
    "+1.7144063219273374333839633702672570668126560625E-20",
    "+1.3373517304936931148647813951222680228750594717E-22",
    "-2.0542335517666727893250253513557337966820379352E-22",
    "+2.7360300486079998448315099043309820148653116958E-23",
    "-1.7323564459105166390574284515647797990697491087E-24",
    "-2.3606190244992872873434507354275310079264135521E-26",
    "+1.8649829417172944307184131618786668989458684290E-26",
    "+2.2180956242071972043997169136268603797317795006E-27",
    "+1.2977819749479936688244144863305941656194998646E-28",
    "+1.1806974749665284062227454155099715185596846378E-30",
    "-1.1245843492770880902936546742614395121194117955E-30",
    "+1.2770851751408662039902066777511246477487720656E-31",
    "-7.3914511696151408234612893301085528237105689924E-33",
    "+1.1347502575542157609541652594693063930086121959E-35",
    "+4.6391346410587220299448049079522284630579686797E-35"
  };
  doubledouble ss=x,f=1.0,sum=0.0,one=1.0;
  while (ss>one) { ss-=1; f*=ss; }
  while (ss<one) { f/=ss; ss+=1; }
  if (ss==one) return f;
  ss-=1.0;
  for (int i=n-1; i>=0; i--) sum=c[i]+ss*sum;
  return f/(ss*sum+1);
}

// end of math.cc
