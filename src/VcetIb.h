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
#include <vector>

#include "VcetBo.h"

class VcetContext;

class VcetIb
{
    private:
        static const uint64_t kSizeBytes = 4096;
        static const uint64_t kAlignment = 4096;
        static const uint32_t kNopCmd = 0;
        static const bool kClearOnReset = true;

    public:
        VcetIb( VcetContext *pContext );
        ~VcetIb();

        /**
         * Initialize
         */
        bool Init();

        bool Reset();
        bool WriteNop( uint32_t count );
        bool WriteCalculateMv( VcetBo *oldFrame, VcetBo *newFrame, VcetBo *mvBo, uint32_t width, uint32_t height );
        bool WriteCreateSession();
        bool WriteoDestroySession();

        bool WaitFromCompletion( uint64_t timeout = AMDGPU_TIMEOUT_INFINITE );

        uint32_t GetSizeDw() { return mSizeDw; }
        uint64_t GetGpuAddress() { return mBo.GetGpuAddr(); }
        uint32_t GetNumResources() { return mReferencedResources.size(); }
        amdgpu_bo_handle *GetResources() { return mReferencedResources.data(); }

        uint64_t GetSeqNo() { return mSeqNo; }
        void SetSeqNo( uint64_t seq ) { mSeqNo = seq; }

    private:
        void Write( uint32_t cmd );
        void Write( uint32_t *pCmd, uint32_t count );

        void WriteCreate();
        void WriteDestroy();
        void WriteSession();
        void WriteTaskInfo( uint32_t id );
        void WriteVceConfig();
        void WriteConfigInit();
        void WriteBsBuffer();
        void WriteFeedbackBuffer();
        void WriteContextBuffer();
        void WriteAuxBuffer( uint64_t frameSizeBytes );
        void WriteMvCmd( VcetBo *refFrame, VcetBo *mvBo, uint32_t width, uint32_t height );
        void WriteEncodeCmd( VcetBo *frame, uint32_t width, uint32_t height );

        void RefResource( VcetBo *bo );

        VcetContext *mContext;
        VcetBo mBo;

        uint32_t *mIbData;
        uint64_t mSeqNo;
        uint32_t mSizeDw;

        std::vector<amdgpu_bo_handle> mReferencedResources;
};
