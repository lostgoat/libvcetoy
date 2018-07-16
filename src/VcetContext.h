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

class VcetIb;
class VcetBo;

class VcetContext
{
    private:
        static constexpr int kNumCpbBuffers = 10;
        static constexpr int kNumIbs = 8;
        static constexpr bool kForceSubmitSync = true;

    public:
        VcetContext( );
        ~VcetContext();

        bool Init( uint32_t maxWidth, uint32_t maxHeight );
        bool IsMvDumpSupported();

        bool CalculateMv( VcetBo *oldFrame, VcetBo *newFrame, VcetBo *mvBo, uint32_t width, uint32_t height );

        amdgpu_device_handle GetDevice() { return mDevice; }
        amdgpu_context_handle GetDeviceContext() { return mDeviceContext; }

        uint32_t GetIpType();
        uint32_t GetFamilyId();
        uint32_t GetSessionId() { return mSesionId; }

        VcetBo *GetFb() { return mBoFb; };
        VcetBo *GetBs() { return mBoBs; };
        VcetBo *GetCpb() { return mBoCpb; };

    private:
        int AllocateResources();
        bool AllocateResource( VcetBo*& bo, uint64_t size, bool mappable );
        int CreateSession();
        int DestroySession();

        uint64_t GetFbSize();
        uint64_t GetBsSize();
        uint64_t GetCpbSize();

        VcetIb *GetNextIb();
        bool Submit( VcetIb *ib );

        int mDrmFd;
        struct amdgpu_gpu_info mGpuInfo;
        amdgpu_device_handle mDevice;
        amdgpu_context_handle mDeviceContext;


        uint32_t mSesionId;
        uint32_t mMaxWidth;
        uint32_t mMaxHeight;

        VcetBo *mBoFb;
        VcetBo *mBoBs;
        VcetBo *mBoCpb;

        uint32_t mIbIdx;
        VcetIb *mIbs[ kNumIbs ];
};
