---
title: "GeoJSON"
date: 2021-10-04T14:18:42-07:00
draft: false
---

"GeoJSON" is a standard for structuring JSON when encoding geometry and features. By using GeoJSON, rather than some other arbitrary scheme for structuring JSON, you maximize the interoperability of your JSON output. There are dozens of tools and [websites](http://geojson.io) that happily consume and emit GeoJSON.

The GeoJSON standard is formally maintained by the IETF as "[RFC 7946](https://datatracker.ietf.org/doc/html/rfc7946)".

Unlike [WKB]({{< ref "wkb" >}}) and [WKT]({{< ref "wkt" >}}), GeoJSON does not restrict itself to just geometry representation. It also standardizes the transport of attribution. The three key levels of GeoJSON are:

* [Geometry](https://datatracker.ietf.org/doc/html/rfc7946#section-3.1), representation of Points, LineStrings, Polygons, etc.
* [Feature](https://datatracker.ietf.org/doc/html/rfc7946#section-3.2), representation of an object that has a "geometry" and an arbitrary set of other non-geometric "properties".
* [FeatureCollection](https://datatracker.ietf.org/doc/html/rfc7946#section-3.3), representation of a list of Features.

Since GEOS is almost 100% concerned with geometry operations, there is no GEOS abstraction to translate the non-geometric properties of GeoJSON Feature into, so handling of Feature properties is only available via the C++ [GeoJSON.h](https://github.com/libgeos/geos/blob/main/include/geos/io/GeoJSON.h) utilities.


### Writing GeoJSON

The GeoJSON writer, in both the C and C++ APIs, only supports writing out the Geometry portion of GeoJSON. So for writing features, the writer will end up embedded in some larger JSON emitter.

```c
/* Read a linestring */
const char* linestring = "LINESTRING(0 0 1, 1 1 1, 2 1 2)";
GEOSWKTReader* reader = GEOSWKTReader_create();
GEOSGeom* geom = GEOSWKTReader_read(reader, linestring);

/* Write it out as GeoJSON */
GEOSGeoJSONWriter* writer = GEOSGeoJSONWriter_create();

/* Generate the JSON, with an indentation of 2 */
int indentation = 2;
unsigned char* json = GEOSGeoJSONWriter_writeGeometry(writer, geom, indentation);

/* do something ... */

/* Free the WKB */
GEOSFree(json);
GEOSGeom_destroy(geom);
GEOSGeoJSONWriter_destroy(writer);
GEOSWKTReader_destroy(reader);
```

### Reading GeoJSON

The C++ GeoJSON reader does include the option to read full `Feature` and `FeatureCollection` objects, with a narrow API for reading the resulting objects.

```c++
#include <geos/io/GeoJSON.h> // GeoJSONFeatureCollection, etc
#include <geos/io/GeoJSONReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <fstream>

using namespace geos::io;
using namespace geos::geom;

int main()
{
    // Read file into string
    std::ifstream ifs("geojson.json");
    std::string content((std::istreambuf_iterator<char>(ifs) ),
                        (std::istreambuf_iterator<char>()    ));

    // Parse GeoJSON string into GeoJSON objects
    GeoJSONReader reader;
    GeoJSONFeatureCollection fc = reader.readFeatures(content);

    // Prepare WKT writer
    WKTWriter writer;
    writer.setTrim(true); // Only needed before GEOS 3.12
    writer.setRoundingPrecision(2);

    // Print out the features
    for (auto& feature : fc.getFeatures()) {

        // Read the geometry
        const Geometry* geom = feature.getGeometry();

        // Read the properties
        const std::map<std::string, GeoJSONValue>& props = feature.getProperties();

        // Write all properties
        std::cout << "----------" << std::endl;
        for (const auto& prop : props) {
            std::cout << prop.first << ": " << prop.second.getString() << std::endl;
        }

        // Write WKT feometry
        std::cout << "geometry: " << writer.write(geom) << std::endl;
    }
}
```




