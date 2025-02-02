/*
 * Copyright (c) 2018-2021 Arm Limited.
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
#ifndef My_print
#include "arm_compute/gl_vs.h"
#endif

#include "arm_compute/graph/backends/NEON/NEDeviceBackend.h"

#include "arm_compute/graph/Graph.h"
#include "arm_compute/graph/GraphContext.h"
#include "arm_compute/graph/INode.h"
#include "arm_compute/graph/Logger.h"
#include "arm_compute/graph/Tensor.h"
#include "arm_compute/graph/backends/BackendRegistrar.h"
#include "arm_compute/graph/backends/NEON/NEFunctionFactory.h"
#include "arm_compute/graph/backends/NEON/NENodeValidator.h"
#include "arm_compute/graph/backends/NEON/NESubTensorHandle.h"
#include "arm_compute/graph/backends/NEON/NETensorHandle.h"

#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/runtime/Allocator.h"
#include "arm_compute/runtime/BlobLifetimeManager.h"
#include "arm_compute/runtime/IWeightsManager.h"
#include "arm_compute/runtime/MemoryGroup.h"
#include "arm_compute/runtime/MemoryManagerOnDemand.h"
#include "arm_compute/runtime/OffsetLifetimeManager.h"
#include "arm_compute/runtime/PoolManager.h"
#include "arm_compute/runtime/Scheduler.h"

#include "support/ToolchainSupport.h"

//Ehsan
//#include "arm_compute/gl_vs.h"

namespace arm_compute
{
namespace graph
{
namespace backends
{
/** Register Neon backend */
static detail::BackendRegistrar<NEDeviceBackend> NEDeviceBackend_registrar(Target::NEON);

NEDeviceBackend::NEDeviceBackend()
    : _allocator()
{
	//std::cout<<"creating nedevicebackend\n";
	//std::string s;
	//std::cin>>s;
}

void NEDeviceBackend::initialize_backend()
{
    //Nothing to do
}

void NEDeviceBackend::release_backend_context(GraphContext &ctx)
{
    //Nothing to do
    ARM_COMPUTE_UNUSED(ctx);
}

void NEDeviceBackend::setup_backend_context(GraphContext &ctx)
{
    // Set number of threads
    if(ctx.config().num_threads >= 0)
    {
/*	//Ehsan
        //Scheduler::get().set_num_threads(ctx.config().num_threads);
	Scheduler::get().set_num_threads_with_affinity(ctx.config().num_threads,[](int t_id,int max_cores){
#if My_print > 0
		std::cout<<"max_cores: "<<max_cores<<std::endl;
#endif
		return (5-(t_id%6));
	});*/
    //Ehsan
    //Scheduler::get().set_num_threads(ctx.config().num_threads);
    //std::cout<<"cluster:"<<ctx.config().cluster<<std::endl;
    std::cerr<<"NEDevicebackend num_threads: "<<ctx.config().num_threads<<std::endl;
    Scheduler::get().set_num_threads_with_affinity(ctx.config().num_threads,ctx.config(),[](int t_id,int max_cores, arm_compute::graph::GraphConfig cfg){
    #if My_print > 0
    		std::cout<<"max_cores: "<<max_cores<<std::endl;
    #endif
		int total_cores=cfg.total_cores;
		int big_cores=cfg.big_cores;
		int little_cores=cfg.little_cores;
		bool first_big=cfg.first_big;
		if (first_big){
			//big
			if(cfg.cluster==1)
				//return ((total_cores-1)-(t_id%total_cores));
				return t_id;
			//little
			else if(cfg.cluster==0)
				return (big_cores+t_id)%total_cores;
		}
		else{
			if(cfg.cluster==1)
				//return ((total_cores-1)-(t_id%total_cores));
				return (little_cores+t_id)%total_cores;
			else if(cfg.cluster==0)
				return t_id;
			}
		}
		);
    }

    // Create function level memory manager
    if(ctx.memory_management_ctx(Target::NEON) == nullptr)
    {
        MemoryManagerContext mm_ctx;
        mm_ctx.target      = Target::NEON;
        mm_ctx.intra_mm    = create_memory_manager(MemoryManagerAffinity::Offset);
        mm_ctx.cross_mm    = create_memory_manager(MemoryManagerAffinity::Offset);
        mm_ctx.cross_group = std::make_shared<MemoryGroup>(mm_ctx.cross_mm);
        mm_ctx.allocator   = &_allocator;

        ctx.insert_memory_management_ctx(std::move(mm_ctx));
    }

    // Create function level weights manager
    if(ctx.weights_management_ctx(Target::NEON) == nullptr)
    {
        WeightsManagerContext wm_ctx;
        wm_ctx.target = Target::NEON;
        wm_ctx.wm     = create_weights_manager();

        ctx.insert_weights_management_ctx(std::move(wm_ctx));
    }
}

bool NEDeviceBackend::is_backend_supported()
{
    return true;
}

IAllocator *NEDeviceBackend::backend_allocator()
{
    return &_allocator;
}

std::unique_ptr<ITensorHandle> NEDeviceBackend::create_tensor(const Tensor &tensor)
{
    // Get tensor descriptor
    const TensorDescriptor &tensor_desc = tensor.desc();
    ARM_COMPUTE_ERROR_ON(tensor_desc.target != Target::NEON);

    // Create backend tensor handle
    TensorInfo info(tensor_desc.shape, 1, tensor_desc.data_type, tensor_desc.quant_info);
    info.set_data_layout(tensor_desc.layout);

    return std::make_unique<NETensorHandle>(info);
}

std::unique_ptr<ITensorHandle> NEDeviceBackend::create_subtensor(ITensorHandle *parent, TensorShape shape, Coordinates coords, bool extend_parent)
{
    if(parent == nullptr)
    {
        return nullptr;
    }

    return std::make_unique<NESubTensorHandle>(parent, shape, coords, extend_parent);
}

std::unique_ptr<arm_compute::IFunction> NEDeviceBackend::configure_node(INode &node, GraphContext &ctx)
{
    ARM_COMPUTE_LOG_GRAPH_VERBOSE("Configuring Neon node with ID : " << node.id() << std::endl);
    ARM_COMPUTE_ERROR_ON(node.assigned_target() != Target::NEON);

    // Configure node
    return NEFunctionFactory::create(&node, ctx);
}

arm_compute::Status NEDeviceBackend::validate_node(INode &node)
{
    ARM_COMPUTE_LOG_GRAPH_VERBOSE("Validating Neon node with ID : " << node.id() << std::endl);
    ARM_COMPUTE_ERROR_ON(node.assigned_target() != Target::NEON);

    return NENodeValidator::validate(&node);
}

std::shared_ptr<arm_compute::IMemoryManager> NEDeviceBackend::create_memory_manager(MemoryManagerAffinity affinity)
{
    std::shared_ptr<ILifetimeManager> lifetime_mgr = nullptr;
    if(affinity == MemoryManagerAffinity::Buffer)
    {
        lifetime_mgr = std::make_shared<BlobLifetimeManager>();
    }
    else
    {
        lifetime_mgr = std::make_shared<OffsetLifetimeManager>();
    }
    auto pool_mgr = std::make_shared<PoolManager>();
    auto mm       = std::make_shared<MemoryManagerOnDemand>(lifetime_mgr, pool_mgr);

    return mm;
}

std::shared_ptr<arm_compute::IWeightsManager> NEDeviceBackend::create_weights_manager()
{
    auto weights_mgr = std::make_shared<IWeightsManager>();
    return weights_mgr;
}
} // namespace backends
} // namespace graph
} // namespace arm_compute
