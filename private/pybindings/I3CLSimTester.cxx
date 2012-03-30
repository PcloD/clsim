/**
 * Copyright (c) 2011, 2012
 * Claudio Kopper <claudio.kopper@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * $Id$
 *
 * @file I3CLSimTester.cxx
 * @version $Revision$
 * @date $Date$
 * @author Claudio Kopper
 */

#include <sstream>

#include <test/I3CLSimTesterBase.h>
#include <test/I3CLSimRandomDistributionTester.h>
#include <test/I3CLSimFunctionTester.h>
#include <test/I3CLSimMediumPropertiesTester.h>

#include <boost/preprocessor/seq.hpp>

#include <boost/foreach.hpp>

using namespace boost::python;
namespace bp = boost::python;


struct I3CLSimTesterBaseWrapper : I3CLSimTesterBase, bp::wrapper<I3CLSimTesterBase>
{

};

void register_I3CLSimTester()
{
    {
        bp::scope I3CLSimTesterBase_scope = 
        bp::class_<I3CLSimTesterBaseWrapper, boost::shared_ptr<I3CLSimTesterBaseWrapper>, boost::noncopyable>
        ("I3CLSimTesterBase", bp::no_init)

        .def("GetMaxWorkgroupSize", &I3CLSimTesterBase::GetMaxWorkgroupSize)
        .add_property("maxWorkgroupSize", &I3CLSimTesterBase::GetMaxWorkgroupSize)
        ;
    }
    bp::implicitly_convertible<shared_ptr<I3CLSimTesterBaseWrapper>, shared_ptr<const I3CLSimTesterBase> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimTesterBaseWrapper>, shared_ptr<I3CLSimTesterBase> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimTesterBaseWrapper>, shared_ptr<const I3CLSimTesterBaseWrapper> >();

    // I3CLSimRandomDistributionTester
    {
        bp::scope I3CLSimRandomDistributionTester_scope = 
        bp::class_<I3CLSimRandomDistributionTester, 
                   boost::shared_ptr<I3CLSimRandomDistributionTester>,
                   bases<I3CLSimTesterBase>,
                   boost::noncopyable>
        ("I3CLSimRandomDistributionTester",
         bp::init<const I3CLSimOpenCLDevice &, uint64_t, uint64_t, I3RandomServicePtr, I3CLSimRandomValueConstPtr>
         (
          (
           bp::arg("device"),
           bp::arg("workgroupSize"),
           bp::arg("workItemsPerIteration"),
           bp::arg("randomService"),
           bp::arg("randomDistribution")
          )
         )
        )
        .def("GenerateRandomNumbers", &I3CLSimRandomDistributionTester::GenerateRandomNumbers, bp::arg("iterations"))
        ;
    }
    bp::implicitly_convertible<shared_ptr<I3CLSimRandomDistributionTester>, shared_ptr<const I3CLSimRandomDistributionTester> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimRandomDistributionTester>, shared_ptr<I3CLSimTesterBase> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimRandomDistributionTester>, shared_ptr<const I3CLSimTesterBase> >();

    
    // I3CLSimFunctionTester
    {
        bp::scope I3CLSimFunctionTester_scope = 
        bp::class_<I3CLSimFunctionTester, 
        boost::shared_ptr<I3CLSimFunctionTester>,
        bases<I3CLSimTesterBase>,
        boost::noncopyable>
        ("I3CLSimFunctionTester",
         bp::init<const I3CLSimOpenCLDevice &, uint64_t, uint64_t, I3CLSimFunctionConstPtr>
         (
          (
           bp::arg("device"),
           bp::arg("workgroupSize"),
           bp::arg("workItemsPerIteration"),
           bp::arg("wlenDependentValue")
           )
          )
         )
        .def("EvaluateFunction", &I3CLSimFunctionTester::EvaluateFunction, bp::arg("xValues"))
        .def("EvaluateDerivative", &I3CLSimFunctionTester::EvaluateDerivative, bp::arg("xValues"))

        .def("EvaluateReferenceFunction", &I3CLSimFunctionTester::EvaluateReferenceFunction, bp::arg("xValues"))
        .def("EvaluateReferenceDerivative", &I3CLSimFunctionTester::EvaluateReferenceDerivative, bp::arg("xValues"))
        ;
    }
    bp::implicitly_convertible<shared_ptr<I3CLSimFunctionTester>, shared_ptr<const I3CLSimFunctionTester> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimFunctionTester>, shared_ptr<I3CLSimTesterBase> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimFunctionTester>, shared_ptr<const I3CLSimTesterBase> >();

    
    // I3CLSimMediumPropertiesTester
    {
        bp::scope I3CLSimMediumPropertiesTester_scope = 
        bp::class_<I3CLSimMediumPropertiesTester, 
        boost::shared_ptr<I3CLSimMediumPropertiesTester>,
        bases<I3CLSimTesterBase>,
        boost::noncopyable>
        ("I3CLSimMediumPropertiesTester",
         bp::init<const I3CLSimOpenCLDevice &, uint64_t, uint64_t, I3CLSimMediumPropertiesConstPtr, I3RandomServicePtr>
         (
          (
           bp::arg("device"),
           bp::arg("workgroupSize"),
           bp::arg("workItemsPerIteration"),
           bp::arg("mediumProperties"),
           bp::arg("randomService") = I3RandomServicePtr()
           )
          )
         )
        .def("EvaluatePhaseRefIndex", &I3CLSimMediumPropertiesTester::EvaluatePhaseRefIndex, bp::arg("xValues"), bp::arg("layer"))
        .def("EvaluateDispersion", &I3CLSimMediumPropertiesTester::EvaluateDispersion, bp::arg("xValues"), bp::arg("layer"))
        .def("EvaluateGroupVelocity", &I3CLSimMediumPropertiesTester::EvaluateGroupVelocity, bp::arg("xValues"), bp::arg("layer"))
        .def("EvaluateAbsorptionLength", &I3CLSimMediumPropertiesTester::EvaluateAbsorptionLength, bp::arg("xValues"), bp::arg("layer"))
        .def("EvaluateScatteringLength", &I3CLSimMediumPropertiesTester::EvaluateScatteringLength, bp::arg("xValues"), bp::arg("layer"))
        ;
    }
    bp::implicitly_convertible<shared_ptr<I3CLSimMediumPropertiesTester>, shared_ptr<const I3CLSimMediumPropertiesTester> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimMediumPropertiesTester>, shared_ptr<I3CLSimTesterBase> >();
    bp::implicitly_convertible<shared_ptr<I3CLSimMediumPropertiesTester>, shared_ptr<const I3CLSimTesterBase> >();

}
