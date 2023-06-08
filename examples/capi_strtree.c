/*
* # GEOS C API example 3
*
* Build a spatial index and search it for a
* nearest neighbor and for a query bounds.
*
* cc -I/usr/local/include capi_strtree.c -o capi_strtree -L/usr/local/lib -lgeos_c
*/

/* System headers */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Only the CAPI header is required */
#include <geos_c.h>

/*
* GEOS requires two message handlers to return
* error and notice message to the calling program.
*
*   typedef void(* GEOSMessageHandler) (const char *fmt,...)
*
* Here we stub out an example that just prints the
* messages to stdout.
*/
static void
geos_message_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}

/*
* An application will want to index items, which have
* some attributes and a geometry part.
*/
typedef struct
{
    GEOSGeometry* geom;
    size_t id;
} item_t;

/*
* Generate a random item with a location in the range of
* POINT(0..range, 0..range). Caller must free.
*/
static item_t *
random_item(double range)
{
    item_t* item = malloc(sizeof(item_t));
    double x = range * rand() / RAND_MAX;
    double y = range * rand() / RAND_MAX;
    /* Make a point in the point grid */
    item->geom = GEOSGeom_createPointFromXY(x, y);
    item->id = rand();
    return item;
}

/*
* Free an item and its geometry.
*/
void
free_item(item_t* item)
{
    if (item && item->geom) GEOSGeom_destroy(item->geom);
    if (item) free(item);
}

/*
* Item distance callback for GEOSSTRtree_nearest_generic()
*/
int
itemDistanceCallback(const void* item1, const void* item2, double* distance, void* userdata)
{
    item_t* obj1 = (item_t*)item1;
    item_t* obj2 = (item_t*)item2;
    return GEOSDistance(obj1->geom, obj2->geom, distance);
}

/*
* Item query callback for GEOSSTRtree_query()
*/
void
itemQueryCallback(void* item, void* userdata)
{
    double x, y;
    item_t* i = (item_t*)item;
    GEOSGeomGetX(i->geom, &x);
    GEOSGeomGetY(i->geom, &y);
    printf("Found item %10zu at (%g, %g)\n", i->id, x, y);
}


int main()
{
    /* Send notice and error messages to our stdout handler */
    initGEOS(geos_message_handler, geos_message_handler);

    /* How many random items to add to our index */
    const size_t nItems = 10000;
    /* The coordinate range of the random locations (0->100.0) */
    const double range = 100.0;
    /* Set the seed for rand() */
    srand(time(NULL));

    /*
    * The tree doesn't take ownership of inputs, it just
    * holds pointers, so we keep a list of allocated items
    * handy in an array for future clean-up
    */
    item_t* items[nItems];
    /*
    * The create parameter for the tree is not the
    * number of inputs, it is the number of entries
    * per node. 10 is a good default number to use.
    */
    GEOSSTRtree* tree = GEOSSTRtree_create(10);
    for (size_t i = 0; i < nItems; i++) {
        /* Make a random point */
        item_t* item = random_item(range);
        /* Store away a reference so we can free it after */
        items[i] = item;
        /* Add an entry for it to the tree */
        GEOSSTRtree_insert(tree, item->geom, item);
    }

    /* Prepare to write some geometries out as text */
    GEOSWKTWriter* writer = GEOSWKTWriter_create();
    /* Trim trailing zeros off output */
    GEOSWKTWriter_setTrim(writer, 1);
    GEOSWKTWriter_setRoundingPrecision(writer, 3);

    /* Prepare to read some geometries in as text */
    GEOSWKTReader* reader = GEOSWKTReader_create();

    /* Random item to query the index with */
    item_t* item_random = random_item(range);
    /* Nearest item in the index to our random item */
    const item_t* item_nearest = GEOSSTRtree_nearest_generic(
        tree,                 // STRTree to query
        item_random,          // Item to use in search
        item_random->geom,    // Geometry to seed search
        itemDistanceCallback, // Callback to process nearest object
        NULL);                // Userdata to hand to the callback

    /* Convert geometry to WKT */
    char* wkt_random  = GEOSWKTWriter_write(writer, item_random->geom);
    char* wkt_nearest = GEOSWKTWriter_write(writer, item_nearest->geom);

    /* Print random query point and nearest point */
    printf(" Random Point: %s\n", wkt_random);
    printf("Nearest Point: %s\n", wkt_nearest);

    /* Don't forget to free memory allocated for WKT! */
    GEOSFree(wkt_random);
    GEOSFree(wkt_nearest);

    /* Set up a query rectangle for index query */
    const char* wkt_bounds = "POLYGON((20 20, 22 20, 22 22, 20 22, 20 20))";
    GEOSGeometry* geom_query = GEOSWKTReader_read(reader, wkt_bounds);

    /* Find all items that touch the bounds */
    /* For non-rectangular query geometry, this will be an over-determined set */
    GEOSSTRtree_query(
        tree,              // STRTree to query
        geom_query,        // GEOSGeometry query bounds
        itemQueryCallback, // Callback to process index entries that pass query
        NULL);             // Userdata to hand to the callback

    /* Free the query bounds geometry */
    GEOSGeom_destroy(geom_query);

    /* Free the WKT writer and reader */
    GEOSWKTWriter_destroy(writer);
    GEOSWKTReader_destroy(reader);

    /* Freeing the tree does not free the tree inputs */
    GEOSSTRtree_destroy(tree);

    /* Free all the items in our random item list */
    for (size_t i = 0; i < nItems; i++) {
        free_item(items[i]);
    }
    /* Free our working random item */
    free_item(item_random);

    /* Clean up the global context */
    finishGEOS();

    /* Done */
    return 0;
}
