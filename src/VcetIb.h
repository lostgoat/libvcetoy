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

#include "VcetBo.h"

class VcetContext;

class VcetIb
{
    private:
        static const uint64_t kSize = 4096;
        static const uint64_t kAlignment = 4096;

    public:
        VcetIb( VcetContext *pContext );
        ~VcetIb();

        /**
         * Initialize
         */
        bool Init();

    private:
        VcetContext *mContext;
        VcetBo mBo;

        uint8_t *mIbData;
};
