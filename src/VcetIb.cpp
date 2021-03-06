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



#include <util/util.h>

#include "Drm.h"
#include "VcetContext.h"
#include "VcetBo.h"

#include "VcetIb.h"

#define UPPER32( val ) ( (val >> 32) & 0xfffffff )
#define LOWER32( val ) ( val & 0xffffffff )

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetIb::VcetIb( VcetContext *pContext )
    : mContext( pContext )
    , mBo( pContext )
    , mIbData( nullptr )
    , mSeqNo( 0 )
    , mSizeDw( 0 )
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
VcetIb::~VcetIb()
{
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::Init()
{
    bool ret;

    ret = mBo.Allocate( kSizeBytes, true, kAlignment );
    FailOnTo( !ret, error, "Failed to allocate IB bo\n" );

    ret = mBo.Map();
    FailOnTo( !ret, error, "Failed to map IB bo\n" );

    mIbData = (uint32_t*) mBo.GetCpuAddr();
    FailOnTo( !mIbData, error, "Failed to get IB cpu address\n" );

    return true;

error:
    // TODO de-allocate intermediate stuff
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WaitFromCompletion( uint64_t timeout )
{
    int err;
    uint32_t expired;
    struct amdgpu_cs_fence fenceStatus = {0};

    fenceStatus.context = mContext->GetDrm()->GetContext();
    fenceStatus.ip_type = mContext->GetIpType();
    fenceStatus.fence = mSeqNo;

    err = mContext->GetDrm()->CsQueryFenceStatus( &fenceStatus, timeout, 0, &expired);
    FailOnTo( err, error, "Failed to wait for ib completion\n" );

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::Reset()
{
    mSizeDw = 0;
    mSeqNo = 0;
    mReferencedResources.clear();

    if ( kClearOnReset ) {
        memset( mIbData, 0, kSizeBytes );
    }

    RefResource( &mBo );

    return true;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::Write( uint32_t cmd )
{
    mIbData[mSizeDw] = cmd;
    mSizeDw++;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::Write( uint32_t *pCmd, uint32_t count )
{
    memcpy( &mIbData[mSizeDw], pCmd, sizeof(uint32_t) * count );
    mSizeDw += count;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WriteNop( uint32_t count )
{
    for ( unsigned i = 0; i < count; ++i ) {
        Write( kNopCmd );
    }

    return true;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WriteCreateSession( uint32_t width, uint32_t height )
{
    FailOnTo( !VcetBo::IsWidthAligned( mContext, width ), error, "unaligned width\n" );
    FailOnTo( !VcetBo::IsHeightAligned( mContext, height ), error, "unaligned height\n" );

    WriteSession();
    WriteTaskInfo( 0 );
    WriteCreate( width, height );
    WriteFeedbackBuffer();

    return true;

error:
    return false;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WriteoDestroySession()
{
    WriteSession();
    WriteTaskInfo( 1 );
    WriteFeedbackBuffer();
    WriteDestroy();

    return true;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool VcetIb::WriteCalculateMv( VcetBo *oldFrame, VcetBo *newFrame, VcetBo *mvBo, uint32_t width, uint32_t height )
{
    RefResource( oldFrame );
    RefResource( newFrame );
    RefResource( mvBo );

    WriteConfigInit();

    WriteSession();
    WriteTaskInfo( 3 );
    WriteBsBuffer();
    WriteContextBuffer();
    WriteAuxBuffer( oldFrame->GetSizeBytes() );
    WriteFeedbackBuffer();

    WriteMvCmd( oldFrame, mvBo, width, height );
    WriteEncodeCmd( newFrame, width, height );
    
    return true;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::RefResource( VcetBo *bo )
{
    mReferencedResources.push_back( bo->GetBoHandle() );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteConfigInit()
{
    WriteSession();
    WriteTaskInfo( 2 );
    WriteVceConfig();
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteCreate( uint32_t width, uint32_t height )
{
    Write( 0x00000030 );    // 00
    Write( 0x01000001 );    // 01
    Write( 0x00000000 );    // 02
    Write( 0x00000042 );    // 03
    Write( 0x0000002a );    // 04
    Write( 0x00000000 );    // 05
    Write( width );         // 06 enc_image_width
    Write( height );        // 07 enc_image_height
    Write( width );         // 08 enc_ref_pic_luma_pitch
    Write( width );         // 09 enc_ref_pic_chroma_pitch
    Write( height / 8 );    // 10 enc_ref_y_height_in_qw

    if ( mContext->GetFamilyId() >= AMDGPU_FAMILY_AI)
        Write( 0x01000001 ); // disableTwoInstance
    else
        Write( 0x01000201 ); // disableTwoInstance
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteDestroy()
{
    Write( 0x00000008 );
    Write( 0x02000001 );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteSession()
{
    Write( 0x0000000c );
    Write( 0x00000001 );
    Write( mContext->GetSessionId() );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteTaskInfo( uint32_t id )
{
    Write( 0x00000020 );
    Write( 0x00000002 );
    Write( 0xffffffff );
    Write( id );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( id == 2 ? 0xffffffff : 0x0 );
    Write( 0x00000000 );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteVceConfig()
{
    // Rate Control
    Write( 0x00000070 );
    Write( 0x04000005 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x0000001c );
    Write( 0x0000001c );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000033 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );

    // Config Ext
    Write( 0x0000000c );
    Write( 0x04000001 );
    Write( 0x00000003 );

    //Motion Est
    Write( 0x00000068 );
    Write( 0x04000007 );
    Write( 0x00000001 );
    Write( 0x00000001 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000010 );
    Write( 0x00000010 );
    Write( 0x00000010 );
    Write( 0x00000010 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x000000fe );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000001 );
    Write( 0x00000001 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );

    // RDO
    Write( 0x0000004c );
    Write( 0x04000008 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );

    // PIC Control
    Write( 0x00000074 );
    Write( 0x04000002 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000aa0 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000040 );
    Write( 0x00000000 );
    Write( 0x00000000 );
    Write( 0x00000001 );
    Write( 0x00000002 );
    Write( 0x00000001 );
    Write( 0x00000001 );
    Write( 0x00000001 );    // encSliceMode
    Write( 0x00000000 );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteBsBuffer()
{
    uint64_t bsAddr = mContext->GetBs()->GetGpuAddr();

    Write( 0x00000014 );
    Write( 0x05000004 );
    Write( UPPER32( bsAddr ) );
    Write( LOWER32( bsAddr ) );
    Write( 0x00154000 );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteFeedbackBuffer()
{
    uint64_t fbAddr = mContext->GetFb()->GetGpuAddr();

    Write( 0x00000014 );
    Write( 0x05000005 );
    Write( UPPER32( fbAddr ) );
    Write( LOWER32( fbAddr ) );
    Write( 0x00000001 );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteContextBuffer()
{
    uint64_t cpbAddr = mContext->GetCpb()->GetGpuAddr();

    Write( 0x00000010 );
    Write( 0x05000001 );
    Write( UPPER32( cpbAddr ) );
    Write( LOWER32( cpbAddr ) );
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteAuxBuffer( uint64_t frameSizeBytes )
{
    Write( 0x00000048 );
    Write( 0x05000002 );

    // Offsets into cpb?
    for ( int i = 0; i < 8; ++i ) {
        Write( frameSizeBytes * ( i + 2 ) );
    }

    for ( int i = 0; i < 8; ++i ) {
        Write( frameSizeBytes );
    }
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteMvCmd( VcetBo *refFrame, VcetBo *mvBo, uint32_t width, uint32_t height )
{
    uint64_t refAddr = refFrame->GetGpuAddr();
    uint64_t mvAddr = mvBo->GetGpuAddr();

    Write( 0x00000038 );            //
    Write( 0x0500000d );            //
    Write( UPPER32( refAddr ) );    //
    Write( LOWER32( refAddr ) );    //
    Write( width );                 // luma pitch
    Write( width );                 // chroma pitch
    Write( width * height );        // chroma offset from refAddr
    Write( UPPER32( mvAddr ) );     //
    Write( LOWER32( mvAddr ) );     //
    Write( 0x00000000 );            //
    Write( 0x00000000 );            //
    Write( 0x00000000 );            //
    Write( 0x00000000 );            //
    Write( 0x00000000 );            //
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void VcetIb::WriteEncodeCmd( VcetBo *frame, uint32_t width, uint32_t height )
{
    uint64_t frameAddr = frame->GetGpuAddr();
    uint64_t frameChromaAddr = frameAddr + ( width * height  * 1 );

    Write( 0x00000160 );    // 00
    Write( 0x03000001 );    // 01
    Write( 0x00000000 );    // 02
    Write( 0x00000000 );    // 03
    Write( 0x00154000 );    // 04
    Write( 0x00000000 );    // 05
    Write( 0x00000000 );    // 06
    Write( 0x00000000 );    // 07
    Write( 0x00000000 );    // 08
    Write( UPPER32( frameAddr ) );    // 09
    Write( LOWER32( frameAddr ) );    // 10
    Write( UPPER32( frameChromaAddr ) );    // 11
    Write( LOWER32( frameChromaAddr ) );    // 12
    Write( height );        // 13 enc_input_frame_y_pitch
    Write( width );         // 14 enc_input_pic_luma_pitch
    Write( width );         // 15 enc_input_pic_chroma_pitch
    /* encDisableMBOffloading-encDisableTwoPipeMode-encInputPicArrayMode-encInputPicAddrMode */
    Write( 0x01010000 );    // 16
    Write( 0x00000000 );    // 17
    Write( 0x00000000 );    // 18 encPicType
    Write( 0x00000000 );    // 19 encIdrFlag
    Write( 0x00000000 );    // 20 encIdrPicId
    Write( 0x00000000 );    // 21 encMgsKeyPic
    Write( 0x00000000 );    // 22 encReferenceFlag
    Write( 0x00000000 );    // 23 encTemporalLayerIndex
    Write( 0x00000000 );    // 24
    Write( 0x00000000 );    // 25
    Write( 0x00000000 );    // 26
    Write( 0x00000000 );    // 27
    Write( 0x00000000 );    // 28
    Write( 0x00000000 );    // 29
    Write( 0x00000000 );    // 30
    Write( 0x00000000 );    // 31
    Write( 0x00000000 );    // 32
    Write( 0x00000000 );    // 33
    Write( 0x00000000 );    // 34
    Write( 0x00000000 );    // 35
    Write( 0x00000000 );    // 36
    Write( 0x00000000 );    // 37
    Write( 0x00000000 );    // 38
    Write( 0x00000000 );    // 39
    Write( 0x00000000 );    // 40
    Write( 0x00000000 );    // 41
    Write( 0x00000000 );    // 42
    Write( 0x00000000 );    // 43
    Write( 0x00000000 );    // 44
    Write( 0x00000000 );    // 45
    Write( 0x00000000 );    // 46
    Write( 0x00000000 );    // 47
    Write( 0x00000000 );    // 48
    Write( 0x00000000 );    // 49
    Write( 0x00000000 );    // 50
    Write( 0x00000000 );    // 51
    Write( 0x00000000 );    // 52
    Write( 0x00000000 );    // 53
    Write( 0x00000000 );    // 54
    Write( 0x00000000 );    // 55 pictureStructure
    Write( 0x00000000 );    // 56 encPicType -ref[0]
    Write( 0x00000000 );    // 57
    Write( 0x00000000 );    // 58
    Write( 0xffffffff );    // 59
    Write( 0xffffffff );    // 60
    Write( 0x00000000 );    // 61 pictureStructure
    Write( 0x00000000 );    // 62 encPicType -ref[1]
    Write( 0x00000000 );    // 63
    Write( 0x00000000 );    // 64
    Write( 0xffffffff );    // 65
    Write( 0xffffffff );    // 66
    Write( 0x00000000 );    // 67 pictureStructure
    Write( 0x00000000 );    // 68 encPicType -ref1
    Write( 0x00000000 );    // 69
    Write( 0x00000000 );    // 70
    Write( 0xffffffff );    // 71
    Write( 0xffffffff );    // 72
    Write( 0xffffffff );    // 73
    Write( 0xffffffff );    // 74
    Write( 0x00000000 );    // 75
    Write( 0x00000000 );    // 76
    Write( 0x00000000 );    // 77
    Write( 0x00000000 );    // 78
    Write( 0x00000000 );    // 79
    Write( 0x00000000 );    // 80
    Write( 0x00000001 );    // 81 frameNumber
    Write( 0x00000002 );    // 82 pictureOrderCount
    Write( 0xffffffff );    // 83 numIPicRemainInRCGOP
    Write( 0xffffffff );    // 84 numPPicRemainInRCGOP
    Write( 0xffffffff );    // 85 numBPicRemainInRCGOP
    Write( 0xffffffff );    // 86 numIRPicRemainInRCGOP
    Write( 0x00000000 );    // 87 remainedIntraRefreshPictures
}
