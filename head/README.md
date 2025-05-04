# PET Simulation – Execution Guide

This folder contains a complete simulation setup for a PET scanner using Geant4.
This file instead provides instructions on how to run the simulation.

---

## 🧠 Head Phantom – PET Simulation

This simulation models a PET scanner operating on a head phantom, including skull and brain structures.  
It is used to evaluate how detector configurations affect resolution and dose in brain imaging scenarios.


---

## 🚀 How to Run the Simulation

1. Open a terminal on **Linux**
2. Navigate to the `build/` directory:

```bash
cd build
```

3. If this is your first time running the simulation, configure the project using CMake:

```bash
cmake ..
```

4. Compile the simulation:

```bash
make
```

5. Run the simulation executable (`ExampleB3b`):

```bash
./ExampleB3b
```

This opens the Geant4 GUI. From there, you can manually enter commands like:

```bash
/run/beamOn 100
```

or run a macro file:

```bash
/control/execute run1.mac
```

---

## 📄 Available Macro Files

You’ll find several `.mac` files in this directory:

- `init_vis.mac` – Initializes visualization settings  
- `run1.mac`, `run2.mac`, `debug.mac` – Ready-to-use macros for different simulation setups  
- `vis.mac` – Optional script for interactive rendering  

They help automate detector setup, source definition, and event generation.

---

## 📂 Source Code Notes

The folder also includes:

- `src/` – Main C++ implementation of geometry, physics list, run/action classes  
- `include/` – Header files for modular structure  
- `CMakeLists.txt` – Build configuration  

---

📌 **Note**: This simulation is meant to run on Linux with Geant4 properly installed and configured (including visualization support).
