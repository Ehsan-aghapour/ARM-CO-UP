/*
 * Copyright (c) 2016-2019 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//Ehsan
#ifndef My_print
#include "arm_compute/gl_vs.h"
#endif

#include "arm_compute/runtime/CL/CLTensor.h"

#include "arm_compute/runtime/CL/CLRuntimeContext.h"
#include "arm_compute/runtime/CL/CLScheduler.h"

namespace arm_compute
{
CLTensor::CLTensor(IRuntimeContext *ctx)
    : _allocator(this, static_cast<CLRuntimeContext *>(ctx)), _ctx(static_cast<CLRuntimeContext *>(ctx))
{
}

CLRuntimeContext *CLTensor::context()
{
    return _ctx;
}

TensorInfo *CLTensor::info() const
{
    return &_allocator.info();
}

TensorInfo *CLTensor::info()
{
    return &_allocator.info();
}

const cl::Buffer &CLTensor::cl_buffer() const
{
    return _allocator.cl_data();
}

CLQuantization CLTensor::quantization() const
{
    return _allocator.quantization();
}

CLTensorAllocator *CLTensor::allocator()
{
    return &_allocator;
}

void CLTensor::map(bool blocking)
{
#if My_print > 0
	//Ehsan: Observation: _ctx is null at setup and runtime several times called
	std::cerr<<"contex is null? "<<(_ctx==nullptr)<<std::endl;
#endif
    ICLTensor::map(_ctx == nullptr ? CLScheduler::get().queue() : _ctx->gpu_scheduler()->queue(), blocking);
}

void CLTensor::unmap()
{
    ICLTensor::unmap(_ctx == nullptr ? CLScheduler::get().queue() : _ctx->gpu_scheduler()->queue());
}

uint8_t *CLTensor::do_map(cl::CommandQueue &q, bool blocking)
{
    return _allocator.map(q, blocking);
}

void CLTensor::do_unmap(cl::CommandQueue &q)
{
    _allocator.unmap(q, buffer());
}

void CLTensor::associate_memory_group(arm_compute::IMemoryGroup *memory_group)
{
    _allocator.set_associated_memory_group(memory_group);
}
} // namespace arm_compute
