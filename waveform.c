// waveform.c
// Contains the mathematical functions to calculate RMS, peak to peak, etc.
// The functions use a field_offset parameter (from offsetof) to know which phase to look at.


#include "waveform.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#include <float.h>    /* DBL_MAX for peak search             */

// Helper function to get the voltage from the struct using pointer arithmetic
static inline double get_voltage(const WaveformSample *p, int field_offset)
{
    // convert to char pointer so we can add the byte offset
    const char *base = (const char *)p;
    double value;
    // copy the bytes into our double variable (needed to prevent weird compiler bugs)
    memcpy(&value, base + field_offset, sizeof(double));
    return value;
}

// calculate RMS
// square each sample, find the mean, then square root
double compute_rms(const WaveformSample *samples, int n, int field_offset)
{
    if (n <= 0) return 0.0;

    double sum_sq = 0.0;
    const WaveformSample *p = samples;          /* pointer arithmetic */

    for (int i = 0; i < n; i++, p++) {
        double v = get_voltage(p, field_offset);
        sum_sq += v * v;
    }
    return sqrt(sum_sq / (double)n);
}

// peak to peak voltage (Vmax - Vmin)
double compute_peak_to_peak(const WaveformSample *samples, int n,
                            int field_offset)
{
    if (n <= 0) return 0.0;

    double v_max = -DBL_MAX;
    double v_min =  DBL_MAX;
    const WaveformSample *p = samples;

    for (int i = 0; i < n; i++, p++) {
        double v = get_voltage(p, field_offset);
        if (v > v_max) v_max = v;
        if (v < v_min) v_min = v;
    }
    return v_max - v_min;
}

// calculates average voltage (should be around 0)
double compute_dc_offset(const WaveformSample *samples, int n,
                         int field_offset)
{
    if (n <= 0) return 0.0;

    double sum = 0.0;
    const WaveformSample *p = samples;

    for (int i = 0; i < n; i++, p++) {
        sum += get_voltage(p, field_offset);
    }
    return sum / (double)n;
}

// returns the total number of clipped samples for the phase
int count_clipped(const WaveformSample *samples, int n, int field_offset)
{
    if (n <= 0) return 0;

    int count = 0;
    const WaveformSample *p = samples;

    for (int i = 0; i < n; i++, p++) {
        if (fabs(get_voltage(p, field_offset)) >= CLIP_THRESHOLD) {
            count++;
        }
    }
    return count;
}

// checks if we are within the ±10% tolerance
int check_compliance(double rms)
{
    return (rms >= TOLERANCE_LOW && rms <= TOLERANCE_HIGH) ? 1 : 0;
}

// extension: standard deviation using the 2 pass formula
double compute_std_dev(const WaveformSample *samples, int n,
                       int field_offset, double mean)
{
    if (n <= 0) return 0.0;

    double sum_sq_dev = 0.0;
    const WaveformSample *p = samples;

    for (int i = 0; i < n; i++, p++) {
        double dev = get_voltage(p, field_offset) - mean;
        sum_sq_dev += dev * dev;
    }
    return sqrt(sum_sq_dev / (double)n);
}

// mean frequency
double mean_frequency(const WaveformSample *samples, int n)
{
    if (n <= 0) return 0.0;

    double sum = 0.0;
    const WaveformSample *p = samples;

    for (int i = 0; i < n; i++, p++) {
        sum += p->frequency;
    }
    return sum / (double)n;
}

// mean pf
double mean_power_factor(const WaveformSample *samples, int n)
{
    if (n <= 0) return 0.0;

    double sum = 0.0;
    const WaveformSample *p = samples;

    for (int i = 0; i < n; i++, p++) {
        sum += p->power_factor;
    }
    return sum / (double)n;
}

// mean thd
double mean_thd(const WaveformSample *samples, int n)
{
    if (n <= 0) return 0.0;

    double sum = 0.0;
    const WaveformSample *p = samples;

    for (int i = 0; i < n; i++, p++) {
        sum += p->thd_percent;
    }
    return sum / (double)n;
}

// builds the PhaseResult struct for one phase
PhaseResult analyse_phase(const WaveformSample *samples, int n,
                          char label, int field_offset)
{
    PhaseResult r;
    r.label         = label;
    r.rms           = compute_rms(samples, n, field_offset);
    r.peak_to_peak  = compute_peak_to_peak(samples, n, field_offset);
    r.dc_offset     = compute_dc_offset(samples, n, field_offset);
    r.clipped_count = count_clipped(samples, n, field_offset);
    r.compliant     = check_compliance(r.rms);
    r.std_dev       = compute_std_dev(samples, n, field_offset, r.dc_offset);

    // Bitwise flags for merit extension
    r.status_flags = 0;
    if (r.clipped_count > 0) r.status_flags |= FLAG_CLIPPING;
    if (!r.compliant)        r.status_flags |= FLAG_OUT_OF_TOL;

    return r;
}
// Phase analysis logic implemented
