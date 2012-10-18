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
 * @file I3CLSimPMTPhotonSimulatorIceCube.h
 * @version $Revision$
 * @date $Date$
 * @author Claudio Kopper
 */

#ifndef I3CLSIMPMTPHOTONSIMULATORICECUBE_H_INCLUDED
#define I3CLSIMPMTPHOTONSIMULATORICECUBE_H_INCLUDED

#include "clsim/I3CLSimPMTPhotonSimulator.h"

#include "icetray/I3Units.h"
#include "phys-services/I3RandomService.h"

// froward declarations
class AfterPulseGenerator;
class LatePulseGenerator;

/**
 * @brief Simulate PMT jitter, late-pulses and
 * after-pulses by using code from hit-maker.
 *
 */
struct I3CLSimPMTPhotonSimulatorIceCube : public I3CLSimPMTPhotonSimulator
{
public:
    static const double DEFAULT_jitter;
    static const double DEFAULT_pre_pulse_probability;
    static const double DEFAULT_late_pulse_probability;
    static const double DEFAULT_after_pulse_probability;
    
    I3CLSimPMTPhotonSimulatorIceCube(double jitter=DEFAULT_jitter,
                                     double pre_pulse_probability=DEFAULT_pre_pulse_probability,
                                     double late_pulse_probability=DEFAULT_late_pulse_probability,
                                     double after_pulse_probability=DEFAULT_after_pulse_probability);
    virtual ~I3CLSimPMTPhotonSimulatorIceCube();

    /**
     * Set the current calibration
     */
    virtual void SetCalibration(I3CalibrationConstPtr calibration);
    
    /**
     * Set the current status
     */
    virtual void SetDetectorStatus(I3DetectorStatusConstPtr status);

    /**
     * Sets the random number generator service.
     * This should be an instance of I3RandomService.
     */
    virtual void SetRandomService(I3RandomServicePtr random);

    /**
     * This function does not clear the vector before filling it. New hits are added to it.
     */
    virtual void ApplyAfterPulseLatePulseAndJitterSim(const OMKey &key,
                                                      const I3MCHit &input_hit,
                                                      std::vector<I3MCHit> &output_vector) const;
    
private:
    double GenerateJitter() const;
    
    I3RandomServicePtr randomService_;
    double jitter_;
    double pre_pulse_probability_;
    double late_pulse_probability_;
    double after_pulse_probability_;
    
    mutable shared_ptr<AfterPulseGenerator> afterPulseGenerator_;
    mutable shared_ptr<LatePulseGenerator> latePulseGenerator_;

    I3CalibrationConstPtr calibration_;
    I3DetectorStatusConstPtr status_;
};


I3_POINTER_TYPEDEFS(I3CLSimPMTPhotonSimulatorIceCube);

#endif //I3CLSIMPMTPHOTONSIMULATORICECUBE_H_INCLUDED
