// io.c
// File IO functions for reading the csv and writing the results report

#include "io.h"
#include "waveform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// max string length for reading a single line
#define LINE_BUF_SIZE 256

// starting size for dynamic array
#define INITIAL_CAPACITY 1024

// opens the csv, reads the header, then mallocs an array and loops through all the lines
WaveformSample *load_csv(const char *filename, int *out_count)
{
    *out_count = 0;

    // open the file
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return NULL;
    }

    // clear the header out first
    char line[LINE_BUF_SIZE];
    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error: file '%s' appears to be empty.\n", filename);
        fclose(fp);
        return NULL;
    }

    // malloc the first array based on our constant
    int capacity = INITIAL_CAPACITY;
    WaveformSample *samples = malloc((size_t)capacity * sizeof(WaveformSample));
    if (samples == NULL) {
        fprintf(stderr, "Error: malloc failed during initial allocation.\n");
        fclose(fp);
        return NULL;
    }

    // loop over everything else
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {

        // skip empty lines at the bottom of the file
        if (line[0] == '\n' || line[0] == '\r') continue;

        // double the size if we hit our capacity using realloc
        if (count >= capacity) {
            capacity *= 2;
            WaveformSample *tmp = realloc(samples,
                                          (size_t)capacity * sizeof(WaveformSample));
            if (tmp == NULL) {
                fprintf(stderr,
                        "Error: realloc failed at row %d. Partial data returned.\n",
                        count);
                break;   /* return what we have so far */
            }
            samples = tmp;
        }

        // pointer arithmetic to parse all columns into the struct
        WaveformSample *s = samples + count;   /* pointer arithmetic */

        int parsed = sscanf(line,
            "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &s->timestamp,
            &s->phase_A_voltage,
            &s->phase_B_voltage,
            &s->phase_C_voltage,
            &s->line_current,
            &s->frequency,
            &s->power_factor,
            &s->thd_percent);

        if (parsed == 8) {
            count++;
        } else {
            fprintf(stderr,
                    "Warning: malformed row %d (parsed %d/8 fields), skipping.\n",
                    count + 1, parsed);
        }
    }

    fclose(fp);

    if (count == 0) {
        fprintf(stderr, "Error: no valid data rows found in '%s'.\n", filename);
        free(samples);
        return NULL;
    }

    *out_count = count;
    return samples;
}

// writes out the final file output
static void print_phase_block(FILE *fp, const PhaseResult *r)
{
    fprintf(fp, "Phase %c\n", r->label);
    fprintf(fp, "\n");
    fprintf(fp, "RMS Voltage         : %8.4f V\n", r->rms);
    fprintf(fp, "Peak-to-Peak        : %8.4f V\n", r->peak_to_peak);
    fprintf(fp, "DC Offset           : %+.6f V\n", r->dc_offset);
    fprintf(fp, "Clipped Samples     : %d\n",      r->clipped_count);
    fprintf(fp, "Standard Deviation  : %8.4f V\n", r->std_dev);

    /* Tolerance compliance */
    if (r->compliant) {
        fprintf(fp, "EN 50160 Compliance : COMPLIANT  "
                    "(%.2f V is within 207–253 V)\n", r->rms);
    } else {
        fprintf(fp, "EN 50160 Compliance : NON-COMPLIANT "
                    "(%.2f V is OUTSIDE 207–253 V) \n", r->rms);
    }

    /* Bitwise status flags (Merit extension) */
    fprintf(fp, "Status Flags (hex)  : 0x%02X", (unsigned)r->status_flags);
    if (r->status_flags == 0) {
        fprintf(fp, "OK — no anomalies\n");
    } else {
        fprintf(fp, "  [");
        if (r->status_flags & FLAG_CLIPPING)   fprintf(fp, "CLIPPING ");
        if (r->status_flags & FLAG_OUT_OF_TOL) fprintf(fp, "OUT-OF-TOLERANCE ");
        fprintf(fp, "]\n");
    }
    fprintf(fp, "\n");
}

// write all the results out to the text file
int write_report(const char *output_path,
                 const PhaseResult results[3],
                 const WaveformSample *samples,
                 int n)
{
    FILE *fp = fopen(output_path, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open output file '%s'\n", output_path);
        return 0;
    }

    /* header */
    fprintf(fp, "POWER QUALITY WAVEFORM ANALYSER — RESULTS REPORT\n");
    fprintf(fp, "UGMFGT-15-1 Programming for Engineers\n");
    fprintf(fp, "\n\n");

    fprintf(fp, "Dataset: %d samples  |  "
                "Window: %.4f – %.4f s\n\n",
            n,
            samples->timestamp,                  /* first sample */
            (samples + n - 1)->timestamp);        /* last sample  */

    /* per-phase metrics */
    fprintf(fp, "\n");
    fprintf(fp, "PER-PHASE VOLTAGE ANALYSIS\n");
    fprintf(fp, "\n\n");

    for (int i = 0; i < 3; i++) {
        print_phase_block(fp, &results[i]);
    }

    /* dataset-wide stats */
    fprintf(fp, "\n");
    fprintf(fp, "  DATASET-WIDE STATISTICS\n");
    fprintf(fp, "\n\n");

    fprintf(fp, "Mean Frequency      : %.6f Hz  (nominal 50.000 Hz)\n",
            mean_frequency(samples, n));
    fprintf(fp, "Mean Power Factor   : %.4f        (nominal 0.95)\n",
            mean_power_factor(samples, n));
    fprintf(fp, "Mean THD            : %.4f %%    "
                "(EN 50160 limit: 8%%)\n",
            mean_thd(samples, n));

    /* Total clipped samples across all phases */
    int total_clipped = results[0].clipped_count
                      + results[1].clipped_count
                      + results[2].clipped_count;
    fprintf(fp, "Total Clipped Samples (all phases): %d\n\n", total_clipped);

    /* anomaly summary */
    fprintf(fp, "\n");
    fprintf(fp, "ANOMALY SUMMARY\n");
    fprintf(fp, "\n\n");

    int any_anomaly = 0;
    for (int i = 0; i < 3; i++) {
        if (results[i].status_flags != 0) {
            any_anomaly = 1;
            fprintf(fp, "Phase %c: ", results[i].label);
            if (results[i].status_flags & FLAG_CLIPPING)
                fprintf(fp, "[CLIPPING: %d samples] ",
                        results[i].clipped_count);
            if (results[i].status_flags & FLAG_OUT_OF_TOL)
                fprintf(fp, "[OUT OF TOLERANCE: %.4f V] ",
                        results[i].rms);
            fprintf(fp, "\n");
        }
    }
    if (!any_anomaly) {
        fprintf(fp, "No compliance anomalies detected.\n");
    }

    fprintf(fp, "\n\n");
    fprintf(fp, "END OF REPORT\n");
    fprintf(fp, "\n");

    fclose(fp);
    return 1;
}
// Documented CSV loader
