#!/bin/sh

runner=$1
shift
srcdir=$1
shift

cat <<EOF > ${runner}
#!/bin/sh
./geostest ${srcdir}/test.wkt > test.out
diff ${srcdir}/test.expected test.out
EOF

chmod +x ${runner}
echo "Runner: ${runner} created"
