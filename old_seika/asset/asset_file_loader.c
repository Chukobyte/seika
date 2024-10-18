#include "asset_file_loader.h"

#include <stb_image/stb_image.h>
#include <zip.h>

#include "seika/string.h"
#include "seika/file_system.h"
#include "seika/memory.h"

SkaAssetFileLoaderReadMode globalReadMode = SkaAssetFileLoaderReadMode_DISK;
struct zip_t* packageArchive = NULL;

static void clear_package_archive() {
    if (packageArchive != NULL) {
        zip_entry_close(packageArchive);
        packageArchive = NULL;
    }
}

void ska_asset_file_loader_initialize() {}

void ska_asset_file_loader_finalize() {
    clear_package_archive();
}

bool ska_asset_file_loader_load_archive(const char* filePath) {
    if (ska_fs_does_file_exist(filePath)) {
        clear_package_archive();
        packageArchive = zip_open(filePath, 0, 'r');
        return true;
    }
    return false;
}

void ska_asset_file_loader_set_read_mode(SkaAssetFileLoaderReadMode readMode) {
    globalReadMode = readMode;
}

SkaAssetFileLoaderReadMode ska_asset_file_loader_get_read_mode() {
    return globalReadMode;
}

SkaArchiveFileAsset ska_asset_file_loader_get_asset(const char* path) {
    void* fileBuffer = NULL;
    usize fileBufferSize;
    zip_entry_open(packageArchive, path);
    {
        zip_entry_read(packageArchive, &fileBuffer, &fileBufferSize);
    }
    return (SkaArchiveFileAsset) {
        .buffer = fileBuffer,
        .bufferSize = fileBufferSize
    };
}

SkaArchiveFileAsset ska_asset_file_loader_load_asset_from_disk(const char* path) {
    SkaArchiveFileAsset asset = {NULL, 0 };
    if (ska_fs_does_file_exist(path)) {
        asset.buffer = ska_fs_read_file_contents(path, &asset.bufferSize);
    }
    return asset;
}

bool ska_asset_file_loader_is_asset_valid(SkaArchiveFileAsset* fileAsset) {
    return fileAsset != NULL && fileAsset->buffer != NULL;
}

SkaAssetFileImageData* ska_asset_file_loader_load_image_data(const char* filePath) {
    SkaAssetFileImageData* imageData = NULL;
    stbi_set_flip_vertically_on_load(false);
    if (globalReadMode == SkaAssetFileLoaderReadMode_DISK) {
        imageData = SKA_MEM_ALLOCATE(SkaAssetFileImageData);
        imageData->data = stbi_load(filePath, &imageData->width, &imageData->height, &imageData->nrChannels, 0);
    } else if (globalReadMode == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset fileAsset = ska_asset_file_loader_get_asset(filePath);
        if (ska_asset_file_loader_is_asset_valid(&fileAsset)) {
            imageData = SKA_MEM_ALLOCATE(SkaAssetFileImageData);
            imageData->data = stbi_load_from_memory(fileAsset.buffer, (int32)fileAsset.bufferSize, &imageData->width, &imageData->height, &imageData->nrChannels, 0);
        }
    }
    return imageData;
}

void ska_asset_file_loader_free_image_data(SkaAssetFileImageData* data) {
    stbi_image_free(data->data);
    SKA_MEM_FREE(data);
}

char* ska_asset_file_loader_read_file_contents_as_string(const char* filePath, usize* size) {
    char* fileString = NULL;
    usize len = 0;
    if (globalReadMode == SkaAssetFileLoaderReadMode_DISK) {
        if (ska_fs_does_file_exist(filePath)) {
            fileString = ska_fs_read_file_contents(filePath, &len);
        }
    } else if (globalReadMode == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset fileAsset = ska_asset_file_loader_get_asset(filePath);
        if (ska_asset_file_loader_is_asset_valid(&fileAsset)) {
            fileString = ska_strdup_from_memory(fileAsset.buffer, fileAsset.bufferSize);
            len = fileAsset.bufferSize;
        }
    }
    if (size) {
        *size = len;
    }
    return fileString;
}

char* ska_asset_file_loader_read_file_contents_as_string_without_raw(const char* filePath, usize* size) {
    char* fileString = NULL;
    usize len = 0;
    if (globalReadMode == SkaAssetFileLoaderReadMode_DISK) {
        if (ska_fs_does_file_exist(filePath)) {
            fileString = ska_fs_read_file_contents_without_raw(filePath, &len);
        }
    } else if (globalReadMode == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset fileAsset = ska_asset_file_loader_get_asset(filePath);
        if (ska_asset_file_loader_is_asset_valid(&fileAsset)) {
            fileString = ska_strdup_from_memory(fileAsset.buffer, fileAsset.bufferSize);
            ska_str_remove_char(fileString, '\r');
            len = fileAsset.bufferSize;
        }
    }
    if (size) {
        *size = len;
    }
    return fileString;
}
