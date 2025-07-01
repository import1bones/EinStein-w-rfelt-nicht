@echo off
setlocal enabledelayedexpansion

REM Einstein Game Build Script for Windows
REM Requires Visual Studio 2019 or later, CMake, and Python 3.7+

set PROJECT_NAME=EinStein_Game
set BUILD_TYPE=Release
set INSTALL_DEPS=false
set CLEAN_BUILD=false
set RUN_TESTS=false
set PACKAGE=false

echo Building Einstein Game for Windows...

REM Parse command line arguments
:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%~1"=="--install-deps" (
    set INSTALL_DEPS=true
    shift
    goto parse_args
)
if "%~1"=="--clean" (
    set CLEAN_BUILD=true
    shift
    goto parse_args
)
if "%~1"=="--test" (
    set RUN_TESTS=true
    shift
    goto parse_args
)
if "%~1"=="--package" (
    set PACKAGE=true
    shift
    goto parse_args
)
if "%~1"=="--help" (
    echo Usage: %0 [options]
    echo Options:
    echo   --debug         Build in debug mode
    echo   --install-deps  Install Python dependencies
    echo   --clean         Clean build directory
    echo   --test          Run tests after build
    echo   --package       Create distribution package
    echo   --help          Show this help
    exit /b 0
)
shift
goto parse_args
:end_parse

REM Check for required tools
echo [INFO] Checking for required tools...

cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found. Please install CMake and add it to PATH.
    exit /b 1
)

python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found. Please install Python 3.7+ and add it to PATH.
    exit /b 1
)

where cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Visual Studio C++ compiler not found.
    echo Please run this script from a Visual Studio Developer Command Prompt.
    exit /b 1
)

REM Install Python dependencies
if "%INSTALL_DEPS%"=="true" (
    echo [INFO] Installing Python dependencies...
    python -m pip install --upgrade pip
    python -m pip install -r requirements.txt
    if errorlevel 1 (
        echo [ERROR] Failed to install Python dependencies.
        exit /b 1
    )
)

REM Setup build environment
echo [INFO] Setting up build environment...

if "%CLEAN_BUILD%"=="true" (
    if exist build (
        echo [INFO] Cleaning build directory...
        rmdir /s /q build
    )
)

if not exist build mkdir build
cd build

REM Configure CMake
echo [INFO] Configuring CMake...
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    cd ..
    exit /b 1
)

REM Build the project
echo [INFO] Building project in %BUILD_TYPE% mode...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Build failed.
    cd ..
    exit /b 1
)

REM Run tests
if "%RUN_TESTS%"=="true" (
    echo [INFO] Running tests...
    ctest --output-on-failure
    if errorlevel 1 (
        echo [WARNING] Some tests failed.
    )
)

REM Package application
if "%PACKAGE%"=="true" (
    echo [INFO] Creating package...
    cpack -G "NSIS;ZIP"
    if errorlevel 1 (
        echo [WARNING] Packaging failed.
    )
)

cd ..

echo [INFO] Build completed successfully!
echo [INFO] Executable location: build\%BUILD_TYPE%\%PROJECT_NAME%.exe

if "%PACKAGE%"=="true" (
    echo [INFO] Package files created in build\ directory
)

pause
