
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
    'hello.cpp'
)


# A simple cli interface
clivcetoy_files = files(
    'cli.cpp'
)

vcetoy_lib = shared_library(
    'vcetoy',
    libvcetoy_files,
    include_directories : libvcetoy_include,
    dependencies : sdl_dep,
)

executable(
    'vcetoy-cli',
    clivcetoy_files,
    include_directories : libvcetoy_include,
    dependencies : [ sdl_dep ],
    link_with : vcetoy_lib,
    #link_args : '-lSDL2'
)
