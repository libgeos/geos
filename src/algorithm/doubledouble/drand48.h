// Source: https://gist.github.com/mortennobel/8665258
// drand48() and srand48(long) on windows. (based on Freebsd: http://fxr.watson.org/fxr/ident?v=FREEBSD-LIBC;im=bigexcerpts;i=_dorand48 )
 
double erand48(unsigned short xseed[3]);
double drand48();
void srand48(long seed);