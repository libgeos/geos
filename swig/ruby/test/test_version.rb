#!/usr/bin/env ruby

require 'test/unit'
require 'geos'


class TestVersion < Test::Unit::TestCase
  def test_versions
    assert_equal("3.0.0rc4-CAPI-1.3.3", Geos::version)
    assert_equal(3, Geos::GEOS_VERSION_MAJOR)
    assert_equal(0, Geos::GEOS_VERSION_MINOR)
    assert_equal("3.0.0rc4", Geos::GEOS_VERSION)
    assert_equal("1.7.1", Geos::GEOS_JTS_PORT)
    assert_equal(1, Geos::GEOS_CAPI_VERSION_MAJOR)
    assert_equal(3, Geos::GEOS_CAPI_VERSION_MINOR)
    assert_equal(3, Geos::GEOS_CAPI_VERSION_PATCH)
    assert_equal(1, Geos::GEOS_CAPI_FIRST_INTERFACE)
    assert_equal(4, Geos::GEOS_CAPI_LAST_INTERFACE)
    assert_equal("3.0.0rc4-CAPI-1.3.3", Geos::GEOS_CAPI_VERSION)
  end
end
