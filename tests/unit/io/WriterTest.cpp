// 
// Test Suite for geos::io::Writer

// tut
#include <tut.hpp>
// geos
#include <geos/io/Writer.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_writer_data
	{
		test_writer_data()
    {
    }
	};

	typedef test_group<test_writer_data> group;
	typedef group::object object;

	group test_writer_group("geos::io::Writer");


	//
	// Test Cases
	//

	template<>
	template<>
	void object::test<1>()
	{         
    geos::io::Writer writer;

    writer.write("Hello ");
    writer.write("World!");
    ensure_equals(writer.toString(), "Hello World!");
  }

} // namespace tut


