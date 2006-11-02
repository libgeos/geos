#!/bin/sh

runner=$1
shift
srcdir=$1
shift

cat <<EOF > ${runner}
#!/bin/sh
./geostest ${srcdir}/test.wkt > test.out
ret=$?
# It's enough that it doesn't segfault for now...
# computing a diff doesn't currently work as geostest
# was not initially made taking arch differences into
# account
diff ${srcdir}/test.expected test.out
exit $ret
EOF

chmod +x ${runner}
echo "Runner: ${runner} created"
