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
