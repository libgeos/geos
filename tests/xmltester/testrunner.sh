#!/bin/sh

runner=$1
shift

echo "./XMLTester -v $@" > ${runner}
chmod +x ${runner}

echo "Runner: ${runner} created"
