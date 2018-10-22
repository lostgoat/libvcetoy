//
// Copyright (C) 2018 Valve Software
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the
// Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall
// be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//



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
