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

#include <gtest/gtest.h>

#include <util/util.h>
#include <vcetoy/vcetoy.h>
#include <minivk/MiniVk.h>

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

static void DumpDataToFile( uint8_t *pData, uint64_t size, const char *prefix, uint32_t width = 0, uint32_t height = 0 )
{
    static int id = 0;
    char path[255];

    if ( width && height )
        sprintf( path, "%s_%03d-%d-%d.dump", prefix, id++, width, height );
    else
        sprintf( path, "%s_%03d.dump", prefix, id++ );

    FILE *file = fopen( path, "wb+");
    fwrite( pData, size, sizeof(uint8_t), file );
}

class VcetTest : public ::testing::Test
{
    protected:
        virtual void SetUp()
        {
            mCtx = nullptr;
            mMappableBo = nullptr;
            mUnmappableBo = nullptr;
            mTinyImage = nullptr;

            ASSERT_TRUE( VcetContextCreate( &mCtx, GetWidth(), GetHeight() ) );
            ASSERT_NE( mCtx, nullptr );

            mBoSize = MAX_WIDTH * MAX_HEIGHT * 1.5;
            ASSERT_TRUE( VcetBoCreate( mCtx, mBoSize, true, &mMappableBo ) );
            ASSERT_TRUE( VcetBoCreate( mCtx, mBoSize, false, &mUnmappableBo ) );
            ASSERT_TRUE( VcetBoAlignDimensions( mCtx, 1, 1, &mWidthAlignment, &mHeightAlignment ) );
            ASSERT_NE( mMappableBo, nullptr );
            ASSERT_NE( mUnmappableBo, nullptr );
        }

        virtual void TearDown()
        {
            VcetBoDestroy( &mTinyImage );
            ASSERT_EQ( mTinyImage, nullptr );

            VcetBoDestroy( &mMappableBo );
            ASSERT_EQ( mMappableBo, nullptr );

            VcetBoDestroy( &mUnmappableBo );
            ASSERT_EQ( mUnmappableBo, nullptr );

            VcetContextDestroy( &mCtx );
            ASSERT_EQ( mCtx, nullptr );
        }

        virtual uint32_t GetWidth()
        {
            return MAX_WIDTH;
        }

        virtual uint32_t GetHeight()
        {
            return MAX_HEIGHT;
        }

        VcetCtxHandle mCtx;
        VcetBoHandle mMappableBo;
        VcetBoHandle mUnmappableBo;
        VcetBoHandle mTinyImage;

        uint32_t mBoSize;
        uint32_t mWidthAlignment;
        uint32_t mHeightAlignment;
};

TEST_F(VcetTest, Sanity)
{
}

TEST_F( VcetTest, CtxCreateBadParam )
{
    VcetCtxHandle ctx;

    ASSERT_FALSE( VcetContextCreate( nullptr, MAX_WIDTH, MAX_HEIGHT ) );
    ASSERT_FALSE( VcetContextCreate( &ctx, 0, MAX_HEIGHT ) );
    ASSERT_FALSE( VcetContextCreate( &ctx, MAX_WIDTH, 0 ) );
}

TEST_F( VcetTest, CtxDestroyBadParam )
{
    VcetContextDestroy( nullptr );
}

TEST_F( VcetTest, CtxDestroyDoubleFree )
{
    VcetCtxHandle ctx;
    ASSERT_TRUE( VcetContextCreate( &ctx, MAX_WIDTH, MAX_HEIGHT ) );
    VcetContextDestroy( &ctx );
    ASSERT_EQ( nullptr, ctx );
    VcetContextDestroy( &ctx );
    ASSERT_EQ( nullptr, ctx );
}

TEST_F( VcetTest, BoUnmapBadParam )
{
    ASSERT_FALSE( VcetBoUnmap( nullptr ) );
    ASSERT_FALSE( VcetBoUnmap( mMappableBo ) );
    ASSERT_FALSE( VcetBoUnmap( mUnmappableBo ) );
}

class VcetTestFrames : public VcetTest
{
    protected:
        static const int kFrameMax = 5;

        class Frame {
        public:
            uint32_t mWidth;
            uint32_t mHeight;
            uint32_t mAlignedWidth;
            uint32_t mAlignedHeight;
            uint32_t mWidthAlignment;
            uint32_t mHeightAlignment;
            uint64_t mSize;
            VcetBoHandle mBo;
            uint8_t *mBoData;

            void DumpToFile()
            {
                DumpDataToFile( mBoData, mSize, "frame", mAlignedWidth, mAlignedHeight );
            }

            void FromBitmap( VcetCtxHandle ctx, const char *path )
            {
                uint8_t *pFrameData = util::GetNV21Data( path, mWidthAlignment, mHeightAlignment, &mWidth, &mHeight );

                ASSERT_NE( (uint32_t)0, mWidth );
                ASSERT_NE( (uint32_t)0, mHeight );
                ASSERT_NE( nullptr, pFrameData );

                ASSERT_TRUE( VcetBoCreateImage( ctx, mWidth, mHeight, true, &mBo, &mAlignedWidth, &mAlignedHeight ) );
                ASSERT_NE( nullptr, mBo );

                ASSERT_TRUE( VcetBoMap( mBo, &mBoData ) );
                ASSERT_NE( nullptr, mBoData );

                mSize = mAlignedWidth * mAlignedHeight * 1.5;
                memcpy( mBoData, pFrameData, mSize );

                DumpToFile();

                free( pFrameData );
            }

            Frame( uint32_t alignWidth, uint32_t alignHeight )
                : mWidth( 0 )
                , mHeight( 0 )
                , mAlignedWidth( 0 )
                , mAlignedHeight( 0 )
                , mWidthAlignment( alignWidth )
                , mHeightAlignment( alignHeight )
                , mSize( 0 )
                , mBo( nullptr )
                , mBoData( nullptr )
            {}

            ~Frame()
            {
                VcetBoDestroy( &mBo );
            }

        };

        virtual void SetUp()
        {
            GetDimensionData( "test/frames/001.bmp" );
            VcetTest::SetUp();

            for ( int i = 0; i < kFrameMax; ++i ) {
                mFrame[i] = new Frame( mWidthAlignment, mHeightAlignment );
            }

            mFrame[0]->FromBitmap( mCtx, "test/frames/001.bmp" );
            mFrame[1]->FromBitmap( mCtx, "test/frames/002.bmp" );
            mFrame[2]->FromBitmap( mCtx, "test/frames/003.bmp" );
            mFrame[3]->FromBitmap( mCtx, "test/frames/001.bmp" );
            mFrame[4]->FromBitmap( mCtx, "test/frames/pattern.bmp" );
        }

        virtual void TearDown()
        {
            for ( int i = 0; i < kFrameMax; ++i ) {
                delete mFrame[i];
            }

            VcetTest::TearDown();
        }

        virtual uint32_t GetWidth()
        {
            return mWidth;
        }

        virtual uint32_t GetHeight()
        {
            return mHeight;
        }

        void GetDimensionData( const char *path )
        {
            uint32_t stride;
            uint8_t *data = util::GetBmpData( path, &mWidth, &mHeight, &stride );
            delete data;
        }

        Frame* mFrame[ kFrameMax ];
        uint32_t mWidth, mHeight;
};

TEST_F(VcetTestFrames, Sanity)
{
}

TEST_F(VcetTestFrames, CalculateMv )
{
    uint8_t *mvData = nullptr;

    ASSERT_EQ( true, VcetBoMap( mMappableBo, &mvData ) );
    memset( mvData, 0, mBoSize );

    ASSERT_TRUE( VcetCalculateMv( mCtx, mFrame[0]->mBo, mFrame[1]->mBo,
                                  mMappableBo,
                                  mFrame[0]->mWidth, mFrame[0]->mHeight ));
    DumpDataToFile( mvData, mBoSize, "mv01", MAX_WIDTH, MAX_HEIGHT );

    uint64_t sum = 0;
    for ( uint32_t i = 0; i < mBoSize; ++i ) {
        sum += mvData[i];
    }

    ASSERT_NE( 0u, sum );
}

TEST_F(VcetTestFrames, CalculateMvNoMovement )
{
    uint8_t *mvData = nullptr;

    ASSERT_EQ( true, VcetBoMap( mMappableBo, &mvData ) );
    memset( mvData, 0, mBoSize );

    // Two equal frames should produce a zero motion vector
    ASSERT_TRUE( VcetCalculateMv( mCtx, mFrame[0]->mBo, mFrame[3]->mBo,
                                  mMappableBo,
                                  mFrame[0]->mWidth, mFrame[0]->mHeight ));

    uint64_t sum = 0;
    for ( uint32_t i = 0; i < mBoSize; ++i ) {
        sum += mvData[i];
    }

    ASSERT_EQ( 0u, sum );
}

TEST_F(VcetTestFrames, MultipleSubmissions )
{
    for ( int i = 0; i < 20; i++ ) {
        ASSERT_TRUE( VcetCalculateMv( mCtx, mFrame[0]->mBo, mFrame[1]->mBo,
                                      mMappableBo,
                                      mFrame[0]->mWidth, mFrame[0]->mHeight ));
    }
}

class VcetTestParams : public VcetTest,
    public ::testing::WithParamInterface<std::tuple<
                      bool, uint64_t, bool, bool, bool
                      >>
{};

TEST_P( VcetTestParams, MemoryAllocAndMap )
{
    bool goodContext = std::tr1::get<0>(GetParam());
    uint64_t sizeBytes = std::tr1::get<1>(GetParam());
    bool mappable = std::tr1::get<2>(GetParam());
    bool goodBo = std::tr1::get<3>(GetParam());
    bool goodData = std::tr1::get<4>(GetParam());

    VcetCtxHandle ctx =  goodContext ? mCtx : nullptr;

    VcetBoHandle bo = nullptr;
    VcetBoHandle *pBo = goodBo ? &bo : nullptr;

    uint8_t *pData = nullptr;
    uint8_t **ppData = goodData ? &pData : nullptr;

    bool ExpectGoodAlloc =
        goodContext
        && sizeBytes > 0
        && goodBo;

    bool ExpectGoodMap =
        ExpectGoodAlloc
        && mappable
        && goodData;

    ASSERT_EQ( ExpectGoodAlloc, VcetBoCreate( ctx, sizeBytes, mappable, pBo ) );
    ASSERT_EQ( ExpectGoodMap, VcetBoMap( bo, ppData ) );
    ASSERT_EQ( ExpectGoodMap, pData != nullptr );
    ASSERT_EQ( ExpectGoodMap, VcetBoUnmap( bo ) );
    VcetBoDestroy( pBo );
}

INSTANTIATE_TEST_CASE_P( MemoryAllocAndMap, VcetTestParams, testing::Combine(
    testing::Values( true, false ),
    testing::Values( 0, 1, 1024, 4096 ),
    testing::Values( true, false ),
    testing::Values( true, false ),
    testing::Values( true, false )
));

class VulkanTest : public ::testing::Test {
        protected:
                virtual void SetUp()
                {
                        ASSERT_EQ( mMiniVk.Init(), 0 );
                }

                virtual void TearDown()
                {
                }

                MiniVk mMiniVk;
};

TEST_F(VulkanTest, Sanity)
{
}

TEST_F(VulkanTest, CreateImageVram)
{
    MiniVk::MiniVkImage *pImage;
    ASSERT_TRUE( mMiniVk.CreateImage( 100, 100, VK_FORMAT_B8G8R8A8_SRGB, false, &pImage ) );
    ASSERT_NE( nullptr, pImage );
    ASSERT_NE( nullptr, pImage->pImage );
    ASSERT_NE( nullptr, pImage->pMemory );
    ASSERT_GT( pImage->fd, -1 );

    mMiniVk.FreeImage( &pImage );
    ASSERT_EQ( nullptr, pImage );
}

TEST_F(VulkanTest, CreateImageMappable)
{
    MiniVk::MiniVkImage *pImage;
    ASSERT_TRUE( mMiniVk.CreateImage( 100, 100, VK_FORMAT_B8G8R8A8_SRGB, true, &pImage ) );
    ASSERT_NE( nullptr, pImage );
    ASSERT_NE( nullptr, pImage->pImage );
    ASSERT_NE( nullptr, pImage->pMemory );
    ASSERT_GT( pImage->fd, -1 );

    uint8_t *pData = mMiniVk.MapImage( pImage );
    ASSERT_NE( nullptr, pData );
    *pData = 1;

    mMiniVk.FreeImage( &pImage );
    ASSERT_EQ( nullptr, pImage );
}

TEST_F(VulkanTest, CreateBufferVram)
{
    MiniVk::MiniVkBuffer *pBuffer;
    ASSERT_TRUE( mMiniVk.CreateBuffer( 100, false, &pBuffer ) );
    ASSERT_NE( nullptr, pBuffer );
    ASSERT_NE( nullptr, pBuffer->pBuffer );
    ASSERT_NE( nullptr, pBuffer->pMemory );
    ASSERT_GT( pBuffer->fd, -1 );

    mMiniVk.FreeBuffer( &pBuffer );
    ASSERT_EQ( nullptr, pBuffer );
}

TEST_F(VulkanTest, CreateBufferMappable)
{
    size_t bufSize = 100;

    MiniVk::MiniVkBuffer *pBuffer;
    ASSERT_TRUE( mMiniVk.CreateBuffer( bufSize, true, &pBuffer ) );
    ASSERT_NE( nullptr, pBuffer );
    ASSERT_NE( nullptr, pBuffer->pBuffer );
    ASSERT_NE( nullptr, pBuffer->pMemory );
    ASSERT_GT( pBuffer->fd, -1 );

    uint8_t *pData = mMiniVk.MapBuffer( pBuffer );
    ASSERT_NE( nullptr, pData );
    memset( pData, 1, bufSize );
    *pData = 1;

    mMiniVk.FreeBuffer( &pBuffer );
    ASSERT_EQ( nullptr, pBuffer );
}

TEST_F(VulkanTest, MapUnmappableBuffer)
{
    MiniVk::MiniVkBuffer *pBuffer;
    ASSERT_TRUE( mMiniVk.CreateBuffer( 100, false, &pBuffer ) );
    ASSERT_NE( nullptr, pBuffer );
    ASSERT_NE( nullptr, pBuffer->pBuffer );
    ASSERT_NE( nullptr, pBuffer->pMemory );
    ASSERT_GT( pBuffer->fd, -1 );

    uint8_t *pData = mMiniVk.MapBuffer( pBuffer );
    ASSERT_EQ( nullptr, pData );

    mMiniVk.FreeBuffer( &pBuffer );
    ASSERT_EQ( nullptr, pBuffer );
}

TEST_F(VulkanTest, MapUnmappableImage)
{
    MiniVk::MiniVkImage *pImage;
    ASSERT_TRUE( mMiniVk.CreateImage( 100, 100, VK_FORMAT_B8G8R8A8_SRGB, false, &pImage ) );
    ASSERT_NE( nullptr, pImage );
    ASSERT_NE( nullptr, pImage->pImage );
    ASSERT_NE( nullptr, pImage->pMemory );
    ASSERT_GT( pImage->fd, -1 );

    uint8_t *pData = mMiniVk.MapImage( pImage );
    ASSERT_EQ( nullptr, pData );

    mMiniVk.FreeImage( &pImage );
    ASSERT_EQ( nullptr, pImage );
}

class InteropFrames : public ::testing::Test
{
    protected:
        static const int kFrameMax = 5;

        class Frame {
        public:
            uint32_t mWidth;
            uint32_t mHeight;
            uint32_t mAlignedWidth;
            uint32_t mAlignedHeight;
            uint32_t mWidthAlignment;
            uint32_t mHeightAlignment;
            uint64_t mSize;
            VcetBoHandle mBo;
            uint8_t *mBoData;
            MiniVk::MiniVkBuffer *mBuffer;
            uint8_t *mBufferData;

            void DumpToFile()
            {
                DumpDataToFile( mBoData, mSize, "interop", mAlignedWidth, mAlignedHeight );
            }

            void FromBitmap( VcetCtxHandle ctx, MiniVk *pVk, const char *path )
            {
                static const float kNv21Bpp = 1.5;

                uint8_t *pFrameData = util::GetNV21Data( path, mWidthAlignment, mHeightAlignment, &mWidth, &mHeight );
                ASSERT_NE( (uint32_t)0, mWidth );
                ASSERT_NE( (uint32_t)0, mHeight );
                ASSERT_NE( nullptr, pFrameData );

                ASSERT_TRUE( VcetBoAlignDimensions( ctx, mWidth, mHeight, &mAlignedWidth, &mAlignedHeight ) );
                ASSERT_NE( (uint32_t)0, mAlignedWidth );
                ASSERT_NE( (uint32_t)0, mAlignedHeight );

                // Allocate in Vulkan
                uint32_t bufSize = mAlignedWidth * mAlignedHeight * kNv21Bpp;
                ASSERT_TRUE( pVk->CreateBuffer( bufSize, true, &mBuffer ) );
                ASSERT_NE( nullptr, mBuffer );

                mBufferData = pVk->MapBuffer( mBuffer );
                ASSERT_NE( nullptr, mBufferData );

                // Populate with data
                mSize = bufSize;
                memcpy( mBufferData, pFrameData, mSize );

                // Create a VcetBo alias
                ASSERT_TRUE( VcetBoImport( ctx, mBuffer->fd, true, &mBo ) );
                ASSERT_NE( nullptr, mBo );

                ASSERT_TRUE( VcetBoMap( mBo, &mBoData ) );
                ASSERT_NE( nullptr, mBoData );

                // Check pixel by pixel for precise error location
                for ( unsigned i = 0; i < mSize; i++ )
                {
                    ASSERT_EQ( mBoData[i], mBufferData[i] );
                }

                DumpToFile();

                free( pFrameData );
            }

            Frame( uint32_t alignWidth, uint32_t alignHeight )
                : mWidth( 0 )
                , mHeight( 0 )
                , mWidthAlignment( alignWidth )
                , mHeightAlignment( alignHeight )
                , mSize( 0 )
                , mBo( nullptr )
                , mBoData( nullptr )
            {}

            ~Frame()
            {
                VcetBoDestroy( &mBo );
            }

        };

        virtual void SetUp()
        {
            mCtx = nullptr;
            mMvBuffer = nullptr;
            mMvData = nullptr;
            mWidthAlignment = 0;
            mHeightAlignment = 0;
            mWidth = 0;
            mHeight = 0;
            mMvDataSize = MAX_WIDTH * MAX_HEIGHT * 1.5; // Fix this

            GetDimensionData( "test/frames/001.bmp" );

            ASSERT_TRUE( VcetContextCreate( &mCtx, GetWidth(), GetHeight() ) );
            ASSERT_NE( mCtx, nullptr );

            ASSERT_EQ( mMiniVk.Init(), 0 );

            ASSERT_TRUE( VcetBoAlignDimensions( mCtx, 1, 1, &mWidthAlignment, &mHeightAlignment ) );
            ASSERT_NE( 0u, mWidthAlignment );
            ASSERT_NE( 0u, mHeightAlignment );

            for ( int i = 0; i < kFrameMax; ++i ) {
                mFrame[i] = new Frame( mWidthAlignment, mHeightAlignment );
            }

            mFrame[0]->FromBitmap( mCtx, &mMiniVk, "test/frames/001.bmp" );
            mFrame[1]->FromBitmap( mCtx, &mMiniVk, "test/frames/002.bmp" );
            mFrame[2]->FromBitmap( mCtx, &mMiniVk, "test/frames/003.bmp" );
            mFrame[3]->FromBitmap( mCtx, &mMiniVk, "test/frames/001.bmp" );
            mFrame[4]->FromBitmap( mCtx, &mMiniVk, "test/frames/pattern.bmp" );

            ASSERT_TRUE( mMiniVk.CreateBuffer( mMvDataSize, true, &mMvBuffer ) );
            ASSERT_NE( nullptr, mMvBuffer );

            mMvData = mMiniVk.MapBuffer( mMvBuffer );
            ASSERT_NE( nullptr, mMvData );
            memset( mMvData, 0, mMvDataSize );

            ASSERT_TRUE( VcetBoImport( mCtx, mMvBuffer->fd, true, &mMvBo ) );
            ASSERT_NE( nullptr, mMvBo );

        }

        virtual void TearDown()
        {
            for ( int i = 0; i < kFrameMax; ++i ) {
                delete mFrame[i];
            }
        }

        virtual uint32_t GetWidth()
        {
            return mWidth;
        }

        virtual uint32_t GetHeight()
        {
            return mHeight;
        }

        void GetDimensionData( const char *path )
        {
            uint32_t stride;
            uint8_t *data = util::GetBmpData( path, &mWidth, &mHeight, &stride );
            delete data;
        }

        bool IsMovementDetected()
        {
            uint64_t sum = 0;
            for ( uint32_t i = 0; i < mMvDataSize; ++i ) {
                sum += mMvData[i];
            }

            return sum != 0;
        }

        MiniVk mMiniVk;
        VcetCtxHandle mCtx;

        uint32_t mWidth, mHeight;
        uint32_t mWidthAlignment;
        uint32_t mHeightAlignment;

        uint32_t mMvDataSize;
        MiniVk::MiniVkBuffer *mMvBuffer;
        uint8_t *mMvData;
        VcetBoHandle mMvBo;

        Frame* mFrame[ kFrameMax ];
};

TEST_F( InteropFrames, Sanity)
{
}

TEST_F( InteropFrames, CalculateMv )
{

    ASSERT_TRUE( VcetCalculateMv( mCtx, mFrame[0]->mBo, mFrame[1]->mBo, mMvBo,
                                  mFrame[0]->mWidth, mFrame[0]->mHeight ));
    ASSERT_EQ( true, IsMovementDetected() );

    DumpDataToFile( mMvData, mMvDataSize, "interop-mv01", MAX_WIDTH, MAX_HEIGHT );

}

TEST_F( InteropFrames, CalculateMvNoMovement )
{
    // Two equal frames should produce a zero motion vector
    ASSERT_TRUE( VcetCalculateMv( mCtx, mFrame[0]->mBo, mFrame[3]->mBo, mMvBo,
                                  mFrame[0]->mWidth, mFrame[0]->mHeight ));
    ASSERT_EQ( false, IsMovementDetected() );
}

TEST_F( InteropFrames, MultipleSubmissions )
{
    for ( int i = 0; i < 20; i++ ) {
        ASSERT_TRUE( VcetCalculateMv( mCtx, mFrame[0]->mBo, mFrame[1]->mBo, mMvBo,
                                      mFrame[0]->mWidth, mFrame[0]->mHeight ));
        ASSERT_EQ( true, IsMovementDetected() );
    }
}
