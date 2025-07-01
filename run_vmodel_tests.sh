#!/bin/bash

# V-Model Test Framework Runner for Einstein Game Refactor
# This script demonstrates the V-Model testing approach for validating the refactor

echo "🔬 V-Model Test Framework for Einstein Game Refactor"
echo "====================================================="
echo ""
echo "The V-Model emphasizes testing at each development level:"
echo "  Left Side (Development)     |  Right Side (Verification)"
echo "  Requirements Analysis  <--> |  Acceptance Testing"  
echo "  System Design         <--> |  System Testing"
echo "  Module Design         <--> |  Integration Testing"
echo "  Implementation        <--> |  Unit Testing"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if build directory exists
if [ ! -d "build" ]; then
    print_status "Creating build directory..."
    mkdir -p build
fi

cd build

# Configure and build if needed
if [ ! -f "Makefile" ] && [ ! -f "build.ninja" ]; then
    print_status "Configuring CMake..."
    cmake .. || { print_error "CMake configuration failed"; exit 1; }
fi

print_status "Building test framework..."
make EinStein_Game_vmodel_tests || { print_error "Build failed"; exit 1; }

# Check if test executable exists
if [ ! -f "./EinStein_Game_vmodel_tests" ]; then
    print_error "Test executable not found"
    exit 1
fi

print_success "Build completed successfully"
echo ""

# Run V-Model tests in sequence
echo "🧪 Running V-Model Test Sequence"
echo "================================="

# Phase 1: Unit Tests (Bottom of V - Implementation Validation)
echo ""
print_status "Phase 1: Unit Testing (Implementation Validation)"
echo "  - Testing individual components and functions"
echo "  - Validating core types, ChessBoard, and basic operations"
echo "  - Performance benchmarking for critical components"
echo ""

./EinStein_Game_vmodel_tests --unit
unit_result=$?

if [ $unit_result -eq 0 ]; then
    print_success "Unit tests passed ✅"
else
    print_error "Unit tests failed ❌"
fi

# Phase 2: Integration Tests (Module Interaction Validation)
echo ""
print_status "Phase 2: Integration Testing (Module Interaction Validation)"
echo "  - Testing component interactions"
echo "  - Validating data flow between modules"
echo "  - Testing configuration and state management"
echo ""

./EinStein_Game_vmodel_tests --integration
integration_result=$?

if [ $integration_result -eq 0 ]; then
    print_success "Integration tests passed ✅"
else
    print_error "Integration tests failed ❌"
fi

# Phase 3: System Tests (End-to-End Validation)
echo ""
print_status "Phase 3: System Testing (End-to-End Validation)"
echo "  - Testing overall system behavior"
echo "  - Performance and resource usage validation"
echo "  - Thread safety and error handling"
echo ""

./EinStein_Game_vmodel_tests --system
system_result=$?

if [ $system_result -eq 0 ]; then
    print_success "System tests passed ✅"
else
    print_error "System tests failed ❌"
fi

# Phase 4: Acceptance Tests (Requirements Validation)
echo ""
print_status "Phase 4: Acceptance Testing (Requirements Validation)"
echo "  - Validating against original requirements"
echo "  - Cross-platform compatibility verification"
echo "  - User experience and usability validation"
echo ""

./EinStein_Game_vmodel_tests --acceptance
acceptance_result=$?

if [ $acceptance_result -eq 0 ]; then
    print_success "Acceptance tests passed ✅"
else
    print_error "Acceptance tests failed ❌"
fi

# Generate comprehensive report
echo ""
print_status "Generating V-Model validation report..."
./EinStein_Game_vmodel_tests --report vmodel_report.html
./EinStein_Game_vmodel_tests --report vmodel_report.json

# Calculate overall result
total_passed=0
if [ $unit_result -eq 0 ]; then ((total_passed++)); fi
if [ $integration_result -eq 0 ]; then ((total_passed++)); fi
if [ $system_result -eq 0 ]; then ((total_passed++)); fi
if [ $acceptance_result -eq 0 ]; then ((total_passed++)); fi

echo ""
echo "📊 V-Model Validation Summary"
echo "=============================="
echo "Unit Tests:        $([ $unit_result -eq 0 ] && echo "✅ PASS" || echo "❌ FAIL")"
echo "Integration Tests: $([ $integration_result -eq 0 ] && echo "✅ PASS" || echo "❌ FAIL")"
echo "System Tests:      $([ $system_result -eq 0 ] && echo "✅ PASS" || echo "❌ FAIL")"
echo "Acceptance Tests:  $([ $acceptance_result -eq 0 ] && echo "✅ PASS" || echo "❌ FAIL")"
echo ""
echo "Overall V-Model Validation: $total_passed/4 levels passed"

if [ $total_passed -eq 4 ]; then
    print_success "🎉 Complete V-Model validation successful!"
    echo ""
    echo "✅ Refactor Quality Assessment:"
    echo "  • All test levels passed"
    echo "  • Requirements validated"
    echo "  • System integrity confirmed"
    echo "  • Ready for next development phase"
    echo ""
    echo "📈 Next Steps:"
    echo "  1. Review detailed reports (vmodel_report.html)"
    echo "  2. Address any performance optimization opportunities"
    echo "  3. Expand test coverage in identified areas"
    echo "  4. Continue with advanced feature implementation"
    
    exit_code=0
else
    print_error "❌ V-Model validation incomplete"
    echo ""
    echo "🔧 Required Actions:"
    echo "  1. Review failed test details in reports"
    echo "  2. Fix implementation issues at failing levels"
    echo "  3. Re-run validation after fixes"
    echo "  4. Ensure all V-Model levels are satisfied before proceeding"
    echo ""
    echo "💡 V-Model Testing Philosophy:"
    echo "  Each level must pass before proceeding to the next"
    echo "  Failures indicate design or implementation issues"
    echo "  Fix lower levels before testing higher levels"
    
    exit_code=1
fi

echo ""
echo "📋 Report Files Generated:"
echo "  • vmodel_report.html - Detailed HTML report"
echo "  • vmodel_report.json - Machine-readable JSON report"
echo ""
echo "🔍 For component-specific testing:"
echo "  ./EinStein_Game_vmodel_tests --component ChessBoard"
echo "  ./EinStein_Game_vmodel_tests --component Core"
echo "  ./EinStein_Game_vmodel_tests --component Config"

exit $exit_code
