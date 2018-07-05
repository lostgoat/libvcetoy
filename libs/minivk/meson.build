
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

minivk_files = files(
    'src/SdlWindow.cpp',
    'src/MiniVk.cpp',
)

minivk_include = include_directories('include')

minivk_lib = shared_library(
    'minivk',
    minivk_files,
    include_directories : [ minivk_include, libvcetoy_include ],
    dependencies : [ vulkan_dep, vktools_dep, sdl_dep, thread_dep ],
)

minivk_dep = declare_dependency(
    link_with : minivk_lib,
    include_directories : minivk_include
)
