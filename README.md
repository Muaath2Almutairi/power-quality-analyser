# Power Quality Waveform Analyser
**UGMFGT-15-1 Programming for Engineers**
Electrical and Electronic Engineering | University of the West of England | 2025–26

## What this program does

This is a command-line C program for my coursework. It reads the power quality sensor log (CSV), calculates the required waveform metrics for Phase A, B, and C, and writes the final report to `results.txt`.

Metrics computed per phase:
- RMS voltage (compared against EN 50160 ±10% tolerance: 207–253 V)
- Peak-to-peak voltage
- DC offset (arithmetic mean — should be ≈ 0 V for clean AC)
- Clipped sample count (|V| ≥ 324.9 V)
- Standard deviation (Merit extension)
- Bitwise status flags encoding clipping and compliance anomalies

Dataset-wide statistics also reported:
- Mean frequency (nominal 50.000 Hz)
- Mean power factor (nominal 0.95)
- Mean THD % (EN 50160 limit: 8%)

---

## File structure

```
power_quality_analyser/
├── main.c          Main file that handles command line arguments and calls the functions
├── waveform.c      All analysis functions (RMS, peak-to-peak, DC offset, …)
├── waveform.h      WaveformSample struct, PhaseResult struct, function prototypes
├── io.c            CSV loader and report writer
├── io.h            I/O function prototypes
├── CMakeLists.txt  CMake build configuration
└── README.md       This file
```

---

## How to compile and run

### Option 1 — CLion (recommended IDE)

1. Open CLion and choose **File → Open**, then select the `power_quality_analyser` folder.
2. CLion detects `CMakeLists.txt` and configures the project automatically.
3. Click the green **Run** button (or press `Shift+F10`).
4. To pass the CSV path as an argument: **Run → Edit Configurations → Program arguments** → enter:
   ```
   power_quality_log.csv
   ```
5. Click **Run**.

### Option 2 — Command-line with `cmake`

```bash
cd power_quality_analyser
mkdir build && cd build
cmake ..
make
./analyser ../power_quality_log.csv
```

### Option 3 — Single `gcc` command (no CMake)

```bash
gcc -std=c99 -Wall -Wextra main.c waveform.c io.c -lm -o analyser
./analyser power_quality_log.csv
```

> **Note:** The `-lm` flag is required because the program uses `sqrt()` and `fabs()` from `<math.h>`.

---

## Expected output (excerpt)

```
Loaded 1000 samples from 'power_quality_log.csv'.

  Phase A:
    RMS voltage     : 229.xxxx V  [COMPLIANT]
    Peak-to-peak    : 650.xxxx V
    DC offset       : +0.000000 V
    Clipped samples : N
    Std deviation   : xxx.xxxx V
    Status flags    : 0x01

  Mean Frequency  : 50.000xxx Hz
  Mean PF         : 0.9560
  Mean THD        : 2.0xxx %
  Total clipped   : 60 samples across all phases

Report written to 'results.txt'.
```

---

## Error handling

| Scenario | Behaviour |
|---|---|
| File not found | Prints an error message to stderr and exits with `EXIT_FAILURE` |
| Empty file | Prints an error message and exits |
| Malformed row | Skips the row with a warning; continues processing |
| Cannot write results.txt | Prints a warning; program still exits cleanly |

---

## Key design decisions

- **One big struct**: I put all 8 CSV columns into a single `WaveformSample` struct because it's much easier to keep track of rows than using 8 separate arrays.
- **`offsetof()` trick**: Instead of copy-pasting the RMS and math functions three times, I use `offsetof()` to pass the exact byte location of the column I want to process.
- **Dynamic memory**: The program starts by allocating memory for 1024 rows. If the CSV is bigger than that, it doubles the size using `realloc`. I free the memory at the end to prevent leaks.
- **Organised files**: `main.c` runs the program, `waveform.c` does the math, and `io.c` handles the files. Keeping them separate makes it easier to read.


## GitHub Repository
https://github.com/Muaath2Almutairi/power-quality-analyser
