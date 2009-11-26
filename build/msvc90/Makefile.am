#
# NOTE: Make sure that 'source' appears first.
# 'capi' and 'doc' surely rely on availability of libgeos
# which is built under 'source' and it seems that automake
# is not able to detect required build order but blindly
# relies on the order items in SUBDIRS appear.
#
SUBDIRS = \
	geos_c_dll \
	geos_lib \
	geos_python \
	geos_ruby \
	geos_unit \
	geos_xmltester

EXTRA_DIST = \
	README \
	geos.sln \
	geos.vsprops
