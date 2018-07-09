/*
 * Copyright (C) 2018  Valve Corporation.
 *
 * This file is part of libvcetoy.
 *
 * libvcetoy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * libvcetoy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libvcetoy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <signal.h>

#include <vcetoy/vcetoy.h>
#include <vcetoy/vulkan.h>

#include <minivk/MiniVk.h>

void RenderLoop()
{
    MiniVk miniVk;
    miniVk.Init();

	float red = 0;
	uint64_t frameNum = 0;
	while ( !miniVk.GetWindow()->ShouldQuit() ) {
		red += 0.0001;
		if ( red > 1 )
			red = 0;

		miniVk.BeginScene( red, 0, 1, 0 );
		miniVk.EndScene();
		printf("frame num: %lu\n", frameNum++ );
	}
}

int main(int argc, char **argv)
{
	RenderLoop();
    return 0;
}
