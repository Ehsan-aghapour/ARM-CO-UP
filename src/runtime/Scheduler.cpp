/*
 * Copyright (c) 2017-2020 Arm Limited.
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
#include "arm_compute/runtime/Scheduler.h"

#include "arm_compute/core/Error.h"

#if ARM_COMPUTE_CPP_SCHEDULER
#include "arm_compute/runtime/CPP/CPPScheduler.h"
#endif /* ARM_COMPUTE_CPP_SCHEDULER */

#include "arm_compute/runtime/SingleThreadScheduler.h"

#if ARM_COMPUTE_OPENMP_SCHEDULER
#include "arm_compute/runtime/OMP/OMPScheduler.h"
#endif /* ARM_COMPUTE_OPENMP_SCHEDULER */

using namespace arm_compute;

#if !ARM_COMPUTE_CPP_SCHEDULER && ARM_COMPUTE_OPENMP_SCHEDULER
Scheduler::Type Scheduler::_scheduler_type = Scheduler::Type::OMP;
#elif ARM_COMPUTE_CPP_SCHEDULER && !ARM_COMPUTE_OPENMP_SCHEDULER
Scheduler::Type Scheduler::_scheduler_type = Scheduler::Type::CPP;
#elif ARM_COMPUTE_CPP_SCHEDULER && ARM_COMPUTE_OPENMP_SCHEDULER
Scheduler::Type Scheduler::_scheduler_type = Scheduler::Type::CPP;
#else  /* ARM_COMPUTE_*_SCHEDULER */
Scheduler::Type Scheduler::_scheduler_type = Scheduler::Type::ST;
#endif /* ARM_COMPUTE_*_SCHEDULER */

const int Little_cores=4;

std::shared_ptr<IScheduler> Scheduler::_custom_scheduler = nullptr;

namespace
{
std::map<Scheduler::Type, std::unique_ptr<IScheduler>> init()
{
    std::map<Scheduler::Type, std::unique_ptr<IScheduler>> m;
    //std::cerr<<"init ST\n";
    m[Scheduler::Type::ST] = std::make_unique<SingleThreadScheduler>();
#if defined(ARM_COMPUTE_CPP_SCHEDULER)
    //std::cerr<<"init CPP\n";
    m[Scheduler::Type::CPP] = std::make_unique<CPPScheduler>();
    //std::cerr<<"now init CPP2\n";
    m[Scheduler::Type::CPP2] = std::make_unique<CPPScheduler>();
#endif // defined(ARM_COMPUTE_CPP_SCHEDULER)
#if defined(ARM_COMPUTE_OPENMP_SCHEDULER)
    m[Scheduler::Type::OMP] = std::make_unique<OMPScheduler>();
#endif // defined(ARM_COMPUTE_OPENMP_SCHEDULER)

    return m;
}
} // namespace

std::map<Scheduler::Type, std::unique_ptr<IScheduler>> Scheduler::_schedulers{};

void Scheduler::set(Type t)
{
    ARM_COMPUTE_ERROR_ON(!Scheduler::is_available(t));
    _scheduler_type = t;
}

bool Scheduler::is_available(Type t)
{
    if(t == Type::CUSTOM)
    {
        return _custom_scheduler != nullptr;
    }
    else
    {
        return _schedulers.find(t) != _schedulers.end();
    }
}

Scheduler::Type Scheduler::get_type()
{
    return _scheduler_type;
}

IScheduler &Scheduler::get()
{
    //std::cerr<<"Scheduler.cpp:: get scheduler, type: "<<int(_scheduler_type)<<std::endl;
    if(_scheduler_type == Type::CUSTOM)
    {
        if(_custom_scheduler == nullptr)
        {
            ARM_COMPUTE_ERROR("No custom scheduler has been setup. Call set(std::shared_ptr<IScheduler> &scheduler) before Scheduler::get()");
        }
        else
        {
            return *_custom_scheduler;
        }
    }
    else
    {

        if(_schedulers.empty())
        {
        	//std::cerr<<"init of schedulers\n";
            _schedulers = init();
            //std::cerr<<"init done\n";
        }
        //std::cerr<<"Running CPU is: "<<sched_getcpu()<<std::endl;
        if(sched_getcpu()>(Little_cores-1)){
            //std::cerr<<"Setting scheduler to CPP2  for big cluster ...\n";
			if(_scheduler_type==Scheduler::Type::CPP){
				_scheduler_type=Scheduler::Type::CPP2;
			}
		}
		if(sched_getcpu()<(Little_cores)){
            //std::cerr<<"setting scheduler to CPP for litte cluster\n";
			if(_scheduler_type==Scheduler::Type::CPP2){
				_scheduler_type=Scheduler::Type::CPP;
			}
		}
		//std::cout<<"\ncore:"<<sched_getcpu();
        auto it = _schedulers.find(_scheduler_type);
        //std::cout<<"\nsch type:"<<int(_scheduler_type)<<"\t num_threads:"<<(*it->second).num_threads()<<std::endl;
        if(it != _schedulers.end())
        {
			//Ehsan
			//std::cout<<"scheduler.cpp, scheduler type: "<<static_cast<std::underlying_type<Type>::type>(_scheduler_type)<<std::endl;
			//std::cerr<<"scheduler.cpp, scheduler type: "<<static_cast<std::underlying_type<Type>::type>(_scheduler_type)<<std::endl;
			//it->second->print_threads();
            return *it->second;
        }
        else
        {
            ARM_COMPUTE_ERROR("Invalid Scheduler type");
        }
    }
}

void Scheduler::set(std::shared_ptr<IScheduler> scheduler)
{
    _custom_scheduler = std::move(scheduler);
    set(Type::CUSTOM);
}
