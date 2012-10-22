 /* Copyright (c) 2011, 2012
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
 * @file I3CLSimLightSourceToStepConverterPointSource.cxx
 * @version $Revision$
 * @date $Date$
 * @author Claudio Kopper
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include <limits>

#include "phys-services/I3Calculator.h"

#include "clsim/I3CLSimLightSourceToStepConverterPointSource.h"

#include "clsim/function/I3CLSimFunction.h"

#include "clsim/I3CLSimLightSourceToStepConverterUtils.h"
using namespace I3CLSimLightSourceToStepConverterUtils;


const uint32_t I3CLSimLightSourceToStepConverterPointSource::default_photonsPerStep=400;


I3CLSimLightSourceToStepConverterPointSource::I3CLSimLightSourceToStepConverterPointSource
(uint32_t photonsPerStep)
:
initialized_(false),
barrier_is_enqueued_(false),
bunchSizeGranularity_(1),
maxBunchSize_(512000),
photonsPerStep_(photonsPerStep)
{
    if (photonsPerStep_<=0)
        throw I3CLSimLightSourceToStepConverter_exception("photonsPerStep may not be <= 0!");
    
    spectrumSourceTypeIndex_ = 0; //static_cast<uint8_t>(spectrumSourceTypeIndex);
    
    //log_debug("PointSource spectrum registered in table. got index %zu.", spectrumSourceTypeIndex);
}

I3CLSimLightSourceToStepConverterPointSource::~I3CLSimLightSourceToStepConverterPointSource()
{

}

void I3CLSimLightSourceToStepConverterPointSource::Initialize()
{
    if (initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource already initialized!");

    if (!randomService_)
        throw I3CLSimLightSourceToStepConverter_exception("RandomService not set!");

    if (!wlenBias_)
        throw I3CLSimLightSourceToStepConverter_exception("WlenBias not set!");

    if (!mediumProperties_)
        throw I3CLSimLightSourceToStepConverter_exception("MediumProperties not set!");
    
    if (bunchSizeGranularity_ > maxBunchSize_)
        throw I3CLSimLightSourceToStepConverter_exception("BunchSizeGranularity must not be greater than MaxBunchSize!");

    if (maxBunchSize_%bunchSizeGranularity_ != 0)
        throw I3CLSimLightSourceToStepConverter_exception("MaxBunchSize is not a multiple of BunchSizeGranularity!");
    
    // make a copy of the medium properties
    {
        I3CLSimMediumPropertiesConstPtr copiedMediumProperties(new I3CLSimMediumProperties(*mediumProperties_));
        mediumProperties_ = copiedMediumProperties;
    }

    photonNumberCorrectionFactorForBias_ = 1;

    log_debug("photon number correction factor for spectrum and bias is %f", photonNumberCorrectionFactorForBias_);

    initialized_=true;
}


bool I3CLSimLightSourceToStepConverterPointSource::IsInitialized() const
{
    return initialized_;
}

void I3CLSimLightSourceToStepConverterPointSource::SetBunchSizeGranularity(uint64_t num)
{
    if (initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource already initialized!");
    
    if (num<=0)
        throw I3CLSimLightSourceToStepConverter_exception("BunchSizeGranularity of 0 is invalid!");

    if (num!=1)
        throw I3CLSimLightSourceToStepConverter_exception("A BunchSizeGranularity != 1 is currently not supported!");

    bunchSizeGranularity_=num;
}

void I3CLSimLightSourceToStepConverterPointSource::SetMaxBunchSize(uint64_t num)
{
    if (initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource already initialized!");

    if (num<=0)
        throw I3CLSimLightSourceToStepConverter_exception("MaxBunchSize of 0 is invalid!");

    maxBunchSize_=num;
}

void I3CLSimLightSourceToStepConverterPointSource::SetWlenBias(I3CLSimFunctionConstPtr wlenBias)
{
    if (initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource already initialized!");
    
    wlenBias_=wlenBias;
}

void I3CLSimLightSourceToStepConverterPointSource::SetRandomService(I3RandomServicePtr random)
{
    if (initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource already initialized!");
    
    randomService_=random;
}

void I3CLSimLightSourceToStepConverterPointSource::SetMediumProperties(I3CLSimMediumPropertiesConstPtr mediumProperties)
{
    if (initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource already initialized!");

    mediumProperties_=mediumProperties;
}

void I3CLSimLightSourceToStepConverterPointSource::EnqueueLightSource(const I3CLSimLightSource &lightSource, uint32_t identifier)
{
    if (!initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource is not initialized!");

    if (barrier_is_enqueued_)
        throw I3CLSimLightSourceToStepConverter_exception("A barrier is enqueued! You must receive all steps before enqueuing a new particle.");

    if (lightSource.GetType() != I3CLSimLightSource::Flasher)
        throw I3CLSimLightSourceToStepConverter_exception("The I3CLSimLightSourceToStepConverterPointSource parameterization only works on flashers.");
    
    const I3CLSimFlasherPulse &flasherPulse = lightSource.GetFlasherPulse();

    // just skip the entry if there are no photons to generate
    if (flasherPulse.GetNumberOfPhotonsNoBias() <= 0.) return;
    
    const double numPhotonsWithBias = flasherPulse.GetNumberOfPhotonsNoBias()*photonNumberCorrectionFactorForBias_;
    if (numPhotonsWithBias <= 0.) return;
    
    uint64_t numPhotons = flasherPulse.GetNumberOfPhotonsNoBias(); // set to constant if only a fixed number of photons desired

    if (numPhotons==0) return;
    

    log_debug("Generating %zu photons for flasher (after bias). Requested number was %f, with bias %f",
              static_cast<std::size_t>(numPhotons),
              flasherPulse.GetNumberOfPhotonsNoBias(),
              numPhotonsWithBias);
    
    LightSourceData_t newEntry;
    newEntry.isBarrier = false;
    newEntry.flasherPulse = flasherPulse;
    newEntry.identifier = identifier;
    newEntry.numPhotonsWithBias = numPhotons;
    inputQueue_.push_back(newEntry);
}

void I3CLSimLightSourceToStepConverterPointSource::EnqueueBarrier()
{
    if (!initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource is not initialized!");

    if (barrier_is_enqueued_)
        throw I3CLSimLightSourceToStepConverter_exception("A barrier is already enqueued!");

    // actually enqueue the barrier
    log_trace("== enqueue barrier");
    LightSourceData_t newEntry;
    newEntry.isBarrier = true;
    inputQueue_.push_back(newEntry);

    barrier_is_enqueued_=true;
}

bool I3CLSimLightSourceToStepConverterPointSource::BarrierActive() const
{
    if (!initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource is not initialized!");

    return barrier_is_enqueued_;
}

bool I3CLSimLightSourceToStepConverterPointSource::MoreStepsAvailable() const
{
    if (!initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource is not initialized!");

    if (inputQueue_.size() > 0) return true;
    return false;
}


I3CLSimStepSeriesConstPtr I3CLSimLightSourceToStepConverterPointSource::GetConversionResultWithBarrierInfo(bool &barrierWasReset, double timeout)
{
    if (!initialized_)
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource is not initialized!");
    
    barrierWasReset=false;
    
    if (inputQueue_.empty())
    {
        throw I3CLSimLightSourceToStepConverter_exception("I3CLSimLightSourceToStepConverterPointSource: no PointSource pulse is in queue!");
        return I3CLSimStepSeriesConstPtr();
    }
    
    I3CLSimStepSeriesConstPtr returnSteps = MakeSteps(barrierWasReset);
    if (!returnSteps) log_fatal("logic error. returnSteps==NULL");

    if (barrierWasReset) {
        if (!barrier_is_enqueued_)
            log_fatal("logic error: barrier encountered, but enqueued flag is false.");
        
        barrier_is_enqueued_=false;
    }
    
    return returnSteps;
}



// the actual work is done here:
I3CLSimStepSeriesConstPtr I3CLSimLightSourceToStepConverterPointSource::MakeSteps(bool &barrierWasReset)
{
    barrierWasReset=false;
    
    if (inputQueue_.empty()) return I3CLSimStepSeriesConstPtr(); // queue is empty
    
    LightSourceData_t &currentElement = inputQueue_.front();

    // barrier?
    if (currentElement.isBarrier) {
        inputQueue_.pop_front(); // remove the element
        barrierWasReset=true;
        return I3CLSimStepSeriesConstPtr(new I3CLSimStepSeries());
    }

    I3CLSimStepSeriesPtr outputSteps(new I3CLSimStepSeries());
    
    bool entryCanBeRemoved;
    
    uint64_t numSteps;                  // number of steps to generate
    uint64_t numAppendedDummySteps;     // number of dummy steps to append in order to achieve the correct steps vector size granularity
    uint32_t numPhotonsInLastStep;      // number of photons in the last step
    // (the number of photons in all other steps is photonsPerStep_)
    
    const uint64_t maxPhotonsPerResult = maxBunchSize_*static_cast<uint64_t>(photonsPerStep_);
    if (currentElement.numPhotonsWithBias >= maxPhotonsPerResult) {
        numSteps = maxBunchSize_;
        numPhotonsInLastStep = photonsPerStep_; // nothing special for the last step
        
        numAppendedDummySteps=0;
        
        currentElement.numPhotonsWithBias -= numSteps*static_cast<uint64_t>(photonsPerStep_);
        entryCanBeRemoved = (currentElement.numPhotonsWithBias == 0);
        
        outputSteps->reserve(maxBunchSize_);
    } else {
        if (currentElement.numPhotonsWithBias <= static_cast<uint64_t>(photonsPerStep_)) {
            // only a single step
            numSteps = 1;
            numPhotonsInLastStep=static_cast<uint32_t>(currentElement.numPhotonsWithBias);
        } else {
            numSteps = currentElement.numPhotonsWithBias / static_cast<uint64_t>(photonsPerStep_);
            numPhotonsInLastStep = static_cast<uint32_t>(currentElement.numPhotonsWithBias % static_cast<uint64_t>(photonsPerStep_));
            if (numPhotonsInLastStep>0) {
                // one step more with less photons
                numSteps;
            }
        }

        entryCanBeRemoved=true;
        currentElement.numPhotonsWithBias = 0;

        const uint64_t modulo = numSteps % bunchSizeGranularity_;
        numAppendedDummySteps=0;
        if (modulo>0) {
            // we need some dummy steps
            
            numAppendedDummySteps = bunchSizeGranularity_-modulo;
        }
        
        outputSteps->reserve(numSteps + numAppendedDummySteps);
    }
    
    // now make the steps
    for (uint64_t i=0;i<numSteps;i)
    {
        const uint32_t numberOfPhotonsForThisStep = (i==numSteps-1)?numPhotonsInLastStep:photonsPerStep_;
        if (numberOfPhotonsForThisStep==0) {numAppendedDummySteps; continue;}
        
        outputSteps->push_back(I3CLSimStep());
        I3CLSimStep &newStep = outputSteps->back();

        FillStep(newStep,
                 numberOfPhotonsForThisStep,
                 currentElement.flasherPulse,
                 currentElement.identifier
                );
    }

    // and the dummy steps
    for (uint64_t i=0;i<numAppendedDummySteps;i)
    {
        outputSteps->push_back(I3CLSimStep());
        I3CLSimStep &newStep = outputSteps->back();

        newStep.SetPosX(0.); newStep.SetPosY(0.); newStep.SetPosZ(0.);
        newStep.SetTime(0.);
        newStep.SetDirTheta(0.); newStep.SetDirPhi(0.);
        newStep.SetLength(0.);
        newStep.SetBeta(1.);
        newStep.SetNumPhotons(0);
        newStep.SetWeight(0);
        newStep.SetSourceType(0);
        newStep.SetID(currentElement.identifier);
    }
    
    if (entryCanBeRemoved) inputQueue_.pop_front();
    
    return outputSteps;
}


void I3CLSimLightSourceToStepConverterPointSource::FillStep(I3CLSimStep &step,
                                                        uint32_t numberOfPhotons,
                                                        const I3CLSimFlasherPulse &flasherPulse,
                                                        uint32_t identifier)
{
    //////// bunch direction
    
    // set the direction
    const double polarAngle = flasherPulse.GetDir().CalcTheta();
    const double azimuthalAngle = flasherPulse.GetDir().CalcPhi();
    
    // the smearing angle
    const double smearPolar     = randomService_->Gaus(0., flasherPulse.GetAngularEmissionSigmaPolar());
    const double smearAzimuthal = randomService_->Gaus(0., flasherPulse.GetAngularEmissionSigmaAzimuthal());

    // calculate the new azimuthal angle (in the horizontal plane)
    const double smearedAzimuth = azimuthalAngle + smearAzimuthal;
    
    I3Direction smearedDirection;
    // no polar direction yet (theta==0 <=>  x-axis)
    smearedDirection.SetThetaPhi(90.*I3Units::deg, smearedAzimuth);
    
    // this is the rotation axis 
    I3Direction rotAxis;
    rotAxis.SetThetaPhi(90.*I3Units::deg, smearedAzimuth-90.*I3Units::deg);
    
    // now rotate to the polar angle (plus smearing)
    I3Calculator::Rotate(rotAxis, smearedDirection, (90.*I3Units::deg-polarAngle) + smearPolar);

    
    //////// bunch time
    
    const double timeWidthFWHM = flasherPulse.GetPulseWidth();
    const double timeWidthSigma = timeWidthFWHM / 2.3548; // convert from FWHM to sigma (assuming a gaussian distribution)
    
    // Use a gaussian for now. TODO: There is a non-gaussian tail/after-glow, which should be implemented.
    const double smearedTime = flasherPulse.GetTime() + randomService_->Gaus(0., timeWidthSigma);
    
    //////// done!
    
    step.SetPos(flasherPulse.GetPos());
    step.SetTime(smearedTime);
    step.SetDir(smearedDirection);
    step.SetLength(0.);
    step.SetBeta(1.);
    step.SetNumPhotons(numberOfPhotons);
    step.SetWeight(1.);
    step.SetID(identifier);
    step.SetSourceType(spectrumSourceTypeIndex_);
}