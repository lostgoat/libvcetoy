# Copyright (C) 2018 Valve Software
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the
# Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute,
# sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall
# be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

project(
    'libvcetoy',
    ['cpp'],
    version : '0.0.1',
    license : 'MIT',
    meson_version : '>= 0.47',
    default_options : ['cpp_std=c++11']
)

cpp = meson.get_compiler('cpp')

dl_dep      = cpp.find_library('dl', required : true)
sdl_dep     = dependency('sdl2', version : '>=2.0.0')
vulkan_dep  = dependency('vulkan')
thread_dep  = dependency('threads')
gtest_dep   = dependency('gtest', main : true)

libvcetoy_include = include_directories('include')
libdrm_include = include_directories('libs/libdrm/include')
amdgpu_include = include_directories('libs/libdrm/include/libdrm')

subdir('libs')
subdir('src')
subdir('test')
