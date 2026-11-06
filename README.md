# Community Water Quality Test Manager

**Course:** Object Oriented Programming using C++ (CAP455) — Minor Project
**Student:** Shazin Adinan K P (Reg. No. 12625822, Roll No. 55)
**Faculty:** Mr. Adarsh Kumar Singh
**Repository:** https://github.com/shazinadinan/Community-Water-Quality-Test-Manager.git

## Problem Statement

Community water sources (wells, borewells, lakes, rivers, tanks) are usually tested and recorded on paper. This leads to repeated sample numbers, inconsistent dates, mistyped readings (e.g. pH 78 instead of 7.8) and no quick way to compare readings with reference limits.

This project is a menu-driven C++ console application that registers water sources and samples, validates every input, compares pH, turbidity and hardness with configurable example limits, flags readings that need attention and prints a summary report.

> **Disclaimer:** This is an educational simulation. The limits are example values and are **not** official standards. The program never issues a drinking-water certificate.

## Current Status — CA-1 (Increment 1)

Implemented in `src/ca1/main.cpp`:

- Register water sources (ID 1–999, name, type, location from a location tree; max 10)
- Register samples (unique ID, valid date, pH / turbidity / hardness; max 50)
- Manual entry or `rand()`-simulated readings (Typical, After heavy rain, Hard-water area, Fully random) — simulated data is always labelled
- Parameter-wise findings, status (TESTED / NEEDS ATTENTION) and a combined example score (0–100)
- View all samples, view one sample, summary report (counts, flagged readings, averages by source type)
- Edit example limits (all stored samples are re-assessed)
- Full input validation (menu, numbers, dates, duplicate IDs, end-of-input safe)

### C++ concepts covered (Units I and II)
Classes and objects, enumerations, structures, static members, inline / non-inline functions, friend function and friend class, function overloading, references, pointers, `this` pointer, call by value / address / reference, recursion, arrays, multidimensional arrays, arrays of objects, strings, `switch-case`, `rand()` / `srand()`.

Later increments (CA-2 to CA-4) are planned to add constructors/destructors, file handling, inheritance, polymorphism, STL and exceptions.

## Project Structure

```
CPP_PROJECT/
├── README.md
├── src/
│   └── ca1/main.cpp            # CA-1 source code
├── tests/
│   ├── inputs/ca1.txt          # scripted input for a test run
│   └── expected/ca1.out        # expected output of that run
├── data/
│   ├── limits.cfg              # sample limits file (phMin, phMax, turbidityMax, hardnessMax)
│   ├── meta.txt                # metadata
│   ├── samples.csv             # sample records (CSV)
│   ├── samples.dat             # sample records (binary)
│   └── sources.dat             # source records
└── docs/diagrams/
    ├── uml_ca1.png             # UML class diagram
    └── flow_ca1.png            # program flow chart
```

> The files in `data/` are sample data for later increments; the CA-1 program keeps all data in memory and does not read them.

## Requirements

- A C++11-or-later compiler (written and tested with g++ 13.3, C++17)
- Runs on Windows, Linux and macOS (console application)

## Build

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -o ca1 src/ca1/main.cpp
```

(Dev-C++, Code::Blocks or VS Code work equally well.)

## Run

```bash
./ca1                # normal run (random seed from the clock)
./ca1 --seed 1       # repeatable run, same rand() values every time
```

### Menu

| # | Option | # | Option |
|---|--------|---|--------|
| 1 | Register water source | 6 | View one sample (findings) |
| 2 | List sources | 7 | Summary report |
| 3 | New sample (manual) | 8 | Edit example limits |
| 4 | New sample (simulated) | 9 | Location tree (recursion) |
| 5 | View all samples | 10 | Exit |

### Default example limits

| Parameter | Limit |
|-----------|-------|
| pH | 6.5 – 8.5 |
| Turbidity | ≤ 5 NTU |
| Hardness | ≤ 300 mg/L |

## Testing

`tests/inputs/ca1.txt` is a scripted input and `tests/expected/ca1.out` is the output recorded for it (the last line `rc=0` is the exit code).

```bash
./ca1 --seed 1 < tests/inputs/ca1.txt > actual.out
diff actual.out tests/expected/ca1.out
```

Everything matches except the values of **simulated** samples, because `rand()` gives different numbers on different compilers/platforms, and the `rc=0` line. Manual samples, findings and validation messages are identical.

## Author

Shazin Adinan K P — CA-1, CAP455.
