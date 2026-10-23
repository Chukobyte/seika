#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

typedef enum SkaAssetFileLoaderReadMode {
    SkaAssetFileLoaderReadMode_DISK,
    SkaAssetFileLoaderReadMode_ARCHIVE,
} SkaAssetFileLoaderReadMode;

typedef struct SkaArchiveFileAsset {
    void* buffer;
    usize buffer_size;
} SkaArchiveFileAsset;

typedef struct SkaAssetFileImageData {
    unsigned char* data;
    int32 width;
    int32 height;
    int32 nr_channels;
} SkaAssetFileImageData;

void ska_asset_file_loader_initialize();
void ska_asset_file_loader_finalize();
bool ska_asset_file_loader_load_archive(const char* file_path);
void ska_asset_file_loader_set_read_mode(SkaAssetFileLoaderReadMode read_mode);
SkaAssetFileLoaderReadMode ska_asset_file_loader_get_read_mode();
SkaArchiveFileAsset ska_asset_file_loader_get_asset(const char* path);
SkaArchiveFileAsset ska_asset_file_loader_load_asset_from_disk(const char* path);
bool ska_asset_file_loader_is_asset_valid(SkaArchiveFileAsset* file_asset);

// Asset loading types
char* ska_asset_file_loader_read_file_contents_as_string(const char* file_path, usize* size);
char* ska_asset_file_loader_read_file_contents_as_string_without_raw(const char* file_path, usize* size);

#if SKA_RENDERING
SkaAssetFileImageData* ska_asset_file_loader_load_image_data(const char* file_path);
void ska_asset_file_loader_free_image_data(SkaAssetFileImageData* data);
#endif  // #if SKA_RENDERING

#ifdef __cplusplus
}
#endif
