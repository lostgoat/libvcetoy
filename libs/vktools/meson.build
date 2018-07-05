
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

vktools_files = files(
    'src/VulkanTools.cpp'
)

vktools_include = include_directories('include')

vktools_lib = shared_library(
    'vktools',
    vktools_files,
    include_directories : vktools_include,
    dependencies : [ vulkan_dep ],
)

vktools_dep = declare_dependency(
    link_with : vktools_lib,
    include_directories : vktools_include
)
