/**
 * # GEOS C++ example 2
 *
 * Using a custom object, generates multiple random
 * objects and then allows searching on them.
 *
 * This uses the TemplateSTRtree, a relatively new API.
 *
 * Program fills a 100x100 grid with random "Person" objects,
 * builds an index on those objects, and then queries the
 * index with a query shape. The default shape is a square.
 * Supply WKT on the commandline for alternate shapes.
 *
 * ./cpp_strtree 'POLYGON((30 30, 50 30, 50 70, 30 30))'
 */

#include <iostream>
#include <cstdlib>

/* For geometry operations */
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>

/* For indexing */
#include <geos/index/strtree/TemplateSTRtree.h>

/* For WKT read/write */
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>

/* Geometry */
using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Point;
using geos::index::strtree::TemplateSTRtree;


/* WKTReader/WKTWriter */
using geos::io::WKTReader;
using geos::io::WKTWriter;

/*
* An example of a class encapsulating some spatial and
* non-spatial information. A Person has a name and
* a location.
*/
class Person {

public:

    Person(const std::string& name, unsigned int x, unsigned int y, const GeometryFactory* gf)
        : m_location(gf->createPoint(Coordinate(x, y)))
        , m_name(name)
        {};

    // For the index to work, the class must have a
    // getEnvelopeInternal() method.
    const Envelope* getEnvelopeInternal() const
    {
        return m_location->getEnvelopeInternal();
    }

    const std::string& getName() const {
        return m_name;
    }

    const std::string getWkt() const {
        WKTWriter writer;
        writer.setTrim(true);
        return writer.write(m_location.get());
    }

    const bool intersects(const Geometry* geom) const {
        return m_location->intersects(geom);
    }

private:

    std::unique_ptr<Point> m_location;
    std::string m_name;

};

// For fun we build up the Person name automatically.
static std::string
number_to_name(unsigned int num)
{
    std::vector<std::string> first = {"John", "Paul", "Peter", "Matthew", "James", "Mary", "Ruth", "Eliza", "Margaret", "Enid"};
    std::vector<std::string> last = {"Smith", "John", "Ng", "Wong", "Kim", "Singh", "Ono", "Woo", "Cage", "Chandra"};
    unsigned int onenum = num % 10;
    unsigned int tennum = num / 10;
    std::string word = first[tennum] + " " + last[onenum];
    return word;
}

int
main(int argc, char **argv)
{
    // New factory with default (float) precision model
    GeometryFactory::Ptr factory = GeometryFactory::create();

    // A place to store our Person objects
    std::vector<std::unique_ptr<Person>> people;

    // STRtree index using Person* as the indexed item
    TemplateSTRtree<Person*> index;

    // Populate the index with Persons
    for (unsigned int i = 0; i < 100; i++) {

        // Calculate the name and location of this Person
        unsigned int x = std::rand() % 100;
        unsigned int y = std::rand() % 100;
        std::string name = number_to_name(i);

        // Create the object, store it, and add to the index
        Person *person = new Person(name, x, y, factory.get());
        people.emplace_back(person);
        index.insert(person);

        // Log what we did
        std::cerr << "  LOG: Inserted " << person->getName() << " at " << x << "," << y << std::endl;
    }

    // Query shape
    std::unique_ptr<Geometry> query(nullptr);

    // Read query WKT string, if provided on the commandline.
    WKTReader reader(*factory);
    std::string wkt("POLYGON((10 10, 10 35, 35 35, 35 10, 10 10))");
    if (argc == 2) {
        wkt = std::string(argv[1]);
    }
    try {
        query = reader.read(wkt);
    }
    catch (std::exception& e) {
        std::cerr << "ERROR: Unable to parse WKT: " << wkt << std::endl;
        return 1;
    }

    // Log
    std::cerr << "  LOG: Querying index with " << wkt << std::endl;

    // Place to store query result
    std::vector<const Person*> query_result;

    // Lambda for the STRtree index search.
    // The visited index entry is returned as a parameter.
    // Since our entry type is Person*, so is this parameter type.
    // We capture the query and query_result so we can use the query
    // shape in the exact test for intersection, and push the
    // return results into the query_result vector.
    auto visitor = [&query, &query_result](const Person* person) {
        if (person->intersects(query.get())) {
            query_result.push_back(person);
        }
    };

    // Actually run the index query. STRtree query takes an
    // Envelope as the query key.
    const Envelope* query_env = query->getEnvelopeInternal();
    index.query(*query_env, visitor);

    // Read back the results
    for (auto* person: query_result) {
        const std::string& person_name = person->getName();
        std::string person_wkt = person->getWkt();
        std::cerr << "  LOG: Found entry '" << person_name << "' at " << person_wkt << std::endl;
    }

    // That's it!
    std::cerr << " DONE: Done" << std::endl;
    return 0;

}
