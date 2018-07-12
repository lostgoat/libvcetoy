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

#include <unistd.h>

#include <util/util.h>
#include <xf86drm.h>
#include <drm/amdgpu_drm.h>

#include "VcetIb.h"
#include "VcetBo.h"

#include "VcetContext.h"

/**
 * This firmware version introduces support for MV dumping
 */
#define FW_53_0_03 ((53 << 24) | (0 << 16) | (03 << 8))

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetContext::VcetContext()
    : mDrmFd( -1 )
    , mDevice( 0 )
    , mMaxWidth( 0 )
    , mMaxHeight( 0 )
    , mBoFb( nullptr )
    , mBoBs( nullptr )
    , mBoCpb( nullptr )
{
    memset( mIbs, 0, sizeof(mIbs) );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetContext::~VcetContext()
{
    delete mBoFb;
    mBoFb = nullptr;

    delete mBoBs;
    mBoBs = nullptr;

    delete mBoCpb;
    mBoCpb = nullptr;

    if ( mDevice ) {
        amdgpu_device_deinitialize(mDevice);
        mDevice = 0;
    }

    if ( mDrmFd >= 0 ) {
        close( mDrmFd );
        mDrmFd = -1;
    }
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::Init( uint32_t maxWidth, uint32_t maxHeight )
{
    int err;
    uint32_t devMajor, devMinor;

    FailOnTo( !maxWidth || !maxHeight, error, "Bad dimensions\n" );
    mMaxWidth = maxWidth;
    mMaxHeight = maxHeight;

    mDrmFd = drmOpenWithType( "amdgpu", NULL, DRM_NODE_RENDER );
    FailOnTo( mDrmFd < 0, error, "Failed to open amdgpu fd\n" );

    err = amdgpu_device_initialize( mDrmFd, &devMajor, &devMinor, &mDevice );
    FailOnTo( err, error, "Failed to initialize amdgpu device\n" );

    err = amdgpu_query_gpu_info( mDevice, &mGpuInfo );
    FailOnTo( err, error, "Failed to query gpu info\n" );

	err = amdgpu_cs_ctx_create( mDevice, &mDeviceContext );
    FailOnTo( err, error, "Failed to create device context\n" );

    err = AllocateResources();
    FailOnTo( err, error, "Failed to allocate context resources\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::IsMvDumpSupported()
{
    int err;
    uint32_t fwVersion, fwFeature;
    uint32_t chipId = mGpuInfo.chip_external_rev;

    // Latest supported gpu
    if ( mGpuInfo.family_id >= AMDGPU_FAMILY_RV )
        return false;

    // Minimum required gpu family
    if ( mGpuInfo.family_id < AMDGPU_FAMILY_CI )
        return false;

    err = amdgpu_query_firmware_version( mDevice, AMDGPU_INFO_FW_VCE, 0,
                                         0, &fwVersion, &fwFeature);
    FailOnTo( err, error, "Failed to query firmware version\n" );

    // These chips have a fw requirement for MV support
    // TODO: this needs a cleanup so we can switch-case
    if (chipId == (mGpuInfo.chip_rev + 0x3C) || /* FIJI */
        chipId == (mGpuInfo.chip_rev + 0x50) || /* Polaris 10*/
        chipId == (mGpuInfo.chip_rev + 0x5A) || /* Polaris 11*/
        chipId == (mGpuInfo.chip_rev + 0x64))   /* Polaris 12*/
    {
        if ( fwVersion >= FW_53_0_03 )
            return true;
    }

error:
    // Assume we don't support anything else
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::AllocateResource( VcetBo*& bo, uint64_t size, bool mappable )
{
    bool ret;

    bo = new VcetBo( this );
    FailOnTo( !bo, error, "Failed to create bo\n" );

    ret = bo->Allocate( size, true );
    FailOnTo( !ret, error, "Failed to allocate fb bo\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int VcetContext::AllocateResources()
{
    bool ret;

    ret = AllocateResource( mBoFb, GetFbSize(), true );
    FailOnTo( !ret, error, "Failed to allocate fb bo\n" );

    ret = AllocateResource( mBoBs, GetBsSize(), true );
    FailOnTo( !ret, error, "Failed to allocate bs bo\n" );

    ret = AllocateResource( mBoCpb, GetCpbSize(), false );
    FailOnTo( !ret, error, "Failed to allocate cpb bo\n" );

    for ( int i = 0; i < kNumIbs; ++i ) {
        mIbs[i] = new VcetIb( this );
        FailOnTo( !mIbs[i], error, "Failed to create ib\n" );

        ret = mIbs[i]->Init();
        FailOnTo( !ret, error, "Failed to init IB\n" );
    }

    return 0;

error:
    return -1;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint64_t VcetContext::GetFbSize()
{
    // TODO: might need per-family values
    return 4096;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint64_t VcetContext::GetBsSize()
{
    // TODO: need per-family values
    return 0x154000;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint64_t VcetContext::GetCpbSize()
{
    uint32_t alignedWidth = ALIGN( mMaxWidth, GetAlignmentWidth() );
    uint32_t alignedHeight = ALIGN( mMaxWidth, GetAlignmentHeight() );

    return alignedWidth * alignedHeight * kNv21Bpp * kNumCpbBuffers;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetContext::GetAlignmentWidth()
{
    // TODO: need per-family values
    return 16;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetContext::GetAlignmentHeight()
{
    // TODO: need per-family values
    return 16;
}
