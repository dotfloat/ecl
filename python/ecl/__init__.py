#  Copyright (C) 2011  Equinor ASA, Norway.
#
#  The file '__init__.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.
"""
ert - Ensemble Reservoir Tool - a package for reservoir modeling.

The ert package itself has no code, but contains several subpackages:

ecl.ecl: Package for working with ECLIPSE files. The far most mature
   package in ert.
"""
import os.path
import sys
import ctypes

import warnings

warnings.simplefilter("always", DeprecationWarning)  # see #1437

from cwrap import Prototype

from ._ecl import libecl_handle, __version__


class EclPrototype(Prototype):
    lib = ctypes.PyDLL(None, ctypes.RTLD_GLOBAL, handle=libecl_handle)

    def __init__(self, prototype, bind=True):
        super(EclPrototype, self).__init__(EclPrototype.lib, prototype, bind=bind)


from .ecl_type import EclTypeEnum, EclDataType
from .ecl_util import (
    EclFileEnum,
    EclFileFlagEnum,
    EclPhaseEnum,
    EclUnitTypeEnum,
    EclUtil,
)

from .util.util import EclVersion
