# Installation Guide

Quick installation instructions for all supported platforms.

## macOS

### Step 1: Install Dependencies
```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required packages
brew install cmake ninja glfw
```

### Step 2: Clone and Build
```bash
git clone https://github.com/cebdan/driver-solution-cad.git
cd driver-solution-cad
mkdir build && cd build
cmake -G Ninja ..
ninja
```

### Step 3: Run
```bash
./bin/driver-solution-cad
```

**That's it!** The application should launch with a 3D viewport.

---

## Windows

### Step 1: Install Prerequisites

#### Option A: Using vcpkg (Recommended)
```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install GLFW
.\vcpkg install glfw3:x64-windows
```

#### Option B: Manual Installation
1. Download **CMake** from https://cmake.org/download/
2. Download **GLFW** from https://www.glfw.org/download.html
3. Install **Visual Studio 2019** or later with C++ support

### Step 2: Clone and Build
```powershell
git clone https://github.com/cebdan/driver-solution-cad.git
cd driver-solution-cad

mkdir build
cd build

# With vcpkg
cmake -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ..
cmake --build . --config Release
```

### Step 3: Run
```powershell
.\bin\Release\driver-solution-cad.exe
```

---

## Linux (Ubuntu/Debian)

### Step 1: Install Dependencies
```bash
sudo apt update
sudo apt install cmake build-essential ninja-build libglfw3-dev
```

### Step 2: Clone and Build
```bash
git clone https://github.com/cebdan/driver-solution-cad.git
cd driver-solution-cad
mkdir build && cd build
cmake -G Ninja ..
ninja
```

### Step 3: Run
```bash
./bin/driver-solution-cad
```

---

## Troubleshooting

### macOS: GLFW not found
```bash
brew install glfw
# If still fails, check installation:
brew list glfw
```

### Windows: GLFW not found
```powershell
# Ensure vcpkg is integrated:
vcpkg integrate install

# Or specify path manually:
cmake -DGLFW_INCLUDE_DIR=C:\path\to\glfw\include -DGLFW_LIBRARY=C:\path\to\glfw\lib\glfw3.lib ..
```

### Linux: GLFW not found
```bash
# Ubuntu/Debian
sudo apt install libglfw3-dev

# Fedora
sudo dnf install glfw-devel
```

### Build without UI
If you only need the core library (no UI):
```bash
cmake -DBUILD_UI=OFF ..
```

### Build without Tests
If Google Test is not installed:
```bash
cmake -DBUILD_TESTING=OFF ..
```

---

## Verification

After successful build, you should see:
```
build/
├── bin/
│   └── driver-solution-cad    # Main application
└── lib/
    ├── libCADCore.a           # Core library
    └── libCADUI.a             # UI library
```

Run the application - you should see:
- 3D viewport with coordinate axes
- View navigator cube (top-right)
- Window title with rendering statistics

---

## Next Steps

1. Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand the design
2. Read [BUILD.md](BUILD.md) for detailed build options
3. Explore the code in `include/` and `src/`
4. Check `docs/instructions/` for implementation details

---

## Cross-Platform Development

This project is designed for cross-platform development. You can:
- Develop on macOS, test on Windows
- Develop on Windows, test on Linux
- Use Git to sync between platforms

All code uses standard C++17 and cross-platform libraries (GLFW, OpenGL).

