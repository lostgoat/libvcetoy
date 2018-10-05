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

#include <libdrm/amdgpu.h>

#include <memory>

class VcetContext;

class VcetBo
{
    private:
        static const int kDefaultAlignment = 4096;
        static const int kVaAlignment = 4096;
        static const uint64_t kVaAllocFlags = 0;

    public:
        static constexpr float kNv21Bpp = 1.5;
        static uint32_t GetWidthAlignment( VcetContext *ctx );
        static uint32_t GetHeightAlignment( VcetContext *ctx );
        static bool IsWidthAligned( VcetContext *ctx, uint32_t width );
        static bool IsHeightAligned( VcetContext *ctx, uint32_t width );

    public:

        VcetBo( VcetContext *pContext );
        ~VcetBo();

        /**
         * Allocate sizeBytes of GPU addressable memory
         *
         * If mappable is set, then the memory will be CPU visible as well.
         */
        bool Allocate( uint64_t sizeBytes, bool mappable, uint32_t alignment = kDefaultAlignment );

        /**
         * Allocate an NV21 image of dimensions width x height
         *
         * If mappable is set, then the memory will be CPU visible as well.
         */
        bool Allocate( uint32_t width, uint32_t height, bool mappable );

        /**
         * Import a BO from a dma buf fd
         *
         * Caller must specify whether the buffer was allocated with mappable properties
         */
        bool Import( int fd, bool bMappable );

        /**
         * Map the BO for cpu usage
         *
         * Bo must've been allocated as mappable, otherwise an error will be produced
         */
        bool Map();

        /**
         * Release the BO's cpu mapping
         */
        bool Unmap();

        /**
         * Getters/Setters
         */
        uint8_t*    GetCpuAddr()        { return mCpuAddr; }
        uint64_t    GetGpuAddr()        { return mGpuAddr; }
        amdgpu_bo_handle GetBoHandle()  { return mBoHandle; }
        uint64_t    GetSizeBytes()      { return mSizeBytes; }
        uint32_t    GetWidth()          { return mWidth; }
        uint32_t    GetHeight()         { return mHeight; }
        uint32_t    GetAlignedWidth()   { return mAlignedWidth; }
        uint32_t    GetAlignedHeight()  { return mAlignedHeight; }

    private:
        uint32_t GetWidthAlignment();
        uint32_t GetHeightAlignment();

        VcetContext *mContext;

        bool mMappable;

        uint64_t mSizeBytes;
        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mAlignedWidth;
        uint32_t mAlignedHeight;

        amdgpu_bo_handle mBoHandle;
        amdgpu_va_handle mVaHandle;

        uint64_t mGpuAddr;
        uint8_t *mCpuAddr;
};
