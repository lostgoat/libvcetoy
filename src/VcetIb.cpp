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

#include "VcetIb.h"

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetIb::VcetIb( VcetContext *pContext )
    : mContext( pContext )
    , mBo( pContext )
    , mIbData( nullptr )
    , mSeqNo( 0 )
    , mSize( 0 )
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetIb::~VcetIb()
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::Init()
{
    bool ret;

    ret = mBo.Allocate( kSize, true );
    FailOnTo( !ret, error, "Failed to allocate IB bo\n" );

    ret = mBo.Map();
    FailOnTo( !ret, error, "Failed to map IB bo\n" );

    mIbData = mBo.GetCpuAddr();
    FailOnTo( !mIbData, error, "Failed to get IB cpu address\n" );

    return true;

error:
    // TODO de-allocate intermediate stuff
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WaitFromCompletion( uint64_t timeout )
{
    int err;
    uint32_t expired;
    struct amdgpu_cs_fence fenceStatus = {0};

    fenceStatus.context = mContext->GetDeviceContext();
    fenceStatus.ip_type = mContext->GetIpType();
    fenceStatus.fence = mSeqNo;

    err = amdgpu_cs_query_fence_status( &fenceStatus, timeout, 0, &expired);
    FailOnTo( err, error, "Failed to wait for ib completion\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::Reset()
{
    mSize = 0;
    mSeqNo = 0;

    if ( kClearOnReset ) {
        memset( mIbData, 0, kSize );
    }

    return true;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WriteNop( uint32_t count )
{
    return true;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WriteCalculateMv( VcetBo *oldFrame, VcetBo *newFrame, VcetBo *mvBo, uint32_t width, uint32_t height )
{
    return true;
}
