# System Validation Report

**Project:** Traffic Management System (Simulation)
**Course:** Software Engineering UE24CS341A, PES University
**Prepared by:** Ramya (PES2UG24CS653), Test Engineer
**Date:** 03-11-2026
**Reference documents:** Software Test Plan v1.0, SAD v1.0, SRS v1.0

## 1. Scope and method

This report records the validation of the Traffic Management System (Simulation) against the Software Test Plan. Testing was carried out on the integrated build of the `main` branch, which includes the Simulation Engine, Queue, Signal, Vehicle and Statistics managers, the Console Interface, the Input Validator and the File Manager.

| Item | Value |
|---|---|
| Environment | Ubuntu 24.04 (WSL2), Windows 11 host |
| Compiler | g++ 13.3.0, C++17, flags `-std=c++17 -g -O0 -Wall -Wextra` |
| Memory tool | Valgrind 3.22.0 (`--leak-check=full --track-origins=yes`) |
| Coverage tool | gcovr 7.0 / gcov, `src/` only (main.cpp excluded) |
| Unit test method | each `tests/test_*.cpp` is compiled with all `src/*.cpp` except `main.cpp` and run |
| System test method | scripted console sessions and driven `ConsoleInterface` sessions |

## 2. Summary of results

| Measure | Result |
|---|---|
| Unit test executables | 12 run, 0 failures |
| Line coverage | **96%** (525 of 542 lines) |
| Branch coverage | **90%** (428 of 471 branches) |
| Valgrind on unit tests | 0 errors, no leaks, see `docs/valgrind_unit_tests.txt` |
| Conflicting signal states observed | 0 (TC-SAFE-01, 200 ticks plus manual mode) |
| Open Critical or High defects | 0 |

## 3. Test case results

| TC ID | Requirement | Scenario | How tested | Expected | Actual | Result |
|---|---|---|---|---|---|---|
| TC-VM-01..05 | TMS-F-001..005 | Vehicle creation, unique IDs, approach assignment, auto-generation | `test_managers`, `test_vehicle_generation`, `test_console_actions` | IDs unique and increasing; correct approach and entry tick; probability 0 gives none, 1 gives always; invalid probability rejected | All checks passed | Pass |
| TC-Q-01..05 | TMS-F-006..010 | FIFO per approach, empty-queue handling, live counts, reset | `test_queue_manager`, `test_managers` | FIFO order kept; dequeue on empty returns false; counts correct; reset clears queues | All checks passed | Pass |
| TC-SIG-01 | TMS-F-011 | Signal states at start | `test_managers` | N+S green, E+W red at tick 0 | As expected | Pass |
| TC-SIG-02 | TMS-F-012 | Automatic safe phase cycle | `test_managers` | Green, yellow, then other axis green, cycle repeats | As expected | Pass |
| TC-SIG-03 | TMS-F-013 | Configurable durations, invalid values ignored | `test_managers`, `test_console_actions` | New durations applied; invalid values rejected | As expected | Pass |
| TC-SIG-04 | TMS-F-014, F-015 | Manual signal control, automatic resume | `test_managers`, `test_console_actions` | One approach green in manual mode; automatic cycle resumes | As expected | Pass |
| TC-SAFE-01 | TMS-NF-006 | No conflicting greens | `test_managers` | No conflicting signals over 200 ticks and in manual mode | 0 conflicts | Pass |
| TC-SIM-01..06 | TMS-F-016..021 | Tick loop; Start, Pause, Resume, Reset, Stop; actions in wrong state | `test_simulation_engine`, `test_console_actions`, `test_console_interface` | Clock advances only when running; frozen when paused; Reset clears clock, queues, statistics; invalid actions rejected without crash | All checks passed | Pass |
| TC-MET-01..05 | TMS-F-022..026 | Waiting and processed counts, average wait, throughput, summary | `test_statistics_manager`, `test_statistics_summary` | Average wait = total wait / processed; zero-data safe; negative wait clamped to 0; summary printed | All checks passed | Pass |
| TC-DATA-01..03 | TMS-F-027..029 | Save and load settings, export CSV, malformed file | `test_file_manager`, `test_console_files` | Valid files saved and loaded; CSV created; missing or malformed file rejected, no crash | All checks passed | Pass |
| TC-SEC-01 | TMS-SR-001 | Input validation (menu, integers, decimals) | `test_input_validator`, `test_input_validator_double`, `test_console_actions` | Non-numeric and out-of-range input rejected with message | All checks passed | Pass |
| TC-SEC-02 | TMS-SR-002 | Empty-queue dequeue | `test_queue_manager`, `test_simulation_engine` | Returns false, no invalid access | As expected | Pass |
| TC-SEC-03 | TMS-SR-003 | Configuration file validation | `test_file_manager`, `test_console_files` | Malformed and missing files rejected | As expected | Pass |
| TC-SEC-04 | TMS-SR-004 | Unexpected input and end of input | `test_console_interface` | Clean exit, no abnormal termination | As expected | Pass |
| TC-SEC-05 | TMS-SR-005 | Memory de-allocation | Valgrind on all unit tests | No leaks, no invalid accesses | 0 errors, all heap blocks freed | Pass |
| TC-PERF-01 | TMS-NF-001 | 100 active vehicles, no noticeable delay | `tests/test_stress_100.cpp` and `tests/stress_input.txt` (PR #27) | All 100 processed, no delay | 100 processed in 70 ticks, 0.1 to 0.2 ms | Pass on PR #27 branch; confirm in `main` after merge |
| TC-MEM-01 | TMS-NF-007 | Memory safety, full program under load | Valgrind on `traffic_sim` with `stress_input.txt` and `edge_input.txt` (PR #27) | 0 errors, no leaks | 0 errors, no leaks | Pass on PR #27 branch; confirm in `main` after merge |
| TC-REL-01 | TMS-NF-002 | Determinism | Review | Reproducible runs with fixed seed | Vehicle generation uses `std::random_device`, no seed option | **Partial**, see observation 2 |
| TC-UX-01 | TMS-NF-003 | Console usability | Manual walkthrough of the 15-option menu with valid and invalid input (add vehicles, start, step, view, statistics, durations, manual signals, stop) | Clear menu, status line after every action, clear error messages with re-prompt | Status line (state, tick, signal mode, auto-generation) shown after every action; invalid input such as `abc` or `99` gives "Invalid input. Enter a whole number between 0 and 15." and re-prompts; per-tick line shows signals, queue sizes and each vehicle that crossed with its wait; statistics shows ticks, processed, average wait, throughput | Pass |
| TC-MAINT-01 | TMS-NF-004 | Maintainability | Code review of module boundaries and build warnings | One responsibility per module; clean build | 9 source files with one responsibility each (largest: ConsoleInterface.cpp, 459 lines); main.cpp is 26 lines; build with `-Wall -Wextra` gives 0 warnings | Pass |
| TC-PORT-01 | TMS-NF-005 | Windows and Linux build | Makefile on Ubuntu (WSL) and GitHub Actions CI | Builds on both | Clean build with 0 warnings on Ubuntu 24.04 (g++ 13.3) and in GitHub Actions CI; Makefile has a Windows target (`traffic_sim.exe`) | Pass on Linux; Windows build of the final code: confirm with the command in the note below |

## 4. Code coverage

Unit tests were instrumented with `--coverage`. `main.cpp` is excluded because it only creates the engine and starts the console; the console logic in `ConsoleInterface.cpp` is included.

| File | Lines | Branches |
|---|---|---|
| ConsoleInterface.cpp | 94% | 89% |
| FileManager.cpp | 94% | 95% |
| InputValidator.cpp | 100% | 94% |
| QueueManager.cpp | 100% | 100% |
| SignalManager.cpp | 100% | 93% |
| SimulationEngine.cpp | 100% | 86% |
| StatisticsManager.cpp | 100% | 100% |
| vehicle_manager.cpp | 100% | 76% |
| **Total** | **96%** | **90%** |

Branch figures exclude compiler-generated exception and unreachable branches (`--exclude-throw-branches --exclude-unreachable-branches`). The remaining uncovered lines are error paths such as file-system failures and I/O stream failures that are not practical to trigger in unit tests. Full per-line detail is in `docs/code_coverage_report.txt` and `docs/branch_coverage_report.txt`.

## 5. Observations and defects

1. **Duplicate auto-generation.** The console has its own auto-generation (`ConsoleInterface::maybeGenerateVehicle`) and does not call `SimulationEngine::setAutoGeneration`. The console version is used at runtime; the engine version is exercised only by tests. Recommended for the team to keep one implementation. Not a functional failure.
2. **Fixed seeds (TMS-NF-002).** The Test Plan asks for fixed PRNG seeds for reproducible runs, but `VehicleManager::generateVehicle` seeds from `std::random_device` and offers no way to set a seed. Tests avoid the dependency by using probability 0.0 and 1.0.
3. **Vehicle ID restart.** The team agreed that Reset restarts vehicle IDs at 1. This is implemented on the PR #27 branch.

No open Critical or High defects were found.

## 6. Conclusion

All executed test cases passed. Unit coverage is 96% lines and 90% branches, and Valgrind reported no memory errors or leaks. TC-PERF-01 and TC-MEM-01 are verified on the PR #27 branch and are to be re-confirmed in `main` after the merge. TC-UX-01 and TC-MAINT-01 were executed manually on the final `main` code. TC-PORT-01 is verified on Linux; the Windows build of the final code is confirmed with `g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o traffic_sim_win.exe` (record the result here).

## 7. Evidence in `docs/`

- `unit_test_report.txt`
- `code_coverage_report.txt`
- `branch_coverage_report.txt`
- `valgrind_unit_tests.txt`
- `system_validation_report.md` (this file)
- `test_case_document.md`
