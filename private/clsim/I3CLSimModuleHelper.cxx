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
 * @file I3CLSimModuleHelper.cxx
 * @version $Revision$
 * @date $Date$
 * @author Claudio Kopper
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#include "clsim/I3CLSimModuleHelper.h"

#include "clsim/function/I3CLSimFunctionConstant.h"
#include "clsim/function/I3CLSimFunctionFromTable.h"
#include "clsim/random_value/I3CLSimRandomValueInterpolatedDistribution.h"
#include "clsim/random_value/I3CLSimRandomValueWlenCherenkovNoDispersion.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/variant/get.hpp>

#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"


namespace I3CLSimModuleHelper {
    
    namespace {
        double CherenkovYieldDistribution(double wlen, I3CLSimMediumPropertiesPtr mediumProperties, double beta=1.)
        {
            I3CLSimFunctionConstPtr nPhaseDist =
            mediumProperties->GetPhaseRefractiveIndex(0); // this assumes the refractive index does not change between layers
            
            if (!nPhaseDist->HasNativeImplementation()) 
                log_fatal("The refractive index distribution needs a native implementation to be usable!");
            
            const double nPhase = nPhaseDist->GetValue(wlen);
            
            return (2.*M_PI/(137.*(wlen*wlen)))*(1. - 1./ ( std::pow(beta*nPhase,2.) ) ); // dN/dxdwlen
        }

        // the normalization will not be correct here
        double CherenkovYieldDistributionNoDispersion(double wlen)
        {
            return 1./(wlen*wlen); // dN/dxdwlen
        }

    };
    
    
    I3CLSimRandomValueConstPtr
    makeWavelengthGenerator(I3CLSimFunctionConstPtr unbiasedSpectrum,
                            I3CLSimFunctionConstPtr wavelengthGenerationBias,
                            I3CLSimMediumPropertiesPtr mediumProperties)
    {
        double minWlen = unbiasedSpectrum->GetMinWlen();
        double maxWlen = unbiasedSpectrum->GetMaxWlen();
        
        // check if the spectrum is from a tabulated distribution (instead of
        // a parameterized one)
        I3CLSimFunctionFromTableConstPtr unbiasedSpectrumFromTable;
        unbiasedSpectrumFromTable =
        dynamic_pointer_cast<const I3CLSimFunctionFromTable>(unbiasedSpectrum);

        if (!unbiasedSpectrumFromTable) {
            // do not clip wavelengths if they are from a tabulated distribution. In that case,
            // re-use the entire table binning
            if (mediumProperties->GetMinWavelength() > minWlen) minWlen=mediumProperties->GetMinWavelength();
            if (mediumProperties->GetMaxWavelength() < maxWlen) maxWlen=mediumProperties->GetMaxWavelength();
        }
        
        const double wlenRange = maxWlen-minWlen;
        if (wlenRange <= 0.) log_fatal("Internal error, wavelength range <= 0!");

        if (wavelengthGenerationBias->GetMinWlen() > minWlen)
            log_fatal("wavelength generation bias has to have a wavelength range larger or equal to the spectrum wavelength range!");
        if (wavelengthGenerationBias->GetMaxWlen() < maxWlen)
            log_fatal("wavelength generation bias has to have a wavelength range larger or equal to the spectrum wavelength range!");

        // Check if the spectrum values are from a tabulated distribution.
        // If yes, use the table binning, if not make up a binning.
        if (unbiasedSpectrumFromTable)
        {
            std::size_t wlenPoints = unbiasedSpectrumFromTable->GetNumEntries();
            const double firstWlen = unbiasedSpectrumFromTable->GetFirstWavelength();
            
            std::vector<double> spectrum(wlenPoints, NAN);
            std::vector<double> wavelengths(wlenPoints, NAN);
            for (std::size_t i=0;i<wlenPoints;++i)
            {
                const double wavelength = unbiasedSpectrumFromTable->GetEntryWavelength(i);
                const double entry = unbiasedSpectrumFromTable->GetEntryValue(i);
                const double bias = wavelengthGenerationBias->GetValue(wavelength);
                
                spectrum[i] = bias * entry;
                wavelengths[i] = wavelength;
            }
            
            if (unbiasedSpectrumFromTable->GetInEqualSpacingMode()) {
                const double wlenStep = unbiasedSpectrumFromTable->GetWavelengthStepping();

                return I3CLSimRandomValueInterpolatedDistributionConstPtr
                (new I3CLSimRandomValueInterpolatedDistribution(firstWlen,
                                                                wlenStep,
                                                                spectrum));
            } else {
                // slightly less efficient if non-equally spaced
                return I3CLSimRandomValueInterpolatedDistributionConstPtr
                (new I3CLSimRandomValueInterpolatedDistribution(wavelengths,
                                                                spectrum));
            }
        }
        else
        {
            // use a pre-defined binning of 10ns (an arbitrary value..)
            
            std::size_t wlenPoints = static_cast<std::size_t>(wlenRange/(10.*I3Units::nanometer))+2;
            const double firstWlen = minWlen;
            const double wlenStep = wlenRange/static_cast<double>(wlenPoints-1);
            
            std::vector<double> spectrum(wlenPoints, NAN);
            for (std::size_t i=0;i<wlenPoints;++i)
            {
                const double wavelength = firstWlen + static_cast<double>(i)*wlenStep;
                const double entry = unbiasedSpectrum->GetValue(wavelength);
                const double bias = wavelengthGenerationBias->GetValue(wavelength);
                
                spectrum[i] = bias * entry;
            }
            
            return I3CLSimRandomValueInterpolatedDistributionConstPtr
            (new I3CLSimRandomValueInterpolatedDistribution(firstWlen,
                                                            wlenStep,
                                                            spectrum));
        }
    }
    
    I3CLSimRandomValueConstPtr
    makeCherenkovWavelengthGenerator(I3CLSimFunctionConstPtr wavelengthGenerationBias,
                                     bool generateCherenkovPhotonsWithoutDispersion,
                                     I3CLSimMediumPropertiesPtr mediumProperties)
    {
        const double minWlen = mediumProperties->GetMinWavelength();
        const double maxWlen = mediumProperties->GetMaxWavelength();
        const double wlenRange = maxWlen-minWlen;
        if (wlenRange <= 0.) log_fatal("Internal error, wavelength range <= 0!");

        if (wavelengthGenerationBias->GetMinWlen() > minWlen)
            log_fatal("wavelength generation bias has to have a wavelength range larger or equal to the medium property range!");
        if (wavelengthGenerationBias->GetMaxWlen() < maxWlen)
            log_fatal("wavelength generation bias has to have a wavelength range larger or equal to the medium property range!");
        
        bool noBias=false;
        //bool biasIsConstant=false;
        
        {
            I3CLSimFunctionConstantConstPtr wavelengthGenerationBiasConstant =
            dynamic_pointer_cast<const I3CLSimFunctionConstant>(wavelengthGenerationBias);
            
            if (wavelengthGenerationBiasConstant)
            {
                //biasIsConstant=true;
                
                if ( std::abs(wavelengthGenerationBiasConstant->GetValue((minWlen+maxWlen)/2.)-1.) < 1e-10 )
                    noBias=true;
            }
        }
        
        I3CLSimFunctionFromTableConstPtr wavelengthGenerationBiasFromTable;
        wavelengthGenerationBiasFromTable =
        dynamic_pointer_cast<const I3CLSimFunctionFromTable>(wavelengthGenerationBias);
        
        

        if ((!noBias) && (generateCherenkovPhotonsWithoutDispersion))
        {
            log_warn("**********");
            log_warn(" Using the \"GenerateCherenkovPhotonsWithoutDispersion\" option");
            log_warn(" with a biased photon spectrum generation does not yield a performance");
            log_warn(" increase. You might consider turning this option off to get a better");
            log_warn(" approximation of the Cherenkov spectrum.");
            log_warn("**********");
        }
        
        // Check if the bias values are from a tabulated distribution.
        // If yes, use the table binning, if not make up a binning.
        if (wavelengthGenerationBiasFromTable)
        {
            std::size_t wlenPoints = wavelengthGenerationBiasFromTable->GetNumEntries();
            const double firstWlen = wavelengthGenerationBiasFromTable->GetFirstWavelength();
            
            std::vector<double> spectrum(wlenPoints, NAN);
            std::vector<double> wavelengths(wlenPoints, NAN);
            for (std::size_t i=0;i<wlenPoints;++i)
            {
                const double wavelength = wavelengthGenerationBiasFromTable->GetEntryWavelength(i);
                const double bias = wavelengthGenerationBiasFromTable->GetEntryValue(i);
                
                if (generateCherenkovPhotonsWithoutDispersion)
                {
                    spectrum[i] =
                    bias * CherenkovYieldDistributionNoDispersion(wavelength);
                }
                else
                {
                    spectrum[i] =
                    bias * CherenkovYieldDistribution(wavelength, mediumProperties);
                }
                
                wavelengths[i] = wavelength;
            }
            
            if (wavelengthGenerationBiasFromTable->GetInEqualSpacingMode()) {
                const double wlenStep = wavelengthGenerationBiasFromTable->GetWavelengthStepping();

                return I3CLSimRandomValueInterpolatedDistributionConstPtr
                (new I3CLSimRandomValueInterpolatedDistribution(firstWlen,
                                                                wlenStep,
                                                                spectrum));
            } else {
                // slightly less efficient if non-equally spaced
                return I3CLSimRandomValueInterpolatedDistributionConstPtr
                (new I3CLSimRandomValueInterpolatedDistribution(wavelengths,
                                                                spectrum));
            }
        }
        else if ((noBias) && (generateCherenkovPhotonsWithoutDispersion))
        {
            return I3CLSimRandomValueWlenCherenkovNoDispersionConstPtr
            (new I3CLSimRandomValueWlenCherenkovNoDispersion(minWlen, maxWlen));
        }
        else
        {
            std::size_t wlenPoints = static_cast<std::size_t>(wlenRange/(10.*I3Units::nanometer))+2;
            const double firstWlen = minWlen;
            const double wlenStep = wlenRange/static_cast<double>(wlenPoints-1);
            
            std::vector<double> spectrum(wlenPoints, NAN);
            for (std::size_t i=0;i<wlenPoints;++i)
            {
                const double wavelength = firstWlen + static_cast<double>(i)*wlenStep;
                const double bias = wavelengthGenerationBias->GetValue(wavelength);
                
                if (generateCherenkovPhotonsWithoutDispersion)
                {
                    spectrum[i] =
                    bias * CherenkovYieldDistributionNoDispersion(wavelength);
                }
                else
                {
                    spectrum[i] =
                    bias * CherenkovYieldDistribution(wavelength, mediumProperties);
                }
            }

            return I3CLSimRandomValueInterpolatedDistributionConstPtr
            (new I3CLSimRandomValueInterpolatedDistribution(firstWlen,
                                                            wlenStep,
                                                            spectrum));
        }

    
    }

    
    I3CLSimStepToPhotonConverterOpenCLPtr initializeOpenCL(const I3CLSimOpenCLDevice &device,
                                                           I3RandomServicePtr rng,
                                                           I3CLSimSimpleGeometryFromI3GeometryPtr geometry,
                                                           I3CLSimMediumPropertiesPtr medium,
                                                           I3CLSimFunctionConstPtr wavelengthGenerationBias,
                                                           const std::vector<I3CLSimRandomValueConstPtr> &wavelengthGenerators,
                                                           bool enableDoubleBuffering,
                                                           bool doublePrecision,
                                                           bool stopDetectedPhotons,
                                                           bool saveAllPhotons,
                                                           double saveAllPhotonsPrescale,
                                                           uint32_t photonHistoryEntries)
    {
        I3CLSimStepToPhotonConverterOpenCLPtr conv(new I3CLSimStepToPhotonConverterOpenCL(rng, device.GetUseNativeMath()));

        conv->SetDevice(device);

        conv->SetWlenGenerators(wavelengthGenerators);
        conv->SetWlenBias(wavelengthGenerationBias);

        conv->SetMediumProperties(medium);
        conv->SetGeometry(geometry);

        conv->SetEnableDoubleBuffering(enableDoubleBuffering);
        conv->SetDoublePrecision(doublePrecision);
        conv->SetStopDetectedPhotons(stopDetectedPhotons);
        conv->SetSaveAllPhotons(saveAllPhotons);
        conv->SetSaveAllPhotonsPrescale(saveAllPhotonsPrescale);
        
        conv->SetPhotonHistoryEntries(photonHistoryEntries);

        conv->Compile();
        //log_trace("%s", conv.GetFullSource().c_str());
        
        const std::size_t maxWorkgroupSize = conv->GetMaxWorkgroupSize();
        conv->SetWorkgroupSize(maxWorkgroupSize);

        const std::size_t workgroupSize = conv->GetWorkgroupSize();
        
        // use approximately the given number of work items, convert to a multiple of the workgroup size
        std::size_t maxNumWorkitems = (static_cast<std::size_t>(device.GetApproximateNumberOfWorkItems())/workgroupSize)*workgroupSize;
        if (maxNumWorkitems==0) maxNumWorkitems=workgroupSize;
        
        conv->SetMaxNumWorkitems(maxNumWorkitems);

        log_info("maximum workgroup size is %zu", maxWorkgroupSize);
        log_info("configured workgroup size is %zu", workgroupSize);
        if (maxNumWorkitems != device.GetApproximateNumberOfWorkItems()) {
            log_warn("maximum number of work items is %zu (user configured was %" PRIu32 ")", maxNumWorkitems, device.GetApproximateNumberOfWorkItems());
        } else {
            log_info("maximum number of work items is %zu (user configured was %" PRIu32 ")", maxNumWorkitems, device.GetApproximateNumberOfWorkItems());
        }

        conv->Initialize();
        
        return conv;
    }

    I3CLSimLightSourceToStepConverterGeant4Ptr initializeGeant4(I3RandomServicePtr rng,
                                                             I3CLSimMediumPropertiesPtr medium,
                                                             I3CLSimFunctionConstPtr wavelengthGenerationBias,
                                                             uint64_t bunchSizeGranularity,
                                                             uint64_t maxBunchSize,
                                                             const I3CLSimLightSourceParameterizationSeries &parameterizationList,
                                                             const std::string &physicsListName,
                                                             double maxBetaChangePerStep,
                                                             uint32_t maxNumPhotonsPerStep,
                                                             bool multiprocessor)
    {
        I3CLSimLightSourceToStepConverterGeant4Ptr conv
        (
         new I3CLSimLightSourceToStepConverterGeant4
         (
          physicsListName,
          maxBetaChangePerStep,
          maxNumPhotonsPerStep
         )
        );
        
        conv->SetRandomService(rng);
        conv->SetWlenBias(wavelengthGenerationBias);
        conv->SetMediumProperties(medium);
        conv->SetMaxBunchSize(maxBunchSize);
        conv->SetBunchSizeGranularity(bunchSizeGranularity);
        
        conv->SetLightSourceParameterizationSeries(parameterizationList);
        
        conv->Initialize();
        
        return conv;
    }

}
