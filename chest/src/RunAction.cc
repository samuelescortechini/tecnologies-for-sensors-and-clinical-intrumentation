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
/// \file RunAction.cc
/// \brief Implementation of the B3b::RunAction class

#include "RunAction.hh"
#include "Run.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

using namespace B3;

namespace B3b
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction()
{
  //add new units for dose
  //
  const G4double milligray = 1.e-3*gray;
  const G4double microgray = 1.e-6*gray;
  const G4double nanogray  = 1.e-9*gray;
  const G4double picogray  = 1.e-12*gray;

  new G4UnitDefinition("milligray", "milliGy" , "Dose", milligray);
  new G4UnitDefinition("microgray", "microGy" , "Dose", microgray);
  new G4UnitDefinition("nanogray" , "nanoGy"  , "Dose", nanogray);
  new G4UnitDefinition("picogray" , "picoGy"  , "Dose", picogray);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* RunAction::GenerateRun()
{ return new Run; }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* run)
{
  G4cout << "### Run " << run->GetRunID() << " start." << G4endl;

  //inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  // Run conditions
  //  note: There is no primary generator action object for "master"
  //        run manager for multi-threaded mode.
  const PrimaryGeneratorAction* generatorAction
    = static_cast<const PrimaryGeneratorAction*>(
        G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String partName;
  if (generatorAction)
  {
    G4ParticleDefinition* particle
      = generatorAction->GetParticleGun()->GetParticleDefinition();
    partName = particle->GetParticleName();
  }

  //results
  //
  const Run* b3Run = static_cast<const Run*>(run);
  G4int nbGoodEvents = b3Run->GetNbGoodEvents();
  G4double sumDoseLeftLung   = b3Run->GetSumDoseLeftLung();
  G4StatAnalysis statDoseLeftLung = b3Run->GetStatDoseLeftLung();
  G4double sumDoseRightLung   = b3Run->GetSumDoseRightLung();
  G4StatAnalysis statDoseRightLung = b3Run->GetStatDoseRightLung();
  G4double sumDoseHeart   = b3Run->GetSumDoseHeart();
  G4StatAnalysis statDoseHeart = b3Run->GetStatDoseHeart();
  G4double sumDoseRibs   = b3Run->GetSumDoseRibs();
  G4StatAnalysis statDoseRibs = b3Run->GetStatDoseRibs();
  G4double sumDoseRibCage   = b3Run->GetSumDoseRibCage();
  G4StatAnalysis statDoseRibCage = b3Run->GetStatDoseRibCage();

  //print
  //
  if (IsMaster())
  {
    G4cout
     << G4endl
     << "--------------------End of Global Run-----------------------"
     << G4endl
     << "  The run was " << nofEvents << " events ";
  }
  else
  {
    G4cout
     << G4endl
     << "--------------------End of Local Run------------------------"
     << G4endl
     << "  The run was " << nofEvents << " "<< partName;
  }
  statDoseLeftLung /= gray;
  statDoseRightLung /= gray;
  statDoseHeart /= gray;
  statDoseRibs /= gray;
  statDoseRibCage /= gray;
  G4cout
     << "; Nb of 'good' e+ annihilations: " << nbGoodEvents  << G4endl
     << " Total dose in the left lung : " << G4BestUnit(sumDoseLeftLung, "Dose") << G4endl
     << " Total dose in the left lung  : " << statDoseLeftLung << " Gy" << G4endl
     << " Total dose in the right lung : " << G4BestUnit(sumDoseRightLung, "Dose") << G4endl
     << " Total dose in the right lung  : " << statDoseRightLung << " Gy" << G4endl
     << " Total dose in the heart : " << G4BestUnit(sumDoseHeart, "Dose") << G4endl
     << " Total dose in the heart : " << statDoseHeart << " Gy" << G4endl
     << " Total dose in the ribs : " << G4BestUnit(sumDoseRibs, "Dose") << G4endl
     << " Total dose in the ribs : " << statDoseRibs << " Gy" << G4endl
     << " Total dose in the ribcage : " << G4BestUnit(sumDoseRibCage, "Dose") << G4endl
     << " Total dose in the ribcage : " << statDoseRibCage << " Gy" << G4endl
     << "------------------------------------------------------------" << G4endl
     << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}

