# Test Case Document (Updated for Part-3)

**Project:** Traffic Management System (Simulation)
**Course:** Software Engineering UE24CS341A, PES University
**Version:** 1.1 (updated after Sprint 2 changes)
**Date:** 03-11-2026
**Owner:** Ramya (PES2UG24CS653), Test Engineer

## 1. Changes since the previous version

| Area | Change |
|---|---|
| Console menu | Now 15 options. Added Save Settings (13), Load Settings (14), Export Statistics to CSV (15). |
| Architecture | Console logic moved from `main.cpp` into `ConsoleInterface`; input checks moved into `InputValidator`; file handling in `FileManager`. |
| Vehicle IDs | Team requested that Reset restarts IDs at 1. Not implemented on `main` yet (TC-SIM-07). |
| Auto-generation | Probability per tick, range 0.01 to 1.00. |
| New cases | 100-vehicle stress run, Stop and Reset with vehicles queued, repeated load and reset cycles, settings save and load, CSV export, malformed file handling. |

## 2. Test cases

Status: **P** = Passed, **X** = Not executed yet. Automation is the test file that executes the case.

| TC ID | Req ID | Title | Steps | Expected result | Automation | Status |
|---|---|---|---|---|---|---|
| TC-VM-01 | TMS-F-001 | Create vehicle | Create a vehicle for an approach at tick N | Vehicle has the approach and entry tick N | test_managers | P |
| TC-VM-02 | TMS-F-002 | Unique IDs | Create several vehicles | IDs unique and increasing | test_vehicle_generation | P |
| TC-VM-03 | TMS-F-003 | Assign to queue | Add vehicles via menu option 6 | Vehicles appear in the chosen approach queue | test_console_actions | P |
| TC-VM-04 | TMS-F-004 | Display waiting vehicles | Menu option 10 | Queue sizes and signals shown | test_console_actions | P |
| TC-VM-05 | TMS-F-005 | Auto-generation | Option 7 with p=1.0, step 5 ticks | Vehicles generated; p=0 generates none; invalid p rejected | test_vehicle_generation, test_console_actions | P |
| TC-Q-01 | TMS-F-006 | Separate queue per approach | Enqueue to North and South | Sizes tracked independently | test_queue_manager | P |
| TC-Q-02 | TMS-F-007 | FIFO order | Enqueue then dequeue | Vehicles leave in arrival order | test_queue_manager | P |
| TC-Q-03 | TMS-F-008 | Empty queue dequeue | Dequeue from empty queue | Returns false, no crash | test_queue_manager | P |
| TC-Q-04 | TMS-F-009 | Live counts | Enqueue and dequeue | Counts update | test_queue_manager | P |
| TC-Q-05 | TMS-F-010 | Queue reset | Reset | All queues empty | test_queue_manager | P |
| TC-SIG-01 | TMS-F-011 | Initial signal state | Read signals at tick 0 | N+S green, E+W red | test_managers | P |
| TC-SIG-02 | TMS-F-012 | Automatic cycle | Run through one full cycle | Green, yellow, other axis green, repeat | test_managers | P |
| TC-SIG-03 | TMS-F-013 | Configure durations | Option 8 with valid and invalid values | Valid applied; invalid rejected | test_managers, test_console_actions | P |
| TC-SIG-04 | TMS-F-014, F-015 | Manual mode | Option 9: manual, choose East, then automatic | Only East green; automatic resumes | test_managers, test_console_actions | P |
| TC-SAFE-01 | TMS-NF-006 | No conflicting greens | 200 ticks plus manual mode | No conflict at any tick | test_managers | P |
| TC-SIM-01 | TMS-F-016 | Tick advances clock | Start, step N ticks | Clock equals N | test_simulation_engine, test_console_actions | P |
| TC-SIM-02 | TMS-F-017 | Vehicles move on green only | Queue vehicles, run | Only green approaches dequeue | test_simulation_engine | P |
| TC-SIM-03 | TMS-F-018 | Start, Pause, Resume | Options 1, 2, 3 | States RUNNING, PAUSED, RUNNING; clock frozen while paused | test_simulation_engine, test_console_actions | P |
| TC-SIM-04 | TMS-F-019 | Stop | Option 5 | STOPPED; statistics kept for summary | test_console_actions | P |
| TC-SIM-05 | TMS-F-020 | Reset | Option 4 | Clock, queues, statistics and signals reset | test_simulation_engine, test_console_actions | P |
| TC-SIM-07 | TMS-F-020 | Vehicle IDs restart after Reset | Create vehicles, Reset, create a vehicle | New vehicle has ID 1 | none | Fail: IDs continue (open, Low) |
| TC-SIM-06 | TMS-F-021 | Invalid action in wrong state | Pause, Resume, Stop while stopped; Step while not running | Rejected with message, no crash | test_console_actions | P |
| TC-MET-01 | TMS-F-022 | Waiting and processed counts | Run a scenario | Counts correct | test_statistics_manager | P |
| TC-MET-02 | TMS-F-023 | Average waiting time | Two vehicles each waiting 5 | Average is 5.0 | test_statistics_summary | P |
| TC-MET-03 | TMS-F-024 | Throughput | Calculate over N ticks | Non-negative, safe for 0 ticks | test_statistics_summary | P |
| TC-MET-04 | TMS-F-025 | Negative wait clamped | Exit tick before entry tick | Clamped to 0 with warning | test_statistics_manager | P |
| TC-MET-05 | TMS-F-026 | Final summary | Option 11 or Stop | Summary printed, also for empty data | test_statistics_summary | P |
| TC-DATA-01 | TMS-F-027 | Save settings | Option 13 | File written, success message | test_console_files, test_file_manager | P |
| TC-DATA-02 | TMS-F-028 | Load settings | Option 14 after save | Settings restored | test_console_files, test_file_manager | P |
| TC-DATA-03 | TMS-F-029 | Export CSV | Option 15 after a run | CSV created with statistics | test_console_files, test_file_manager | P |
| TC-SEC-01 | TMS-SR-001 | Input validation | Letters, out-of-range, blank input | Rejected with message and re-prompt | test_input_validator, test_input_validator_double | P |
| TC-SEC-02 | TMS-SR-002 | Empty-queue safety | Dequeue from empty and unused queue | No invalid access | test_queue_manager | P |
| TC-SEC-03 | TMS-SR-003 | Malformed config file | Load missing and garbage file | Rejected, no crash, state unchanged | test_file_manager, test_console_files | P |
| TC-SEC-04 | TMS-SR-004 | End of input | Close input mid-session | Clean exit | test_console_interface | P |
| TC-SEC-05 | TMS-SR-005 | Memory freed | Valgrind on all unit tests | 0 errors, no leaks | Valgrind | P |
| TC-PERF-01 | TMS-NF-001 | 100 active vehicles | Queue 100 vehicles, run until all processed; 100 engine-driven arrivals; console scripts | All processed, no conflicting greens, no noticeable delay | test_stress_100, stress_input.txt, autogen_input.txt | P |
| TC-MEM-01 | TMS-NF-007 | Memory safety under load | Valgrind on test_stress_100 and on traffic_sim with stress, edge and autogen scripts | 0 errors, no leaks | Valgrind | P |
| TC-REL-01 | TMS-NF-002 | Reproducibility | Run with a fixed seed twice | Identical results | none | Partial: no seed option |
| TC-UX-01 | TMS-NF-003 | Usability | Manual walkthrough of all 15 menu options | Clear menu, status line, error messages | manual | P |
| TC-MAINT-01 | TMS-NF-004 | Maintainability | Code review of module boundaries | One responsibility per module; build with -Wall -Wextra has no warnings | manual | P |
| TC-PORT-01 | TMS-NF-005 | Portability | Build on Linux and Windows | Builds on both | Makefile, CI | Linux P; Windows: confirm on final code |

## 3. RTM status

| Req ID | Module | Test cases | Status |
|---|---|---|---|
| TMS-F-001..005 | Vehicle Manager | TC-VM-01..05 | A |
| TMS-F-006..010 | Queue Manager | TC-Q-01..05 | A |
| TMS-F-011..015 | Signal Manager | TC-SIG-01..04 | A |
| TMS-F-016..021 | Simulation Engine, Console | TC-SIM-01..07 | P (TC-SIM-07 open) |
| TMS-F-022..026 | Statistics Manager | TC-MET-01..05 | A |
| TMS-F-027..029 | File Manager | TC-DATA-01..03 | A |
| TMS-NF-001 | Engine | TC-PERF-01 | A |
| TMS-NF-002 | Vehicle Manager | TC-REL-01 | P (no seed option) |
| TMS-NF-003 | Console | TC-UX-01 | A |
| TMS-NF-004 | All | TC-MAINT-01 | A |
| TMS-NF-005 | Build | TC-PORT-01 | P |
| TMS-NF-006 | Signal Manager | TC-SAFE-01 | A |
| TMS-NF-007 | Queue, Vehicle | TC-MEM-01 | A |
| TMS-SR-001..005 | Console, File, Queue | TC-SEC-01..05 | A |

Legend: N = Not started, P = Partially verified, A = Accepted.
