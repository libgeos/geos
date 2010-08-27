#!/bin/sh

runner=$1
shift

echo "#!/bin/sh" > ${runner}
echo "./XMLTester -v $@" >> ${runner}
chmod +x ${runner}

echo "Runner: ${runner} created"
