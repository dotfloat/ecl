# Define custom utilities
# Test for macOS with [ -n "$IS_OSX" ]

function pre_build {
    # Any stuff that you need to do before you start building the wheels
    # Runs in the root directory of this repository.
    :
}

function run_tests {
    # Runs tests on installed distribution from an empty directory

    # Make python source dirs unreadable for pytest so that it uses the
    # system-installed version of libecl
    chmod 0000 /io/python/{ecl,ert}
    pytest /io/python/tests
    chmod 0755 /io/python/{ecl,ert}
}
