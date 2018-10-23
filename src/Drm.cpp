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

#include <dlfcn.h>
#include <unistd.h>
#include <cstring>
#include <xf86drm.h>

#include <util/util.h>
#include "VcetIb.h"

#include "Drm.h"

#define DRM_CALL( fnName, ... ) mEntrypoints.mPfn_##fnName( __VA_ARGS__ )

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
Drm::Drm()
    : mDrmLib( nullptr )
    , mDrmAmdgpuLib( nullptr )
    , mDrmFd( -1 )
    , mDevice( nullptr )
    , mDeviceContext( nullptr )
    , mDevMajor( 0 )
    , mDevMinor( 0 )
{
    ::memset( &mEntrypoints, 0, sizeof(mEntrypoints) );
    ::memset( &mGpuInfo, 0, sizeof(mGpuInfo) );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
Drm::~Drm()
{
    if ( mDeviceContext ) {
        DRM_CALL( amdgpu_cs_ctx_free, mDeviceContext );
        mDeviceContext = 0;
    }

    if ( mDevice ) {
        DRM_CALL( amdgpu_device_deinitialize, mDevice );
        mDevice = 0;
    }

    if ( mDrmFd >= 0 ) {
        close( mDrmFd );
        mDrmFd = -1;
    }
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::Init()
{
    int err;

    err = LoadEntrypoints();
    FailOnTo( err, error, "Failed to load libdrm entrypoints\n" );

    mDrmFd = DRM_CALL( drmOpenWithType, "amdgpu", NULL, DRM_NODE_RENDER );
    FailOnTo( mDrmFd < 0, error, "Failed to open amdgpu fd\n" );

    err = DRM_CALL( amdgpu_device_initialize, mDrmFd, &mDevMajor, &mDevMinor, &mDevice );
    FailOnTo( err, error, "Failed to initialize amdgpu device\n" );

    err = DRM_CALL( amdgpu_query_gpu_info, mDevice, &mGpuInfo );
    FailOnTo( err, error, "Failed to query gpu info\n" );

    err = DRM_CALL( amdgpu_cs_ctx_create, mDevice, &mDeviceContext );
    FailOnTo( err, error, "Failed to create device context\n" );

    return 0;

error:
    return err;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::LoadEntrypoints()
{
    mDrmLib = dlopen( "libdrm.so", RTLD_NOW );
    FailOnTo( !mDrmLib, error, "Failed to load libdrm\n" );

    mDrmAmdgpuLib = dlopen( "libdrm_amdgpu.so", RTLD_NOW );
    FailOnTo( !mDrmAmdgpuLib, error, "Failed to load libdrm_amdgpu\n" );

#define DRM_DLSYM_ENTRYPOINT(handle, name) \
    mEntrypoints.mPfn_##name = (Pfn_##name) dlsym( handle, #name ); \
    if ( !mEntrypoints.mPfn_##name ) \
        goto error;

    DRM_DLSYM_ENTRYPOINT(mDrmLib, drmOpenWithType);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_device_initialize);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_query_gpu_info);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_cs_ctx_create);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_query_firmware_version);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_cs_submit);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_cs_query_fence_status);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_list_create);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_list_destroy);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_alloc);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_free);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_import);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_cpu_map);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_cpu_unmap);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_cs_ctx_free);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_device_deinitialize);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_va_range_alloc);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_va_range_free);
    DRM_DLSYM_ENTRYPOINT(mDrmAmdgpuLib, amdgpu_bo_va_op);

#undef DRM_DLSYM_ENTRYPOINT

    return 0;

error:
    return -1;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::QueryFirmwareVersion( unsigned fw_type, unsigned ip_instance, unsigned index, uint32_t *version, uint32_t *feature )
{
    return DRM_CALL( amdgpu_query_firmware_version, mDevice, fw_type, ip_instance, index, version, feature );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::CsSubmit( uint64_t flags, struct amdgpu_cs_request *ibs_request, uint32_t number_of_requests)
{
    return DRM_CALL( amdgpu_cs_submit, mDeviceContext, flags, ibs_request, number_of_requests );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::CsQueryFenceStatus( struct amdgpu_cs_fence *fence, uint64_t timeoutNs, uint64_t flags, uint32_t *expired )
{
    return DRM_CALL( amdgpu_cs_query_fence_status, fence, timeoutNs, flags, expired );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoListCreate( uint32_t numberOfResources, amdgpu_bo_handle *resources, uint8_t *resourcePrios, amdgpu_bo_list_handle *result )
{
    return DRM_CALL( amdgpu_bo_list_create, mDevice, numberOfResources, resources, resourcePrios, result );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoListDestroy( amdgpu_bo_list_handle handle )
{
    return DRM_CALL( amdgpu_bo_list_destroy, handle );
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoAlloc( struct amdgpu_bo_alloc_request *alloc_buffer, amdgpu_bo_handle *buf_handle )
{
    return DRM_CALL( amdgpu_bo_alloc, mDevice, alloc_buffer, buf_handle );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoFree( amdgpu_bo_handle bo )
{
    return DRM_CALL( amdgpu_bo_free, bo );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoImport( enum amdgpu_bo_handle_type type, uint32_t sharedHandle, struct amdgpu_bo_import_result *result )
{
    return DRM_CALL( amdgpu_bo_import, mDevice, type, sharedHandle, result );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::VaRangeAlloc( enum amdgpu_gpu_va_range type, uint64_t size, uint64_t vaBaseAlignment, uint64_t vaBaseRequired, uint64_t *vaBaseAllocated, amdgpu_va_handle *vaRangeHandle, uint64_t flags )
{
    return DRM_CALL( amdgpu_va_range_alloc, mDevice, type, size, vaBaseAlignment, vaBaseRequired, vaBaseAllocated, vaRangeHandle, flags );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::VaRangeFree( amdgpu_va_handle va )
{
    return DRM_CALL( amdgpu_va_range_free, va );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoVaOp( amdgpu_bo_handle bo, uint64_t offset, uint64_t size, uint64_t addr, uint64_t flags, uint32_t ops )
{
    return DRM_CALL( amdgpu_bo_va_op, bo, offset, size, addr, flags, ops );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoCpuMap( amdgpu_bo_handle buf_handle, uint8_t **cpu )
{
    return DRM_CALL( amdgpu_bo_cpu_map, buf_handle, (void**)cpu );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int Drm::BoCpuUnmap( amdgpu_bo_handle buf_handle )
{
    return DRM_CALL( amdgpu_bo_cpu_unmap, buf_handle );
}
