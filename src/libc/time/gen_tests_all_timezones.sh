#!/bin/sh
set -e
rm -Rf tmp
mkdir tmp
cd tmp
tar -xf ../tzdata2015g.tar.gz
mkdir output
zic -d output africa antarctica asia australasia backward etcetera europe \
    northamerica pacificnew southamerica
cd output
i=0
curdir=`pwd`
find * -type f | while read f; do
  echo "TEST_BEGIN(generated_$i, \"$f\")"
  zdump -v "$curdir/$f" | ../../convert_zdump_to_tests.py
  echo TEST_END
  i=$(($i+1))
done
