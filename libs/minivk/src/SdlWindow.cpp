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



#define SDL_VIDEO_DRIVER_X11
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <util/util.h>
#include <vcetoy/vulkan.h>

#include <minivk/SdlWindow.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
SdlWindow::SdlWindow()
    : mWindow(nullptr)
    , mVkInstance( VK_NULL_HANDLE )
    , mVkSurface( VK_NULL_HANDLE )
	, mShouldQuit( false )
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
SdlWindow::~SdlWindow()
{
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int SdlWindow::Init( VkInstance instance )
{
    int err;
    SDL_Window *window = nullptr;

    FailOnTo( instance == VK_NULL_HANDLE, error, "Invalid Vk Instance\n" );

    err = SDL_Init(SDL_INIT_VIDEO);
    FailOnTo(err, error, "Failed to init SDL\n");

    window = SDL_CreateWindow(sDefaultName,
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              sDefaultWidth, sDefaultHeight, SDL_WINDOW_OPENGL);
    FailOnTo(!window, error, "Failed to create an SDL window\n");

    mWindow = window;
    mVkInstance = instance;

    return 0;

error:
    SDL_DestroyWindow(window);
    return -1;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VkSurfaceKHR SdlWindow::GetVulkanSurface()
{
    if( mVkSurface != VK_NULL_HANDLE )
        return mVkSurface;

    VkResult err;
    SDL_SysWMinfo wmInfo;
    VkXlibSurfaceCreateInfoKHR xlibsci = {};

    SDL_VERSION( &wmInfo.version );
    SDL_GetWindowWMInfo( mWindow, &wmInfo );

    const char *subsystem = nullptr;
    switch( wmInfo.subsystem ) {
    case SDL_SYSWM_WINDOWS:   subsystem = "Microsoft Windows(TM)";  break;
    case SDL_SYSWM_X11:       subsystem = "X Window System";        break;
#if SDL_VERSION_ATLEAST(2, 0, 3)
    case SDL_SYSWM_WINRT:     subsystem = "WinRT";                  break;
#endif
    case SDL_SYSWM_DIRECTFB:  subsystem = "DirectFB";               break;
    case SDL_SYSWM_COCOA:     subsystem = "Apple OS X";             break;
    case SDL_SYSWM_UIKIT:     subsystem = "UIKit";                  break;
#if SDL_VERSION_ATLEAST(2, 0, 2)
    case SDL_SYSWM_WAYLAND:   subsystem = "Wayland";                break;
    case SDL_SYSWM_MIR:       subsystem = "Mir";                    break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
    case SDL_SYSWM_ANDROID:   subsystem = "Android";                break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 5)
    case SDL_SYSWM_VIVANTE:   subsystem = "Vivante";                break;
#endif
    case SDL_SYSWM_UNKNOWN:   subsystem = "unknown";                break;
    default:                  subsystem = "unhandled";              break;
    }

    FailOnTo( wmInfo.subsystem != SDL_SYSWM_X11, error, "Unsupported SDL WM: %s\n", subsystem );

    xlibsci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    xlibsci.flags = 0;
    xlibsci.dpy = wmInfo.info.x11.display;
    xlibsci.window = wmInfo.info.x11.window;

    err = vkCreateXlibSurfaceKHR( mVkInstance, &xlibsci, nullptr, &mVkSurface );
    FailOnTo( err != VK_SUCCESS, error, "Failed to create xlib surface\n" );

    printf( "Initialized SDL %d.%d.%d '%s' window\n",
            (int)wmInfo.version.major,
            (int)wmInfo.version.minor,
            (int)wmInfo.version.patch,
            subsystem );

    return mVkSurface;

error:
    return VK_NULL_HANDLE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool SdlWindow::ShouldQuit()
{
	ConsumeEvents();
	return mShouldQuit;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SdlWindow::ConsumeEvents()
{
	SDL_Event event;

	while( SDL_PollEvent( &event ) != 0 ) {
		switch( event.type )
		{
		case SDL_QUIT:
			mShouldQuit = true;
			break;
		default:
			//ignored
			break;
		}
	}
}
