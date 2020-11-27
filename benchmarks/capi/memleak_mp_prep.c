#include <stdio.h>
#include <assert.h>
#include "geos_c.h"

int main(void) {
  GEOSWKTReader *reader;
  GEOSGeometry *mp;
  GEOSGeometry *p;
  const GEOSPreparedGeometry *prep_mp;
  unsigned long int i;
  unsigned long int count = 1000000;

  initGEOS(NULL, NULL);

  reader = GEOSWKTReader_create();

  mp = GEOSWKTReader_read(reader,
    "MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0)))");

  p = GEOSWKTReader_read(reader,
    "POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))");

  assert(GEOSisValid(mp));
  assert(GEOSisValid(p));

  prep_mp = GEOSPrepare(mp);

  for (i=0; i<count; i++) {

    if ( !(i%100) ) printf("%lu iterations\n", i);

    /* does not leak */
    /* GEOSContains(mp, p); */

    /* leaks */
    GEOSPreparedContains(prep_mp, p);
  }

  printf("%lu iterations (END)\n", i);

  return 0;
}
