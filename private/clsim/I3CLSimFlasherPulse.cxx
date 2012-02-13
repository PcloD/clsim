#include <clsim/I3CLSimFlasherPulse.h>

#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>



// construction & destruction
I3CLSimFlasherPulse::I3CLSimFlasherPulse()
:
flasherPulseType_(I3CLSimFlasherPulse::Unknown),
time_(NAN),
numberOfPhotonsNoBias_(NAN),
pulseWidth_(NAN),
angularEmissionSigmaPolar_(NAN),
angularEmissionSigmaAzimuthal_(NAN)
{
    
}



I3CLSimFlasherPulse::~I3CLSimFlasherPulse() 
{ 
    
}


template <class Archive>
void I3CLSimFlasherPulse::serialize(Archive &ar, unsigned version)
{
	if (version > i3clsimflasherpulse_version_)
		log_fatal("Attempting to read version %u from file but running version %u of I3CLSimFlasherPulse class.",version,i3clsimflasherpulse_version_);
	
	ar & make_nvp("flasherPulseType", flasherPulseType_);
	ar & make_nvp("dir", dir_);
	ar & make_nvp("pos", pos_);
	ar & make_nvp("time", time_);
	ar & make_nvp("numberOfPhotonsNoBias", numberOfPhotonsNoBias_);
	ar & make_nvp("pulseWidth", pulseWidth_);
	ar & make_nvp("angularEmissionSigmaPolar", angularEmissionSigmaPolar_);
	ar & make_nvp("angularEmissionSigmaAzimuthal", angularEmissionSigmaAzimuthal_);
}     

I3_SERIALIZABLE(I3CLSimFlasherPulse);
I3_SERIALIZABLE(I3CLSimFlasherPulseSeries);


// comparison
bool operator==(const I3CLSimFlasherPulse &a, const I3CLSimFlasherPulse &b)
{
    if (a.flasherPulseType_ != b.flasherPulseType_) return false;

    if (a.dir_.GetZenith() != b.dir_.GetZenith()) return false;
    if (a.dir_.GetAzimuth() != b.dir_.GetAzimuth()) return false;
    if (a.pos_.GetX() != b.pos_.GetX()) return false;
    if (a.pos_.GetY() != b.pos_.GetY()) return false;
    if (a.pos_.GetZ() != b.pos_.GetZ()) return false;
    
    if (isnan(a.time_) || isnan(b.time_)) {
        if ((!isnan(a.time_)) || (!isnan(b.time_))) return false;
    } else {
        if (a.time_ != b.time_) return false;
    }
    
    if (isnan(a.numberOfPhotonsNoBias_) || isnan(b.numberOfPhotonsNoBias_)) {
        if ((!isnan(a.numberOfPhotonsNoBias_)) || (!isnan(b.numberOfPhotonsNoBias_))) return false;
    } else {
        if (a.numberOfPhotonsNoBias_ != b.numberOfPhotonsNoBias_) return false;
    }
    
    if (isnan(a.pulseWidth_) || isnan(b.pulseWidth_)) {
        if ((!isnan(a.pulseWidth_)) || (!isnan(b.pulseWidth_))) return false;
    } else {
        if (a.pulseWidth_ != b.pulseWidth_) return false;
    }

    if (isnan(a.angularEmissionSigmaPolar_) || isnan(b.angularEmissionSigmaPolar_)) {
        if ((!isnan(a.angularEmissionSigmaPolar_)) || (!isnan(b.angularEmissionSigmaPolar_))) return false;
    } else {
        if (a.angularEmissionSigmaPolar_ != b.angularEmissionSigmaPolar_) return false;
    }

    if (isnan(a.angularEmissionSigmaAzimuthal_) || isnan(b.angularEmissionSigmaAzimuthal_)) {
        if ((!isnan(a.angularEmissionSigmaAzimuthal_)) || (!isnan(b.angularEmissionSigmaAzimuthal_))) return false;
    } else {
        if (a.angularEmissionSigmaAzimuthal_ != b.angularEmissionSigmaAzimuthal_) return false;
    }

    
    return true;
}

