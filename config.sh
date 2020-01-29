# Define custom utilities
# Test for macOS with [ -n "$IS_OSX" ]

function pre_build {
    # Any stuff that you need to do before you start building the wheels
    # Runs in the root directory of this repository.
    :
}

function run_tests {
    # Runs tests on installed distribution from an empty directory

    # pytest adds every directory up-to and including python/ into sys.path,
    # meaning that "import ecl" will import python/ecl and not the installed one.
    # This doesn't work because the _ecl.so module only exists in site-packages,
    # so we copy directories required by the tests out into its own temporary
    # directory.
    mkdir -p {.git,python}
    ln -s {/io,$PWD}/bin
    ln -s {/io,$PWD}/lib
    ln -s {/io,$PWD}/test-data
    cp -R {/io,$PWD}/python/tests
    python -m pytest python/tests
}
