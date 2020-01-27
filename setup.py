#!/usr/bin/env python3

import os
import skbuild
import setuptools


class get_pybind_include(object):
    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        # postpone importing pybind11 until building actually happens
        import pybind11

        return pybind11.get_include(self.user)


def src(x):
    root = os.path.dirname(__file__)
    return os.path.abspath(os.path.join(root, "python", x))


def getversion():
    pkgversion = {"version": "0.0.0"}
    versionfile = "ecl/version.py"

    if not os.path.exists(versionfile):
        return {
            "use_scm_version": {
                # look for git in ../
                "relative_to": src(""),
                # write to ./python
                "write_to": os.path.join(src(""), versionfile),
            }
        }

    import ast

    with open(os.path.join("python", versionfile)) as f:
        root = ast.parse(f.read())

    for node in ast.walk(root):
        if not isinstance(node, ast.Assign):
            continue
        if len(node.targets) == 1 and node.targets[0].id == "version":
            pkgversion["version"] = node.value.s

    return pkgversion


pybind_includes = [str(get_pybind_include()), str(get_pybind_include(user=True))]

with open("README.md") as f:
    long_description = f.read()

skbuild.setup(
    name="libecl",
    author="Equinor ASA",
    author_email="zom@equinor.com",
    description="Package for reading and writing the result files from the ECLIPSE reservoir simulator",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/equinor/libecl",
    packages=setuptools.find_packages(where='python', exclude=["*.tests", "*.tests.*", "tests.*", "tests"]),
    package_dir={"": "python"},
    license="GPL-3.0",
    platforms="any",
    install_requires=[
        "cwrap",
        "numpy==1.16.6",
        "pandas<=0.25.3",
        "future",
        "six",
        "functools32;python_version=='2.7'"
    ],
    setup_requires=[
        "setuptools",
        "pybind11",
        "setuptools_scm",
        "pytest-runner",
        "sphinx;python_version>='3.5'",
        "cmake",
        "ninja"
    ],
    tests_require=["pytest"],
    # we're building with the pybind11 fetched from pip. Since we don't rely on
    # a cmake-installed pybind there's also no find_package(pybind11) -
    # instead, the get include dirs from the package and give directly from
    # here
    cmake_args=[
        "-DCMAKE_SKIP_BUILD_RPATH:BOOL=FALSE",
        "-DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=TRUE",
        "-DCMAKE_INSTALL_RPATH=$ORIGIN/.libs",
        "-DPYBIND11_INCLUDE_DIRS=" + ";".join(pybind_includes),
        # we can safely pass OSX_DEPLOYMENT_TARGET as it's ignored on
        # everything not OS X. We depend on C++11, which makes our minimum
        # supported OS X release 10.9
        "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.9",
    ],
    # skbuild's test imples develop, which is pretty obnoxious instead, use a
    # manually integrated pytest.
    cmdclass={"test": setuptools.command.test.test},
    classifiers=[
        "Development Status :: 1 - Planning",
        "Environment :: Other Environment",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Natural Language :: English",
        "Programming Language :: Python",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Topic :: Scientific/Engineering",
        "Topic :: Scientific/Engineering :: Physics",
        "Topic :: Software Development :: Libraries",
        "Topic :: Utilities"
    ],
    # **getversion(),
    version='2.6.0'
)
