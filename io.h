#ifndef IO_H
#define IO_H

#include "waveform.h"

// loads the csv into an array
WaveformSample *load_csv(const char *filename, int *out_count);

// writes out the results.txt file
int write_report(const char *output_path,
                 const PhaseResult results[3],
                 const WaveformSample *samples,
                 int n);

#endif /* IO_H */
