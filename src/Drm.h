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

#include <libdrm/amdgpu.h>
#include <drm/amdgpu_drm.h>

class VcetIb;

class Drm
{
    public:
        Drm();
        ~Drm();

        /**
         * Load libdrm and initialize the local context
         */
        int Init();

        /**
         * Returns true if this version of libdrm supports motion estimation
         */
        int QueryFirmwareVersion( unsigned fw_type,
                                  unsigned ip_instance,
                                  unsigned index,
                                  uint32_t *version,
                                  uint32_t *feature );

        /**
         * Submit
         */
        int CsSubmit( uint64_t flags,
                       struct amdgpu_cs_request *ibs_request,
                       uint32_t number_of_requests);

        /**
         * Query fence status
         */
        int CsQueryFenceStatus(struct amdgpu_cs_fence *fence,
                                uint64_t timeout_ns,
                                uint64_t flags,
                                uint32_t *expired);
        /**
         * Create a BO list
         */
        int BoListCreate( uint32_t numberOfResources,
                           amdgpu_bo_handle *resources,
                           uint8_t *resourcePrios,
                           amdgpu_bo_list_handle *result);

        /**
         * Destroy a BO list
         */
        int BoListDestroy( amdgpu_bo_list_handle handle );


        /**
         * Allocate a BO
         */
        int BoAlloc( struct amdgpu_bo_alloc_request *alloc_buffer, amdgpu_bo_handle *buf_handle );

        /**
         * Free a Bo
         */
        int BoFree( amdgpu_bo_handle bo );

        /**
         * Import a BO
         */
        int BoImport( enum amdgpu_bo_handle_type type,
                      uint32_t sharedHandle,
                      struct amdgpu_bo_import_result *result );
        /**
         * Allocate a VA range
         */
        int VaRangeAlloc( enum amdgpu_gpu_va_range type,
                          uint64_t size,
                          uint64_t vaBaseAlignment,
                          uint64_t vaBaseRequired,
                          uint64_t *vaBaseAllocated,
                          amdgpu_va_handle *vaRangeHandle,
                          uint64_t flags );

        /**
         * Free a VA range
         */
        int VaRangeFree( amdgpu_va_handle va );

        /**
         * Perform a VA operation
         */
        int BoVaOp( amdgpu_bo_handle bo,
                     uint64_t offset,
                     uint64_t size,
                     uint64_t addr,
                     uint64_t flags,
                     uint32_t ops );

        /**
         * Map a BO
         */
        int BoCpuMap( amdgpu_bo_handle buf_handle, uint8_t **cpu );

        /**
         * Unmap a BO
         */
        int BoCpuUnmap( amdgpu_bo_handle buf_handle );

        /**
         * Getters/Setters
         */
        struct amdgpu_gpu_info *GetGpuInfo() { return &mGpuInfo; }
        amdgpu_context_handle GetContext() { return mDeviceContext; }

    private:
        int LoadEntrypoints();

        int mDrmFd;
        amdgpu_device_handle mDevice;
        amdgpu_context_handle mDeviceContext;

        uint32_t mDevMajor;
        uint32_t mDevMinor;
        struct amdgpu_gpu_info mGpuInfo;
};
