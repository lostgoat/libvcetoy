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
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetContext::~VcetContext()
{
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
bool VcetContext::Init()
{
    int err;
    uint32_t devMajor, devMinor;
    amdgpu_device_handle hDevice = 0;

    mDrmFd = drmOpenWithType( "amdgpu", NULL, DRM_NODE_RENDER );
    FailOnTo( mDrmFd < 0, error, "Failed to open amdgpu fd\n" );

    err = amdgpu_device_initialize( mDrmFd, &devMajor, &devMinor, &hDevice );
    FailOnTo( err, error, "Failed to initialize amdgpu device\n" );

    err = amdgpu_query_gpu_info( hDevice, &mGpuInfo );
    FailOnTo( err, error, "Failed to query gpu info\n" );

    mDevice = hDevice;

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
