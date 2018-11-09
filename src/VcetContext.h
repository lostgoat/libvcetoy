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

#include "Drm.h"

class VcetIb;
class VcetBo;
class VcetJob;

class VcetContext
{
    private:
        static constexpr int kNumCpbBuffers = 10;
        static constexpr int kNumIbs = 8;
        static constexpr bool kForceSubmitSync = false;

    public:
        VcetContext( );
        ~VcetContext();

        bool Init( uint32_t width, uint32_t height );
        bool IsMvDumpSupported();

        bool CalculateMv( VcetBo *oldFrame, VcetBo *newFrame, VcetBo *mvBo, uint32_t width, uint32_t height, VcetJob *pJob );

        uint32_t GetIpType();
        uint32_t GetFamilyId();
        uint32_t GetSessionId() { return mSesionId; }

        VcetBo *GetFb() { return mBoFb; }
        VcetBo *GetBs() { return mBoBs; }
        VcetBo *GetCpb() { return mBoCpb; }

        Drm *GetDrm() { return &mDrm; }

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

        Drm mDrm;

        uint32_t mSesionId;
        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mAlignedWidth;
        uint32_t mAlignedHeight;

        VcetBo *mBoFb;
        VcetBo *mBoBs;
        VcetBo *mBoCpb;

        uint32_t mIbIdx;
        VcetIb *mIbs[ kNumIbs ];
};
