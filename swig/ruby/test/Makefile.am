#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
#geos_tests.rb runs the full Ruby geos test suite
#
RUBY_TESTS = \
	example.rb \
	geos_tests.rb \
	test_buffer.rb \
	test_combinations.rb \
	test_coordinate_sequence.rb \
	test_envelope.rb \
	test_geometry.rb \
	test_helper.rb \
	test_io.rb \
	test_operations.rb \
	test_relations.rb \
	test_srid.rb \
	test_version.rb

EXTRA_DIST = $(RUBY_TESTS)

test:
	$(RUBY) geos_tests.rb
