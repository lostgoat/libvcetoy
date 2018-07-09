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

class VcetContext
{
    public:
        VcetContext();
        ~VcetContext();

        bool Init();
        bool IsMvDumpSupported();

        amdgpu_device_handle GetDevice() { return mDevice; }

    private:
        int mDrmFd;
        amdgpu_device_handle mDevice;
        struct amdgpu_gpu_info mGpuInfo; 
};
