# System Validation Report

**Project:** Traffic Management System (Simulation)
**Course:** Software Engineering UE24CS341A, PES University
**Prepared by:** Ramya (PES2UG24CS653), Test Engineer
**Date:** 03-11-2026
**Tested revision:** `main` @ 089bf3a plus the test files and evidence added by this PR. No application source file is changed by this PR.
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
| Unit test executables | 13 run (includes `test_stress_100`), 0 failures. Unit test report, Valgrind unit report and this report all count the same 13 programs. |
| Line coverage | **96.9%** (525 of 542 lines) |
| Branch coverage | **90.9%** (428 of 471 branches) |
| Valgrind | 0 errors, no leaks on all 13 unit test programs (`docs/valgrind_unit_tests.txt`) and on `test_stress_100` plus three scripted runs of `traffic_sim` (`docs/valgrind_full_program.txt`) |
| Conflicting signal states observed | 0 (TC-SAFE-01, 200 ticks plus manual mode) |
| Open Critical or High defects | 0 (three Low-severity items are open, see section 5) |

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
| TC-SIM-07 | TMS-F-020 | Vehicle IDs restart at 1 after Reset | Create vehicles, Reset, create a vehicle (manual check against `main`) | New vehicle has ID 1 | IDs keep increasing after Reset; `SimulationEngine::reset()` does not reset the vehicle counter | **Fail (open, Low)**, see observation 3 |
| TC-MET-01..05 | TMS-F-022..026 | Waiting and processed counts, average wait, throughput, summary | `test_statistics_manager`, `test_statistics_summary` | Average wait = total wait / processed; zero-data safe; negative wait clamped to 0; summary printed | All checks passed | Pass |
| TC-DATA-01..03 | TMS-F-027..029 | Save and load settings, export CSV, malformed file | `test_file_manager`, `test_console_files` | Valid files saved and loaded; CSV created; missing or malformed file rejected, no crash | All checks passed | Pass |
| TC-SEC-01 | TMS-SR-001 | Input validation (menu, integers, decimals) | `test_input_validator`, `test_input_validator_double`, `test_console_actions` | Non-numeric and out-of-range input rejected with message | All checks passed | Pass |
| TC-SEC-02 | TMS-SR-002 | Empty-queue dequeue | `test_queue_manager`, `test_simulation_engine` | Returns false, no invalid access | As expected | Pass |
| TC-SEC-03 | TMS-SR-003 | Configuration file validation | `test_file_manager`, `test_console_files` | Malformed and missing files rejected | As expected | Pass |
| TC-SEC-04 | TMS-SR-004 | Unexpected input and end of input | `test_console_interface` | Clean exit, no abnormal termination | As expected | Pass |
| TC-SEC-05 | TMS-SR-005 | Memory de-allocation | Valgrind on all unit tests | No leaks, no invalid accesses | 0 errors, all heap blocks freed | Pass |
| TC-PERF-01 | TMS-NF-001 | 100 active vehicles, no noticeable delay | `tests/test_stress_100.cpp` (100 queued vehicles; 100 auto-generated arrivals using the same generate-and-tick loop as the console) and the real program with `tests/stress_input.txt` and `tests/autogen_input.txt` | All 100 processed, no conflicting greens, unique IDs, no noticeable delay | 100 processed in 70 ticks in about 0.1 ms; both console runs end with 100 processed and 0 waiting; see `docs/stress_test_output.txt`, `docs/autogen_run_output.txt` | Pass |
| TC-MEM-01 | TMS-NF-007 | Memory safety, full program under load | Valgrind on `test_stress_100` and on `traffic_sim` with `stress_input.txt`, `edge_input.txt`, `autogen_input.txt` | 0 errors, no leaks | All four runs: exit 0, all heap blocks freed, 0 errors; see `docs/valgrind_full_program.txt` | Pass |
| TC-REL-01 | TMS-NF-002 | Reproducibility with a fixed seed | Review of `VehicleManager::generateVehicle` | Same seed gives identical runs | Generation uses `std::random_device`; no way to set a seed, so the requirement cannot be tested | **Not passed (outstanding)**, see observation 2 |
| TC-UX-01 | TMS-NF-003 | Console usability | Manual walkthrough of the 15-option menu with valid and invalid input (add vehicles, start, step, view, statistics, durations, manual signals, stop) | Clear menu, status line after every action, clear error messages with re-prompt | Status line (state, tick, signal mode, auto-generation) shown after every action; invalid input such as `abc` or `99` gives "Invalid input. Enter a whole number between 0 and 15." and re-prompts; per-tick line shows signals, queue sizes and each vehicle that crossed with its wait; statistics shows ticks, processed, average wait, throughput | Pass |
| TC-MAINT-01 | TMS-NF-004 | Maintainability | Code review of module boundaries and build warnings | One responsibility per module; clean build | 9 source files with one responsibility each (largest: ConsoleInterface.cpp, 459 lines); main.cpp is 26 lines; build with `-Wall -Wextra` gives 0 warnings | Pass |
| TC-PORT-01 | TMS-NF-005 | Windows and Linux build | Makefile on Ubuntu (WSL) and GitHub Actions CI; Windows build command in section 6 | Builds on both | Linux: clean build, 0 warnings (Ubuntu 24.04, g++ 13.3) and CI green. Windows: builds with g++ using the command in section 6 and the program runs. | Pass on Linux and Windows |

## 4. Code coverage

Unit tests were instrumented with `--coverage`. `main.cpp` is excluded because it only creates the engine and starts the console; the console logic in `ConsoleInterface.cpp` is included. All percentages in the reports are given to one decimal place (the gcovr tables show whole percentages; each report ends with the one-decimal total).

| File | Lines | Branches |
|---|---|---|
| ConsoleInterface.cpp | 254/268 (94.8%) | 188/210 (89.5%) |
| FileManager.cpp | 52/55 (94.5%) | 78/82 (95.1%) |
| InputValidator.cpp | 48/48 (100.0%) | 65/69 (94.2%) |
| QueueManager.cpp | 24/24 (100.0%) | 12/12 (100.0%) |
| SignalManager.cpp | 45/45 (100.0%) | 28/30 (93.3%) |
| SimulationEngine.cpp | 51/51 (100.0%) | 32/37 (86.5%) |
| StatisticsManager.cpp | 34/34 (100.0%) | 6/6 (100.0%) |
| vehicle_manager.cpp | 17/17 (100.0%) | 19/25 (76.0%) |
| **Total** | **525/542 (96.9%)** | **428/471 (90.9%)** |

Branch figures exclude compiler-generated exception and unreachable branches (`--exclude-throw-branches --exclude-unreachable-branches`). The remaining uncovered lines are error paths such as file-system failures and I/O stream failures that are not practical to trigger in unit tests; the uncovered branches in `vehicle_manager.cpp` are the random-number paths. Full detail is in `docs/code_coverage_report.txt` and `docs/branch_coverage_report.txt`.

## 5. Observations and open items

This PR adds tests and test evidence only; it does not change application code. Three items found during testing are **open** and are reported to the team for a decision. None is marked as passed.

| # | Item | Severity | Status and disposition |
|---|---|---|---|
| 1 | **Duplicate automatic generation.** `ConsoleInterface::maybeGenerateVehicle` (called from `doStep`) and `SimulationEngine::generateVehicleIfDue` (called from `tick`) are two separate implementations with separate enable flags and probability. The console does not call `SimulationEngine::setAutoGeneration`. The console version is what the application uses; the engine version is used only by tests. | Low (design risk, no failing test) | **Open.** Recommended fix: the console configures the engine through `setAutoGeneration` and the console copy is removed. Mitigation in this PR: the real console path is tested through the real program (`stress_input.txt`, `autogen_input.txt`) as well as through the unit tests. Owner and timing to be agreed with the team. |
| 2 | **No fixed seed (TC-REL-01, TMS-NF-002).** `VehicleManager::generateVehicle` seeds a static generator from `std::random_device` and offers no way to set a seed. | Low | **Open, TC-REL-01 not passed.** The requirement is not met. It is recorded as an outstanding requirement. Tests avoid the dependency by using probability 0.0 and 1.0. Disposition to be agreed with the team (implement a seed option, or accept the gap for this release). |
| 3 | **Vehicle IDs do not restart at 1 after Reset (TC-SIM-07).** The team asked for the restart. `SimulationEngine::reset()` does not reset the `VehicleManager` counter. IDs stay unique, so no automated test fails. | Low | **Open, TC-SIM-07 failed.** It stays open in this report, the test case document and the RTM until the behaviour is implemented and tested. |

No Critical or High defects were found.

## 6. Conclusion

All automated test cases that can be executed passed on the tested revision: 13 unit test programs with 0 failures, 96.9% line coverage and 90.9% branch coverage, and Valgrind reported no memory errors or leaks on any of the 13 programs or on the full program under the 100-vehicle stress, edge-case and auto-generation scripts. TC-UX-01 and TC-MAINT-01 were executed manually.

Not passed or not complete:
- TC-REL-01 (fixed seed): not passed, requirement outstanding (observation 2).
- TC-SIM-07 (IDs restart after Reset): failed, open (observation 3).

**Windows build (to be recorded):** `g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o traffic_sim_win.exe` on Windows. Result: built without errors; traffic_sim_win.exe starts and runs.

## 7. Evidence in `docs/`

- `unit_test_report.txt`
- `code_coverage_report.txt`
- `branch_coverage_report.txt`
- `valgrind_unit_tests.txt`
- `valgrind_full_program.txt`
- `stress_test_output.txt`
- `autogen_run_output.txt`
- `system_validation_report.md` (this file)
- `test_case_document.md`
