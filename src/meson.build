
# Copyright (C) 2018  Valve Corporation.

# This file is part of libvcetoy.

# libvcetoy is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.

# libvcetoy is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with libvcetoy.  If not, see <http://www.gnu.org/licenses/>.

# Our library
libvcetoy_files = files(
    'entrypoints.cpp',
    'VcetContext.cpp',
    'VcetBo.cpp',
)


# A simple cli interface
clivcetoy_files = files(
    'cli.cpp',
)

vcetoy_lib = shared_library(
    'vcetoy',
    libvcetoy_files,
    dependencies : [ drm_dep, amdgpu_dep ],
    include_directories : [ libvcetoy_include ],
)

vcetoy_dep = declare_dependency(
    link_with : vcetoy_lib,
    include_directories : libvcetoy_include
)

executable(
    'vcetoy-cli',
    clivcetoy_files,
    dependencies : [ thread_dep, minivk_dep, vcetoy_dep ],
)
