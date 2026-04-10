// main.c
// Main entry point for the power quality waveform analyser
// Parses command line arguments and calls the analysis and IO functions


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>   /* offsetof() */

#include "waveform.h"
#include "io.h"

// output file for results
#define OUTPUT_FILE "results.txt"

int main(int argc, char *argv[])
{
    // check if the user gave the right number of arguments
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s <path_to_csv>\n"
                "Example: %s power_quality_log.csv\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    const char *csv_path = argv[1];

    // load the CSV into our dynamic array
    int n = 0;
    WaveformSample *samples = load_csv(csv_path, &n);

    if (samples == NULL || n == 0) {
        // error already printed by load_csv
        free(samples);
        return EXIT_FAILURE;
    }

    printf("Loaded %d samples from '%s'.\n\n", n, csv_path);

    // analyse the 3 phases 
    // using offsetof to pass the byte offset of each voltage column so we can 
    // reuse the analysis functions and not copy paste them.
    PhaseResult results[3];
    results[0] = analyse_phase(samples, n, 'A',
                    (int)offsetof(WaveformSample, phase_A_voltage));
    results[1] = analyse_phase(samples, n, 'B',
                    (int)offsetof(WaveformSample, phase_B_voltage));
    results[2] = analyse_phase(samples, n, 'C',
                    (int)offsetof(WaveformSample, phase_C_voltage));

    // print summary to screen
    printf("POWER QUALITY WAVEFORM ANALYSER - CONSOLE SUMMARY\n");

    for (int i = 0; i < 3; i++) {
        const PhaseResult *r = &results[i];
        printf("Phase %c:\n", r->label);
        printf("RMS voltage     : %.4f V  %s\n",
               r->rms,
               r->compliant ? "COMPLIANT" : "NON-COMPLIANT");
        printf("Peak-to-peak    : %.4f V\n",  r->peak_to_peak);
        printf("DC offset       : %+.6f V\n", r->dc_offset);
        printf("Clipped samples : %d\n",       r->clipped_count);
        printf("Std deviation   : %.4f V\n",   r->std_dev);
        printf("Status flags    : 0x%02X\n\n", (unsigned)r->status_flags);
    }

    int total_clipped = results[0].clipped_count
                      + results[1].clipped_count
                      + results[2].clipped_count;

    printf("Mean Frequency  : %.6f Hz\n", mean_frequency(samples, n));
    printf("Mean PF         : %.4f\n",    mean_power_factor(samples, n));
    printf("Mean THD        : %.4f %%\n", mean_thd(samples, n));
    printf("Total clipped   : %d samples across all phases\n\n",
           total_clipped);

    // create the results txt file
    if (write_report(OUTPUT_FILE, results, samples, n)) {
        printf("Report written to '%s'.\n", OUTPUT_FILE);
        printf("\n");
    } else {
        fprintf(stderr, "Warning: report could not be written to '%s'.\n",
                OUTPUT_FILE);
    }

    // free the malloc array so no memory leaks
    free(samples);
    samples = NULL;

    return EXIT_SUCCESS;
}
