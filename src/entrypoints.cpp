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

#include <stdio.h>
#include <memory>

#include <util/util.h>
#include <vcetoy/vcetoy.h>

#include "VcetContext.h"
#include "VcetBo.h"

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

    FailOnTo( ctx.get() != nullptr, error, "ANDRES test move\n");

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
bool VcetCalculateMv( VcetCtxHandle _ctx, VcetBoHandle _oldFrame, VcetBoHandle _newFrame, VcetBoHandle _mvBo, uint32_t width, uint32_t height )
{
    bool ret;

    VCET_CTX_B( ctx, _ctx );
    VCET_BO_B( oldFrame, _oldFrame );
    VCET_BO_B( newFrame, _newFrame );
    VCET_BO_B( mvBo, _mvBo );

    FailOnTo( !ctx , error, "Failed to calculate mv: bad parameter\n" );

    ret = ctx->CalculateMv( oldFrame, newFrame, mvBo, width, height );
    FailOnTo( !ret, error, "Failed to calculate mv: processing failure\n" );
    
    return true;

error:
    return false;
}
