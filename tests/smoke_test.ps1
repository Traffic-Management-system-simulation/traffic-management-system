
$ErrorActionPreference = "Stop"

Write-Host "=== Traffic Management Smoke Test ==="

# 1. Build the main traffic simulation
Write-Host "`n[1/4] Building traffic simulation..."

g++ -std=c++17 -Wall -Wextra -Iinclude (Get-ChildItem src/*.cpp).FullName -o traffic_sim.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "FAIL: Simulation compilation"
    exit 1
}

Write-Host "PASS: Simulation compilation"

# 2. Run the main program
Write-Host "`n[2/4] Running traffic simulation..."

# Feed "0" (Quit) to the menu so the program exits instead of waiting for input.
$output = "0" | & .\traffic_sim.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "FAIL: Program execution"
    exit 1
}

$output

if (($output -join "`n") -match "Traffic Management System Simulation") {
    Write-Host "PASS: Startup message"
} else {
    Write-Host "FAIL: Startup message"
    exit 1
}

# 3. Build the manager tests
Write-Host "`n[3/4] Building manager tests..."

g++ -std=c++17 -Wall -Wextra -Iinclude tests/test_managers.cpp src/vehicle_manager.cpp src/SignalManager.cpp src/QueueManager.cpp -o manager_tests.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "FAIL: Manager test compilation"
    exit 1
}

Write-Host "PASS: Manager test compilation"

# 4. Execute the manager tests
Write-Host "`n[4/4] Running manager tests..."

& .\manager_tests.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "FAIL: Manager tests"
    exit 1
}

Write-Host "PASS: Manager tests"
Write-Host "`n=== ALL CHECKS PASSED ==="