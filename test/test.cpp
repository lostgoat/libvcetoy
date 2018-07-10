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

#include <vcetoy/vcetoy.h>
#include <minivk/MiniVk.h>

class VcetTest : public ::testing::Test {
    protected:
        static const int kFrameMax = 4;

		virtual void SetUp()
		{
            mCtx = nullptr;
            mMappableBo = nullptr;
            mUnmappableBo = nullptr;
            memset( mFrame, 0, sizeof(mFrame) );

			ASSERT_TRUE( VcetContextCreate( &mCtx ) );
            ASSERT_NE( mCtx, nullptr );

            ASSERT_TRUE( VcetBoCreate( mCtx, 1024, true, &mMappableBo ) );
            ASSERT_TRUE( VcetBoCreate( mCtx, 1024, false, &mUnmappableBo ) );
            ASSERT_NE( mMappableBo, nullptr );
            ASSERT_NE( mUnmappableBo, nullptr );

            for ( int i = 0; i < kFrameMax; ++i )
            {
                LoadFrame( i, nullptr, 4096 );
            }
		}

		virtual void TearDown()
		{
            VcetBoDestroy( &mMappableBo );
            ASSERT_EQ( mMappableBo, nullptr );

            VcetBoDestroy( &mUnmappableBo );
            ASSERT_EQ( mUnmappableBo, nullptr );

            VcetContextDestroy( &mCtx );
            ASSERT_EQ( mCtx, nullptr );
		}

        void LoadFrame( int slot, uint8_t *data, uint64_t sizeBytes )
        {
            ASSERT_TRUE( VcetBoCreate( mCtx, sizeBytes, true, &mFrame[slot] ) );
        }

        VcetCtxHandle mCtx;
        VcetBoHandle mFrame[ kFrameMax ];
        VcetBoHandle mMappableBo;
        VcetBoHandle mUnmappableBo;

};

class VcetTestParams : public VcetTest,
    public ::testing::WithParamInterface<std::tuple<
                      bool, uint64_t, bool, bool, bool
                      >>
{};

TEST_F(VcetTest, Sanity)
{
}

TEST_F( VcetTest, CtxCreateBadParam )
{
    ASSERT_FALSE( VcetContextCreate( nullptr ) );
}

TEST_F( VcetTest, CtxDestroyBadParam )
{
    VcetContextDestroy( nullptr );
}

TEST_F( VcetTest, CtxDestroyDoubleFree )
{
    VcetContextDestroy( &mCtx );
}

TEST_F( VcetTest, BoUnmapBadParam )
{
    ASSERT_FALSE( VcetBoUnmap( nullptr ) );
    ASSERT_FALSE( VcetBoUnmap( mMappableBo ) );
    ASSERT_FALSE( VcetBoUnmap( mUnmappableBo ) );
}

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
