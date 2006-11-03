#!/bin/sh

runner=$1
shift
srcdir=$1
shift

cat <<EOF > ${runner}
#!/bin/sh
./geostest ${srcdir}/test.wkt > test.out
ret=\$?

if test "\$ret" -ne "0"; then
	echo "geostest exited with non-zero status (\$ret)"
fi

# It's enough that it doesn't segfault for now...
# computing a diff doesn't currently work as geostest
# was not initially made taking arch differences into
# account
diff ${srcdir}/test.expected test.out >&2

exit \$ret

EOF

chmod +x ${runner}
echo "Runner: ${runner} created"
