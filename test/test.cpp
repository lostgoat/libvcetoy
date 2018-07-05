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

#include <minivk/MiniVk.h>

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

TEST_F(VulkanTest, FixtureSetup)
{ 
}
