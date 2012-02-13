//
//   Copyright (c) 2011  Claudio Kopper
//   
//   $Id$
//
//   This file is part of the IceTray module clsim.
//
//   clsim is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 3 of the License, or
//   (at your option) any later version.
//
//   IceTray is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef I3CLSIMTESTERBASE_H_INCLUDED
#define I3CLSIMTESTERBASE_H_INCLUDED

#include "dataclasses/I3Vector.h"
#include "clsim/I3CLSimRandomValue.h"

#include "clsim/I3CLSimOpenCLDevice.h"

#include "boost/python/tuple.hpp"
#include "boost/python/extract.hpp"

#include <vector>
#include <string>

#define __CL_ENABLE_EXCEPTIONS
#include "clsim/cl.hpp"

class I3CLSimTesterBase
{
public:
    I3CLSimTesterBase();
    
    inline uint64_t GetMaxWorkgroupSize() const {return maxWorkgroupSize;}
    
protected:
    void DoSetup(const I3CLSimOpenCLDevice &device,
                 uint64_t workgroupSize_,
                 uint64_t workItemsPerIteration_,
                 const std::vector<std::string> &source,
                 const std::string compilerOptions="");
    
    std::vector<std::string> sourceStrings_;
    
    shared_ptr<cl::Context> context;
    shared_ptr<cl::Program> program;
    
    shared_ptr<cl::Kernel> kernel;
    uint64_t workgroupSize;
    uint64_t workItemsPerIteration;
    
    shared_ptr<cl::CommandQueue> queue;

private:
    uint64_t maxWorkgroupSize;
    
public:    
    SET_LOGGER("I3CLSimTesterBase");
   
};



#endif //I3CLSIMTESTERBASE_H_INCLUDED
