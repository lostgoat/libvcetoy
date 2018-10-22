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



#ifndef VK_DEVICE_EXT_FN
#define VK_DEVICE_EXT_FN(name)
#endif

#ifndef VK_INSTANCE_EXT_FN
#define VK_INSTANCE_EXT_FN(name)
#endif

#ifndef VK_INSTANCE_EXT
#define VK_INSTANCE_EXT(name)
#endif

#ifndef VK_DEVICE_EXT
#define VK_DEVICE_EXT(name)
#endif

VK_INSTANCE_EXT( VK_KHR_SURFACE_EXTENSION )
VK_INSTANCE_EXT( VK_KHR_XCB_SURFACE_EXTENSION )
VK_INSTANCE_EXT( VK_KHR_XLIB_SURFACE_EXTENSION )

VK_DEVICE_EXT( VK_KHR_SWAPCHAIN_EXTENSION )
VK_DEVICE_EXT( VK_KHR_MAINTENANCE1_EXTENSION )
VK_DEVICE_EXT( VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION )
VK_DEVICE_EXT( VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION )
VK_DEVICE_EXT( VK_KHR_EXTERNAL_MEMORY_EXTENSION )
VK_DEVICE_EXT( VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION )
VK_DEVICE_EXT( VK_KHR_DEDICATED_ALLOCATION_EXTENSION )
VK_DEVICE_EXT( VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION )

VK_DEVICE_EXT_FN( GetMemoryFdKHR )
VK_DEVICE_EXT_FN( CreateXcbSurfaceKHR )
VK_DEVICE_EXT_FN( CreateSwapchainKHR )
VK_DEVICE_EXT_FN( DestroySwapchainKHR )
VK_DEVICE_EXT_FN( GetSwapchainImagesKHR )
VK_DEVICE_EXT_FN( AcquireNextImageKHR )
VK_DEVICE_EXT_FN( QueuePresentKHR )
VK_DEVICE_EXT_FN( GetPhysicalDeviceXcbPresentationSupportKHR )
VK_DEVICE_EXT_FN( GetImageMemoryRequirements2KHR )
VK_DEVICE_EXT_FN( GetBufferMemoryRequirements2KHR )

#undef VK_INSTANCE_EXT
#undef VK_INSTANCE_EXT_FN
#undef VK_DEVICE_EXT
#undef VK_DEVICE_EXT_FN
