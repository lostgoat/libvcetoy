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

class VcetTest : public ::testing::Test
{
    protected:
        virtual void SetUp()
        {
            mCtx = nullptr;
            mMappableBo = nullptr;
            mUnmappableBo = nullptr;

            ASSERT_TRUE( VcetContextCreate( &mCtx, MAX_WIDTH, MAX_HEIGHT ) );
            ASSERT_NE( mCtx, nullptr );

            ASSERT_TRUE( VcetBoCreate( mCtx, MAX_WIDTH * MAX_HEIGHT * 1.5 , true, &mMappableBo ) );
            ASSERT_TRUE( VcetBoCreate( mCtx, MAX_WIDTH * MAX_HEIGHT * 1.5, false, &mUnmappableBo ) );
            ASSERT_TRUE( VcetBoCreateImage( mCtx, 1, 1, true, &mTinyImage, &mWidthAlignment, &mHeightAlignment ) );
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

        VcetCtxHandle mCtx;
        VcetBoHandle mMappableBo;
        VcetBoHandle mUnmappableBo;
        VcetBoHandle mTinyImage;

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
                static int id = 0;
                char path[255];

                sprintf( path, "frame_%d-%d-%d.dump", id++, mAlignedWidth, mAlignedHeight );

                FILE *file = fopen( path, "wb+");
                fwrite( mBoData, mSize, sizeof(uint8_t), file );

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
            VcetTest::SetUp();

            for ( int i = 0; i < kFrameMax; ++i ) {
                mFrame[i] = new Frame( mWidthAlignment, mHeightAlignment );
            }

            mFrame[0]->FromBitmap( mCtx, "frames/001.bmp" );
            mFrame[1]->FromBitmap( mCtx, "frames/002.bmp" );
            mFrame[2]->FromBitmap( mCtx, "frames/003.bmp" );
            mFrame[3]->FromBitmap( mCtx, "frames/pattern.bmp" );
            mFrame[4]->FromBitmap( mCtx, "frames/pattern.bmp" );
        }

        virtual void TearDown()
        {
            for ( int i = 0; i < kFrameMax; ++i ) {
                delete mFrame[i];
            }

            VcetTest::TearDown();
        }

        Frame* mFrame[ kFrameMax ];
};

TEST_F(VcetTestFrames, Sanity)
{
}

TEST_F(VcetTestFrames, CalculateMv )
{
    ASSERT_TRUE( VcetCalculateMv( mCtx, mFrame[0]->mBo, mFrame[1]->mBo,
                                  mMappableBo,
                                  mFrame[0]->mWidth, mFrame[0]->mHeight ));
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
                        ASSERT_EQ( miniVk.Init(), 0 );
                }

                virtual void TearDown()
                {
                }

                MiniVk miniVk;
};

TEST_F(VulkanTest, Sanity)
{
}
