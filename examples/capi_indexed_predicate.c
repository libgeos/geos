/*
* # GEOS C API example 4
*
* Build a spatial index and search it for all points
* completely contained in arbitrary query polygon.
*
* cc -I/usr/local/include capi_indexed_predicate.c -o capi_indexed_predicate -L/usr/local/lib -lgeos_c
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
* A user data struct to pass to the index callback function
*/
typedef struct
{
    const GEOSPreparedGeometry* prepgeom;
    item_t** items;
    size_t  nItems;
    size_t  szItems;
} userdata_t;

/*
* Userdata both holds our output list of found items and
* our input PreparedGeometry for fast spatial tests.
*/
userdata_t *
userdata_init(GEOSGeometry* geom)
{
    userdata_t* ud = malloc(sizeof(userdata_t));
    ud->prepgeom = GEOSPrepare(geom);
    ud->nItems = 0;
    ud->szItems = 16;
    ud->items = malloc(sizeof(item_t*) * ud->szItems);
    return ud;
}

/*
* Free the items list and the PreparedGeometry
*/
void
userdata_free(userdata_t* ud)
{
    GEOSPreparedGeom_destroy(ud->prepgeom);
    free(ud->items);
    free(ud);
}


/*
* Generate a random item with a location in the range of
* POINT(0..range, 0..range). Caller must free.
*/
static item_t *
item_random(double range)
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
item_free(item_t* item)
{
    if (item && item->geom) GEOSGeom_destroy(item->geom);
    if (item) free(item);
}

/*
* Utility function to write out contents of item
*/
void
item_print(const item_t* item)
{
    double x, y;
    GEOSGeomGetX(item->geom, &x);
    GEOSGeomGetY(item->geom, &y);
    printf("item %10zu (%g, %g)\n", item->id, x, y);
}

/*
* Item query callback for GEOSSTRtree_query()
*/
void
itemQueryCallback(void* item, void* userdata)
{
    userdata_t* ud = (userdata_t*)userdata;
    item_t* indexitem = (item_t*)item;
    if (GEOSPreparedIntersects(ud->prepgeom, indexitem->geom)) {
        if (ud->nItems == ud->szItems) {
            ud->szItems *= 2;
            ud->items = realloc(ud->items, sizeof(item_t*) * ud->szItems);
        }
        ud->items[ud->nItems++] = indexitem;
    }
    return;
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
        item_t* item = item_random(range);
        /* Store away a reference so we can free it after */
        items[i] = item;
        /* Add an entry for it to the tree */
        GEOSSTRtree_insert(tree, item->geom, item);
    }

    /* Prepare to read geometries in as text */
    GEOSWKTReader* reader = GEOSWKTReader_create();

    /* Set up a query rectangle for index query */
    const char* wkt_bounds = "POLYGON((20 20, 20 24, 24 24, 24 23, 21 23, 21 21, 24 21, 24 20, 20 20))";
    GEOSGeometry* geom_query = GEOSWKTReader_read(reader, wkt_bounds);

    /* Set up the prepared geometry for the exact tests */
    userdata_t* ud = userdata_init(geom_query);

    /* Find all items that touch the bounds */
    /* For non-rectangular query geometry, this will be an over-determined set */
    GEOSSTRtree_query(
        tree,              // STRTree to query
        geom_query,        // GEOSGeometry query bounds
        itemQueryCallback, // Callback to process index entries that pass query
        ud);             // Userdata to hand to the callback

    /* Print out the items we found */
    printf("Found %zu items in the polygon: %s\n", ud->nItems, wkt_bounds);
    for (size_t i = 0; i < ud->nItems; i++) {
        item_print(ud->items[i]);
    }

    /* Done with the found items and prepared geometry now */
    userdata_free(ud);

    /* Free the query bounds geometry */
    GEOSGeom_destroy(geom_query);

    /* Freeing the tree does not free the tree inputs */
    GEOSSTRtree_destroy(tree);

    /* Free all the items in our random item list */
    for (size_t i = 0; i < nItems; i++) {
        item_free(items[i]);
    }

    /* Clean up the global context */
    finishGEOS();

    /* Done */
    return 0;
}
