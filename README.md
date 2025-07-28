# MDFI
# Qt Battery Defect Detection

This repository contains a Qt-based C++ application for battery defect detection. The GUI and core logic are implemented in Qt Widgets, allowing rapid deployment on any platform with Qt support. It also provides the core code for our TCE journal paper, “Advancing Industrial Battery Defect Analysis via Multi‑Domain Feature Integration and Optimized Neural Pruning for Edge System.”
[Overview (PDF)](figure/overview.pdf)


## Project Structure
├── main.cpp # Application entry point

├── mainwindow.cpp # Main window implementation

├── mainwindow.h # Main window class definition

├── mainwindow.ui # Qt Designer UI file

├── mythread.cpp # Background thread implementation

├── mythread.h # Thread class definition

├── test.cpp # Test harness / sample usage

├── qt_battery.pro # Qt project file

└── README.md # This file

## Prerequisites

- **Qt 5** or **Qt 6** (any desktop edition)
- A C++17-compatible compiler (e.g. GCC, Clang, MSVC)

## Build Instructions

1. **Install Qt**  
   Download and install the Qt SDK from [https://www.qt.io](https://www.qt.io).

2. **Open the Project**  
   In Qt Creator, open `qt_battery.pro`.

3. **Configure and Build**  
   - Select your desired kit (compiler + Qt version).  
   - Click **Build** → **Build Project** (or press **Ctrl+B**).

## Run Instructions

After a successful build, run the executable:

```bash
./qt_battery        # on Linux/macOS
qt_battery.exe      # on Windows

No additional dependencies are required—Qt handles all GUI and threading.
Usage

    Load an Image

    Click Begin. The app will:

        Locate the battery via bilateral scanning projection.

        Extract spatial‐statistical and frequency‐domain features.

        Classify the sample as “BAD” or “OK.”

    Threading
    Defect detection runs in a background thread (MyThread), ensuring the GUI remains responsive.

License

This code is released under the MIT License. See LICENSE for details.
