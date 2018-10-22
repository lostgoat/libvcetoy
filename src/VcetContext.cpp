//
// Copyright (C) 2018 Valve Software
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the
// Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall
// be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <unistd.h>

#include <util/util.h>
#include <xf86drm.h>
#include <drm/amdgpu_drm.h>

#include "VcetIb.h"
#include "VcetBo.h"

#include "VcetContext.h"

/**
 * This firmware version introduces support for MV dumping
 */
#define FW_53_0_03 ((53 << 24) | (0 << 16) | (03 << 8))

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
static uint32_t GenSessionId()
{
    static uint32_t sNextSessionId = 1;
    return  ( 0xA3D << 16 ) | sNextSessionId++;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetContext::VcetContext()
    : mDrmFd( -1 )
    , mDevice( 0 )
    , mDeviceContext( 0 )
    , mSesionId( GenSessionId() )
    , mWidth( 0 )
    , mHeight( 0 )
    , mAlignedWidth( 0 )
    , mAlignedHeight( 0 )
    , mBoFb( nullptr )
    , mBoBs( nullptr )
    , mBoCpb( nullptr )
    , mIbIdx( 0 )
{
    memset( mIbs, 0, sizeof(mIbs) );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetContext::~VcetContext()
{
    bool err;

    err = DestroySession();
    WarnOn( err, "Failed to destroy VCE session\n" );

    delete mBoFb;
    mBoFb = nullptr;

    delete mBoBs;
    mBoBs = nullptr;

    delete mBoCpb;
    mBoCpb = nullptr;

    for ( int i = 0; i < kNumIbs; ++i ) {
        delete mIbs[i];
    }

    if ( mDeviceContext ) {
        amdgpu_cs_ctx_free( mDeviceContext );
        mDeviceContext = 0;
    }

    if ( mDevice ) {
        amdgpu_device_deinitialize(mDevice);
        mDevice = 0;
    }

    if ( mDrmFd >= 0 ) {
        close( mDrmFd );
        mDrmFd = -1;
    }
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::Init( uint32_t width, uint32_t height )
{
    int err;
    uint32_t devMajor, devMinor;

    FailOnTo( !width || !height, error, "Bad dimensions\n" );

    mDrmFd = drmOpenWithType( "amdgpu", NULL, DRM_NODE_RENDER );
    FailOnTo( mDrmFd < 0, error, "Failed to open amdgpu fd\n" );

    err = amdgpu_device_initialize( mDrmFd, &devMajor, &devMinor, &mDevice );
    FailOnTo( err, error, "Failed to initialize amdgpu device\n" );

    err = amdgpu_query_gpu_info( mDevice, &mGpuInfo );
    FailOnTo( err, error, "Failed to query gpu info\n" );

    mWidth = width;
    mHeight = height;
    mAlignedWidth = ALIGN( mWidth, VcetBo::GetWidthAlignment( this ) );
    mAlignedHeight = ALIGN( mHeight, VcetBo::GetHeightAlignment( this ) );

    err = amdgpu_cs_ctx_create( mDevice, &mDeviceContext );
    FailOnTo( err, error, "Failed to create device context\n" );

    err = AllocateResources();
    FailOnTo( err, error, "Failed to allocate context resources\n" );

    err = CreateSession();
    FailOnTo( err, error, "Failed to create session\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::IsMvDumpSupported()
{
    int err;
    uint32_t fwVersion, fwFeature;
    uint32_t chipId = mGpuInfo.chip_external_rev;

    // Latest supported gpu
    if ( mGpuInfo.family_id >= AMDGPU_FAMILY_RV )
        return false;

    // Minimum required gpu family
    if ( mGpuInfo.family_id < AMDGPU_FAMILY_CI )
        return false;

    err = amdgpu_query_firmware_version( mDevice, AMDGPU_INFO_FW_VCE, 0,
                                         0, &fwVersion, &fwFeature);
    FailOnTo( err, error, "Failed to query firmware version\n" );

    // These chips have a fw requirement for MV support
    // TODO: this needs a cleanup so we can switch-case
    if (chipId == (mGpuInfo.chip_rev + 0x3C) || /* FIJI */
        chipId == (mGpuInfo.chip_rev + 0x50) || /* Polaris 10*/
        chipId == (mGpuInfo.chip_rev + 0x5A) || /* Polaris 11*/
        chipId == (mGpuInfo.chip_rev + 0x64))   /* Polaris 12*/
    {
        if ( fwVersion >= FW_53_0_03 )
            return true;
    }

    return true;

error:
    // Assume we don't support anything else
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::AllocateResource( VcetBo*& bo, uint64_t size, bool mappable )
{
    bool ret;

    bo = new VcetBo( this );
    FailOnTo( !bo, error, "Failed to create bo\n" );

    ret = bo->Allocate( size, true );
    FailOnTo( !ret, error, "Failed to allocate fb bo\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int VcetContext::AllocateResources()
{
    bool ret;

    ret = AllocateResource( mBoFb, GetFbSize(), true );
    FailOnTo( !ret, error, "Failed to allocate fb bo\n" );

    ret = AllocateResource( mBoBs, GetBsSize(), true );
    FailOnTo( !ret, error, "Failed to allocate bs bo\n" );

    ret = AllocateResource( mBoCpb, GetCpbSize(), false );
    FailOnTo( !ret, error, "Failed to allocate cpb bo\n" );

    for ( int i = 0; i < kNumIbs; ++i ) {
        mIbs[i] = new VcetIb( this );
        FailOnTo( !mIbs[i], error, "Failed to create ib\n" );

        ret = mIbs[i]->Init();
        FailOnTo( !ret, error, "Failed to init IB\n" );
    }

    return 0;

error:
    return -1;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int VcetContext::CreateSession()
{
    bool ret;
    VcetIb *ib = nullptr;

    ib = GetNextIb();
    FailOnTo( !ib, error, "Invalid ib\n" );

    ret = ib->WriteCreateSession( mAlignedWidth, mAlignedHeight );
    FailOnTo( !ret, error, "Failed to prepare create session ib\n" );

    ret = Submit( ib );
    FailOnTo( !ret, error, "Failed to submit create session ib\n" );

    return 0;

error:
    return -1;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
int VcetContext::DestroySession()
{
    bool ret;
    VcetIb *ib = nullptr;

    ib = GetNextIb();
    FailOnTo( !ib, error, "Invalid ib\n" );

    ret = ib->WriteoDestroySession();
    FailOnTo( !ret, error, "Failed to prepare destroy session ib\n" );

    ret = Submit( ib );
    FailOnTo( !ret, error, "Failed to submit destroy session ib\n" );

    return 0;

error:
    return -1;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint64_t VcetContext::GetFbSize()
{
    // TODO: might need per-family values
    return 4096;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint64_t VcetContext::GetBsSize()
{
    // TODO: need per-family values
    return 0x154000;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint64_t VcetContext::GetCpbSize()
{
    return mAlignedWidth * mAlignedHeight * VcetBo::kNv21Bpp * kNumCpbBuffers;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetContext::GetIpType()
{
    return AMDGPU_HW_IP_VCE;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
uint32_t VcetContext::GetFamilyId()
{
    return mGpuInfo.family_id;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::CalculateMv( VcetBo *oldFrame, VcetBo *newFrame, VcetBo *mvBo, uint32_t width, uint32_t height )
{
    bool ret;
    VcetIb *ib = nullptr;

    FailOnTo( !oldFrame || !newFrame || !mvBo, error, "Bad bo\n" );
    FailOnTo( width != mWidth || height != mHeight, error, "Invalid frame dimensions\n" );

    ib = GetNextIb();
    FailOnTo( !ib, error, "Invalid ib\n" );

    ret = ib->WriteCalculateMv( oldFrame, newFrame, mvBo, width, height );
    FailOnTo( !ret, error, "Failed to prepare mv dump ib\n" );

    ret = Submit( ib );
    FailOnTo( !ret, error, "Failed to submit ib\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetIb *VcetContext::GetNextIb()
{
    bool ret;
    VcetIb *ib;

    mIbIdx = ( mIbIdx + 1 ) % kNumIbs;
    ib = mIbs[ mIbIdx ];
    FailOnTo( !ib, error, "Invalid ib\n" );

    ret = ib->Reset();
    FailOnTo( !ret, error, "Failed to reset ib\n" );

    return ib;

error:
    return nullptr;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetContext::Submit( VcetIb *ib )
{
    int err;
    struct amdgpu_cs_request ibsRequest = {0};
    struct amdgpu_cs_ib_info ibInfo = {0};
    amdgpu_bo_list_handle boList = nullptr;

    ibInfo.ib_mc_address = ib->GetGpuAddress();
    ibInfo.size = ib->GetSizeDw();

    err = amdgpu_bo_list_create( mDevice, ib->GetNumResources(),
                                 ib->GetResources(), nullptr,
                                 &boList);
    FailOnTo( err, error, "Failed to create bo list\n" );

    ibsRequest.ip_type = GetIpType();
    ibsRequest.number_of_ibs = 1;
    ibsRequest.ibs = &ibInfo;
    ibsRequest.fence_info.handle = nullptr;
    ibsRequest.resources = boList;

    err = amdgpu_cs_submit( mDeviceContext, 0, &ibsRequest, 1);
    FailOnTo( err, error, "Failed to submit ib\n" );

    ib->SetSeqNo( ibsRequest.seq_no );

    err = amdgpu_bo_list_destroy( ibsRequest.resources );
    boList = nullptr;
    WarnOn( err,  "Failed to destroy bo list\n" );

    if ( kForceSubmitSync ) {
        bool ret = ib->WaitFromCompletion();
        FailOnTo( !ret, error, "Failed to wait for ib completion\n" );
    }

    return true;

error:
    if ( boList )
        amdgpu_bo_list_destroy( boList );

    return false;
}
