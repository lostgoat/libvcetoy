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

#include <util/util.h>
#include <drm/amdgpu_drm.h>

#include "VcetContext.h"
#include "VcetBo.h"

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetBo::VcetBo( VcetContext *pContext )
    : mContext( pContext )
    , mMappable( false )
    , mSizeBytes( 0 )
    , mWidth( 0 )
    , mHeight( 0 )
    , mAlignedWidth( 0 )
    , mAlignedHeight( 0 )
    , mBoHandle( 0 )
    , mVaHandle( 0 )
    , mGpuAddr( 0 )
    , mCpuAddr( nullptr )
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetBo::~VcetBo()
{
    int err;

    if ( mCpuAddr )
        Unmap();

    if ( mBoHandle ) {
        err = amdgpu_bo_va_op( mBoHandle, 0, mSizeBytes, mGpuAddr, 0, AMDGPU_VA_OP_UNMAP );
        WarnOn( err, "Failed to unmap gpu va range\n" );

        err = amdgpu_bo_free( mBoHandle );
        WarnOn( err, "Failed to free amdgpu bo\n" );

        mBoHandle = nullptr;
    }

    if ( mVaHandle ) {
        err = amdgpu_va_range_free( mVaHandle );
        WarnOn( err, "Failed to free va range\n" );

        mVaHandle = 0;
    }
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBo::Allocate( uint64_t sizeBytes, bool mappable, uint32_t alignment )
{
    int err;
    uint64_t gpuAddr = 0;
    amdgpu_va_handle vaHandle;
    amdgpu_bo_handle boHandle;
    uint64_t alignedSize = ALIGN(sizeBytes, alignment );
    int domain = mappable ? AMDGPU_GEM_DOMAIN_GTT :AMDGPU_GEM_DOMAIN_VRAM;
    struct amdgpu_bo_alloc_request req = {};

    FailOnTo( sizeBytes == 0, error, "Invalid bo size\n" );

    req.alloc_size = alignedSize;
    req.phys_alignment = alignment;
    req.preferred_heap = domain;
    req.flags = 0;
    err = amdgpu_bo_alloc( mContext->GetDevice(), &req, &boHandle );
    FailOnTo( err, error, "Failed to allocate amdgpu bo\n" );

    err = amdgpu_va_range_alloc( mContext->GetDevice(),
                                 amdgpu_gpu_va_range_general,
                                 alignedSize, kVaAlignment, 0,
                                 &gpuAddr, &vaHandle, kVaAllocFlags);
    FailOnTo( err, error, "Failed to allocate gpuAddr for bo\n" );

    err = amdgpu_bo_va_op( boHandle, 0, alignedSize, gpuAddr, 0, AMDGPU_VA_OP_MAP);
    FailOnTo( err, error, "Failed to map gpuAddr for bo\n" );

    mGpuAddr = gpuAddr;
    mSizeBytes = alignedSize;
    mBoHandle = boHandle;
    mVaHandle = vaHandle;
    mMappable = mappable;

    return true;

error:
    // TODO de-allocate intermediate stuff
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBo::Allocate( uint32_t width, uint32_t height, bool mappable )
{
    bool ret;
    uint32_t alignedWidth = ALIGN( width, GetWidthAlignment() );
    uint32_t alignedHeight = ALIGN( height, GetHeightAlignment() );
    uint64_t nv21Size = alignedWidth * alignedHeight * kNv21Bpp;

    ret = Allocate( nv21Size, mappable, kDefaultAlignment );
    FailOnTo( !ret, error, "Failed to allocate bo by size\n" );

    mWidth = width;
    mHeight = height;
    mAlignedWidth = alignedWidth;
    mAlignedHeight = alignedHeight;

    return true;

error:
    return false;
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBo::Import( int fd, bool bMappable )
{
    int err;
	struct amdgpu_bo_import_result importResult = {};
	struct amdgpu_bo_info info = {};
    uint64_t gpuAddr = 0;
	amdgpu_va_handle vaHandle;

    err = amdgpu_bo_import( mContext->GetDevice(),
                            amdgpu_bo_handle_type_dma_buf_fd,
                            fd,
                            &importResult );
    FailOnTo( err, error, "Failed to import fd %d\n", fd );

    err = amdgpu_va_range_alloc( mContext->GetDevice(),
                                 amdgpu_gpu_va_range_general,
                                 importResult.alloc_size, kVaAlignment, 0,
                                 &gpuAddr, &vaHandle, kVaAllocFlags);
    FailOnTo( err, error, "Failed to allocate gpuAddr for import bo\n" );

    err = amdgpu_bo_va_op( importResult.buf_handle, 0,
                           importResult.alloc_size,
                           gpuAddr, 0, AMDGPU_VA_OP_MAP);
    FailOnTo( err, error, "Failed to map gpuAddr for bo\n" );

    mGpuAddr = gpuAddr;
    mSizeBytes = importResult.alloc_size;
    mBoHandle = importResult.buf_handle;
    mVaHandle = vaHandle;
    mMappable = bMappable; // Lazy approach

    return true;

error:
    // TODO de-allocate intermediate stuff
    return false;
}

bool VcetBo::Map()
{
    int err;
    uint8_t *cpuAddr = nullptr;

    FailOnTo( !mBoHandle, error, "Attempted to map un-allocated BO\n" );
    FailOnTo( !mMappable, error, "Attempted to map un-mappable BO\n" );
    FailOnTo( mCpuAddr, error, "Attempted to map already-mapped BO\n" );

    err = amdgpu_bo_cpu_map( mBoHandle, (void**) &cpuAddr);
    FailOnTo( err, error, "Failed to cpu map bo\n" );

    mCpuAddr = cpuAddr;

    return true;

error:
    return false;
}

bool VcetBo::Unmap()
{
    int err;

    FailOnTo( !mBoHandle, error, "Attempted to unmap un-allocated BO\n" );
    FailOnTo( !mCpuAddr, error, "Attempted to unmap unmapped BO\n" );

    err = amdgpu_bo_cpu_unmap( mBoHandle );
    FailOnTo( err, error, "Failed to unmap bo\n" );

    mCpuAddr = nullptr;

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBo::IsWidthAligned( VcetContext *ctx, uint32_t width )
{
    uint32_t alignment = GetWidthAlignment( ctx );
    return ( width % alignment ) == 0;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBo::IsHeightAligned( VcetContext *ctx, uint32_t width )
{
    uint32_t alignment = GetHeightAlignment( ctx );
    return ( width % alignment ) == 0;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetBo::GetWidthAlignment()
{
    return GetWidthAlignment( mContext );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetBo::GetHeightAlignment()
{
    return GetHeightAlignment( mContext );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetBo::GetWidthAlignment( VcetContext *ctx )
{
    uint32_t familyId = ctx->GetFamilyId();

    switch ( familyId ) {
    case AMDGPU_FAMILY_RV:
    case AMDGPU_FAMILY_AI:
        return 256;
    case AMDGPU_FAMILY_SI:
    case AMDGPU_FAMILY_CI:
    case AMDGPU_FAMILY_KV:
    case AMDGPU_FAMILY_VI:
    case AMDGPU_FAMILY_CZ:
        return 16;
    default:
        Warn( "Unknown family id: %d - default to highest alignment\n", familyId );
        return 256;
    }
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetBo::GetHeightAlignment( VcetContext *ctx )
{
    uint32_t familyId = ctx->GetFamilyId();

    switch ( familyId ) {
    case AMDGPU_FAMILY_RV:
    case AMDGPU_FAMILY_AI:
    case AMDGPU_FAMILY_SI:
    case AMDGPU_FAMILY_CI:
    case AMDGPU_FAMILY_KV:
    case AMDGPU_FAMILY_VI:
    case AMDGPU_FAMILY_CZ:
        return 16;
    default:
        Warn( "Unknown family id: %d - default to highest alignment\n", familyId );
        return 16;
    }
}
