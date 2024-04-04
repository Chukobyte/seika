#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include "seika/defines.h"

typedef enum SkaAssetFileLoaderReadMode {
    SkaAssetFileLoaderReadMode_DISK,
    SkaAssetFileLoaderReadMode_ARCHIVE,
} SkaAssetFileLoaderReadMode;

typedef struct SkaArchiveFileAsset {
    void* buffer;
    usize bufferSize;
} SkaArchiveFileAsset;

typedef struct SkaAssetFileImageData {
    unsigned char* data;
    int32 width;
    int32 height;
    int32 nrChannels;
} SkaAssetFileImageData;

void ska_asset_file_loader_initialize();
void ska_asset_file_loader_finalize();
bool ska_asset_file_loader_load_archive(const char* filePath);
void ska_asset_file_loader_set_read_mode(SkaAssetFileLoaderReadMode readMode);
SkaAssetFileLoaderReadMode ska_asset_file_loader_get_read_mode();
SkaArchiveFileAsset ska_asset_file_loader_get_asset(const char* path);
SkaArchiveFileAsset ska_asset_file_loader_load_asset_from_disk(const char* path);
bool ska_asset_file_loader_is_asset_valid(SkaArchiveFileAsset* fileAsset);

// Asset loading types
SkaAssetFileImageData* ska_asset_file_loader_load_image_data(const char* filePath);
void ska_asset_file_loader_free_image_data(SkaAssetFileImageData* data);
char* ska_asset_file_loader_read_file_contents_as_string(const char* filePath, usize* size);
char* ska_asset_file_loader_read_file_contents_as_string_without_raw(const char* filePath, usize* size);

#ifdef __cplusplus
}
#endif
