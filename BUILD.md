# Build Instructions

## macOS (Primary Development Platform)

### Prerequisites
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ninja
brew install glfw          # For UI windowing
brew install googletest    # For testing (optional)
```

### Build (Release)
```bash
git clone https://github.com/cebdan/driver-solution-cad.git
cd driver-solution-cad

mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja

# Run tests (if Google Test installed)
ctest

# Run UI application
./bin/driver-solution-cad
```

### Build (Debug)
```bash
mkdir build-debug && cd build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
ninja
./bin/driver-solution-cad
```

### Build without UI
```bash
cmake -G Ninja -DBUILD_UI=OFF ..
ninja
```

### Xcode Project (Optional)
```bash
mkdir build-xcode && cd build-xcode
cmake -G Xcode ..
open driver-solution-cad.xcodeproj
```

### Apple Silicon + Intel Universal Binary
```bash
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
ninja
```

---

## Windows

### Prerequisites

#### Option 1: Using vcpkg (Recommended)
```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install dependencies
.\vcpkg install glfw3:x64-windows
.\vcpkg install gtest:x64-windows
```

#### Option 2: Manual Installation
1. **CMake**: Download from https://cmake.org/download/
2. **GLFW**: Download from https://www.glfw.org/download.html
   - Extract to `C:\glfw` or add to PATH
3. **Visual Studio**: Install Visual Studio 2019 or later with C++ support

### Build (Visual Studio)
```powershell
git clone https://github.com/cebdan/driver-solution-cad.git
cd driver-solution-cad

mkdir build
cd build

# With vcpkg
cmake -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ..
cmake --build . --config Release

# Or without vcpkg (if GLFW manually installed)
cmake ..
cmake --build . --config Release
```

### Build (Ninja on Windows)
```powershell
# Install Ninja
choco install ninja  # Or download from GitHub

# Build
cmake -G Ninja ..
ninja
```

### Run Application
```powershell
.\bin\Release\driver-solution-cad.exe
# or
.\bin\driver-solution-cad.exe
```

### Troubleshooting Windows

**GLFW not found:**
```powershell
# Specify GLFW path manually
cmake -DGLFW_INCLUDE_DIR=C:\glfw\include -DGLFW_LIBRARY=C:\glfw\lib-vc2019\glfw3.lib ..
```

**OpenGL not found:**
- Windows includes OpenGL by default
- No additional installation needed

**Visual Studio version:**
- Use Visual Studio 2019 or later
- Ensure "Desktop development with C++" workload is installed

---

## Linux

### Prerequisites (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install cmake build-essential ninja-build
sudo apt install libglfw3-dev    # GLFW for UI
sudo apt install libgtest-dev    # Google Test (optional)
```

### Build
```bash
git clone https://github.com/cebdan/driver-solution-cad.git
cd driver-solution-cad

mkdir build && cd build
cmake -G Ninja ..
ninja

# Run application
./bin/driver-solution-cad
```

### Prerequisites (Fedora)
```bash
sudo dnf install cmake gcc-c++ ninja-build
sudo dnf install glfw-devel
sudo dnf install gtest-devel
```

---

## Haiku OS (Future)
```bash
# Install dependencies
pkgman install cmake ninja
pkgman install opencascade

# Build
mkdir build && cd build
cmake -G Ninja ..
ninja
```

---

## IDE Support

### VS Code (macOS/Linux/Windows)
```bash
# Generate compile_commands.json for IntelliSense
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# Use CMake Tools extension
```

### CLion (All platforms)
```
Open folder → CLion auto-detects CMake
```

### Xcode (macOS only)
```bash
cmake -G Xcode ..
```

---

## Troubleshooting

### macOS: "xcrun: error: invalid active developer path"
```bash
xcode-select --install
```

### macOS: GLFW not found
```bash
brew install glfw
# If still fails, check path:
brew list glfw
# Specify manually if needed:
cmake -DGLFW_INCLUDE_DIR=/usr/local/include -DGLFW_LIBRARY=/usr/local/lib/libglfw3.a ..
```

### macOS: OpenGL deprecated warnings
- These are expected on macOS 10.14+
- OpenGL 2.1 is used for compatibility
- Warnings are suppressed with `GL_SILENCE_DEPRECATION`

### Windows: GLFW not found
```powershell
# If using vcpkg, ensure it's integrated:
vcpkg integrate install

# Or specify path manually:
cmake -DGLFW_INCLUDE_DIR=C:\path\to\glfw\include -DGLFW_LIBRARY=C:\path\to\glfw\lib\glfw3.lib ..
```

### Windows: Linker errors
- Ensure Visual Studio C++ tools are installed
- Try building in Release mode first
- Check that GLFW library matches your architecture (x64 vs x86)

### Any platform: GTest not found
```bash
# Build without tests:
cmake -DBUILD_TESTING=OFF ..
```

### Any platform: UI not building
```bash
# Check if GLFW is found:
cmake -DBUILD_UI=ON ..
# If GLFW not found, install it or disable UI:
cmake -DBUILD_UI=OFF ..
```

---

## CMake Options
```bash
# Build type
-DCMAKE_BUILD_TYPE=Release          # Release build (optimized)
-DCMAKE_BUILD_TYPE=Debug            # Debug build (with symbols)

# Features
-DBUILD_UI=ON                       # Enable UI application (default)
-DBUILD_UI=OFF                      # Disable UI (core only)
-DBUILD_TESTING=ON                  # Enable tests (default)
-DBUILD_TESTING=OFF                 # Disable tests

# Universal binary (macOS)
-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# Generator
-G Ninja                            # Use Ninja (fast, recommended)
-G Xcode                            # Generate Xcode project (macOS)
-G "Visual Studio 16 2019"         # Visual Studio 2019 (Windows)
-G "Unix Makefiles"                 # Use make (Linux)
```

---

## Build Output

After successful build:
```
build/
├── bin/
│   ├── driver-solution-cad   # Main UI application
│   └── test_*                 # Test executables
├── lib/
│   ├── libCADCore.a          # Core library
│   └── libCADUI.a            # UI library (if BUILD_UI=ON)
└── Testing/
    └── test_*                 # Test executables
```

## Running the Application

### macOS/Linux
```bash
cd build
./bin/driver-solution-cad
```

### Windows
```powershell
cd build
.\bin\driver-solution-cad.exe
# or
.\bin\Release\driver-solution-cad.exe
```

### Controls
- **Left Mouse + Drag**: Rotate camera
- **Mouse Wheel**: Zoom in/out
- **Click Navigator Cube**: Switch to standard views (Front, Left, Right, Top, Bottom)
- **ESC**: Close window

---

## Next Steps

After successful build:
1. Run tests: `ctest` (if Google Test installed)
2. Run UI application: `./bin/driver-solution-cad`
3. Read architecture: `ARCHITECTURE.md`
4. Read implementation guide: `docs/instructions/01_minimal_kernel.md`

## Cross-Platform Development

This project is designed for cross-platform development:
- **macOS**: Primary development platform
- **Windows**: Fully supported
- **Linux**: Supported
- **Haiku OS**: Planned

You can develop on one platform and test on another. All code uses:
- Standard C++17 (STL)
- CMake for build system
- GLFW for windowing (cross-platform)
- OpenGL 2.1 for rendering (works on all platforms)

Git workflow:
```bash
# On macOS
git add .
git commit -m "Your changes"
git push

# On Windows
git pull
# Build and test
git add .
git commit -m "Windows fixes"
git push
```