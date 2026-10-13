#ifndef MUSYX_PC_MAP_STEP_TIMING_H
#define MUSYX_PC_MAP_STEP_TIMING_H
// Supplied reference: envelope peak 53.89 Hz, lag ~18.96 ms at 1 ms resolution.
// Nearest whole period at the native 32 kHz mixer: 594 samples (53.872 Hz).
#define MUSYX_MAP_STEP_NATIVE_HZ (32000.0 / 594.0)
#define MUSYX_MAP_STEP_MAX_RATIO 1.30
#define MUSYX_MAP_STEP_MAX_HZ (MUSYX_MAP_STEP_NATIVE_HZ * MUSYX_MAP_STEP_MAX_RATIO)
#endif
