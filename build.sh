#!/bin/bash

# Build script for Einstein Game (Cross-platform)
# This script sets up the build environment and compiles the project

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Einstein Game Build Script ===${NC}"

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect OS
OS="unknown"
case "$OSTYPE" in
    darwin*)  OS="macos" ;;
    linux*)   OS="linux" ;;
    msys*)    OS="windows" ;;
    cygwin*)  OS="windows" ;;
    *)        OS="unknown" ;;
esac

print_status "Detected OS: $OS"

# Configuration
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"
INSTALL_PREFIX="${INSTALL_PREFIX:-install}"
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

print_status "Build configuration:"
echo "  Build type: $BUILD_TYPE"
echo "  Build directory: $BUILD_DIR"
echo "  Install prefix: $INSTALL_PREFIX"
echo "  Parallel jobs: $JOBS"

# Check dependencies
check_dependency() {
    if ! command -v "$1" &> /dev/null; then
        print_error "$1 is required but not installed"
        return 1
    fi
    print_status "$1 found"
    return 0
}

print_status "Checking dependencies..."

# Essential build tools
check_dependency "cmake" || exit 1
check_dependency "git" || exit 1

# C++ compiler
if command -v "g++" &> /dev/null; then
    CXX_COMPILER="g++"
elif command -v "clang++" &> /dev/null; then
    CXX_COMPILER="clang++"
elif command -v "cl" &> /dev/null; then
    CXX_COMPILER="cl"
else
    print_error "No C++ compiler found"
    exit 1
fi
print_status "C++ compiler: $CXX_COMPILER"

# Python
check_dependency "python3" || check_dependency "python" || exit 1

# Platform-specific dependency checks
case "$OS" in
    "linux")
        print_status "Checking Linux dependencies..."
        # Check for SDL2 development packages
        if ! pkg-config --exists sdl2; then
            print_warning "SDL2 development packages not found"
            print_status "Install with: sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev"
        fi
        ;;
    "macos")
        print_status "Checking macOS dependencies..."
        if ! command -v "brew" &> /dev/null; then
            print_warning "Homebrew not found. Consider installing for easier dependency management"
        fi
        ;;
    "windows")
        print_status "Windows detected. Make sure SDL2 libraries are available."
        ;;
esac

# Setup Python environment
print_status "Setting up Python environment..."
PYTHON_CMD="python3"
if ! command -v "python3" &> /dev/null; then
    PYTHON_CMD="python"
fi

# Check if virtual environment exists
if [ ! -d "venv" ]; then
    print_status "Creating Python virtual environment..."
    $PYTHON_CMD -m venv venv
fi

# Activate virtual environment
print_status "Activating Python virtual environment..."
if [ "$OS" = "windows" ]; then
    source venv/Scripts/activate
else
    source venv/bin/activate
fi

# Install Python dependencies
print_status "Installing Python dependencies..."
pip install --upgrade pip
pip install -r requirements.txt

# Create build directory
print_status "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring with CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="../$INSTALL_PREFIX"
    -DCMAKE_CXX_STANDARD=17
)

# Platform-specific CMake arguments
case "$OS" in
    "windows")
        CMAKE_ARGS+=(-G "Visual Studio 16 2019" -A x64)
        ;;
    *)
        CMAKE_ARGS+=(-G "Unix Makefiles")
        ;;
esac

cmake "${CMAKE_ARGS[@]}" ..

# Build
print_status "Building project..."
cmake --build . --config "$BUILD_TYPE" --parallel "$JOBS"

# Install
print_status "Installing project..."
cmake --install . --config "$BUILD_TYPE"

# Run tests if available
if [ -f "test/einstein_tests" ] || [ -f "test/einstein_tests.exe" ]; then
    print_status "Running tests..."
    ctest --output-on-failure --parallel "$JOBS"
fi

cd ..

print_status "Build completed successfully!"
print_status "Executable location: $INSTALL_PREFIX/bin/"
print_status "To run the game: ./$INSTALL_PREFIX/bin/EinStein_Game"

# Create run script
print_status "Creating run script..."
cat > run_game.sh << 'EOF'
#!/bin/bash
# Run script for Einstein Game

# Activate Python environment
if [ -d "venv" ]; then
    source venv/bin/activate
fi

# Change to install directory
cd install/bin 2>/dev/null || cd bin 2>/dev/null || true

# Run game
if [ -f "EinStein_Game" ]; then
    ./EinStein_Game "$@"
elif [ -f "EinStein_Game.exe" ]; then
    ./EinStein_Game.exe "$@"
else
    echo "Game executable not found!"
    exit 1
fi
EOF

chmod +x run_game.sh

print_status "Use './run_game.sh' to start the game"

echo -e "${GREEN}=== Build Complete ===${NC}"
