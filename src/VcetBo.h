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
        VcetBo( VcetContext *pContext );
        ~VcetBo();

        /**
         * Allocate sizeBytes of GPU addressable memory
         *
         * If mappable is set, then the memory will be CPU visible as well.
         */
        bool Allocate( uint64_t sizeBytes, bool mappable, uint32_t alignment = kDefaultAlignment );

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
         * Get the current cpu address
         */
        uint8_t *GetCpuAddr() { return mCpuAddr; }

        /**
         * Get the current Gpu address
         */
        uint64_t GetGpuAddr() { return mGpuAddr; }

        /**
         * Get the amdgpu bo handle
         */
        amdgpu_bo_handle GetBoHandle() { return mBoHandle; }

        /**
         * Get the bo's size
         */
        uint64_t GetSizeBytes() { return mSizeBytes; }

    private:
        VcetContext *mContext;

        bool mMappable;
        uint64_t mSizeBytes;

        amdgpu_bo_handle mBoHandle;
        amdgpu_va_handle mVaHandle;

        uint64_t mGpuAddr;
        uint8_t *mCpuAddr;
};
