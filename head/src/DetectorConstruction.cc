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
/// \file DetectorConstruction.cc
/// \brief Implementation of the B3::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4NistManager.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSDoseDeposit.hh"
#include "G4VisAttributes.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4Ellipsoid.hh"
#include "G4EllipticalTube.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"

namespace B3
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
{
  DefineMaterials();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{
  G4NistManager* man = G4NistManager::Instance();

  G4bool isotopes = false;

  G4Element*  O = man->FindOrBuildElement("O" , isotopes);
  G4Element* Si = man->FindOrBuildElement("Si", isotopes);
  G4Element* Lu = man->FindOrBuildElement("Lu", isotopes);

  G4Material* LSO = new G4Material("Lu2SiO5", 7.4*g/cm3, 3);
  LSO->AddElement(Lu, 2);
  LSO->AddElement(Si, 1);
  LSO->AddElement(O , 5);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Gamma detector Parameters
  //
  G4double cryst_dX = 6*cm, cryst_dY = 6*cm, cryst_dZ = 3*cm;
  G4int nb_cryst = 32;
  G4int nb_rings = 9;
  //
  G4double dPhi = twopi/nb_cryst, half_dPhi = 0.5*dPhi;
  G4double cosdPhi = std::cos(half_dPhi);
  G4double tandPhi = std::tan(half_dPhi);
  //
  G4double ring_R1 = 0.5*cryst_dY/tandPhi;
  G4double ring_R2 = (ring_R1+cryst_dZ)/cosdPhi;
  //
  G4double detector_dZ = nb_rings*cryst_dX;
  //
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* default_mat = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* cryst_mat   = nist->FindOrBuildMaterial("Lu2SiO5");

  //
  // World
  //
  G4double world_sizeXY = 2.4*ring_R2;
  G4double world_sizeZ  = 1.2*detector_dZ;

  G4Box* solidWorld =
    new G4Box("World",                       //its name
       0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ); //its size

  G4LogicalVolume* logicWorld =
    new G4LogicalVolume(solidWorld,          //its solid
                        default_mat,         //its material
                        "World");            //its name

  G4VPhysicalVolume* physWorld =
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      fCheckOverlaps);       // checking overlaps

  //
  // ring
  //
  G4Tubs* solidRing =
    new G4Tubs("Ring", ring_R1, ring_R2, 0.5*cryst_dX, 0., twopi);

  G4LogicalVolume* logicRing =
    new G4LogicalVolume(solidRing,           //its solid
                        default_mat,         //its material
                        "Ring");             //its name

  //
  // define crystal
  //
  G4double gap = 0.5*mm;        //a gap for wrapping
  G4double dX = cryst_dX - gap, dY = cryst_dY - gap;
  G4Box* solidCryst = new G4Box("crystal", dX/2, dY/2, cryst_dZ/2);

  G4LogicalVolume* logicCryst =
    new G4LogicalVolume(solidCryst,          //its solid
                        cryst_mat,           //its material
                        "CrystalLV");        //its name

  // place crystals within a ring
  //
  for (G4int icrys = 0; icrys < nb_cryst ; icrys++) {
    G4double phi = icrys*dPhi;
    G4RotationMatrix rotm  = G4RotationMatrix();
    rotm.rotateY(90*deg);
    rotm.rotateZ(phi);
    G4ThreeVector uz = G4ThreeVector(std::cos(phi),  std::sin(phi),0.);
    G4ThreeVector position = (ring_R1+0.5*cryst_dZ)*uz;
    G4Transform3D transform = G4Transform3D(rotm,position);

    new G4PVPlacement(transform,             //rotation,position
                      logicCryst,            //its logical volume
                      "crystal",             //its name
                      logicRing,             //its mother  volume
                      false,                 //no boolean operation
                      icrys,                 //copy number
                      fCheckOverlaps);       // checking overlaps
  }

  //
  // full detector
  //
  G4Tubs* solidDetector =
    new G4Tubs("Detector", ring_R1, ring_R2, 0.5*detector_dZ, 0., twopi);

  G4LogicalVolume* logicDetector =
    new G4LogicalVolume(solidDetector,       //its solid
                        default_mat,         //its material
                        "Detector");         //its name

  //
  // place rings within detector
  //
  G4double OG = -0.5*(detector_dZ + cryst_dX);
  for (G4int iring = 0; iring < nb_rings ; iring++) {
    OG += cryst_dX;
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(0,0,OG), //position
                      logicRing,             //its logical volume
                      "ring",                //its name
                      logicDetector,         //its mother  volume
                      false,                 //no boolean operation
                      iring,                 //copy number
                      fCheckOverlaps);       // checking overlaps
  }

  //
  // place detector in world
  //
  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(),         //at (0,0,0)
                    logicDetector,           //its logical volume
                    "Detector",              //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    fCheckOverlaps);         // checking overlaps

  //
  // patient
  //

  G4Material* patient_mat = nist->FindOrBuildMaterial("G4_BRAIN_ICRP");

  G4double ax = 6. * cm;
  G4double by= 9. * cm;
  G4double cz= 6.5 * cm;
  G4Ellipsoid* solidPatient = new G4Ellipsoid("Patient", ax, by, cz);


  G4LogicalVolume* logicPatient =
    new G4LogicalVolume(solidPatient,        //its solid
                        patient_mat,         //its material
                        "PatientLV");        //its name


  //
  // skull
  //

  G4Material* bone_mat = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
 
  G4Ellipsoid* craniumIn =  new G4Ellipsoid("CraniumIn", ax, by, cz);
 
  ax = 6.8 * cm;//a out skull
  by = 9.8 * cm; // bout
  cz = 8.3 * cm; //cout
 
  G4Ellipsoid* craniumOut =  new G4Ellipsoid("CraniumOut", ax, by, cz);

 

  G4SubtractionSolid* cranium =  new G4SubtractionSolid("Cranium",
							craniumOut,
							craniumIn,0,
							G4ThreeVector(0.0, 0.0,1. * cm));

  G4LogicalVolume* logicSkull = new G4LogicalVolume(cranium, bone_mat, 
						    "skullLV");

  //
  // place patient in world
  //
  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(),         //at (0,0,0)
                    logicPatient,            //its logical volume
                    "Patient",               //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    fCheckOverlaps);         // checking overlaps

  //
  // place skull in world
  //
  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(),         //at (0,0,0)
                    logicSkull,            //its logical volume
                    "Skull",               //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    fCheckOverlaps);         // checking overlaps



  // Visualization attributes
  //
  logicRing->SetVisAttributes (G4VisAttributes::GetInvisible());
  logicDetector->SetVisAttributes (G4VisAttributes::GetInvisible());


  // definisco colori
  G4VisAttributes * col_patient = new G4VisAttributes(G4Colour(1.0,0.8,0.8));
  col_patient -> SetVisibility (true);
  col_patient-> SetForceSolid (true);
  G4VisAttributes * col_skull = new G4VisAttributes(G4Colour(1.0,1.0,1.0,0.3));
  col_skull -> SetVisibility (true);
  col_skull-> SetForceSolid (true);
  G4VisAttributes * col_det = new G4VisAttributes(G4Colour(0.75,0.75,0.75,0.5));
  col_det -> SetVisibility (true);
  col_det-> SetForceSolid (true);


  //colore visibile
  logicPatient -> SetVisAttributes(col_patient);
  logicSkull -> SetVisAttributes(col_skull);
  logicCryst -> SetVisAttributes(col_det);

  // Print materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;

  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
  G4SDManager::GetSDMpointer()->SetVerboseLevel(1);

  // declare crystal as a MultiFunctionalDetector scorer
  //
  G4MultiFunctionalDetector* cryst = new G4MultiFunctionalDetector("crystal");
  G4SDManager::GetSDMpointer()->AddNewDetector(cryst);
  G4VPrimitiveScorer* primitiv1 = new G4PSEnergyDeposit("edep");
  cryst->RegisterPrimitive(primitiv1);
  SetSensitiveDetector("CrystalLV",cryst);

  // declare patient as a MultiFunctionalDetector scorer
  //
  G4MultiFunctionalDetector* patient = new G4MultiFunctionalDetector("patient");
  G4SDManager::GetSDMpointer()->AddNewDetector(patient);
  G4VPrimitiveScorer* primitiv2 = new G4PSDoseDeposit("dose");
  patient->RegisterPrimitive(primitiv2);
  SetSensitiveDetector("PatientLV",patient);

  // declare patient as a MultiFunctionalDetector scorer
  //
  G4MultiFunctionalDetector* skull = new G4MultiFunctionalDetector("skull");
  G4SDManager::GetSDMpointer()->AddNewDetector(skull);
  G4VPrimitiveScorer* primitiv3 = new G4PSDoseDeposit("doseSkull");
  skull->RegisterPrimitive(primitiv3);
  SetSensitiveDetector("skullLV",skull);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}

