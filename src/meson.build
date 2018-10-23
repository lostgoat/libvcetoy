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

# Our library
libvcetoy_files = files(
    'entrypoints.cpp',
    'VcetContext.cpp',
    'VcetBo.cpp',
    'VcetIb.cpp',
    'Drm.cpp'
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
