//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file Run.cc
/// \brief Implementation of the B3b::Run class

#include "Run.hh"

#include "G4RunManager.hh"
#include "G4Event.hh"

#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4THitsMap.hh"
#include "G4SystemOfUnits.hh"

namespace B3b
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Run::Run()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Run::~Run()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::RecordEvent(const G4Event* event)
{
  if ( fCollID_cryst < 0 ) {
   fCollID_cryst
     = G4SDManager::GetSDMpointer()->GetCollectionID("crystal/edep");
   //G4cout << " fCollID_cryst: " << fCollID_cryst << G4endl;
  }

  if ( fCollID_leftLung < 0 ) {
   fCollID_leftLung
     = G4SDManager::GetSDMpointer()->GetCollectionID("leftLung/doseLeftLung");
   //G4cout << " fCollID_patient: " << fCollID_patient << G4endl;
  }

if ( fCollID_rightLung < 0 ) {
   fCollID_rightLung
     = G4SDManager::GetSDMpointer()->GetCollectionID("rightLung/doseRightLung");
   //G4cout << " fCollID_patient: " << fCollID_patient << G4endl;
  }

if ( fCollID_heart < 0 ) {
   fCollID_heart
     = G4SDManager::GetSDMpointer()->GetCollectionID("heart/doseHeart");
   //G4cout << " fCollID_patient: " << fCollID_patient << G4endl;
  }

if ( fCollID_ribs < 0 ) {
   fCollID_ribs
     = G4SDManager::GetSDMpointer()->GetCollectionID("ribs/doseRibs");
   //G4cout << " fCollID_patient: " << fCollID_patient << G4endl;
  }

if ( fCollID_ribCage < 0 ) {
   fCollID_ribCage
     = G4SDManager::GetSDMpointer()->GetCollectionID("ribCage/doseRibCage");
   //G4cout << " fCollID_patient: " << fCollID_patient << G4endl;
  }


  G4int evtNb = event->GetEventID();

  if (evtNb%fPrintModulo == 0) {
    G4cout << G4endl << "---> end of event: " << evtNb << G4endl;
  }

  //Hits collections
  //
  G4HCofThisEvent* HCE = event->GetHCofThisEvent();
  if(!HCE) return;

  //Energy in crystals : identify 'good events'
  //
  const G4double eThreshold = 500*keV;
  G4int nbOfFired = 0;

  G4THitsMap<G4double>* evtMap =
    static_cast<G4THitsMap<G4double>*>(HCE->GetHC(fCollID_cryst));

  std::map<G4int,G4double*>::iterator itr;
  for (itr = evtMap->GetMap()->begin(); itr != evtMap->GetMap()->end(); itr++) {
    G4double edep = *(itr->second);
    if (edep > eThreshold) nbOfFired++;
    ///G4int copyNb  = (itr->first);
    ///G4cout << G4endl << "  cryst" << copyNb << ": " << edep/keV << " keV ";
  }
  if (nbOfFired == 2) fGoodEvents++;

  //Dose deposit in patient
  //
  G4double doseLeftLung = 0.;

  evtMap = static_cast<G4THitsMap<G4double>*>(HCE->GetHC(fCollID_leftLung));

  for (itr = evtMap->GetMap()->begin(); itr != evtMap->GetMap()->end(); itr++) {
    ///G4int copyNb  = (itr->first);
    doseLeftLung = *(itr->second);
  }
  fSumDoseLeftLung += doseLeftLung;
  fStatDoseLeftLung += doseLeftLung;

  G4Run::RecordEvent(event);

  G4double doseRightLung = 0.;

  evtMap = static_cast<G4THitsMap<G4double>*>(HCE->GetHC(fCollID_rightLung));

  for (itr = evtMap->GetMap()->begin(); itr != evtMap->GetMap()->end(); itr++) {
    ///G4int copyNb  = (itr->first);
    doseRightLung = *(itr->second);
  }
  fSumDoseRightLung += doseRightLung;
  fStatDoseRightLung += doseRightLung;

  G4Run::RecordEvent(event);

  G4double doseHeart = 0.;

  evtMap = static_cast<G4THitsMap<G4double>*>(HCE->GetHC(fCollID_heart));

  for (itr = evtMap->GetMap()->begin(); itr != evtMap->GetMap()->end(); itr++) {
    ///G4int copyNb  = (itr->first);
    doseHeart = *(itr->second);
  }
  fSumDoseHeart += doseHeart;
  fStatDoseHeart += doseHeart;

  G4Run::RecordEvent(event);

  G4double doseRibs = 0.;

  evtMap = static_cast<G4THitsMap<G4double>*>(HCE->GetHC(fCollID_ribs));

  for (itr = evtMap->GetMap()->begin(); itr != evtMap->GetMap()->end(); itr++) {
    ///G4int copyNb  = (itr->first);
    doseRibs = *(itr->second);
  }
  fSumDoseRibs += doseRibs;
  fStatDoseRibs += doseRibs;

  G4Run::RecordEvent(event);

  G4double doseRibCage = 0.;

  evtMap = static_cast<G4THitsMap<G4double>*>(HCE->GetHC(fCollID_ribCage));

  for (itr = evtMap->GetMap()->begin(); itr != evtMap->GetMap()->end(); itr++) {
    ///G4int copyNb  = (itr->first);
    doseRibCage = *(itr->second);
  }
  fSumDoseRibCage += doseRibCage;
  fStatDoseRibCage += doseRibCage;

  G4Run::RecordEvent(event);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::Merge(const G4Run* aRun)
{
  const Run* localRun = static_cast<const Run*>(aRun);
  fGoodEvents += localRun->fGoodEvents;
  fSumDoseLeftLung    += localRun->fSumDoseLeftLung ;
  fStatDoseLeftLung    += localRun->fStatDoseLeftLung ;
  fSumDoseRightLung     += localRun->fSumDoseRightLung;
  fStatDoseRightLung   += localRun->fStatDoseRightLung;
  fSumDoseHeart    += localRun->fSumDoseHeart ;
  fStatDoseHeart    += localRun->fStatDoseHeart ;
  fSumDoseRibs    += localRun->fSumDoseRibs;
  fStatDoseRibs   += localRun->fStatDoseRibs;
  fSumDoseRibCage    += localRun->fSumDoseRibCage;
  fStatDoseRibCage   += localRun->fStatDoseRibCage;
  G4Run::Merge(aRun);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}

