#!/bin/sh
set -ev

cd ../samples/http_server/build/
./build_clang.sh
#./build_clang_mth.sh
#./build_clang_no_msafty.sh

cd ../../http_server_handlers/build/
./build_clang.sh
#./build_clang_mth.sh
#./build_clang_no_msafty.sh

