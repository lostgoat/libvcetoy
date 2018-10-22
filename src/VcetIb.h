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
        bool WriteCreateSession( uint32_t width, uint32_t height );
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

        void WriteCreate( uint32_t width, uint32_t height );
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
