# Battery
# Qt Battery Defect Inspection

This repository contains a Qt-based C++ application for battery defect inspection. The GUI and core logic are implemented in Qt Widgets, allowing rapid deployment on any platform with Qt support.

## Project Structure


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
    Use the “Open” menu to select a battery sample image (e.g. BMP, PNG).

    Inspect Defects
    Click Analyze. The app will:

        Locate the battery via bilateral scanning projection.

        Extract spatial‐statistical and frequency‐domain features.

        Classify the sample as “Defective” or “OK.”

    Threading
    Defect detection runs in a background thread (MyThread), ensuring the GUI remains responsive.

License

This code is released under the MIT License. See LICENSE for details.
