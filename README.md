# 🧠🫁 PET Simulation Project – Geant4 for Clinical Instrumentation

This repository contains a project developed for the *Technologies for Sensors and Clinical Instrumentation* course at Politecnico di Milano.  
It includes two full **Geant4-based PET simulations** designed to evaluate how different detector geometries influence the quality of image reconstruction and the efficiency of good interaction detection.

🎯 The ultimate goal is to **maximize image quality** while **minimizing the radiation dose** delivered to the patient.

---

## 🧪 Project Overview

Two anatomical scenarios are simulated:

- **Head phantom** → skull and brain  
- **Thorax phantom** → rib cage, lungs, heart, and soft tissues  

Each simulation is used to test PET scanner configurations with varying:

- **Detector materials** (e.g., BGO, LYSO, LFS, etc.)
- **Crystal geometry**
- **Detector layout**
- **Source position**

The project evaluates how these variables affect:

- Detection rate of true coincidence events
- Absorbed dose
- Image reconstruction potential

---

## ⚙️ System Requirements

- ✅ **Operating system**: Linux only (due to Geant4 dependencies)
- ❌ Not runnable on Windows without complex porting
- 💡 Requires Geant4 installed and configured with visualization enabled

---

## 📁 Repository Structure

```plaintext
technologies-for-sensors-and-clinical-instrumentation/
├── head/ <- Simulation for head phantom (PET + skull + brain)
│ ├── build/ <- Executable compiled via CMake
│ ├── src/ <- Source code (not explained here for brevity)
│ ├── (..)
├── thorax/ <- Simulation for thoracic phantom (PET + ribs + heart + lungs)
│ ├── build/ <- Executable compiled via CMake
│ ├── src/ <- Source code (not explained here for brevity)
│ ├── (..)
├── presentation/ <- PowerPoint summarizing methodology and results
└── README.md <- General overview and usage instructions (this file)
```

---

## 📄 Source Code Notes

Each simulation folder (`head/` and `thorax/`) includes a `src/` directory containing all C++ source files used for modeling geometry, materials, physics processes, and run management.

⚠️ The source code is **not detailed** in this README for brevity.  
Refer to the `.cc` and `.hh` files in `src/` if you wish to inspect the implementation.

---

## 📊 Summary of Results

The project explored different geometries and materials with key findings such as:

- **Thicker crystals** generally improve the detection of 511 keV gamma photons
- **More detectors per ring** tend to reduce efficiency but can improve resolution
- **Detector material** significantly affects performance: BGO gave the best simulation results
- The **thorax model** presented higher dose variation due to structural complexity

These insights can guide future PET design strategies.

---

## 👥 Authors

- **Samuele Scortechini** – [samuele.scortechini@mail.polimi.it](mailto:samuele.scortechini@mail.polimi.it) – [LinkedIn](https://www.linkedin.com/in/samuele-scortechini-12june2001/)
- Andrea Baroni  
- Anna Emilia Candela  
- Petra Cipolla  
- Francesca Di Bella  
- Fabiana Zummo  

*Politecnico di Milano – DEIB*  
Academic Year 2024–2025