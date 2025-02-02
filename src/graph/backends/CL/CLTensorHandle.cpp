/*
 * Copyright (c) 2018-2019 Arm Limited.
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
#include "arm_compute/graph/backends/CL/CLTensorHandle.h"

#include "arm_compute/runtime/IMemoryGroup.h"

namespace arm_compute
{
namespace graph
{
namespace backends
{
CLTensorHandle::CLTensorHandle(const ITensorInfo &info)
    //: _tensor()
{
    //_tensor.allocator()->init(info);
    _tensor2=new arm_compute::CLTensor();
    _tensor2->allocator()->init(info);
}

void CLTensorHandle::allocate()
{
    //_tensor.allocator()->allocate();
    _tensor2->allocator()->allocate();
}

void CLTensorHandle::free()
{
    //_tensor.allocator()->free();
    _tensor2->allocator()->free();
}

void CLTensorHandle::manage(IMemoryGroup *mg)
{
    if(mg != nullptr)
    {
    	//mg->manage(&_tensor);
        mg->manage(_tensor2);
    }
}

void CLTensorHandle::map(bool blocking)
{
    //_tensor.map(blocking);
	//std::cerr<<"mapping of cltensorhandle\n";
    _tensor2->map(blocking);
}

void CLTensorHandle::unmap()
{
    //_tensor.unmap();
    _tensor2->unmap();
}

void CLTensorHandle::release_if_unused()
{
    // TODO (geopin01): Release tensor only if all sub-tensors are marked as not used
    /*if(!_tensor.is_used())
    {
        _tensor.allocator()->free();
    }*/
    if(!_tensor2->is_used())
    {
    	_tensor2->allocator()->free();
    }
}

const arm_compute::ITensor &CLTensorHandle::tensor() const
{
	//std::cerr<<"const tensor in CL\n";
    //return _tensor;
	return *_tensor2;
}

arm_compute::ITensor &CLTensorHandle::tensor()
{
	//std::cerr<<"tensor in CL\n";
    //return _tensor;
	return *_tensor2;
}

arm_compute::ITensor *CLTensorHandle::tensor_ptr()
{
	//std::cerr<<"tensor in CL\n";
    //return _tensor;
	return _tensor2;
}

ITensorHandle *CLTensorHandle::parent_handle()
{
    return this;
}

bool CLTensorHandle::is_subtensor() const
{
    return false;
}

Target CLTensorHandle::target() const
{
    return Target::CL;
}

//Ehsan
void CLTensorHandle::set_tensor(arm_compute::ITensor* _t){
	_tensor2=dynamic_cast<arm_compute::CLTensor*>(_t);
}

} // namespace backends
} // namespace graph
} // namespace arm_compute
