prefix=@ESCAPED_INSTALL_PREFIX@
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${prefix}/lib

Name: GEOS
Description: Geometry Engine, Open Source - C API
Requires:
Version: @GEOS_VERSION@
Cflags: -I${includedir}
Libs: -L${libdir} -lgeos_c
