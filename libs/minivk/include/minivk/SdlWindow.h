/* * Copyright (C) 2018 Valve Software
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */



#pragma once

#include <string>

#include <SDL2/SDL.h>
#include <vcetoy/vulkan.h>

class SdlWindow
{
    private:
        static const int sDefaultWidth = 640;
        static const int sDefaultHeight = 640;
        static constexpr const char* sDefaultName = "vcetoy";

    public:
        /**
         * Constructor
         */
        SdlWindow();

        /**
         * Destructor
         */
        ~SdlWindow();

        /**
         * Initialize
         */
        int Init(VkInstance instance);

        /**
         * Get a VkSurface that represents this window
         */
        VkSurfaceKHR GetVulkanSurface();

		/**
		 * Check if the user requested we stop execution
		 */
		bool ShouldQuit();

    private:
        SDL_Window *mWindow;
        VkInstance mVkInstance;
        VkSurfaceKHR mVkSurface;

		bool mShouldQuit;

		void ConsumeEvents();
};
