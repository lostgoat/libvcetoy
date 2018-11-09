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

#include <util/util.h>

#include "Drm.h"
#include "VcetContext.h"

#include "VcetJob.h"

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetJob::VcetJob( VcetContext *pContext )
    : mContext( pContext )
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetJob::~VcetJob()
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetJob::Init()
{
    return true;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetJob::WaitForCompletion( uint64_t timeout )
{
    int err;
    uint32_t expired;
    struct amdgpu_cs_fence fenceStatus = {0};

    fenceStatus.context = mContext->GetDrm()->GetContext();
    fenceStatus.ip_type = mContext->GetIpType();
    fenceStatus.fence = mSeqNo;

    err = mContext->GetDrm()->CsQueryFenceStatus( &fenceStatus, timeout, 0, &expired);
    FailOnTo( err, error, "Failed to wait for job completion: query failed\n" );

    return true;

error:
    return false;
}
