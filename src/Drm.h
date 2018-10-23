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
#include <libdrm/amdgpu_drm.h>

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
        typedef int (*Pfn_drmOpenWithType)( const char *name, const char *busid, int type );
        typedef int (*Pfn_amdgpu_device_initialize)(int fd, uint32_t *major_version, uint32_t *minor_version, amdgpu_device_handle *device_handle );
        typedef int (*Pfn_amdgpu_query_gpu_info)( amdgpu_device_handle dev, struct amdgpu_gpu_info *info );
        typedef int (*Pfn_amdgpu_cs_ctx_create)( amdgpu_device_handle dev, amdgpu_context_handle *context );
        typedef int (*Pfn_amdgpu_query_firmware_version)( amdgpu_device_handle dev, unsigned fw_type,unsigned ip_instance, unsigned index, uint32_t *version, uint32_t *feature );
        typedef int (*Pfn_amdgpu_cs_submit)( amdgpu_context_handle context,uint64_t flags, struct amdgpu_cs_request *ibs_request,uint32_t number_of_requests );
        typedef int (*Pfn_amdgpu_cs_query_fence_status)( struct amdgpu_cs_fence *fence, uint64_t timeout_ns, uint64_t flags,uint32_t *expired);
        typedef int (*Pfn_amdgpu_bo_list_create)( amdgpu_device_handle dev, uint32_t number_of_resources, amdgpu_bo_handle *resources, uint8_t *resource_prios, amdgpu_bo_list_handle *result );
        typedef int (*Pfn_amdgpu_bo_list_destroy)(amdgpu_bo_list_handle handle);
        typedef int (*Pfn_amdgpu_bo_alloc)(amdgpu_device_handle dev, struct amdgpu_bo_alloc_request *alloc_buffer, amdgpu_bo_handle *buf_handle);
        typedef int (*Pfn_amdgpu_bo_free)(amdgpu_bo_handle buf_handle);
        typedef int (*Pfn_amdgpu_bo_import)(amdgpu_device_handle dev, enum amdgpu_bo_handle_type type, uint32_t shared_handle, struct amdgpu_bo_import_result *output);
        typedef int (*Pfn_amdgpu_bo_cpu_map)(amdgpu_bo_handle buf_handle, void **cpu);
        typedef int (*Pfn_amdgpu_bo_cpu_unmap)(amdgpu_bo_handle buf_handle);
        typedef int (*Pfn_amdgpu_cs_ctx_free)(amdgpu_context_handle context);
        typedef int (*Pfn_amdgpu_device_deinitialize)(amdgpu_device_handle device_handle);
        typedef int (*Pfn_amdgpu_va_range_alloc)(amdgpu_device_handle dev, enum amdgpu_gpu_va_range va_range_type, uint64_t size, uint64_t va_base_alignment, uint64_t va_base_required, uint64_t *va_base_allocated, amdgpu_va_handle *va_range_handle, uint64_t flags);
        typedef int (*Pfn_amdgpu_va_range_free)(amdgpu_va_handle va_range_handle);
        typedef int (*Pfn_amdgpu_bo_va_op)(amdgpu_bo_handle bo, uint64_t offset, uint64_t size, uint64_t addr, uint64_t flags, uint32_t ops);

        struct DrmEntrypoints {
            Pfn_drmOpenWithType mPfn_drmOpenWithType;
            Pfn_amdgpu_device_initialize mPfn_amdgpu_device_initialize;
            Pfn_amdgpu_query_gpu_info mPfn_amdgpu_query_gpu_info;
            Pfn_amdgpu_cs_ctx_create mPfn_amdgpu_cs_ctx_create;
            Pfn_amdgpu_query_firmware_version mPfn_amdgpu_query_firmware_version;
            Pfn_amdgpu_cs_submit mPfn_amdgpu_cs_submit;
            Pfn_amdgpu_cs_query_fence_status mPfn_amdgpu_cs_query_fence_status;
            Pfn_amdgpu_bo_list_create mPfn_amdgpu_bo_list_create;
            Pfn_amdgpu_bo_list_destroy mPfn_amdgpu_bo_list_destroy;
            Pfn_amdgpu_bo_alloc mPfn_amdgpu_bo_alloc;
            Pfn_amdgpu_bo_free mPfn_amdgpu_bo_free;
            Pfn_amdgpu_bo_import mPfn_amdgpu_bo_import;
            Pfn_amdgpu_bo_cpu_map mPfn_amdgpu_bo_cpu_map;
            Pfn_amdgpu_bo_cpu_unmap mPfn_amdgpu_bo_cpu_unmap;
            Pfn_amdgpu_cs_ctx_free mPfn_amdgpu_cs_ctx_free;
            Pfn_amdgpu_device_deinitialize mPfn_amdgpu_device_deinitialize;
            Pfn_amdgpu_va_range_alloc mPfn_amdgpu_va_range_alloc;
            Pfn_amdgpu_va_range_free mPfn_amdgpu_va_range_free;
            Pfn_amdgpu_bo_va_op mPfn_amdgpu_bo_va_op;
        };

        void *mDrmLib;
        void *mDrmAmdgpuLib;
        struct DrmEntrypoints mEntrypoints;

        int LoadEntrypoints();

        int mDrmFd;
        amdgpu_device_handle mDevice;
        amdgpu_context_handle mDeviceContext;

        uint32_t mDevMajor;
        uint32_t mDevMinor;
        struct amdgpu_gpu_info mGpuInfo;
};
