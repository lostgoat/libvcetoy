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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This handle represents a libvcetoy context instance
 */
typedef void* VcetCtxHandle;

/**
 * Create a libvcetoy context
 *
 * On success, pCtx will be populated with a context handle
 *
 * Returns: true on success, false otherwise
 */
bool VcetContextCreate( VcetCtxHandle *pCtx );

/**
 * Destroy a libvcetoy context
 *
 * Destroys the context referenced by the *pCtx handle
 */
void VcetContextDestroy( VcetCtxHandle *pCtx );

#ifdef __cplusplus
}
#endif
