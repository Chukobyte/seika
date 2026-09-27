#if SKA_AUDIO

#include "audio.h"
#include "audio_manager.h"
#include "seika/assert.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "seika/memory.h"
#include "seika/logger.h"
#include "seika/asset/asset_file_loader.h"

static uint32 audio_wav_sample_rate = 0;
static bool is_audio_initialized = false;

static bool load_wav_data_from_file(const char* file_path, int32* sample_count, int32* channels, int32* sample_rate, void** samples);

bool ska_audio_initialize() {
  SKA_ASSERT(is_audio_initialized == false);
  ska_audio_manager_init();
  is_audio_initialized = true;
  return true;
}

void ska_audio_finalize() {
  SKA_ASSERT(is_audio_initialized);
  ska_audio_manager_finalize();
  is_audio_initialized = false;
}

void ska_audio_print_audio_source(SkaAudioSource *audio_source) {
  ska_logger_debug("audio source | channels = %d, sample rate = %d, sample "
                   "count = %d, samples = %x",
                   audio_source->channels, audio_source->sample_rate,
                   audio_source->sample_count, audio_source->samples);
}

void ska_audio_set_wav_sample_rate(uint32 wav_sample_rate) {
  audio_wav_sample_rate = wav_sample_rate;
}

uint32 ska_audio_get_wav_sample_rate() { return audio_wav_sample_rate; }

SkaAudioSource *ska_audio_load_audio_source_wav(const char *file_name) {
  int32 sample_count;
  int32 channels;
  int32 sample_rate;
  void *samples = NULL;

  if (!load_wav_data_from_file(file_name, &sample_count, &channels,
                               &sample_rate, &samples)) {
    ska_logger_error("Failed to load audio wav file at '%s'", file_name);
    return NULL;
  }

  SkaAudioSource *new_audio_source = SKA_ALLOC(SkaAudioSource);
  new_audio_source->file_path = file_name;
  new_audio_source->pitch = 1.0;
  new_audio_source->sample_count = sample_count;
  new_audio_source->channels = channels;
  new_audio_source->sample_rate = sample_rate;
  new_audio_source->samples = samples;

  if ((uint32)new_audio_source->sample_rate != audio_wav_sample_rate) {
    const int32 input_frame_count = sample_count / channels;
    const f64 resample_ratio = (f64)audio_wav_sample_rate / sample_rate;
    const int32 output_frame_count =
        (int32)(input_frame_count * resample_ratio);

    int16_t *resampled_samples =
        SKA_ALLOC_BYTES(output_frame_count * channels * sizeof(int16_t));
    if (!resampled_samples) {
      ska_logger_error(
          "Failed to allocate memory for resampled audio for '%s'!", file_name);
      SKA_FREE(samples);
      SKA_FREE(new_audio_source);
      return NULL;
    }

    ma_data_converter_config converter_config = ma_data_converter_config_init(
        ma_format_s16, ma_format_s16, channels, channels,
        new_audio_source->sample_rate, audio_wav_sample_rate);
    ma_data_converter converter;
    if (ma_data_converter_init(&converter_config, NULL, &converter) !=
        MA_SUCCESS) {
      ska_logger_error(
          "Failed to initialize data converter for audio file '%s'!",
          file_name);
      SKA_FREE(samples);
      SKA_FREE(resampled_samples);
      SKA_FREE(new_audio_source);
      return NULL;
    }

    ma_uint64 in_frames = input_frame_count;
    ma_uint64 out_frames = output_frame_count;
    if (ma_data_converter_process_pcm_frames(&converter, resampled_samples,
                                             &out_frames, samples,
                                             &in_frames) != MA_SUCCESS) {
      ska_logger_error("Data conversion failed for audio file '%s'!",
                       file_name);
      SKA_FREE(samples);
      SKA_FREE(resampled_samples);
      ma_data_converter_uninit(&converter, NULL);
      SKA_FREE(new_audio_source);
      return NULL;
    }
    ma_data_converter_uninit(&converter, NULL);
    ska_logger_debug(
        "Resampled audio for '%s': inFrames = %d, outFrames = %llu", file_name,
        input_frame_count, out_frames);
    SKA_FREE(samples);
    new_audio_source->samples = resampled_samples;
    new_audio_source->sample_rate = (int32)audio_wav_sample_rate;
    new_audio_source->sample_count = (int32)out_frames * channels;
  }
  ska_audio_manager_register_source(new_audio_source);
  return new_audio_source;
}

bool load_wav_data_from_file(const char* file_path, int32* sample_count, int32* channels, int32* sample_rate, void** samples) {
    usize len = 0;
    char* file_data = ska_asset_file_loader_read_file_contents_as_string(file_path, &len);
    ska_logger_debug("file '%s' size '%u' bytes", file_path, len);

    drwav_uint64 total_pcm_frame_count = 0;
    *samples = drwav_open_memory_and_read_pcm_frames_s16(
        file_data, len, (uint32 *)channels, (uint32 *)sample_rate,
        &total_pcm_frame_count, NULL);
    SKA_FREE(file_data);

    if (!*samples) {
        *samples = NULL;
        ska_logger_error("Could not load .wav file: %s", file_path);
        return false;
    }

    *sample_count = (int32)total_pcm_frame_count * *channels;

    return true;
}

void ska_audio_delete_audio_source_wav(SkaAudioSource *audio_source) {
  ska_audio_manager_unregister_source(audio_source);
  SKA_FREE(audio_source);
}

#endif // #if SKA_AUDIO
