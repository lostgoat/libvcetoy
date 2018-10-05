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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This handle represents a libvcetoy context instance
 */
struct VcetCtxProxy;
typedef VcetCtxProxy* VcetCtxHandle;

/**
 * This handle represents a libvcetoy buffer object
 */
struct VcetBoProxy;
typedef VcetBoProxy* VcetBoHandle;

/**
 * Create a libvcetoy context
 *
 * @param pCtx      On success, populated with the libvcetoy context handle
 * @param width     The frame width the app expects to handle
 * @param height    The frame height the app expects to handle
 *
 * @return true on success, false otherwise
 */
bool VcetContextCreate( VcetCtxHandle *pCtx, uint32_t width, uint32_t height );

/**
 * Destroy a libvcetoy context
 *
 * Destroys the context referenced by the *pCtx handle
 *
 * @return true on success, false otherwise
 */
void VcetContextDestroy( VcetCtxHandle *pCtx );

/**
 * Calculates the required HW alignment for a NV21 image
 *
 * @param ctx               The VcetCtx
 * @param width             Image width
 * @param height            Image height
 * @param pAlignedWidth     Aligned width according to HW requirements
 * @param pAlignedHeight    Aligned height according to HW requirements
 *
 * @return true on success, false otherwise
 */
bool VcetBoAlignDimensions( VcetCtxHandle _ctx, uint32_t width, uint32_t height, uint32_t *pAlignedWidth, uint32_t *pAlignedHeight );

/**
 * Creates a libvcetoy buffer object
 *
 * @param ctx       The VcetCtx from which to allocate the bo
 * @param sizeBytes Requested size of the bo in bytes
 * @param mappable  Request that the allocated bo be CPU mappable
 * @param pBo       On success, populated with the bo handle
 *
 * @return true on success, false otherwise
 */
bool VcetBoCreate( VcetCtxHandle ctx, uint64_t sizeBytes, bool mappable, VcetBoHandle *pBo );

/**
 * Creates a libvcetoy buffer object to represent an NV21 image
 *
 * @param ctx               The VcetCtx from which to allocate the bo
 * @param width             Image width
 * @param height            Image height
 * @param mappable          Request that the allocated bo be CPU mappable
 * @param pBo               On success, populated with the bo handle
 * @param pAlignedWidth     Aligned width according to HW requirements
 * @param pAlignedHeight    Aligned height according to HW requirements
 *
 * @return true on success, false otherwise
 */
bool VcetBoCreateImage( VcetCtxHandle ctx, uint32_t width, uint32_t height, bool mappable, VcetBoHandle *pBo, uint32_t *pAlignedWidth, uint32_t *pAlignedHeight );

/**
 * Creates a libvcetoy buffer object that aliases the memory referenced by fd
 *
 * @param ctx               The VcetCtx from which to allocate the bo
 * @param fd                A dma buf fd representing the memory
 * @param mappable          Specify whether the memory referenced by fd is mappable
 * @param pBo               On success, populated with the bo handle
 *
 * @return true on success, false otherwise
 */
bool VcetBoImport( VcetCtxHandle ctx, int fd, bool mappable, VcetBoHandle *pBo );

/**
 * Destroys a libvcetoy buffer object
 *
 * @param pBo   Pointer to the bo to destroy
 */
void VcetBoDestroy( VcetBoHandle *pBo );

/**
 * Map a buffer object for cpu access
 *
 * @param bo        The bo to map
 * @param ppData    Will be filled with a cpu pointer to bo's memory
 *
 * @return true on success, false otherwise
 */
bool VcetBoMap( VcetBoHandle bo, uint8_t **ppData );

/**
 * Destroy a buffer objects CPU mapping
 *
 * @param bo    The bo to unmap
 *
 * @return true on success, false otherwise
 */
bool VcetBoUnmap( VcetBoHandle bo );

/**
 * Calculate the motion vector delta between oldFrame and newFrame
 *
 * @param _ctx      The vcet context
 * @param _oldFrame The reference frame in NV21 format
 * @param _newFrame The current frame in NV21 format
 * @param _mvBo     The buffer in which to dump the motion vector data
 * @param width     The frame's width dimension
 * @param height    The frame's height dimension
 *
 * @return true on success, false otherwise
 */
bool VcetCalculateMv( VcetCtxHandle _ctx, VcetBoHandle _oldFrame, VcetBoHandle _newFrame, VcetBoHandle _mvBo, uint32_t width, uint32_t height );

#ifdef __cplusplus
}
#endif
