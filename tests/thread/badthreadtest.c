/************************************************************************
 *
 *
 * Test for C-Wrapper of GEOS library
 *
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * Author: Sandro Santilli <strk@keybit.net>
 *
 ***********************************************************************/

#define _GNU_SOURCE

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "geos_c.h"

#define MAXWKTLEN 1047551

void
usage(char *me)
{
	fprintf(stderr, "Usage: %s <wktfile>\n", me);
	exit(1);
}

void
notice1(const char *fmt, ...) {
        va_list ap;

        fprintf( stdout, "NOTICE1: ");

        va_start (ap, fmt);
        vfprintf( stdout, fmt, ap);
        va_end(ap);
        fprintf( stdout, "\n" );
}

void
notice2(const char *fmt, ...) {
        va_list ap;

        fprintf( stdout, "NOTICE2: ");

        va_start (ap, fmt);
        vfprintf( stdout, fmt, ap);
        va_end(ap);
        fprintf( stdout, "\n" );
}

void
notice(const char *fmt, ...) {
	va_list ap;

        fprintf( stdout, "NOTICE: ");
        
	va_start (ap, fmt);
        vfprintf( stdout, fmt, ap);
        va_end(ap);
        fprintf( stdout, "\n" );
}

void
log_and_exit(const char *fmt, ...) {
	va_list ap;

        fprintf( stdout, "ERROR: ");
        
	va_start (ap, fmt);
        vfprintf( stdout, fmt, ap);
        va_end(ap);
        fprintf( stdout, "\n" );
	exit(1);
}

void
log_and_exit1(const char *fmt, ...) {
        va_list ap;

        fprintf( stdout, "ERROR1: ");

        va_start (ap, fmt);
        vfprintf( stdout, fmt, ap);
        va_end(ap);
        fprintf( stdout, "\n" );
        pthread_exit(NULL);
}

void
log_and_exit2(const char *fmt, ...) {
        va_list ap;

        fprintf( stdout, "ERROR2: ");

        va_start (ap, fmt);
        vfprintf( stdout, fmt, ap);
        va_end(ap);
        fprintf( stdout, "\n" );
        pthread_exit(NULL);
}

GEOSGeometry* 
fineGrainedReconstructionTest(const GEOSGeometry* g1)
{
	GEOSCoordSequence* cs;
	GEOSGeometry* g2;
	GEOSGeometry* shell;
	const GEOSGeometry* gtmp;
	GEOSGeometry**geoms;
	unsigned int ngeoms, i;
	int type;

	/* Geometry reconstruction from CoordSeq */
	type = GEOSGeomTypeId(g1);
	switch ( type )
	{
		case GEOS_POINT:
			cs = GEOSCoordSeq_clone(GEOSGeom_getCoordSeq(g1));
			g2 = GEOSGeom_createPoint(cs);
			return g2;
			break;
		case GEOS_LINESTRING:
			cs = GEOSCoordSeq_clone(GEOSGeom_getCoordSeq(g1));
			g2 = GEOSGeom_createLineString(cs);
			return g2;
			break;
		case GEOS_LINEARRING:
			cs = GEOSCoordSeq_clone(GEOSGeom_getCoordSeq(g1));
			g2 = GEOSGeom_createLinearRing(cs);
			return g2;
			break;
		case GEOS_POLYGON:
			gtmp = GEOSGetExteriorRing(g1);
			cs = GEOSCoordSeq_clone(GEOSGeom_getCoordSeq(gtmp));
			shell = GEOSGeom_createLinearRing(cs);
			ngeoms = GEOSGetNumInteriorRings(g1);
			geoms = malloc(ngeoms*sizeof(GEOSGeometry*));
			for (i=0; i<ngeoms; i++)
			{
				gtmp = GEOSGetInteriorRingN(g1, i);
				cs = GEOSCoordSeq_clone(GEOSGeom_getCoordSeq(gtmp));
				geoms[i] = GEOSGeom_createLinearRing(cs);
			}
			g2 = GEOSGeom_createPolygon(shell, geoms, ngeoms);
			free(geoms);
			return g2;
			break;
		case GEOS_MULTIPOINT:
		case GEOS_MULTILINESTRING:
		case GEOS_MULTIPOLYGON:
		case GEOS_GEOMETRYCOLLECTION:
			ngeoms = GEOSGetNumGeometries(g1);
			geoms = malloc(ngeoms*sizeof(GEOSGeometry*));
			for (i=0; i<ngeoms; i++)
			{
				gtmp = GEOSGetGeometryN(g1, i);
				geoms[i] = fineGrainedReconstructionTest(gtmp);
			}
			g2 = GEOSGeom_createCollection(type, geoms, ngeoms);
			free(geoms);
			return g2; 
			break;
		default:
			log_and_exit("Unknown geometry type %d\n", type);
			return NULL;
	}
}

void
printHEX(FILE *where, const unsigned char *bytes, size_t n)
{
	static char hex[] = "0123456789ABCDEF";
	int i;

	for (i=0; i<n; i++)
	{
		fprintf(where, "%c%c", hex[bytes[i]>>4], hex[bytes[i]&0x0F]);
	}
}

int
do_all(char *inputfile)
{
	GEOSGeometry* g1;
	GEOSGeometry* g2;
	GEOSGeometry* g3;
	GEOSGeometry* g4;
	const GEOSGeometry **gg;
	unsigned int npoints, ndims;
        static char wkt[MAXWKTLEN];
	FILE *input;
	char *ptr;
	unsigned char* uptr;
	size_t size;
	double dist, area;

	input = fopen(inputfile, "r");
	if ( ! input ) { perror("fopen"); exit(1); }

	size = fread(wkt, 1, MAXWKTLEN-1, input);
	fclose(input);
	if ( ! size ) { perror("fread"); exit(1); }
	if ( size == MAXWKTLEN-1 ) { perror("WKT input too big!"); exit(1); }
	wkt[size] = '\0'; /* ensure it is null terminated */

	/* WKT input */
	g1 = GEOSGeomFromWKT(wkt);

	/* WKT output */
	ptr = GEOSGeomToWKT(g1);
	printf("Input (WKT): %s\n", ptr); 
	free(ptr);

	/* WKB output */
	uptr = GEOSGeomToWKB_buf(g1, &size);
	printf("Input (WKB): "); printHEX(stdout, uptr, size); putchar('\n');

	/* WKB input */
	g2 = GEOSGeomFromWKB_buf(uptr, size); free(uptr);
	if ( ! GEOSEquals(g1, g2) ) log_and_exit("Round WKB conversion failed");
	GEOSGeom_destroy(g2);

	/* Size and dimension */
	npoints = GEOSGetNumCoordinates(g1);
	ndims = GEOSGeom_getDimensions(g1);
	printf("Geometry coordinates: %dx%d\n", npoints, ndims);

	/* Geometry fine-grained deconstruction/reconstruction  test */
	g2 = fineGrainedReconstructionTest(g1);
	if ( ! GEOSEquals(g1, g2) )
	{
		log_and_exit("Reconstruction test failed\n");
	}
	GEOSGeom_destroy(g2);

	/* Unary predicates */
	if ( GEOSisEmpty(g1) ) printf("isEmpty\n");
	if ( GEOSisValid(g1) ) printf("isValid\n");
	if ( GEOSisSimple(g1) ) printf("isSimple\n");
	if ( GEOSisRing(g1) ) printf("isRing\n");

	/* Convex Hull */
	g2 = GEOSConvexHull(g1);
	if ( ! g2 )
	{
		log_and_exit("GEOSConvexHull() raised an exception");
	}
	ptr = GEOSGeomToWKT(g2);
	printf("ConvexHull: %s\n", ptr); 
	free(ptr);

	/* Buffer */
	GEOSGeom_destroy(g1);
	g1 = GEOSBuffer(g2, 100, 30);
	if ( ! g1 )
	{
		log_and_exit("GEOSBuffer() raised an exception");
	}
	ptr = GEOSGeomToWKT(g1);
	printf("Buffer: %s\n", ptr); 
	free(ptr);


	/* Intersection */
	g3 = GEOSIntersection(g1, g2);
	if ( ! GEOSEquals(g3, g2) )
	{
		GEOSGeom_destroy(g1);
		GEOSGeom_destroy(g2);
		GEOSGeom_destroy(g3);
		log_and_exit("Intersection(g, Buffer(g)) didn't return g");
	}
	ptr = GEOSGeomToWKT(g3);
	printf("Intersection: %s\n", ptr); 
	GEOSGeom_destroy(g3);
	free(ptr);

	/* Difference */
	g3 = GEOSDifference(g1, g2);
	ptr = GEOSGeomToWKT(g3);
	printf("Difference: %s\n", ptr); 
	GEOSGeom_destroy(g3);
	free(ptr);

	/* SymDifference */
	g3 = GEOSSymDifference(g1, g2);
	ptr = GEOSGeomToWKT(g3);
	printf("SymDifference: %s\n", ptr); 
	free(ptr);

	/* Boundary */
	g4 = GEOSBoundary(g3);
	ptr = GEOSGeomToWKT(g4);
	printf("Boundary: %s\n", ptr); 
	GEOSGeom_destroy(g3);
	GEOSGeom_destroy(g4);
	free(ptr);

	/* Union */
	g3 = GEOSUnion(g1, g2);
	if ( ! GEOSEquals(g3, g1) )
	{
		GEOSGeom_destroy(g1);
		GEOSGeom_destroy(g2);
		GEOSGeom_destroy(g3);
		log_and_exit("Union(g, Buffer(g)) didn't return Buffer(g)");
	}
	ptr = GEOSGeomToWKT(g3);
	printf("Union: %s\n", ptr); 
	free(ptr);

	/* PointOnSurcace */
	g4 = GEOSPointOnSurface(g3);
	ptr = GEOSGeomToWKT(g4);
	printf("PointOnSurface: %s\n", ptr); 
	GEOSGeom_destroy(g3);
	GEOSGeom_destroy(g4);
	free(ptr);

	/* Centroid */
	g3 = GEOSGetCentroid(g2);
	ptr = GEOSGeomToWKT(g3);
	printf("Centroid: %s\n", ptr); 
	GEOSGeom_destroy(g3);
	free(ptr);

	/* Relate (and RelatePattern )*/
	ptr = GEOSRelate(g1, g2);
	if ( ! GEOSRelatePattern(g1, g2, ptr) )
	{
		GEOSGeom_destroy(g1);
		GEOSGeom_destroy(g2);
		free(ptr);
		log_and_exit("! RelatePattern(g1, g2, Relate(g1, g2))");
	}
	printf("Relate: %s\n", ptr); 
	free(ptr);

	/* Polygonize */
	gg = (const GEOSGeometry**)malloc(2*sizeof(GEOSGeometry*));
	gg[0] = g1;
	gg[1] = g2;
	g3 = GEOSPolygonize(gg, 2);
	free(gg);
	if ( ! g3 )
	{
		log_and_exit("Exception running GEOSPolygonize");
	}
	ptr = GEOSGeomToWKT(g3);
	GEOSGeom_destroy(g3);
	printf("Polygonize: %s\n", ptr); 
	free(ptr);

	/* LineMerge */
	g3 = GEOSLineMerge(g1);
	if ( ! g3 )
	{
		log_and_exit("Exception running GEOSLineMerge");
	}
	ptr = GEOSGeomToWKT(g3);
	printf("LineMerge: %s\n", ptr); 
	free(ptr);
	GEOSGeom_destroy(g3);

	/* Binary predicates */
	if ( GEOSIntersects(g1, g2) ) printf("Intersect\n");
	if ( GEOSDisjoint(g1, g2) ) printf("Disjoint\n");
	if ( GEOSTouches(g1, g2) ) printf("Touches\n");
	if ( GEOSCrosses(g1, g2) ) printf("Crosses\n");
	if ( GEOSWithin(g1, g2) ) printf("Within\n");
	if ( GEOSContains(g1, g2) ) printf("Contains\n");
	if ( GEOSOverlaps(g1, g2) ) printf("Overlaps\n");

	/* Distance */
	if ( GEOSDistance(g1, g2, &dist) ) printf("Distance: %g\n", dist);

	/* Area */
	if ( GEOSArea(g1, &area) ) printf("Area 1: %g\n", area);
	if ( GEOSArea(g2, &area) ) printf("Area 2: %g\n", area);

	GEOSGeom_destroy(g2);

	/* Simplify */
	g3 = GEOSSimplify(g1, 0.5);
	ptr = GEOSGeomToWKT(g3);
	printf("Simplify: %s\n", ptr); 
	free(ptr);
	GEOSGeom_destroy(g3);
    
	/* Topology Preserve Simplify */
	g3 = GEOSTopologyPreserveSimplify(g1, 0.5);
	ptr = GEOSGeomToWKT(g3);
	printf("Simplify: %s\n", ptr); 
	free(ptr);
	GEOSGeom_destroy(g3);

	GEOSGeom_destroy(g1);

	return 0;
}

void *threadfunc1( void *arg )
{
    initGEOS( notice1, log_and_exit1 );
    printf("GEOS version %s\n", GEOSversion());
    putc('.', stderr); fflush(stderr);
    do_all((char*)arg);
    putc('+', stderr); fflush(stderr);
    finishGEOS();

    pthread_exit(NULL);
}

void *threadfunc2( void *arg )
{
    initGEOS( notice2, log_and_exit2 );
    printf("GEOS version %s\n", GEOSversion());
    putc('.', stderr); fflush(stderr);
    do_all((char *)arg);
    putc('+', stderr); fflush(stderr);
    finishGEOS();

    pthread_exit(NULL);
}

int
main(int argc, char **argv)
{
    pthread_t thread1, thread2;

    if ( argc < 2 ) usage(argv[0]);
    pthread_create( &thread1, NULL, threadfunc1, argv[1] );
    pthread_create( &thread2, NULL, threadfunc2, argv[1] );

    pthread_join( thread1, NULL );
    pthread_join( thread2, NULL );

    return EXIT_SUCCESS;
}

