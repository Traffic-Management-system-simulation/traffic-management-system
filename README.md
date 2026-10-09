# Traffic Management System (Simulation)

A C/C++ console-based simulation of a single four-way traffic intersection. Vehicles queue on the North, South, East and West approaches and cross only when their signal is green, with the simulation advancing in discrete time ticks.

Developed for **Software Engineering (UE24CS341A)**, PES University.

## Features

- **Vehicle management:** manual vehicle insertion and configurable automatic generation, each vehicle with a unique ID
- **FIFO queues:** one queue per approach, with safe handling of empty-queue operations
- **Traffic signals:** Red/Yellow/Green states, automatic cycling through safe non-conflicting phases, and manual override for testing
- **Configurable timing:** adjustable green and yellow durations with input validation
- **Simulation controls:** Start, Pause, Resume, Reset and Stop from a menu-driven console
- **Live monitoring:** signal states, queue lengths, waiting and processed vehicle counts, and current tick
- **Statistics:** average waiting time, throughput, and a final summary by approach
- **Optional file I/O:** save/load configuration and export final statistics as CSV

## Architecture

The system uses a **modular, layered architecture** in which each module has a single responsibility and can be tested independently.

```
User → Console Interface → Simulation Engine → Vehicle / Queue / Signal Managers → Statistics Manager → Console Interface
```

| Module | Responsibility |
|---|---|
| Console Interface | Menus, input validation, display of queues, signals and statistics |
| Simulation Engine | Tick loop, coordination of modules, simulation state control |
| Vehicle Manager | Vehicle creation, unique IDs, automatic generation |
| Queue Manager | Four FIFO approach queues (enqueue at rear, dequeue at front) |
| Traffic Signal Manager | Signal states, phase sequencing, conflict prevention |
| Statistics Manager | Waiting/processed counts, average wait, throughput |
| Logging / File Manager | Optional configuration and CSV file handling |

## Technology

- **Language:** C / C++ (C++17 recommended)
- **Compiler:** GCC/G++, Clang, or MSVC
- **Platform:** Windows 10/11 or Linux
- **Dependencies:** standard library only. No database, network or external APIs.

## Quality and Safety

- Rejection of invalid, non-numeric or out-of-range input without crashing
- Empty-queue checks before every dequeue
- Validated signal transitions, so conflicting approaches never receive green together
- Validation of loaded configuration files
- No memory leaks or invalid accesses (verify with Valgrind or an equivalent tool)

## Scope

**In scope:** one four-way intersection, simplified traffic behavior, console output.

**Out of scope:** hardware or sensors, networking, multiple intersections, GUI, adaptive or ML-based signal control, real-world traffic calibration.

## Documentation

- Software Requirements Specification (SRS), v1.0
- Software Architecture and Design Specification (SAD), v1.0
- Software Test Plan (STP)

## Team

| Name | SRN |
|---|---|
| Hariom N Kini | PES2UG24CS672 |
| Deepthi S | PES2UG24CS632 |
| Ramya | PES2UG24CS653 |
| Peddinti Suhas | PES2UG24CS647 |
