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

#include <util/util.h>
#include <vcetoy/vcetoy.h>

#include "VcetContext.h"

#define VCET_CTX( name, ctx ) \
    VcetContext* name = reinterpret_cast<VcetContext*>(ctx);

bool VcetContextCreate( VcetCtxHandle *pCtx )
{
    bool ret;

    VcetContext *ctx = new VcetContext();
    FailOnTo( !ctx, error, "Failed to allocate VcetContext\n" );

    ret = ctx->Init();
    FailOnTo( !ret, error, "Failed to initialize Vcetcontext\n" );

    FailOnTo( !ctx->IsMvDumpSupported(), error, "MV dump not supported\n" );

    *pCtx = reinterpret_cast<VcetCtxHandle>( ctx );

    return true;

error:
    delete ctx;
    return false;
}

void VcetContextDestroy( VcetCtxHandle *pCtx )
{
    if ( !pCtx )
        return;

    VCET_CTX( ctx, *pCtx );
    delete ctx;

    *pCtx = nullptr;
}
