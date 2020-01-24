#!/bin/bash
set -e

mkdir build
pushd build
cmake .. -DBUILD_TESTS=ON -DENABLE_PYTHON=ON -DBUILD_APPLICATIONS=ON -DINSTALL_ERT_LEGACY=ON -DERT_USE_OPENMP=ON -DCMAKE_C_FLAGS='-Werror=all' -DCMAKE_CXX_FLAGS='-Werror -Wno-unused-result'
make
sudo make install
which python
export PYTHONPATH="/usr/local/lib/python$MB_PYTHON_VERSION/site-packages:/usr/local/lib/python$MB_PYTHON_VERSION/dist-packages:$PYTHONPATH"
python -c "import sys; print('\n'.join(sys.path))"
set -e; python -c "import ecl"; set +e
ctest --output-on-failure $TEST_SUITE
popd
