SWIG generated GEOS module for Ruby
=====================================

:Author: Charlie Savage
:Contact: cfis@interserv.com


Installation
------------

First:

1.  Build GEOS
2.  cd swig/ruby

If you are running VC++ on Windows:

1.  You must first compile GEOS using VC++ since the SWIG bindings
    link against the GEOS C++ API
2.  Create a VC++ project that includes geos_wrap.cxx and the appropriate
    Ruby include files and libraries.
3.  Copy the geos.dll or geos.so file you create to ruby\lib\ruby\site_ruby\1.8\i386-msvcrt


If you are running Linux:

1.  Compile geos_wrap.cxx normally, i.e.:
g++  -I../../source/headers -I<ruby_path/i386-linux> -c geos_wrap.cxx


2.  Link against the GEOS and Ruby libraries:

g++ -shared -lgeos -lruby geos_wrap.o -o geos.so


3.  Copy the geos.so file you create to ruby\lib\ruby\site_ruby\1.8\i386-linux


If you are running MingW on Windows:

1.  Compile geos_wrap.cxx normally, i.e.:
g++  -I../../source/headers -I<ruby_path/i386-mswin32> -c geos_wrap.cxx


2.  Link directly against the GEOS and Ruby dlls as opposed to their libraries.  Thus:

g++ -shared <some_path>/libgeos-3.dll <ruby_path>bin/msvcrt-ruby18.dll geos_wrap.o -o geos.so


3.  Copy the geos.dll or geos.so file you create to ruby\lib\ruby\site_ruby\1.8\i386-msvcrt


Note that if you running Ruby 1.8.2 and MingW on Windows, you'll have to update 
ruby\lib\ruby\1.8\i386-mswin32\defines.h as described here:

http://rubyforge.org/tracker/index.php?func=detail&aid=2206&group_id=426&atid=1698

	
SWIG WRAPPER 
---------------
To regenerate the  SWIG wrapper (must have SWIG 1.3.29 or higher installed on your machine)

swig -c++ -ruby -autorename -o geos_wrap.cxx ../geos.i



Examples
-------

See example.rb for a number examples of using the GEOS library from Ruby.

