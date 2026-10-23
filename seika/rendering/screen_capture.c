#if SKA_RENDERING

#include "screen_capture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stb_image/stb_image_write.h>
#include <msf_gif.h>

#include "ska_opengl.h"
#include "../logger.h"

#define SKA_SCREENSHOT_PATH_MAX 512

static char screenshot_pending_path[SKA_SCREENSHOT_PATH_MAX] = {0};
static bool screenshot_pending = false;

typedef struct GifRecorderState {
    bool active;      // true from start() until stop() is called
    bool capped;       // true once maxFrames is reached; stops sampling but keeps the session open for stop()
    bool msf_started;  // true once msf_gif_begin() has been called for this
                       // session
    uint32 frame_counter;
    uint32 frames_captured;
    SkaGifRecorderConfig config;
    MsfGifState msf_state;
} GifRecorderState;

static GifRecorderState gif_recorder = {0};

// Reads the current default framebuffer into a newly malloc'd buffer, flipped so row 0 is the
// top of the image (OpenGL's readback origin is bottom-left). Caller frees the result.
static uint8 *read_pixels_flipped(int32 width, int32 height, GLenum format,
                                  int32 num_channels) {
  const usize row_size = (usize)width * (usize)num_channels;
  uint8 *pixels = (uint8 *)malloc(row_size * (usize)height);
  if (!pixels) {
    ska_logger_error("Screen capture: failed to allocate readback buffer");
    return NULL;
  }
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);

  uint8 *flipped = (uint8 *)malloc(row_size * (usize)height);
  if (!flipped) {
    ska_logger_error("Screen capture: failed to allocate flip buffer");
    free(pixels);
    return NULL;
  }
  for (int32 row = 0; row < height; row++) {
    memcpy(flipped + (usize)row * row_size,
           pixels + (usize)(height - 1 - row) * row_size, row_size);
  }
  free(pixels);
  return flipped;
}

bool ska_screenshot_capture(const char *file_path) {
  if (screenshot_pending) {
    ska_logger_warn("Screen capture: a screenshot is already pending this "
                    "frame, ignoring '%s'",
                    file_path);
    return false;
  }
  const usize len = strlen(file_path);
  if (len >= SKA_SCREENSHOT_PATH_MAX) {
    ska_logger_error("Screen capture: file path too long: %s", file_path);
    return false;
  }
  memcpy(screenshot_pending_path, file_path, len + 1);
  screenshot_pending = true;
  return true;
}

bool ska_gif_recorder_start(SkaGifRecorderConfig config) {
  if (gif_recorder.active) {
    ska_logger_warn(
        "Screen capture: gif recorder already active, ignoring start request");
    return false;
  }
  if (config.sample_every_n_frames == 0) {
    config.sample_every_n_frames = 1;
  }
  gif_recorder = (GifRecorderState){.active = true, .config = config};
  return true;
}

bool ska_gif_recorder_is_recording(void) {
  return gif_recorder.active && !gif_recorder.capped;
}

static bool gif_recorder_write_result(const char *file_path,
                                      MsfGifResult result) {
  if (!result.data) {
    return false;
  }
  FILE *file = fopen(file_path, "wb");
  if (!file) {
    ska_logger_error("Screen capture: failed to open '%s' for writing gif",
                     file_path);
    msf_gif_free(result);
    return false;
  }
  const size_t written = fwrite(result.data, 1, result.dataSize, file);
  fclose(file);
  msf_gif_free(result);
  return written == result.dataSize;
}

bool ska_gif_recorder_stop(const char *file_path) {
  if (!gif_recorder.active) {
    return false;
  }
  bool success = false;
  if (gif_recorder.msf_started) {
    MsfGifResult result = msf_gif_end(&gif_recorder.msf_state);
    success = gif_recorder_write_result(file_path, result);
  }
  gif_recorder = (GifRecorderState){0};
  return success;
}

static void gif_recorder_on_frame_end(int32 window_width, int32 window_height) {
  if (!gif_recorder.active || gif_recorder.capped) {
    return;
  }
  gif_recorder.frame_counter++;
  if (gif_recorder.frame_counter % gif_recorder.config.sample_every_n_frames !=
      0) {
    return;
  }

  if (!gif_recorder.msf_started) {
    if (!msf_gif_begin(&gif_recorder.msf_state, window_width, window_height)) {
      ska_logger_error("Screen capture: failed to begin gif recording");
      gif_recorder.active = false;
      return;
    }
    gif_recorder.msf_started = true;
  }

  uint8 *pixels = read_pixels_flipped(window_width, window_height, GL_RGBA, 4);
  if (pixels) {
    const int32 stride = window_width * 4;
    msf_gif_frame(&gif_recorder.msf_state, pixels,
                  (int32)gif_recorder.config.centi_seconds_per_frame, 16,
                  stride);
    free(pixels);
    gif_recorder.frames_captured++;
  }

  if (gif_recorder.frames_captured >= gif_recorder.config.max_frames) {
    ska_logger_warn("Screen capture: gif recorder hit max frame cap (%u); call "
                    "ska_gif_recorder_stop() to save it",
                    gif_recorder.config.max_frames);
    gif_recorder.capped = true;
  }
}

void ska_screen_capture_on_frame_end(int32 window_width, int32 window_height) {
  if (window_width <= 0 || window_height <= 0) {
    return;
  }

  if (screenshot_pending) {
    uint8 *pixels = read_pixels_flipped(window_width, window_height, GL_RGB, 3);
    if (pixels) {
      const int32 stride = window_width * 3;
      if (!stbi_write_png(screenshot_pending_path, window_width, window_height,
                          3, pixels, stride)) {
        ska_logger_error("Screen capture: failed to write screenshot to '%s'",
                         screenshot_pending_path);
      }
      free(pixels);
    }
    screenshot_pending = false;
    screenshot_pending_path[0] = '\0';
  }

  gif_recorder_on_frame_end(window_width, window_height);
}

#endif // #if SKA_RENDERING
