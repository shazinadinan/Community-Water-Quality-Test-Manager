# Community Water Quality Test Manager

**Course:** Object Oriented Programming using C++ (CAP455) — Minor Project
**Student:** Shazin Adinan K P (Reg. No. 12625822, Roll No. 55)
**Faculty:** Mr. Adarsh Kumar Singh
**Repository:** https://github.com/shazinadinan/Community-Water-Quality-Test-Manager.git

## Problem Statement

Community water sources (wells, borewells, lakes, rivers, tanks) are usually tested and recorded on paper. This leads to repeated sample numbers, inconsistent dates, mistyped readings (e.g. pH 78 instead of 7.8) and no quick way to compare readings with reference limits.

This project is a menu-driven C++ console application that registers water sources and samples, validates every input, compares pH, turbidity and hardness with configurable example limits, flags readings that need attention and prints a summary report. From CA-2 the records are saved in files, so they survive a restart.

> **Disclaimer:** This is an educational simulation. The limits are example values and are **not** official standards. The program never issues a drinking-water certificate.

## Progress

| Increment | Folder | Units | Status |
|---|---|---|---|
| CA-1 | `src/ca1` | I–II | done |
| CA-2 | `src/ca2` | III | **done (current)** |
| CA-3 | `src/ca3` | IV–V | planned |
| CA-4 | `src/ca4` | VI | planned |

## Current Status — CA-2 (Increment 2): constructors and file persistence

Implemented in `src/ca2/main.cpp` (everything from CA-1 is kept):

- **Constructors and destructor** for `WaterSample`: default, parameterized with default arguments (and initializer list), copy, constructor from a file record, and a destructor (static counter `liveObjects`)
- **Binary files with fixed-size records:** `SampleRecord` (56 bytes) in `samples.dat` and `SourceRecord` (44 bytes) in `sources.dat`
- **Data survives a restart:** all records are loaded when the program starts; every new source and sample is appended to its file at once
- **Random access:** search a sample directly in the file with `seekg` (menu 11) and update it in place with `seekp` (menu 12)
- **Text files:** `log.txt` (appended with `ios::app`) and `report.txt` (menu 13); *Save all* (menu 10) rewrites `samples.dat` with `ios::trunc`
- Summary report also shows the number of live `WaterSample` objects

### C++ concepts covered
CA-1 (Units I and II): classes and objects, enumerations, structures, static members, inline / non-inline functions, friend function and friend class, function overloading, references, pointers, `this`, call by value / address / reference, recursion, arrays, multidimensional arrays, arrays of objects, strings, `switch-case`, `rand()`.
CA-2 (Unit III): default / parameterized / copy constructors, constructors with default arguments, initializer lists, destructor, `ifstream` / `ofstream` / `fstream`, file modes (`ios::app`, `ios::trunc`, `ios::in | ios::out`, `ios::binary`), `read` / `write` / `seekg` / `seekp` / `tellg` / `flush`, sequential and random access, binary and text files, structures and classes with files.

Later increments will add inheritance, polymorphism, the STL, templates and exceptions.

## Project Structure

```
CPP_PROJECT/
├── README.md
├── .gitignore
├── src/
│   ├── ca1/main.cpp            # CA-1 source (in-memory)
│   └── ca2/main.cpp            # CA-2 source (constructors + files)
├── tests/
│   ├── run_tests.sh            # builds CA-1 and CA-2 and runs all checks
│   ├── inputs/                 # scripted inputs (ca1.txt, ca2_s1.txt, ca2_s2.txt)
│   └── expected/               # saved outputs of those runs
├── data/                       # sample data files (binary files are for this program only)
└── docs/
    ├── diagrams/               # uml_ca1, flow_ca1, uml_ca2, flow_ca2, flow_persistence, file_layout_ca2 (+ source/ .dot files)
    └── docx/                   # CA-1 and CA-2 reports (docx + pdf)
```

## Requirements

- A C++11-or-later compiler (written and tested with g++ 13.3, C++17); `<filesystem>` needs g++ 9 or newer
- Runs on Windows, Linux and macOS (console application)

## Build and Run

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -o ca2 src/ca2/main.cpp
./ca2 --data data            # keeps the files in the folder "data"
./ca2 --data data --seed 1   # repeatable rand() values
```

Options: `--data FOLDER` (default `data`, created if missing) and `--seed N`. Run CA-1 the same way with `src/ca1/main.cpp` (it keeps nothing on disk).

### CA-2 menu

| # | Option | # | Option |
|---|--------|---|--------|
| 1 | Register water source | 8 | Edit example limits |
| 2 | List sources | 9 | Location tree (recursion) |
| 3 | New sample (manual) | 10 | Save all samples |
| 4 | New sample (simulated) | 11 | Search in file (seekg) |
| 5 | View all samples | 12 | Update sample (seekp) |
| 6 | View one sample (findings) | 13 | Export text report |
| 7 | Summary report | 14 | Exit |

### Files written by CA-2

| File | Type | Content |
|---|---|---|
| `samples.dat` | binary | one 56-byte record per sample |
| `sources.dat` | binary | one 44-byte record per source |
| `log.txt` | text | one line per event (append mode) |
| `report.txt` | text | sample ID, source ID, status |

### Default example limits

| Parameter | Limit |
|-----------|-------|
| pH | 6.5 – 8.5 |
| Turbidity | ≤ 5 NTU |
| Hardness | ≤ 300 mg/L |

## Testing

```bash
bash tests/run_tests.sh            # builds ca1 and ca2, runs 30 checks, compares with tests/expected/
bash tests/run_tests.sh --update   # regenerate the expected files (only after reviewing the output)
```

The CA-2 tests run the program twice on the same data folder, so they also prove that data survives a restart. The expected files were recorded with `--seed 3` (CA-1) and `--seed 5` (CA-2) on g++/Linux; `rand()` gives different numbers on other systems, so simulated values may differ there (manual samples, findings and messages are identical).

## Author

Shazin Adinan K P — CA-1 and CA-2, CAP455.
