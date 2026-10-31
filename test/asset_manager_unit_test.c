#include <unity.h>
#include <seika/asset/asset_manager.h>
#include <seika/audio/audio.h>
#include <stdio.h>

void setUp(void) {}
void tearDown(void) {}

#if SKA_AUDIO
static const char* WAV_PATH = "asset_manager_unit_test.wav";
#define GROWN_SOURCE_COUNT 48   // past the map's first growth, under SKA_MAX_AUDIO_SOURCES

static void write_u16(FILE* file, uint32 value) {
    fputc(value & 0xFF, file);
    fputc((value >> 8) & 0xFF, file);
}

static void write_u32(FILE* file, uint32 value) {
    write_u16(file, value & 0xFFFF);
    write_u16(file, value >> 16);
}

// 16-bit mono PCM with two silent samples
static void write_test_wav(uint32 sample_rate) {
    FILE* file = fopen(WAV_PATH, "wb");
    TEST_ASSERT_NOT_NULL(file);
    fputs("RIFF", file);
    write_u32(file, 36 + 4);
    fputs("WAVEfmt ", file);
    write_u32(file, 16);
    write_u16(file, 1);
    write_u16(file, 1);
    write_u32(file, sample_rate);
    write_u32(file, sample_rate * 2);
    write_u16(file, 2);
    write_u16(file, 16);
    fputs("data", file);
    write_u32(file, 4);
    write_u32(file, 0);
    fclose(file);
}

// Returned assets stay valid while later loads grow the manager's map
void seika_asset_manager_pointer_survives_growth_test(void) {
    const uint32 sample_rate = 44100;
    write_test_wav(sample_rate);
    ska_audio_set_wav_sample_rate(sample_rate);
    ska_asset_manager_initialize();

    SkaAudioSource* first = ska_asset_manager_try_load_audio_source_wav(WAV_PATH, "key_0");
    TEST_ASSERT_NOT_NULL(first);
    for (int32 i = 1; i < GROWN_SOURCE_COUNT; i++) {
        char key[16];
        snprintf(key, sizeof(key), "key_%d", i);
        TEST_ASSERT_NOT_NULL(ska_asset_manager_try_load_audio_source_wav(WAV_PATH, key));
    }
    TEST_ASSERT_EQUAL_PTR(first, ska_asset_manager_get_audio_source("key_0"));
    TEST_ASSERT_EQUAL_PTR(first, ska_asset_manager_try_load_audio_source_wav(WAV_PATH, "key_0"));
    TEST_ASSERT_TRUE(ska_asset_manager_has_audio_source("key_1"));
    TEST_ASSERT_NULL(ska_asset_manager_get_audio_source("missing"));

    ska_asset_manager_finalize();
    remove(WAV_PATH);
}
#endif

int main(void) {
    UNITY_BEGIN();
#if SKA_AUDIO
    RUN_TEST(seika_asset_manager_pointer_survives_growth_test);
#endif
    return UNITY_END();
}
