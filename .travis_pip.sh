#!/bin/bash
set -e

pushd build
make
ctest --output-on-failure
sudo make install
popd
sudo rm -rf python/_skbuild
build_wheel python x86_64
install_run x86_64
mv wheelhouse python/dist
