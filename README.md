# Power Quality Waveform Analyser
**UGMFGT-15-1 Programming for Engineers**
Electrical and Electronic Engineering | University of the West of England | 2025–26

## What this program does

This command-line C program reads a power quality sensor log (CSV), computes key waveform metrics for each of three phases (A, B, C) of a 3-phase 50 Hz industrial supply, and writes a structured report to `results.txt`.

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
├── main.c          Entry point — argument handling and orchestration only
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
  Total clipped   : 20 samples across all phases

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

- **Single struct, eight fields**: All 8 CSV columns live in one `WaveformSample`. This avoids synchronisation bugs between parallel arrays and maps directly to the CSV row layout.
- **`offsetof()`-based field selection**: Analysis functions accept a `field_offset` parameter (obtained with `offsetof()`) to locate the correct voltage field via pointer arithmetic. This means `compute_rms()`, `compute_dc_offset()`, etc. work on any phase without duplication.
- **Dynamic allocation with realloc growth**: The CSV loader starts at 1 024 slots and doubles capacity when needed. The final pointer is passed to `free()` before `main()` returns.
- **Separation of concerns**: `main.c` orchestrates; `waveform.c` analyses; `io.c` handles all file operations. No analysis logic leaks into I/O code or vice versa.
