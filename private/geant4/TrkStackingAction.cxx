#include "TrkStackingAction.hh"
#include "TrkUserEventInformation.hh"

#include "G4ios.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4Track.hh"
#include "G4Material.hh"
#include "G4MaterialPropertyVector.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"

#include "G4UnitsTable.hh"

#include "I3CLSimI3ParticleGeantConverter.hh"

TrkStackingAction::TrkStackingAction()
{
}

TrkStackingAction::~TrkStackingAction()
{
}

G4ClassificationOfNewTrack TrkStackingAction::ClassifyNewTrack(const G4Track * aTrack)
{
	TrkUserEventInformation* eventInformation =
	(TrkUserEventInformation*)G4EventManager::GetEventManager()
	->GetConstCurrentEvent()->GetUserInformation();

	{
        const double maxRefractiveIndex = eventInformation->maxRefractiveIndex;
        const double BetaInverse = c_light/aTrack->GetVelocity();
        
        // below the Cherekov threshold?
        if (BetaInverse > maxRefractiveIndex)
            return fKill;
    }
    
    /*
    // is it the primary particle?
    if (aTrack->GetParentID()!=0)
    {
        // it's NOT the primary! check if this new particle is inside the (extended) can
        
        const double canHeight = 1000.*m;
        const double canRadius = 750.*m;
        
        G4ThreeVector particlePosRelToCan = aTrack->GetPosition(); // - SSimDetectorConstruction::canPosition;
        
        if (fabs(particlePosRelToCan.z()) > canHeight/2.) {
            // we are above or below the can
            return fKill;
        } else {
            G4double posRadius = std::sqrt(particlePosRelToCan.x()*particlePosRelToCan.x() + particlePosRelToCan.y()*particlePosRelToCan.y());
            if (posRadius > canRadius) {
                return fKill;
            }
        }
        
        //return fUrgent;
    }
    */

    // see if there are eny parameterizations available for this particle
    const I3CLSimParticleParameterizationSeries &parameterizations = eventInformation->parameterizationAvailable;

#ifndef I3PARTICLE_SUPPORTS_PDG_ENCODINGS
    const I3Particle::ParticleType trackI3ParticleType =
    I3CLSimI3ParticleGeantConverter::ConvertPDGEncodingToI3ParticleType(aTrack->GetDefinition()->GetPDGEncoding());
#endif
    
    const G4double trackEnergy = aTrack->GetKineticEnergy();

#ifndef I3PARTICLE_SUPPORTS_PDG_ENCODINGS
    if (trackI3ParticleType==I3Particle::unknown)
    {
        // there are no parameterizations for particles unknown to IceTray
        return fUrgent;
    }
#endif
    
    for (I3CLSimParticleParameterizationSeries::const_iterator it=parameterizations.begin();
         it!=parameterizations.end(); ++it)
    {
        const I3CLSimParticleParameterization &parameterization = *it;
        
#ifndef I3PARTICLE_SUPPORTS_PDG_ENCODINGS
        if (parameterization.IsValid(trackI3ParticleType, trackEnergy*I3Units::GeV/GeV))
#else
        if (parameterization.IsValidForPdgEncoding(aTrack->GetDefinition()->GetPDGEncoding(), trackEnergy*I3Units::GeV/GeV))
#endif
        {
            shared_ptr<std::deque<boost::tuple<I3ParticleConstPtr, uint32_t, const I3CLSimParticleParameterization> > > sendToParameterizationQueue = eventInformation->sendToParameterizationQueue;

            if (!sendToParameterizationQueue) 
                log_fatal("internal error: sendToParameterizationQueue==NULL");
            
            I3ParticlePtr particle(new I3Particle());
            
            const G4ThreeVector &trackPos = aTrack->GetPosition();
            const G4double trackTime = aTrack->GetGlobalTime();
            const G4ThreeVector &trackDir = aTrack->GetMomentumDirection();

#ifdef I3PARTICLE_SUPPORTS_PDG_ENCODINGS
            particle->SetPdgEncoding(aTrack->GetDefinition()->GetPDGEncoding());
#else
            particle->SetType(trackI3ParticleType);
#endif
            particle->SetPos(trackPos.x()*I3Units::m/m,trackPos.y()*I3Units::m/m,trackPos.z()*I3Units::m/m);
            particle->SetDir(trackDir.x(),trackDir.y(),trackDir.z());
            particle->SetTime(trackTime*I3Units::ns/ns);
            particle->SetEnergy(trackEnergy*I3Units::GeV/GeV);

            //G4cout << "Geant4: sending a " << particle->GetTypeString() << " with id " << eventInformation->currentExternalParticleID << " and E=" << particle->GetEnergy()/I3Units::GeV << "GeV to a parameterization handler." << G4endl;

            sendToParameterizationQueue->push_back(boost::make_tuple(particle, eventInformation->currentExternalParticleID, parameterization));

            return fKill;
        }
    }
    
        
	return fUrgent;
}

void TrkStackingAction::NewStage()
{
	//G4cout << "New stage!" << G4endl;
}

void TrkStackingAction::PrepareNewEvent()
{
	//G4cout << "Prepare new event!" << G4endl;
}

