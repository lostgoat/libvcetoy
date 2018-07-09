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

project(
    'libvcetoy',
    ['cpp'],
    version : '0.0.1',
    license : 'GPL3',
    meson_version : '>= 0.43',
    default_options : ['cpp_std=c++11']
)

drm_dep     = dependency('libdrm', version : '>=2.4.91')
amdgpu_dep  = dependency('libdrm_amdgpu', version : '>=2.4.91')
sdl_dep     = dependency('sdl2', version : '>=2.0.0')
vulkan_dep  = dependency('vulkan')
thread_dep  = dependency('threads')
gtest_dep   = dependency('gtest', main : true)

libvcetoy_include = include_directories('include')

subdir('libs')
subdir('src')
subdir('test')
