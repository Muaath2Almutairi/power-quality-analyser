#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stdint.h>

// Struct to hold all the data from a single row in the CSV file
typedef struct {
    double timestamp;        /* seconds — time of this sample        */
    double phase_A_voltage;  /* Volts — instantaneous Phase A voltage */
    double phase_B_voltage;  /* Volts — instantaneous Phase B voltage */
    double phase_C_voltage;  /* Volts — instantaneous Phase C voltage */
    double line_current;     /* Amperes — line current magnitude      */
    double frequency;        /* Hz — instantaneous frequency estimate */
    double power_factor;     /* dimensionless 0–1                    */
    double thd_percent;      /* % — Total Harmonic Distortion         */
} WaveformSample;

// Struct to hold our final analysis results for each phase
typedef struct {
    char   label;            /* 'A', 'B', or 'C'                     */
    double rms;              /* V RMS                                 */
    double peak_to_peak;     /* V  (Vmax − Vmin)                      */
    double dc_offset;        /* V  (arithmetic mean of all samples)   */
    int    clipped_count;    /* number of samples with |V| >= 324.9 V */
    int    compliant;        /* 1 = within EN 50160 band, 0 = outside */
    double std_dev;          /* V — standard deviation (Merit ext.)   */
    uint8_t status_flags;    /* bitmask — see above                   */
} PhaseResult;

// Limit for the sensor clipping
#define CLIP_THRESHOLD    324.9

// EN 50160 limits
#define NOMINAL_VOLTAGE   230.0
#define TOLERANCE_LOW     207.0
#define TOLERANCE_HIGH    253.0

// flags for the merit extension
#define FLAG_CLIPPING     (1u << 0)
#define FLAG_OUT_OF_TOL   (1u << 1)

// functions implemented in waveform.c

// calculates RMS voltage
double compute_rms(const WaveformSample *samples, int n, int field_offset);

// calculates peak to peak
double compute_peak_to_peak(const WaveformSample *samples, int n,
                            int field_offset);

// calculates dc offset
double compute_dc_offset(const WaveformSample *samples, int n,
                         int field_offset);

// count how many times it clips
int count_clipped(const WaveformSample *samples, int n, int field_offset);

// checks if rms is in tolerance
int check_compliance(double rms);

// Merit extension: returns std dev
double compute_std_dev(const WaveformSample *samples, int n,
                       int field_offset, double mean);

// mean freq
double mean_frequency(const WaveformSample *samples, int n);

// mean pf
double mean_power_factor(const WaveformSample *samples, int n);

// mean thd
double mean_thd(const WaveformSample *samples, int n);

// runs all the analysis functions for a given phase
PhaseResult analyse_phase(const WaveformSample *samples, int n,
                          char label, int field_offset);

#endif /* WAVEFORM_H */
// Added struct documentation
// Extra merit flags added
