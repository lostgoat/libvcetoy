/* * Copyright (C) 2018 Valve Software
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * For operations that take a timeout parameter, passing in this
 * constant will result in an infinite wait
 */
#define VCETOY_TIMEOUT_INFINITE             0xffffffffffffffffull

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
 * This handle represents a libvcetoy job
 */
struct VcetJobProxy;
typedef VcetJobProxy* VcetJobHandle;

/**
 * Check if the current system supports the libvcetoy features
 */
bool VcetIsSystemSupported();

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
 * Note: on success, libvcetoy takes ownership of fd
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
 * @param _job      On success, associate the gpu work with _job
 *
 * @return true on success, false otherwise
 */
bool VcetCalculateMv( VcetCtxHandle _ctx, VcetBoHandle _oldFrame, VcetBoHandle _newFrame, VcetBoHandle _mvBo, uint32_t width, uint32_t height, VcetJobHandle _job );

/**
 * Create a VcetJob object
 *
 * @param _ctx       The vcet context
 * @param pJob       On success, populated with the job handle
 *
 * @return true on success, false otherwise
 */
bool VcetJobCreate( VcetCtxHandle _ctx, VcetJobHandle *pJob );

/**
 * Destroys a libvcetoy job object
 *
 * @param pJob   Pointer to the bo to destroy
 */
void VcetJobDestroy( VcetJobHandle *pJob );

/**
 * Wait for a job to complete with a CPU wait
 *
 * @param _ctx       The vcet context
 * @param _job       The job to wait for
 * @param timeout_ns Timeout value for the wait operation in nanoseconds
 *
 * @return true on success, false otherwise
 */
bool VcetJobWait( VcetCtxHandle _ctx, VcetJobHandle _job, uint64_t timeout_ns );

#ifdef __cplusplus
}
#endif
