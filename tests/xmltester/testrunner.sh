#!/bin/sh

runner=$1

echo "./XMLTester -v $@" > ${runner}
chmod +x ${runner}
