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



#include <pthread.h>
#include <unistd.h>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <util/util.h>
#include <vcetoy/vulkan.h>
#include <vktools/VulkanTools.h>

#include <minivk/MiniVk.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MiniVk::MiniVk()
    : mVkSurface( VK_NULL_HANDLE )
    , mVkInstance( VK_NULL_HANDLE )
    , mVkPhysicalDevice( VK_NULL_HANDLE )
    , mVkDevice( VK_NULL_HANDLE )
    , mVkQueue( VK_NULL_HANDLE )
    , mQueueFamilyIdx( UINT32_MAX )
    , mMemoryVramIdx( UINT32_MAX )
    , mMemoryMappableIdx( UINT32_MAX )
    , mVkCmdPool( VK_NULL_HANDLE )
    , mCmdBufferIdx( 0 )
    , mIsCmdBufferOpen( false )
    , mVkSwapchain( VK_NULL_HANDLE )
    , mSwapchainIdx( 0 )
    , mSwapchainWidth( 0 )
    , mSwapchainHeight( 0 )
    , mSwapchainFormat( VK_FORMAT_UNDEFINED )
    , mVkSemRenderDone( VK_NULL_HANDLE )
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MiniVk::~MiniVk()
{
    if ( mVkSwapchain != VK_NULL_HANDLE )
    {
        vkDestroySwapchainKHR( mVkDevice, mVkSwapchain, nullptr );
        mVkSwapchain = VK_NULL_HANDLE;
    }

    if ( mVkCmdBuffer[ 0 ] != VK_NULL_HANDLE )
    {
        vkFreeCommandBuffers( mVkDevice, mVkCmdPool, k_unCmdBufferCount, mVkCmdBuffer );

        for ( unsigned i = 0; i < k_unCmdBufferCount; ++i )
        {
            mVkCmdBuffer[ i ] = VK_NULL_HANDLE;
        }
    }

    for ( unsigned i = 0; i < k_unCmdBufferCount; ++i )
    {
        if ( mVkFence[ i ] != VK_NULL_HANDLE )
        {
            vkDestroyFence( mVkDevice, mVkFence[ i ], nullptr );
            mVkFence[ i ] = VK_NULL_HANDLE;
        }
    }

    if ( mVkCmdPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool( mVkDevice, mVkCmdPool, nullptr );
        mVkCmdPool = VK_NULL_HANDLE;
    }

    if ( mVkDevice != VK_NULL_HANDLE )
    {
        vkDestroyDevice( mVkDevice, nullptr );
        mVkQueue = VK_NULL_HANDLE;
        mVkDevice = VK_NULL_HANDLE;
    }

    mVkPhysicalDevice = VK_NULL_HANDLE;

    if ( mVkInstance != VK_NULL_HANDLE )
    {
        vkDestroyInstance( mVkInstance, nullptr );
        mVkInstance = VK_NULL_HANDLE;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int MiniVk::Init()
{
    int err;

    err = InitVulkanInstance();
    FailOnTo(err, error, "Failed to initialize vulkan instance\n");

    err = InitVulkanDevice();
    FailOnTo(err, error, "Failed to initialize vulkan device\n");

    err = InitVulkanSwapchain();
    FailOnTo(err, error, "Failed to initialize vulkan instance\n");

    return 0;

error:
    return err;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int MiniVk::InitVulkanInstance()
{
    VkResult err;
    std::vector<const char*> instanceExtensions;

#define VK_INSTANCE_EXT(name) instanceExtensions.push_back( name##_NAME );
#include "vkextensionfn.h"
#undef VK_INSTANCE_EXT

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "libvcetoy-cli";
    appInfo.pEngineName = "MiniVk";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

    err = vkCreateInstance(&instanceCreateInfo, nullptr, &mVkInstance);
    FailOnTo( err != VK_SUCCESS, error, "Failed to create vulkan instance\n" );

    return 0;

error:
    return -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int MiniVk::InitVulkanDevice()
{
    VkResult err;

    uint32_t deviceCount = 0;
    err = vkEnumeratePhysicalDevices(mVkInstance, &deviceCount, nullptr);
    FailOn( err != VK_SUCCESS, "Failed to get number of physical devices\n" );

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    err = vkEnumeratePhysicalDevices(mVkInstance, &deviceCount, physicalDevices.data());
    FailOn( err != VK_SUCCESS, "Failed to enumerate physical devices\n" );

    // Assume the first device is what we want
    mVkPhysicalDevice = physicalDevices[0];
    FailOn( mVkPhysicalDevice == VK_NULL_HANDLE, "Failed to find a valid physical device\n" );

    vkGetPhysicalDeviceProperties(mVkPhysicalDevice, &mVkPhysicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(mVkPhysicalDevice, &mVkPhysicalDeviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(mVkPhysicalDevice, &mVkPhysicalDeviceMemoryProperties);

    for ( unsigned i = 0; i < mVkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i )
    {
        VkMemoryType *pType = &mVkPhysicalDeviceMemoryProperties.memoryTypes[i];

        if ( mMemoryVramIdx == UINT32_MAX
             && pType->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )
        {
            mMemoryVramIdx = i;

        }

        if ( mMemoryMappableIdx == UINT32_MAX
             && pType->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT
             && pType->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
        {
            mMemoryMappableIdx = i;
        }
    }

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queueFamilyCount, nullptr);
    mVkQueueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queueFamilyCount, mVkQueueFamilyProperties.data());
    mQueueFamilyIdx = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

    const float defaultQueuePriority = 0.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    VkDeviceQueueCreateInfo queueInfo;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.queueFamilyIndex = mQueueFamilyIdx;;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &defaultQueuePriority;
    queueInfo.flags = 0;
    queueCreateInfos.push_back(queueInfo);

    std::vector<const char*> deviceExtensions;
#define VK_DEVICE_EXT(name) deviceExtensions.push_back( name##_NAME );
#include "vkextensionfn.h"
#undef VK_DEVICE_EXT

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = NULL;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    err = vkCreateDevice(mVkPhysicalDevice, &deviceCreateInfo, nullptr, &mVkDevice);
    FailOn( err != VK_SUCCESS, "Failed to create device\n" );

    vkGetDeviceQueue(mVkDevice, mQueueFamilyIdx, 0, &mVkQueue);

    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = mQueueFamilyIdx;

    err = vkCreateCommandPool(mVkDevice, &poolCreateInfo, nullptr, &mVkCmdPool);
    FailOn( err != VK_SUCCESS, "Failed to create command pool\n" );

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandBufferCount = k_unCmdBufferCount;
    commandBufferAllocateInfo.commandPool = mVkCmdPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    err = vkAllocateCommandBuffers( mVkDevice, &commandBufferAllocateInfo, mVkCmdBuffer );
    FailOn( err != VK_SUCCESS, "Failed to allocate command buffers\n" );

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( unsigned i = 0; i < k_unCmdBufferCount; i++ )
    {
        err = vkCreateFence( mVkDevice, &fenceCreateInfo, nullptr, &mVkFence[ i ] );
        FailOn( err != VK_SUCCESS, "Failed to create fence\n" );
    }

#define VK_DEVICE_EXT_FN(name) mPfn##name = ( PFN_vk##name ) vkGetDeviceProcAddr( mVkDevice, "vk" #name);
#include "vkextensionfn.h"
#undef VK_DEVICE_EXT_FN

    return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int MiniVk::InitVulkanSwapchain()
{
    VkResult err;
    VkBool32 IsSurfaceSupported = false;

    mSdlWindow.Init( mVkInstance );
    mVkSurface = mSdlWindow.GetVulkanSurface();

    vkGetPhysicalDeviceSurfaceSupportKHR(mVkPhysicalDevice, mQueueFamilyIdx, mVkSurface, &IsSurfaceSupported);
    FailOn( !IsSurfaceSupported, "Physical device does not support surface\n" );

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVkPhysicalDevice, mVkSurface, &surfaceCapabilities);
    FailOn( err != VK_SUCCESS, "Failed to get device surface capabilities\n" );

    uint32_t presentModeCount;
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(mVkPhysicalDevice, mVkSurface, &presentModeCount, nullptr);
    FailOn( err != VK_SUCCESS, "Failed to get present modes count\n" );

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(mVkPhysicalDevice, mVkSurface, &presentModeCount, presentModes.data());
    FailOn( err != VK_SUCCESS, "Failed to get present modes\n" );

    uint32_t surfaceFormatCount = 0;
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(mVkPhysicalDevice, mVkSurface, &surfaceFormatCount, nullptr);
    FailOn( err != VK_SUCCESS, "Failed to get surface formats count\n" );

    std::vector<VkSurfaceFormatKHR> surfaceFormats( surfaceFormatCount );
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(mVkPhysicalDevice, mVkSurface, &surfaceFormatCount, surfaceFormats.data());
    FailOn( err != VK_SUCCESS, "Failed to get surface formats count\n" );

    VkExtent2D swapChainSize = surfaceCapabilities.currentExtent;
    mSwapchainWidth = swapChainSize.width;
    mSwapchainHeight = swapChainSize.height;

    VkSurfaceTransformFlagBitsKHR surfaceTransformFlags = surfaceCapabilities.currentTransform;
    if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        surfaceTransformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }

    for ( unsigned i = 0; i < surfaceFormats.size(); i++ )
    {
        if ( surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB )
        {
            mSwapchainColorSpace = surfaceFormats[i].colorSpace;
            mSwapchainFormat = surfaceFormats[i].format;
        }
    }
    FailOn( mSwapchainFormat == VK_FORMAT_UNDEFINED, "Failed to get swapchain format\n" );

    bool IsImmediateModeSupported = false;
    for ( unsigned i = 0; i < presentModeCount; i++ )
    {
        if ( presentModes[ i ] == VK_PRESENT_MODE_IMMEDIATE_KHR )
        {
            IsImmediateModeSupported = true;
        }
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = mVkSurface;
    swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    swapchainCreateInfo.preTransform = surfaceTransformFlags;
    swapchainCreateInfo.imageColorSpace = mSwapchainColorSpace;
    swapchainCreateInfo.imageFormat = mSwapchainFormat;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = mVkSwapchain;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.imageExtent = swapChainSize;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.presentMode = IsImmediateModeSupported ? VK_PRESENT_MODE_IMMEDIATE_KHR : VK_PRESENT_MODE_FIFO_KHR;

    err = vkCreateSwapchainKHR( mVkDevice, &swapchainCreateInfo, nullptr, &mVkSwapchain);
    FailOn( err != VK_SUCCESS, "Failed to create swapchain\n" );

    uint32_t swapchainImageCount = 0;
    err = vkGetSwapchainImagesKHR( mVkDevice, mVkSwapchain, &swapchainImageCount, nullptr );
    FailOn( err != VK_SUCCESS, "Failed to get swapchain image count\n" );

    mVkSwapchainImage.resize( swapchainImageCount );
    mVkSwapchainImageView.resize( swapchainImageCount );
    mVkSwapchainImageLayout.resize( swapchainImageCount );

    err = vkGetSwapchainImagesKHR( mVkDevice, mVkSwapchain, &swapchainImageCount, mVkSwapchainImage.data() );
    FailOn( err != VK_SUCCESS, "Failed to get swapchain images\n" );

    for ( unsigned i = 0; i < swapchainImageCount; i++ )
    {
        mVkSwapchainImageLayout[ i ] = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = mVkSwapchainImage[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = mSwapchainFormat;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        err = vkCreateImageView( mVkDevice, &imageViewCreateInfo, nullptr, &mVkSwapchainImageView[i] );
        FailOn( err != VK_SUCCESS, "Failed to create swapchain image view\n" );
    }

    /**
     * Semaphore for synchronizing presentation
     */
    VkSemaphoreCreateInfo semCreateInfo = {};
    semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    err = vkCreateSemaphore( mVkDevice, &semCreateInfo, nullptr, &mVkSemRenderDone );
    FailOn( err != VK_SUCCESS, "Failed to create present semaphore\n" );

    return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int MiniVk::GetQueueFamilyIndex(VkQueueFlagBits bits)
{
    for ( unsigned i = 0; i < mVkQueueFamilyProperties.size(); i++ ) {
        if ( mVkQueueFamilyProperties[i].queueFlags & bits )
            return i;
    }

    return -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int MiniVk::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
    for ( unsigned i = 0; i < mVkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if ( typeBits & 1 ) {
            if (( mVkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties )
                == properties) {
                return i;
            }
        }
        typeBits >>= 1;
    }

    return -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool MiniVk::AcquireSwapchainImage()
{
    VkResult err;
    VkSemaphore sem = VK_NULL_HANDLE;

    err = vkAcquireNextImageKHR( mVkDevice, mVkSwapchain, UINT64_MAX, sem, VK_NULL_HANDLE, &mSwapchainIdx );
	FailOnTo( err != VK_SUCCESS, error, "Failed to acquire next swapchain image\n" );

    if ( sem != VK_NULL_HANDLE )
    {
        AddFencedResource( FENCED_RESOURCE_SEMAPHORE, (uint64_t) sem );
        mVkSemaphoresToWaitFor.push_back( sem );
        mVkSemaphoreWaitStageMasks.push_back( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT );
    }

    vks::tools::setImageLayout( GetCurrentCommandBuffer(), mVkSwapchainImage[ mSwapchainIdx ],
								VK_IMAGE_ASPECT_COLOR_BIT, mVkSwapchainImageLayout[ mSwapchainIdx ],
								VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );

    return true;

error:
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool MiniVk::PutSwapchainImage()
{
    vks::tools::setImageLayout( GetCurrentCommandBuffer(), mVkSwapchainImage[ mSwapchainIdx ],
                             VK_IMAGE_ASPECT_COLOR_BIT, mVkSwapchainImageLayout[ mSwapchainIdx ],
                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR );

    mVkSemaphoresToSignal.push_back( mVkSemRenderDone );

    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool MiniVk::Present()
{
    VkPresentInfoKHR presInfo = {};
    presInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presInfo.swapchainCount = 1;
    presInfo.pSwapchains = &mVkSwapchain;
    presInfo.pImageIndices = &mSwapchainIdx;
    presInfo.waitSemaphoreCount = 1;
    presInfo.pWaitSemaphores = &mVkSemRenderDone;

    VkResult err = vkQueuePresentKHR( mVkQueue, &presInfo );
	FailOnTo( err != VK_SUCCESS, error, "Failed to present\n" );

    return true;

error:
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::BeginScene()
{
	BeginScene( 0, 0, 0, 0 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::BeginScene( float flRed, float flGreen, float flBlue, float flAlpha )
{
	BeginCommandBuffer();
	AcquireSwapchainImage();
	ClearImage( mVkSwapchainImage[ mSwapchainIdx ], mVkSwapchainImageLayout[ mSwapchainIdx ],
				flRed, flGreen, flBlue, flAlpha );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::EndScene()
{
	PutSwapchainImage();
	EndCommandBuffer();
	Present();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool MiniVk::BeginCommandBuffer()
{
	VkResult err;

	if ( mIsCmdBufferOpen )
		return false;

	// Wait for previous usage to complete
	while ( vkWaitForFences( mVkDevice, 1, &mVkFence[ mCmdBufferIdx ], true, 1000000000ULL ) != VK_SUCCESS ) {
		pthread_yield();
	}

	FreeFencedResources( mCmdBufferIdx  );
	err = vkResetFences( mVkDevice, 1, &mVkFence[ mCmdBufferIdx ] );
	if ( err != VK_SUCCESS )
	{
		return false;
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	err = vkResetCommandBuffer( mVkCmdBuffer[ mCmdBufferIdx ], 0 );
	if ( err != VK_SUCCESS )
	{
		return false;
	}

	err = vkBeginCommandBuffer( mVkCmdBuffer[ mCmdBufferIdx ], &beginInfo);
	if ( err != VK_SUCCESS )
	{
		return false;
	}

	mIsCmdBufferOpen = true;
	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool MiniVk::EndCommandBuffer()
{
	VkResult err;

	if ( !mIsCmdBufferOpen )
	{
		return false;
	}

	mIsCmdBufferOpen = false;

	err = vkEndCommandBuffer( mVkCmdBuffer[ mCmdBufferIdx ] );
	if ( err != VK_SUCCESS )
	{
		return false;
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mVkCmdBuffer[ mCmdBufferIdx ];

	if ( mVkSemaphoresToWaitFor.size() > 0 )
	{
		submitInfo.waitSemaphoreCount = ( uint32_t ) mVkSemaphoresToWaitFor.size();
		submitInfo.pWaitSemaphores = mVkSemaphoresToWaitFor.data();
		submitInfo.pWaitDstStageMask = mVkSemaphoreWaitStageMasks.data();
	}

	if ( mVkSemaphoresToSignal.size() > 0 )
	{
		submitInfo.signalSemaphoreCount = ( uint32_t ) mVkSemaphoresToSignal.size();
		submitInfo.pSignalSemaphores = mVkSemaphoresToSignal.data();
	}

	err = vkQueueSubmit( mVkQueue, 1, &submitInfo, mVkFence[ mCmdBufferIdx ] );
	if ( err != VK_SUCCESS )
	{
		return false;
	}

	mCmdBufferIdx++;
	if ( mCmdBufferIdx >= k_unCmdBufferCount )
	{
		mCmdBufferIdx = 0;
	}

	mVkSemaphoresToWaitFor.clear();
	mVkSemaphoreWaitStageMasks.clear();
	mVkSemaphoresToSignal.clear();

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VkCommandBuffer MiniVk::GetCurrentCommandBuffer()
{
	return mIsCmdBufferOpen ? mVkCmdBuffer[ mCmdBufferIdx ] : VK_NULL_HANDLE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::ClearImage( VkImage pImage, VkImageLayout eLayout, float flRed, float flGreen, float flBlue, float flAlpha )
{
	VkClearColorValue clearColor = {{ flRed, flGreen, flBlue, flAlpha }};
    VkImageSubresourceRange subResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	vkCmdClearColorImage( GetCurrentCommandBuffer(), pImage, eLayout, &clearColor, 1, &subResRange );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::Blit( VkImage srcImage, VkImageLayout srcLayout, uint32_t srcWidth, uint32_t srcHeight, VkImage targetImage, VkImageLayout targetLayout, uint32_t targetOffsetX, uint32_t targetOffsetY, uint32_t targetWidth, uint32_t targetHeight )
{
	VkImageBlit imageBlit = {};
	imageBlit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	imageBlit.srcOffsets[0].x = 0;
	imageBlit.srcOffsets[0].y = 0;
	imageBlit.srcOffsets[0].z = 0;
	imageBlit.srcOffsets[1].x = srcWidth;
	imageBlit.srcOffsets[1].y = srcHeight;
	imageBlit.srcOffsets[1].z = 1;
	imageBlit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	imageBlit.dstOffsets[0].x = targetOffsetX;
	imageBlit.dstOffsets[0].y = targetOffsetY;
	imageBlit.dstOffsets[0].z = 0;
	imageBlit.dstOffsets[1].x = targetOffsetX + targetWidth;
	imageBlit.dstOffsets[1].y = targetOffsetY + targetHeight;
	imageBlit.dstOffsets[1].z = 1;

    vkCmdBlitImage( GetCurrentCommandBuffer(), srcImage, srcLayout,
					targetImage, targetLayout,
				    1,  &imageBlit, VK_FILTER_LINEAR);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::AddFencedResource( FencedResourceType_t nResouceType, uint64_t pResource, uint64_t pPool /* = 0 */ )
{
	FencedResource_t fencedResource = { nResouceType, pResource, pPool };

	// Free at the end of the current command buffer
	uint32_t uCmdBufferIdx = mCmdBufferIdx;

	// If no command buffer is currently open, free on the next one instead
	if ( mIsCmdBufferOpen )
	{
		uCmdBufferIdx = (uCmdBufferIdx + 1 ) % k_unCmdBufferCount;
	}

	mFencedResources[ uCmdBufferIdx ].push_back( fencedResource );
}

//-----------------------------------------------------------------------------
void MiniVk::FreeFencedResources( uint32_t unCmdBufferIdx )
{
	auto resourceIter = mFencedResources[ mCmdBufferIdx ].begin();
	while( resourceIter != mFencedResources[ mCmdBufferIdx ].end() )
	{
		uint64_t pResource = resourceIter->mResourceId;
		uint64_t pPool = resourceIter->mPool;
		switch( resourceIter->mResourceType )
		{
		case FENCED_RESOURCE_SEMAPHORE:
			vkDestroySemaphore( mVkDevice, ( VkSemaphore ) pResource, nullptr );
			break;
		case FENCED_RESOURCE_DESCRIPTOR:
			vkFreeDescriptorSets( mVkDevice, ( VkDescriptorPool ) pPool, 1, ( VkDescriptorSet * ) &pResource );
			break;
		case FENCED_RESOURCE_FRAMEBUFFER:
			vkDestroyFramebuffer( mVkDevice, ( VkFramebuffer ) pResource, nullptr );
			break;
		case FENCED_RESOURCE_RENDER_PASS:
			vkDestroyRenderPass( mVkDevice, ( VkRenderPass ) pResource, nullptr );
			break;
		case FENCED_RESOURCE_DEVICE_MEMORY:
			vkFreeMemory( mVkDevice, ( VkDeviceMemory ) pResource, nullptr );
			break;
		case FENCED_RESOURCE_PIPELINE:
			vkDestroyPipeline( mVkDevice, ( VkPipeline ) pResource, nullptr );
			break;
		case FENCED_RESOURCE_IMAGE:
			vkDestroyImage( mVkDevice, ( VkImage ) pResource, nullptr );
			break;
		case FENCED_RESOURCE_IMAGE_VIEW:
			vkDestroyImageView( mVkDevice, ( VkImageView ) pResource, nullptr );
			break;
		case FENCED_RESOURCE_BUFFER:
			vkDestroyBuffer( mVkDevice, ( VkBuffer ) pResource, nullptr );
			break;
		default:
			break;
		}

		resourceIter++;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool MiniVk::CreateImage( uint32_t unWidth, uint32_t unHeight, VkFormat eFormat, bool bMappable, MiniVkImage **ppImage )
{
    int fd = -1;
    VkResult err;
    VkImage pImage = VK_NULL_HANDLE;
    VkDeviceMemory pMemory = VK_NULL_HANDLE;
    VkImageCreateInfo imageInfo = {};
    VkImageMemoryRequirementsInfo2KHR imageMemInfo = {};
    VkMemoryRequirements2KHR memReqs = {};
    VkMemoryAllocateInfo allocInfo = {};
    VkMemoryDedicatedAllocateInfoKHR dedicatedMemInfo = {};
    VkExportMemoryAllocateInfoKHR exportMemoryInfo = {};
    VkMemoryGetFdInfoKHR memGetFdInfo = {};
    MiniVkImage *pNewImage = nullptr;

    FailOnTo( !ppImage, error, "Invalid parameter\n" );

    pNewImage = new MiniVkImage;
    FailOnTo( !pNewImage, error, "Out of memory\n" );

    pNewImage->bMappable = bMappable;
    pNewImage->pCpuAddr = nullptr;

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = unWidth;
    imageInfo.extent.height = unHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = eFormat;
    imageInfo.tiling = VK_IMAGE_TILING_LINEAR; // Images are linear for interop purposes
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

    err = vkCreateImage( mVkDevice, &imageInfo, nullptr, &pImage );
    FailOnTo( err != VK_SUCCESS, error, "Failed to allocate image\n" );

    imageMemInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR;
    imageMemInfo.image = pImage;
    mPfnGetImageMemoryRequirements2KHR( mVkDevice, &imageMemInfo, &memReqs);

    dedicatedMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
    dedicatedMemInfo.pNext = nullptr;
    dedicatedMemInfo.buffer = VK_NULL_HANDLE;
    dedicatedMemInfo.image = pImage;

    exportMemoryInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
    exportMemoryInfo.pNext = &dedicatedMemInfo;
    exportMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = &exportMemoryInfo;
    allocInfo.allocationSize = memReqs.memoryRequirements.size;
    allocInfo.memoryTypeIndex = bMappable ? mMemoryMappableIdx : mMemoryVramIdx;

    err = vkAllocateMemory( mVkDevice, &allocInfo, nullptr, &pMemory );
    FailOnTo( err != VK_SUCCESS, error, "Failed to allocate image memory\n" );

    err = vkBindImageMemory( mVkDevice, pImage, pMemory, 0 );
    FailOnTo( err != VK_SUCCESS, error, "Failed to bind memory to image\n" );

    memGetFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    memGetFdInfo.memory = pMemory;
    memGetFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    err = mPfnGetMemoryFdKHR( mVkDevice, &memGetFdInfo, &fd );
    FailOnTo( err != VK_SUCCESS, error, "Failed to get memory fd\n" );

    pNewImage->pImage = pImage;
    pNewImage->pMemory = pMemory;
    pNewImage->fd = fd;

    *ppImage = pNewImage;

    return true;

error:
    vkDestroyImage( mVkDevice, pImage, nullptr );
    vkFreeMemory( mVkDevice, pMemory, nullptr );
    delete pNewImage;

    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool MiniVk::CreateBuffer( uint32_t unSize, bool bMappable, MiniVkBuffer **ppBuffer )
{
    int fd = -1;
    VkResult err;
    VkBuffer pBuffer = VK_NULL_HANDLE;
    VkDeviceMemory pMemory = VK_NULL_HANDLE;
    VkBufferCreateInfo bufferInfo = {};
    VkBufferMemoryRequirementsInfo2KHR bufferMemInfo = {};
    VkMemoryRequirements2KHR memReqs = {};
    VkMemoryAllocateInfo allocInfo = {};
    VkMemoryDedicatedAllocateInfoKHR dedicatedMemInfo = {};
    VkExportMemoryAllocateInfoKHR exportMemoryInfo = {};
    VkMemoryGetFdInfoKHR memGetFdInfo = {};
    MiniVkBuffer *pNewBuffer = nullptr;

    FailOnTo( !ppBuffer, error, "Invalid parameter\n" );

    pNewBuffer = new MiniVkBuffer;
    FailOnTo( !pNewBuffer, error, "Out of memory\n" );

    pNewBuffer->bMappable = bMappable;
    pNewBuffer->pCpuAddr = nullptr;

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = unSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

    err = vkCreateBuffer( mVkDevice, &bufferInfo, nullptr, &pBuffer );
    FailOnTo( err != VK_SUCCESS || pBuffer == VK_NULL_HANDLE, error, "Failed to allocate buffer\n" );

    bufferMemInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
    bufferMemInfo.buffer = pBuffer;
    mPfnGetBufferMemoryRequirements2KHR( mVkDevice, &bufferMemInfo, &memReqs);

    dedicatedMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
    dedicatedMemInfo.pNext = nullptr;
    dedicatedMemInfo.buffer = pBuffer;
    dedicatedMemInfo.image = VK_NULL_HANDLE;

    exportMemoryInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
    exportMemoryInfo.pNext = &dedicatedMemInfo;
    exportMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = &exportMemoryInfo;
    allocInfo.allocationSize = memReqs.memoryRequirements.size;
    allocInfo.memoryTypeIndex = bMappable ? mMemoryMappableIdx : mMemoryVramIdx;

    err = vkAllocateMemory( mVkDevice, &allocInfo, nullptr, &pMemory );
    FailOnTo( err != VK_SUCCESS || pMemory == VK_NULL_HANDLE, error, "Failed to allocate buffer memory\n" );

    err = vkBindBufferMemory( mVkDevice, pBuffer, pMemory, 0 );
    FailOnTo( err != VK_SUCCESS, error, "Failed to bind memory to buffer\n" );

    memGetFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    memGetFdInfo.memory = pMemory;
    memGetFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    err = mPfnGetMemoryFdKHR( mVkDevice, &memGetFdInfo, &fd );
    FailOnTo( err != VK_SUCCESS, error, "Failed to get memory fd\n" );

    pNewBuffer->pBuffer = pBuffer;
    pNewBuffer->pMemory = pMemory;
    pNewBuffer->fd = fd;

    *ppBuffer = pNewBuffer;

    return true;

error:
    vkDestroyBuffer( mVkDevice, pBuffer, nullptr );
    vkFreeMemory( mVkDevice, pMemory, nullptr );
    delete pNewBuffer;

    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::FreeImage( MiniVkImage **ppImage )
{
    MiniVkImage *pImage = nullptr;

    FailOnTo( !ppImage || !*ppImage, error, "Invalid parameter\n" );

    pImage = *ppImage;
    *ppImage = nullptr;

    close( pImage->fd );
    vkDestroyImage( mVkDevice, pImage->pImage, nullptr );
    vkFreeMemory( mVkDevice, pImage->pMemory, nullptr );

error:
    return;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MiniVk::FreeBuffer( MiniVkBuffer **ppBuffer )
{
    MiniVkBuffer *pBuffer = nullptr;

    FailOnTo( !ppBuffer || !*ppBuffer, error, "Invalid parameter\n" );

    pBuffer = *ppBuffer;
    *ppBuffer = nullptr;

    close( pBuffer->fd );
    vkDestroyBuffer( mVkDevice, pBuffer->pBuffer, nullptr );
    vkFreeMemory( mVkDevice, pBuffer->pMemory, nullptr );

error:
    return;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
uint8_t *MiniVk::MapImage( MiniVkImage *pImage )
{
    VkResult err;
    void *pData = nullptr;

    FailOnTo( !pImage, error, "Bad parameter\n" );
    FailOnTo( !pImage->bMappable, error, "Attempted to map non-mappable image\n" );

    if ( !pImage->pCpuAddr )
    {
        err = vkMapMemory( mVkDevice, pImage->pMemory, 0, VK_WHOLE_SIZE, 0, &pData );
        FailOnTo( err != VK_SUCCESS || !pData, error, "Failed to map image\n" );
        pImage->pCpuAddr = pData;
    }

    return (uint8_t*) pData;

error:
    return nullptr;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
uint8_t *MiniVk::MapBuffer( MiniVkBuffer *pBuffer )
{
    VkResult err;
    void *pData = nullptr;

    FailOnTo( !pBuffer, error, "Bad parameter\n" );
    FailOnTo( !pBuffer->bMappable, error, "Attempted to map non-mappable buffer\n" );

    if ( !pBuffer->pCpuAddr )
    {
        err = vkMapMemory( mVkDevice, pBuffer->pMemory, 0, VK_WHOLE_SIZE, 0, &pData );
        FailOnTo( err != VK_SUCCESS || !pData, error, "Failed to map buffer\n" );
        pBuffer->pCpuAddr = pData;
    }

    return (uint8_t*) pBuffer->pCpuAddr;

error:
    return nullptr;
}
