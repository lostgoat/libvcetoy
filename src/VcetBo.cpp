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
bool VcetBo::Allocate( uint64_t sizeBytes, bool mappable )
{
    int err;
    uint64_t gpuAddr = 0;
    amdgpu_va_handle vaHandle;
    amdgpu_bo_handle boHandle;
    uint64_t alignedSize = ALIGN(sizeBytes, kMemoryAlignment);
    int domain = mappable ? AMDGPU_GEM_DOMAIN_GTT :AMDGPU_GEM_DOMAIN_VRAM;
    struct amdgpu_bo_alloc_request req = {};

    FailOnTo( sizeBytes == 0, error, "Invalid bo size\n" );

    req.alloc_size = alignedSize;
    req.preferred_heap = domain;
    err = amdgpu_bo_alloc( mContext->GetDevice(), &req, &boHandle );
    FailOnTo( err, error, "Failed to allocate amdgpu bo\n" );

    err = amdgpu_va_range_alloc( mContext->GetDevice(),
                                 amdgpu_gpu_va_range_general,
                                 alignedSize, kVaBaseAlignment, 0,
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
