#pragma once

#if SKA_RENDERING

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "seika/defines.h"

// One-shot screenshot. Captures at the end of the *current* frame (right
// before the next buffer swap) and writes a PNG. Safe to call from anywhere
// in game code during the frame.
bool ska_screenshot_capture(const char* file_path);

// GIF recording — buffers sampled frames in memory; call stop() to encode
// and write the file.
typedef struct SkaGifRecorderConfig {
    uint32 max_frames;            // hard cap on buffered frames (memory safety)
    uint32 sample_every_n_frames;   // e.g. 2 = sample every other frame
    uint32 centi_seconds_per_frame; // playback speed of the encoded gif
} SkaGifRecorderConfig;

bool ska_gif_recorder_start(SkaGifRecorderConfig config);
// Encodes and writes the recording; no-op (returns false) if not recording.
bool ska_gif_recorder_stop(const char* file_path);
bool ska_gif_recorder_is_recording(void);

// Called by the window backend right before swapping buffers each frame.
void ska_screen_capture_on_frame_end(int32 window_width, int32 window_height);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_RENDERING
