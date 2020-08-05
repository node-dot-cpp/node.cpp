#!/bin/sh
set -ev

export PATH=${PWD}/safe_memory/checker/build/travis/bin:${PATH}

rm -Rf build/travis
mkdir -p build/travis
cd build/travis

cmake -DNODECPP_CHECK_AND_DZ_SAMPLES=ON -DCMAKE_BUILD_TYPE=Release -G Ninja ../..

cmake --build .

cd ../..
