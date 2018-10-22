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

#include <list>
#include <vector>

#include <vcetoy/vulkan.h>
#include <minivk/SdlWindow.h>

class MiniVk
{
    private:
        static const size_t k_unCmdBufferCount = 8;
        static constexpr const char* sEngineName = "MiniVk";

        enum FencedResourceType_t
        {
            FENCED_RESOURCE_SEMAPHORE,
            FENCED_RESOURCE_DESCRIPTOR,
            FENCED_RESOURCE_FRAMEBUFFER,
            FENCED_RESOURCE_RENDER_PASS,
            FENCED_RESOURCE_DEVICE_MEMORY,
            FENCED_RESOURCE_PIPELINE,
            FENCED_RESOURCE_IMAGE,
            FENCED_RESOURCE_IMAGE_VIEW,
            FENCED_RESOURCE_BUFFER,
        };

        struct FencedResource_t
        {
            FencedResourceType_t mResourceType;
            uint64_t mResourceId;
            uint64_t mPool;
        };

    public:
        /**
         * Constructor
         */
        MiniVk();

        /**
         * Destructor
         */
        ~MiniVk();

        /**
         * Initialize
         */
        int Init();

    private:
        int InitVulkanInstance();
        int InitVulkanDevice();
        int InitVulkanSwapchain();

        SdlWindow mSdlWindow;
        VkSurfaceKHR mVkSurface;

        VkInstance mVkInstance;
        VkPhysicalDevice mVkPhysicalDevice;

        VkPhysicalDeviceProperties mVkPhysicalDeviceProperties;
        VkPhysicalDeviceFeatures mVkPhysicalDeviceFeatures;
        VkPhysicalDeviceMemoryProperties mVkPhysicalDeviceMemoryProperties;

        VkDevice mVkDevice;
        VkQueue mVkQueue;
        std::vector<VkQueueFamilyProperties> mVkQueueFamilyProperties;
        uint32_t mQueueFamilyIdx;
        uint32_t mMemoryVramIdx;
        uint32_t mMemoryMappableIdx;

        VkCommandPool mVkCmdPool;
        VkCommandBuffer mVkCmdBuffer[ k_unCmdBufferCount ];
        VkFence mVkFence[ k_unCmdBufferCount ];
        uint32_t mCmdBufferIdx;
        bool mIsCmdBufferOpen;
        std::list< FencedResource_t > mFencedResources[ k_unCmdBufferCount ];

        VkSwapchainKHR mVkSwapchain;
        uint32_t mSwapchainIdx;
        uint32_t mSwapchainWidth;
        uint32_t mSwapchainHeight;
        VkFormat mSwapchainFormat;
        VkSemaphore mVkSemRenderDone;
        VkColorSpaceKHR mSwapchainColorSpace;
        std::vector<VkImage> mVkSwapchainImage;
        std::vector<VkImageLayout> mVkSwapchainImageLayout;
        std::vector<VkImageView> mVkSwapchainImageView;

        std::vector< VkSemaphore > mVkSemaphoresToWaitFor;
        std::vector< VkPipelineStageFlags > mVkSemaphoreWaitStageMasks;
        std::vector< VkSemaphore > mVkSemaphoresToSignal;

        int GetQueueFamilyIndex( VkQueueFlagBits bits );
        int GetMemoryType( uint32_t typeBits, VkMemoryPropertyFlags properties );

#define VK_DEVICE_EXT_FN(name) PFN_vk##name mPfn##name;
#include "vkextensionfn.h"
#undef VK_DEVICE_EXT_FN

	public:
        SdlWindow *GetWindow() { return &mSdlWindow; }

        struct MiniVkImage {
            VkImage pImage;
            VkDeviceMemory pMemory;
            int fd;

            bool bMappable;
            void *pCpuAddr;
        };

        struct MiniVkBuffer {
            VkBuffer pBuffer;
            VkDeviceMemory pMemory;
            int fd;

            bool bMappable;
            void *pCpuAddr;
        };

        VkCommandBuffer GetCurrentCommandBuffer();
        bool BeginCommandBuffer();
        bool EndCommandBuffer();

        void AddFencedResource( FencedResourceType_t nResouceType, uint64_t pResource, uint64_t pPool = 0 );
        void FreeFencedResources( uint32_t unCmdBufferIdx );

        bool AcquireSwapchainImage();
        bool PutSwapchainImage();
        bool Present();

		void BeginScene();
		void BeginScene( float flRed, float flGreen, float flBlue, float flAlpha );
		void EndScene();
		void Blit( VkImage srcImage, VkImageLayout srcLayout, uint32_t srcWidth, uint32_t srcHeight, VkImage targetImage, VkImageLayout targetLayout, uint32_t targetOffsetX, uint32_t targetOffsetY, uint32_t targetWidth, uint32_t targetHeight );
        void ClearImage( VkImage pImage, VkImageLayout eLayout, float flRed, float flGreen, float flBlue, float flAlpha );

        bool CreateImage( uint32_t unWidth, uint32_t unHeight, VkFormat eFormat, bool bMappable, MiniVkImage **ppImage );
        void FreeImage( MiniVkImage **ppImage );
        uint8_t *MapImage( MiniVkImage *pImage );

        bool CreateBuffer( uint32_t unSize, bool bMappable, MiniVkBuffer **ppBuffer );
        void FreeBuffer( MiniVkBuffer **ppBuffer );
        uint8_t *MapBuffer( MiniVkBuffer *pBuffer );
};
