#!/bin/bash

# Einstein Game Build Script
# Cross-platform build automation for Windows, Linux, and macOS

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="EinStein_Game"
BUILD_TYPE="Release"
INSTALL_DEPS=false
CLEAN_BUILD=false
RUN_TESTS=false
PACKAGE=false

# Platform detection
PLATFORM=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
else
    echo -e "${RED}Unknown platform: $OSTYPE${NC}"
    exit 1
fi

echo -e "${BLUE}Building Einstein Game for $PLATFORM${NC}"

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

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --install-deps)
            INSTALL_DEPS=true
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --package)
            PACKAGE=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug         Build in debug mode"
            echo "  --install-deps  Install system dependencies"
            echo "  --clean         Clean build directory"
            echo "  --test          Run tests after build"
            echo "  --package       Create distribution package"
            echo "  --help          Show this help"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Install dependencies function
install_dependencies() {
    print_status "Installing dependencies for $PLATFORM..."
    
    case $PLATFORM in
        "linux")
            # Ubuntu/Debian
            if command -v apt-get &> /dev/null; then
                sudo apt-get update
                sudo apt-get install -y \
                    cmake \
                    build-essential \
                    libsdl2-dev \
                    libsdl2-ttf-dev \
                    libsdl2-image-dev \
                    python3-dev \
                    python3-pip \
                    git
            # Fedora/CentOS
            elif command -v dnf &> /dev/null; then
                sudo dnf install -y \
                    cmake \
                    gcc-c++ \
                    SDL2-devel \
                    SDL2_ttf-devel \
                    SDL2_image-devel \
                    python3-devel \
                    python3-pip \
                    git
            else
                print_error "Unsupported Linux distribution"
                exit 1
            fi
            ;;
        "macos")
            # Check if Homebrew is installed
            if ! command -v brew &> /dev/null; then
                print_error "Homebrew not found. Please install Homebrew first."
                exit 1
            fi
            
            brew install cmake sdl2 sdl2_ttf sdl2_image python3
            ;;
        "windows")
            print_warning "Please install dependencies manually on Windows:"
            print_warning "1. Visual Studio 2019 or later with C++ tools"
            print_warning "2. CMake"
            print_warning "3. Python 3.7+"
            print_warning "4. SDL2 development libraries"
            ;;
    esac
    
    # Install Python dependencies
    print_status "Installing Python dependencies..."
    pip3 install -r requirements.txt
}

# Setup build environment
setup_build_env() {
    print_status "Setting up build environment..."
    
    # Create build directory
    if [[ "$CLEAN_BUILD" == true ]] && [[ -d "build" ]]; then
        print_status "Cleaning build directory..."
        rm -rf build
    fi
    
    mkdir -p build
    cd build
    
    # Configure CMake based on platform
    case $PLATFORM in
        "windows")
            cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=$BUILD_TYPE
            ;;
        *)
            cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE
            ;;
    esac
}

# Build the project
build_project() {
    print_status "Building project in $BUILD_TYPE mode..."
    
    case $PLATFORM in
        "windows")
            cmake --build . --config $BUILD_TYPE
            ;;
        *)
            make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
            ;;
    esac
}

# Run tests
run_tests() {
    if [[ "$RUN_TESTS" == true ]]; then
        print_status "Running tests..."
        ctest --output-on-failure
    fi
}

# Package application
package_app() {
    if [[ "$PACKAGE" == true ]]; then
        print_status "Creating package..."
        
        case $PLATFORM in
            "linux")
                cpack -G "TGZ;DEB"
                ;;
            "macos")
                cpack -G "DragNDrop"
                ;;
            "windows")
                cpack -G "NSIS;ZIP"
                ;;
        esac
    fi
}

# Main build process
main() {
    print_status "Starting build process..."
    
    # Check if we're in the right directory
    if [[ ! -f "CMakeLists.txt" ]]; then
        print_error "CMakeLists.txt not found. Are you in the project root?"
        exit 1
    fi
    
    # Install dependencies if requested
    if [[ "$INSTALL_DEPS" == true ]]; then
        install_dependencies
    fi
    
    # Setup and build
    setup_build_env
    build_project
    run_tests
    package_app
    
    cd ..  # Return to project root
    
    print_status "Build completed successfully!"
    print_status "Executable location: build/$PROJECT_NAME"
    
    if [[ "$PACKAGE" == true ]]; then
        print_status "Package files created in build/ directory"
    fi
}

# Trap to handle cleanup on exit
cleanup() {
    if [[ -d "build" ]] && [[ "$PWD" == *"/build" ]]; then
        cd ..
    fi
}
trap cleanup EXIT

# Run main function
main
