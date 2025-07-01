@echo off
REM V-Model Test Framework Runner for Einstein Game Refactor (Windows)
REM This script demonstrates the V-Model testing approach for validating the refactor

echo 🔬 V-Model Test Framework for Einstein Game Refactor
echo =====================================================
echo.
echo The V-Model emphasizes testing at each development level:
echo   Left Side (Development)     ^|  Right Side (Verification)
echo   Requirements Analysis  ^<--^> ^|  Acceptance Testing  
echo   System Design         ^<--^> ^|  System Testing
echo   Module Design         ^<--^> ^|  Integration Testing
echo   Implementation        ^<--^> ^|  Unit Testing
echo.

REM Check if build directory exists
if not exist "build" (
    echo [INFO] Creating build directory...
    mkdir build
)

cd build

REM Configure and build if needed
if not exist "Makefile" if not exist "build.ninja" (
    echo [INFO] Configuring CMake...
    cmake .. || (
        echo [ERROR] CMake configuration failed
        exit /b 1
    )
)

echo [INFO] Building test framework...
cmake --build . --target EinStein_Game_vmodel_tests || (
    echo [ERROR] Build failed
    exit /b 1
)

REM Check if test executable exists
if not exist "EinStein_Game_vmodel_tests.exe" (
    echo [ERROR] Test executable not found
    exit /b 1
)

echo [SUCCESS] Build completed successfully
echo.

REM Run V-Model tests in sequence
echo 🧪 Running V-Model Test Sequence
echo =================================

REM Phase 1: Unit Tests (Bottom of V - Implementation Validation)
echo.
echo [INFO] Phase 1: Unit Testing (Implementation Validation)
echo   - Testing individual components and functions
echo   - Validating core types, ChessBoard, and basic operations
echo   - Performance benchmarking for critical components
echo.

EinStein_Game_vmodel_tests.exe --unit
set unit_result=%ERRORLEVEL%

if %unit_result% equ 0 (
    echo [SUCCESS] Unit tests passed ✅
) else (
    echo [ERROR] Unit tests failed ❌
)

REM Phase 2: Integration Tests (Module Interaction Validation)
echo.
echo [INFO] Phase 2: Integration Testing (Module Interaction Validation)
echo   - Testing component interactions
echo   - Validating data flow between modules
echo   - Testing configuration and state management
echo.

EinStein_Game_vmodel_tests.exe --integration
set integration_result=%ERRORLEVEL%

if %integration_result% equ 0 (
    echo [SUCCESS] Integration tests passed ✅
) else (
    echo [ERROR] Integration tests failed ❌
)

REM Phase 3: System Tests (End-to-End Validation)
echo.
echo [INFO] Phase 3: System Testing (End-to-End Validation)
echo   - Testing overall system behavior
echo   - Performance and resource usage validation
echo   - Thread safety and error handling
echo.

EinStein_Game_vmodel_tests.exe --system
set system_result=%ERRORLEVEL%

if %system_result% equ 0 (
    echo [SUCCESS] System tests passed ✅
) else (
    echo [ERROR] System tests failed ❌
)

REM Phase 4: Acceptance Tests (Requirements Validation)
echo.
echo [INFO] Phase 4: Acceptance Testing (Requirements Validation)
echo   - Validating against original requirements
echo   - Cross-platform compatibility verification
echo   - User experience and usability validation
echo.

EinStein_Game_vmodel_tests.exe --acceptance
set acceptance_result=%ERRORLEVEL%

if %acceptance_result% equ 0 (
    echo [SUCCESS] Acceptance tests passed ✅
) else (
    echo [ERROR] Acceptance tests failed ❌
)

REM Generate comprehensive report
echo.
echo [INFO] Generating V-Model validation report...
EinStein_Game_vmodel_tests.exe --report vmodel_report.html
EinStein_Game_vmodel_tests.exe --report vmodel_report.json

REM Calculate overall result
set /a total_passed=0
if %unit_result% equ 0 set /a total_passed+=1
if %integration_result% equ 0 set /a total_passed+=1
if %system_result% equ 0 set /a total_passed+=1
if %acceptance_result% equ 0 set /a total_passed+=1

echo.
echo 📊 V-Model Validation Summary
echo ==============================
if %unit_result% equ 0 (echo Unit Tests:        ✅ PASS) else (echo Unit Tests:        ❌ FAIL)
if %integration_result% equ 0 (echo Integration Tests: ✅ PASS) else (echo Integration Tests: ❌ FAIL)
if %system_result% equ 0 (echo System Tests:      ✅ PASS) else (echo System Tests:      ❌ FAIL)
if %acceptance_result% equ 0 (echo Acceptance Tests:  ✅ PASS) else (echo Acceptance Tests:  ❌ FAIL)
echo.
echo Overall V-Model Validation: %total_passed%/4 levels passed

if %total_passed% equ 4 (
    echo [SUCCESS] 🎉 Complete V-Model validation successful!
    echo.
    echo ✅ Refactor Quality Assessment:
    echo   • All test levels passed
    echo   • Requirements validated
    echo   • System integrity confirmed
    echo   • Ready for next development phase
    echo.
    echo 📈 Next Steps:
    echo   1. Review detailed reports (vmodel_report.html^)
    echo   2. Address any performance optimization opportunities
    echo   3. Expand test coverage in identified areas
    echo   4. Continue with advanced feature implementation
    
    set exit_code=0
) else (
    echo [ERROR] ❌ V-Model validation incomplete
    echo.
    echo 🔧 Required Actions:
    echo   1. Review failed test details in reports
    echo   2. Fix implementation issues at failing levels
    echo   3. Re-run validation after fixes
    echo   4. Ensure all V-Model levels are satisfied before proceeding
    echo.
    echo 💡 V-Model Testing Philosophy:
    echo   Each level must pass before proceeding to the next
    echo   Failures indicate design or implementation issues
    echo   Fix lower levels before testing higher levels
    
    set exit_code=1
)

echo.
echo 📋 Report Files Generated:
echo   • vmodel_report.html - Detailed HTML report
echo   • vmodel_report.json - Machine-readable JSON report
echo.
echo 🔍 For component-specific testing:
echo   EinStein_Game_vmodel_tests.exe --component ChessBoard
echo   EinStein_Game_vmodel_tests.exe --component Core
echo   EinStein_Game_vmodel_tests.exe --component Config

cd ..
exit /b %exit_code%
