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
brew install opencascade  # Optional, for geometry
brew install googletest   # For testing
```

### Build (Release)
```bash
git clone https://github.com/yourusername/driver-solution-cad.git
cd driver-solution-cad

mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja

# Run tests
ctest

# Run example
./example_point
```

### Build (Debug)
```bash
mkdir build-debug && cd build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
ninja
```

### Xcode Project (Optional)
```bash
mkdir build-xcode && cd build-xcode
cmake -G Xcode ..
open DriverSolutionCAD.xcodeproj
```

### Apple Silicon + Intel Universal Binary
```bash
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
ninja
```

---

## Windows (Future)
```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
vcpkg integrate install

# Install dependencies
vcpkg install opencascade:x64-windows
vcpkg install gtest:x64-windows

# Build
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release
```

---

## Linux (Future)
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install cmake build-essential ninja-build
sudo apt install libocct-dev  # OpenCascade
sudo apt install libgtest-dev # Google Test

# Build
mkdir build && cd build
cmake -G Ninja ..
ninja
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

### macOS: OpenCascade not found
```bash
brew reinstall opencascade
# If still fails, specify manually:
cmake -DOpenCASCADE_DIR=/usr/local/opt/opencascade/lib/cmake/opencascade ..
```

### Any platform: GTest not found
```bash
# Build without tests:
cmake -DBUILD_TESTING=OFF ..
```

---

## CMake Options
```bash
# Build type
-DCMAKE_BUILD_TYPE=Release          # Release build (optimized)
-DCMAKE_BUILD_TYPE=Debug            # Debug build (with symbols)

# Testing
-DBUILD_TESTING=ON                  # Enable tests (default)
-DBUILD_TESTING=OFF                 # Disable tests

# Universal binary (macOS)
-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# Generator
-G Ninja                            # Use Ninja (fast)
-G Xcode                            # Generate Xcode project
-G "Unix Makefiles"                 # Use make
```

---

## Build Output

After successful build:
```
build/
├── bin/
│   └── example_point          # Example executable
├── lib/
│   └── libCADCore.a          # Core library
└── Testing/
    └── test_*                 # Test executables
```

---

## Next Steps

After successful build:
1. Run tests: `ctest`
2. Run example: `./bin/example_point`
3. Read implementation guide: `docs/instructions/01_minimal_kernel.md`
4. Start implementing with Cursor AI