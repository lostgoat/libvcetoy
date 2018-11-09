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

#include <stdio.h>
#include <memory>

#include <util/util.h>
#include <vcetoy/vcetoy.h>

#include "VcetContext.h"
#include "VcetBo.h"
#include "VcetJob.h"

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
struct VcetCtxProxy {
    std::shared_ptr<VcetContext> mPtr;

    VcetCtxProxy( std::shared_ptr<VcetContext> ctx )
        : mPtr(ctx)
    {}
};

static inline VcetContext* VcetCtxFromHandle( VcetCtxHandle hnd )
{
    VcetContext *ctx = nullptr;
    VcetCtxProxy* proxy = reinterpret_cast<VcetCtxProxy*>(hnd);

    FailOnTo( !proxy, error, "Invalid context handle\n" );

    ctx = proxy->mPtr.get();
    FailOnTo( !ctx, error, "Invalid context\n" );

    return ctx;

error:
    return nullptr;
}

#define VCET_CTX_V( name, hnd )                         \
    VcetContext *name = VcetCtxFromHandle( hnd );   \
    if (!name) return;

#define VCET_CTX_B( name, hnd )                         \
    VcetContext *name = VcetCtxFromHandle( hnd );   \
    if (!name) return false;                            \

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
struct VcetBoProxy {
    std::shared_ptr<VcetBo> mPtr;

    VcetBoProxy( std::shared_ptr<VcetBo> bo )
        : mPtr(bo)
    {}
};

static inline VcetBo* VcetBoFromHandle( VcetBoHandle hnd )
{
    VcetBo *bo = nullptr;
    VcetBoProxy* proxy = reinterpret_cast<VcetBoProxy*>(hnd);

    FailOnTo( !proxy, error, "Invalid bo handle\n" );

    bo = proxy->mPtr.get();
    FailOnTo( !bo, error, "Invalid bo\n" );

    return bo;

error:
    return nullptr;
}

#define VCET_BO_V( name, hnd )                          \
    VcetBo *name = VcetBoFromHandle( hnd );             \
    if (!name) return;

#define VCET_BO_B( name, hnd )                          \
    VcetBo *name = VcetBoFromHandle( hnd );             \
    if (!name) return false;

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
struct VcetJobProxy {
    std::shared_ptr<VcetJob> mPtr;

    VcetJobProxy( std::shared_ptr<VcetJob> job )
        : mPtr(job)
    {}
};

static inline VcetJob* VcetJobFromHandle( VcetJobHandle hnd )
{
    VcetJob *job = nullptr;
    VcetJobProxy* proxy = reinterpret_cast<VcetJobProxy*>(hnd);

    FailOnTo( !proxy, error, "Invalid job handle\n" );

    job = proxy->mPtr.get();
    FailOnTo( !job, error, "Invalid job\n" );

    return job;

error:
    return nullptr;
}

#define VCET_JOB_V( name, hnd )                          \
    VcetJob *name = VcetJobFromHandle( hnd );            \
    if (!name) return;

#define VCET_JOB_B( name, hnd )                          \
    VcetJob *name = VcetJobFromHandle( hnd );            \
    if (!name) return false;

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIsSystemSupported()
{
    bool ret;
    std::shared_ptr<VcetContext> ctx = nullptr;

    ctx = std::make_shared<VcetContext>();
    FailOnTo( !ctx, error, "Failed to create context: out of memory\n" );

    ret = ctx->MinimalInit();
    FailOnToQ( !ret, error );

    ret = ctx->IsMvDumpSupported();
    FailOnToQ( !ret, error );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContextCreate( VcetCtxHandle *pCtx, uint32_t maxWidth, uint32_t maxHeight )
{
    bool ret;
    std::shared_ptr<VcetContext> ctx = nullptr;

    FailOnTo( !pCtx, error, "Failed to create context: bad parameter\n" );

    ctx = std::make_shared<VcetContext>();
    FailOnTo( !ctx, error, "Failed to create context: out of memory\n" );

    ret = ctx->Init( maxWidth, maxHeight );
    FailOnTo( !ret, error, "Failed to create context: init failed\n" );

    FailOnTo( !ctx->IsMvDumpSupported(), error, "MV dump not supported\n" );

    *pCtx = new VcetCtxProxy(std::move(ctx));
    FailOnTo( !*pCtx, error, "Failed to create context: failed to allocate handle\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetContextDestroy( VcetCtxHandle *pCtx )
{
    if ( !pCtx )
        return;

    delete *pCtx;
    *pCtx = nullptr;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBoAlignDimensions( VcetCtxHandle _ctx, uint32_t width, uint32_t height, uint32_t *pAlignedWidth, uint32_t *pAlignedHeight )
{
    VCET_CTX_B( ctx, _ctx );

    FailOnTo( !pAlignedWidth || !pAlignedHeight, error, "Failed to align dimensions: bad parameter\n" );

    *pAlignedWidth = ALIGN( width, VcetBo::GetWidthAlignment( ctx ) );
    *pAlignedHeight = ALIGN( height, VcetBo::GetHeightAlignment( ctx ) );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBoCreate( VcetCtxHandle _ctx, uint64_t sizeBytes, bool mappable, VcetBoHandle *pBo )
{
    bool ret;
    std::shared_ptr<VcetBo> bo = nullptr;
    VCET_CTX_B( ctx, _ctx );

    FailOnTo( !pBo, error, "Failed to create bo: bad parameter\n" );

    bo = std::make_shared<VcetBo>( ctx );
    FailOnTo( !bo, error, "Failed to create bo: out of memory\n" );

    ret = bo->Allocate( sizeBytes, mappable );
    FailOnTo( !ret, error, "Failed to create bo: failed to allocate\n" );

    *pBo = new VcetBoProxy(std::move(bo));
    FailOnTo( !*pBo, error, "Failed to create bo: failed to allocate handle\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBoCreateImage( VcetCtxHandle _ctx, uint32_t width, uint32_t height, bool mappable, VcetBoHandle *pBo, uint32_t *pAlignedWidth, uint32_t *pAlignedHeight )
{
    bool ret;
    std::shared_ptr<VcetBo> bo = nullptr;
    VCET_CTX_B( ctx, _ctx );

    FailOnTo( !pBo || !pAlignedWidth || !pAlignedHeight, error, "Failed to create bo: bad parameter\n" );

    bo = std::make_shared<VcetBo>( ctx );
    FailOnTo( !bo, error, "Failed to create bo: out of memory\n" );

    ret = bo->Allocate( width, height, mappable );
    FailOnTo( !ret, error, "Failed to create bo: failed to allocate\n" );

    *pAlignedWidth = bo->GetAlignedWidth();
    *pAlignedHeight = bo->GetAlignedHeight();
    FailOnTo( !*pAlignedWidth || !*pAlignedHeight, error, "Failed to create bo: failed to get aligned dimensions\n" );

    *pBo = new VcetBoProxy(std::move(bo));
    FailOnTo( !*pBo , error, "Failed to create bo: failed to allocate handle\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBoImport( VcetCtxHandle _ctx, int fd, bool mappable, VcetBoHandle *pBo )
{
    bool ret;
    std::shared_ptr<VcetBo> bo = nullptr;
    VCET_CTX_B( ctx, _ctx );

    FailOnTo( !pBo , error, "Failed to import bo: bad parameter\n" );

    bo = std::make_shared<VcetBo>( ctx );
    FailOnTo( !bo, error, "Failed to import bo: out of memory\n" );

    ret = bo->Import( fd, mappable );
    FailOnTo( !ret, error, "Failed to import bo: failed to import\n" );

    *pBo = new VcetBoProxy(std::move(bo));
    FailOnTo( !*pBo , error, "Failed to import bo: failed to allocate handle\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetBoDestroy( VcetBoHandle *pBo )
{
    if ( !pBo )
        return;

    delete *pBo;
    *pBo = nullptr;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBoMap( VcetBoHandle _bo, uint8_t **ppData )
{
    bool ret;
    VCET_BO_B( bo, _bo );

    FailOnTo( !ppData, error, "Failed to map bo: bad parameter\n" );

    ret = bo->Map();
    FailOnTo( !ret, error, "Failed to map bo: map failed\n" );

    *ppData = bo->GetCpuAddr();
    FailOnTo( *ppData == nullptr, error, "Failed to map bo: unexpected cpu addr\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetBoUnmap( VcetBoHandle _bo )
{
    bool ret;
    VCET_BO_B( bo, _bo );

    ret = bo->Unmap();
    FailOnTo( !ret, error, "Failed to unmap bo: unmap failed\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetCalculateMv( VcetCtxHandle _ctx, VcetBoHandle _oldFrame, VcetBoHandle _newFrame, VcetBoHandle _mvBo, uint32_t width, uint32_t height, VcetJobHandle _job )
{
    bool ret;

    VCET_CTX_B( ctx, _ctx );
    VCET_BO_B( oldFrame, _oldFrame );
    VCET_BO_B( newFrame, _newFrame );
    VCET_BO_B( mvBo, _mvBo );
    VCET_JOB_B( job, _job );

    FailOnTo( !ctx , error, "Failed to calculate mv: bad parameter\n" );

    ret = ctx->CalculateMv( oldFrame, newFrame, mvBo, width, height, job );
    FailOnTo( !ret, error, "Failed to calculate mv: processing failure\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetJobCreate( VcetCtxHandle _ctx, VcetJobHandle *pJob )
{
    bool ret;
    std::shared_ptr<VcetJob> job = nullptr;
    VCET_CTX_B( ctx, _ctx );

    FailOnTo( !pJob, error, "Failed to create job: bad parameter\n" );

    job = std::make_shared<VcetJob>( ctx );
    FailOnTo( !job, error, "Failed to create job: out of memory\n" );

    ret = job->Init();
    FailOnTo( !ret, error, "Failed to create job: init failed\n" );

    *pJob = new VcetJobProxy(std::move(job));
    FailOnTo( !*pJob, error, "Failed to create job: failed to allocate handle\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetJobDestroy( VcetJobHandle *pJob )
{
    if ( !pJob )
        return;

    delete *pJob;
    *pJob = nullptr;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetJobWait( VcetCtxHandle _ctx, VcetJobHandle _job, uint64_t timeout_ns )
{
    bool ret;
    VCET_CTX_B( ctx, _ctx );
    VCET_JOB_B( job, _job );

    ret = job->WaitForCompletion( timeout_ns );
    FailOnTo( !ret, error, "Failed to wait for job completion: wait failed.\n" );

    return true;

error:
    return false;
}
