
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
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Ellipsoid.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4EllipticalTube.hh"
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


 // rib cage

/*
  G4Material* soft = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");  

  G4double dx= 17. *cm; // a2
  G4double dy= 9.8 * cm; //b2
  G4double thickness= 32.4 * cm; // z2/2 of cage

 G4EllipticalTube* outCage = new G4EllipticalTube("outCage",dx, dy, thickness/2.);

  dx = 16.4 * cm; // a1
  dy = 9.2 * cm; // b1
  G4double dz = 34. *cm; // z2/2

  G4EllipticalTube* inCage = new G4EllipticalTube("inCage",dx, dy, dz/2.);

  G4SubtractionSolid* cage = new G4SubtractionSolid("Cage",
						    outCage,
						    inCage, 0, G4ThreeVector(0.*cm, 0.*cm, 0. * cm)); 
*/
 
  G4Material* soft = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");  

  G4double dx= 17. *cm; // a2
  G4double dy= 9.8 * cm; //b2
  G4double thickness= 32.4 * cm; // z2/2 of cage

  G4EllipticalTube* cage = new G4EllipticalTube("outCage",dx, dy, thickness/2.);
  G4LogicalVolume* logicRibCage = new G4LogicalVolume(cage, soft, "logicalCage", 0, 0, 0);

/*  G4VPhysicalVolume* physRibCage =*/new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, 0.0),
						     // with respect to the trunk
						     logicRibCage,
						     "physicalRibCage",
						     logicWorld,
						     false,
						     0, fCheckOverlaps);

  //heart

  G4Material* heart_mat = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");

  G4double ax= 4.00* cm;
  G4double by= 4.00 *cm;
  G4double cz= 7.00 *cm;
  G4double zcut1= -7.00 *cm;
  G4double zcut2= 0.0 *cm;
  
  G4Ellipsoid* heart1 =  new G4Ellipsoid("Heart1",ax, by, cz, zcut1, zcut2);
  
  G4double rmin =0.*cm;
  G4double rmax = 3.99*cm;
  G4double startphi = 0. * degree;
  G4double deltaphi = 360. * degree;
  G4double starttheta = 0. * degree;
  G4double deltatheta = 90. * degree;
 
  G4Sphere* heart2 = new G4Sphere("Heart2", rmin,rmax,
				  startphi,   deltaphi,
				  starttheta, deltatheta);

  G4UnionSolid* heart = new G4UnionSolid("Heart", heart1, heart2);

  G4LogicalVolume* logicHeart = new G4LogicalVolume(heart, heart_mat,
						    "HeartVolume",
						    0, 0, 0);
 

  // Define rotation and position here!
  G4RotationMatrix rm = G4RotationMatrix();
  rm.rotateY(25*degree);
  G4ThreeVector positionHeart = G4ThreeVector(0.0, -3.0*cm,0.0);
  G4Transform3D transformHeart = G4Transform3D(rm,positionHeart);
  /*G4VPhysicalVolume* physHeart =*/ new G4PVPlacement(transformHeart,
						   logicHeart,
						   "physicalHeart",
						   logicRibCage,
						   false,
						   0,
                                                   fCheckOverlaps);




 
// ribs
  G4Material* skeleton = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");  

  G4double xx = 17.*cm;
  G4double yy = 9.8*cm;
  G4double ribThickness = 1.4*cm;
  G4EllipticalTube* rib_out = new G4EllipticalTube("rib_out",xx, yy, ribThickness/2.);	
  
  xx = 16.5 *cm;
  yy = 9.3 * cm;
  G4double zz = 1.5 * cm;  
  G4EllipticalTube* rib_in = new G4EllipticalTube("rib_in",xx, yy, zz/2.);
  G4SubtractionSolid* rib = new G4SubtractionSolid("rib",rib_out, rib_in);

  G4LogicalVolume* logicRib= new G4LogicalVolume(rib, skeleton, "logicalRib", 0, 0, 0);

 /* G4VPhysicalVolume*  physRib1 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (- 32.2*cm/2. + 0.8 *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib2 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, ( - 32.2*cm/2. + 0.8 *cm + 2.8 *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib3 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 * cm + 5.6 *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib4 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 * cm + 8.4 *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib5 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 * cm + 11.2*cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib6 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. +  0.8 * cm + 14. *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib7 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 *cm + 16.8 *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib8 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 *cm + 19.6 *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib9 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8*cm + 22.4 *cm)),
			       // with respect to the trunk
			       logicRib,
			       "physicalRib",
			       logicRibCage,
			       false,
			       0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib10 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8*cm + 25.2 *cm)),
				// with respect to the trunk
				logicRib,
			       "physicalRib",
				logicRibCage,
				false,
				0, fCheckOverlaps);

/*  G4VPhysicalVolume* physRib11 =*/ new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8*cm + 28. *cm)),
				// with respect to the trunk
				logicRib,
			       "physicalRib",
				logicRibCage,
				false,
				0, fCheckOverlaps);

/*  G4VPhysicalVolume*  physRib12 = */new G4PVPlacement(0,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8*cm + 30.8 *cm)),
				// with respect to the trunk
				logicRib,
			       "physicalRib",
				logicRibCage,
				false,
				0, fCheckOverlaps);
  

//POLMONE SINISTRO

  ax = 5. *cm; //a
  by = 7.5 *cm; //b
  cz = 24.*cm; //c
  zcut1 = 0.0 *cm; 
  zcut2=24. *cm;
  dx = 5.5* cm;
  dy = 8.5 * cm;
  G4double dz = 24. * cm;

  G4Material* lung_material = nist->FindOrBuildMaterial("G4_LUNG_ICRP");

 auto lung2 = new G4Ellipsoid("OneLung",ax, by, cz, zcut1,zcut2);
/*
  ax= 5.*cm; 
  by= 7.5*cm; 
  cz= 24.*cm;


  auto subtrLung = new G4Ellipsoid("subtrLung",ax, by, cz);

  // y<0



  auto box = new G4Box("Box", dx, dy, dz);
 

  //G4SubtractionSolid* section = new G4SubtractionSolid("BoxSub", subtrLung, box, 0, G4ThreeVector(0.*cm, 8.5* cm, 0.*cm)); 
  auto section2 = new G4SubtractionSolid("BoxSub2", subtrLung, box, nullptr, G4ThreeVector(0.*cm, 8.5* cm,  0.*cm)); 

  //G4SubtractionSolid* lung1 =  new G4SubtractionSolid("Lung1", oneLung,
  //				       section,
  //				       0, G4ThreeVector(6.*cm,0*cm,0.0*cm));
 
  auto lung2 =  new G4SubtractionSolid("Lung2", oneLung,
						      section2,
						      nullptr, G4ThreeVector(-6.*cm,0*cm,0.0*cm));
*/
  // G4RotationMatrix* matrix = new G4RotationMatrix();  
  // matrix->rotateX(180. * degree);
  //matrix ->rotateZ(180.*degree);
  //matrix -> rotateY(180.* degree);

  //G4UnionSolid* lungs = new G4UnionSolid("Lungs", lung1, lung2, matrix, G4ThreeVector(17*cm, 0., 0.));


  auto logicLeftLung = new G4LogicalVolume(lung2,lung_material,
						       "Lung2"); 
 
//auto* logicLeftLung = new G4LogicalVolume(lung2,lung_material,
						       //"logical" + volumeName, nullptr, nullptr, nullptr); cosa hai modificat0
 //G4VPhysicalVolume* physLeftLung =

    new G4PVPlacement(nullptr,
        G4ThreeVector(8.50 *cm, 0.0*cm, 8.5*cm-15.32*cm),
	logicLeftLung,
        "Lung2",
	logicRibCage,					      
        false,					
        0,
        fCheckOverlaps);                                                                                    	

//POLMONE DESTRO 
 
  auto twoLung = new G4Ellipsoid("TwoLung",ax, by, cz, zcut1,zcut2);

  auto subtrLungtwo = new G4Ellipsoid("subtrLungtwo",ax, by, cz);

  // y<0

  auto boxtwo = new G4Box("Boxtwo", dx, dy, dz);
 
  auto sectiontwo = new G4SubtractionSolid("BoxSubtwo", subtrLungtwo, boxtwo, nullptr, G4ThreeVector(0.*cm, 8.5* cm, 0.*cm)); 
  //G4SubtractionSolid* section2 = new G4SubtractionSolid("BoxSub2", subtrLung, box, 0, G4ThreeVector(0.*cm, -8.5* cm, 0.*cm)); 

  auto lung1 =  new G4SubtractionSolid("Lung1", twoLung,
					  sectiontwo,
				          nullptr, G4ThreeVector(6.*cm,0*cm,0.0*cm));
 
  auto logicRightLung = new G4LogicalVolume(lung1,lung_material,
							"Lung1"); 
 
  auto rmLungDX = new G4RotationMatrix();
  rmLungDX->rotateZ(-360.*degree);

new G4PVPlacement(rmLungDX,G4ThreeVector(-8.50 *cm, 0.0*cm, 8.5*cm-15.32*cm),
						       logicRightLung,                    
						       "Lung1",
						       logicRibCage,
						       false,
						       0,fCheckOverlaps);




  // Visualization attributes
  //
  logicRing->SetVisAttributes (G4VisAttributes::GetInvisible());
  logicDetector->SetVisAttributes (G4VisAttributes::GetInvisible());

  // definisco colori
  G4VisAttributes * col_heart = new G4VisAttributes(G4Colour(1.0,0.0,0.0));
  col_heart -> SetVisibility (true);
  col_heart-> SetForceSolid (true);
  G4VisAttributes * col_lungs = new G4VisAttributes(G4Colour(1.0,0.8,0.8));
  col_lungs -> SetVisibility (true);
  col_lungs-> SetForceSolid (true);
  G4VisAttributes * col_ribs = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  col_ribs -> SetVisibility (true);
  col_ribs-> SetForceSolid (true);
  G4VisAttributes * col_det = new G4VisAttributes(G4Colour(0.5,0.5,0.5));
  col_det -> SetVisibility (true);
  col_det-> SetForceSolid (true);

  //colore visibile
  logicHeart -> SetVisAttributes(col_heart);
  logicRightLung -> SetVisAttributes(col_lungs);
  logicLeftLung -> SetVisAttributes(col_lungs);
  logicRib -> SetVisAttributes(col_ribs);
//logicCryst -> SetVisAttributes(col_det);

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
  G4MultiFunctionalDetector* leftLung = new G4MultiFunctionalDetector("leftLung");
  G4SDManager::GetSDMpointer()->AddNewDetector(leftLung);
  G4VPrimitiveScorer* primitiv2 = new G4PSDoseDeposit("doseLeftLung");
  leftLung->RegisterPrimitive(primitiv2);
  SetSensitiveDetector("Lung2",leftLung);

  G4MultiFunctionalDetector* rightLung= new G4MultiFunctionalDetector("rightLung");
  G4SDManager::GetSDMpointer()->AddNewDetector(rightLung);
  G4VPrimitiveScorer* primitiv3 = new G4PSDoseDeposit("doseRightLung");
  rightLung->RegisterPrimitive(primitiv3);
  SetSensitiveDetector("Lung1",rightLung);

  G4MultiFunctionalDetector* heart = new G4MultiFunctionalDetector("heart");
  G4SDManager::GetSDMpointer()->AddNewDetector(heart);
  G4VPrimitiveScorer* primitiv4 = new G4PSDoseDeposit("doseHeart");
  heart->RegisterPrimitive(primitiv4);
  SetSensitiveDetector("HeartVolume",heart);

  G4MultiFunctionalDetector* ribs = new G4MultiFunctionalDetector("ribs");
  G4SDManager::GetSDMpointer()->AddNewDetector(ribs);
  G4VPrimitiveScorer* primitiv5 = new G4PSDoseDeposit("doseRibs");
  ribs->RegisterPrimitive(primitiv5);
  SetSensitiveDetector("logicalRib",ribs);

  G4MultiFunctionalDetector* ribCage = new G4MultiFunctionalDetector("ribCage");
  G4SDManager::GetSDMpointer()->AddNewDetector(ribCage);
  G4VPrimitiveScorer* primitiv6 = new G4PSDoseDeposit("doseRibCage");
  ribCage->RegisterPrimitive(primitiv6);
  SetSensitiveDetector("logicalCage",ribCage);

 //  logicRibCage

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}

