#include "asset_file_loader.h"

#if SKA_RENDERING
#include <stb_image/stb_image.h>

#include "seika/memory.h"
#endif

#include <zip.h>

#include "seika/string.h"
#include "seika/file_system.h"

SkaAssetFileLoaderReadMode global_read_mode = SkaAssetFileLoaderReadMode_DISK;
struct zip_t* package_archive = NULL;

static void clear_package_archive() {
    if (package_archive != NULL) {
        zip_entry_close(package_archive);
        package_archive = NULL;
    }
}

void ska_asset_file_loader_initialize() {}

void ska_asset_file_loader_finalize() {
    clear_package_archive();
}

bool ska_asset_file_loader_load_archive(const char* file_path) {
    if (ska_fs_does_file_exist(file_path)) {
        clear_package_archive();
        package_archive = zip_open(file_path, 0, 'r');
        return true;
    }
    return false;
}

void ska_asset_file_loader_set_read_mode(SkaAssetFileLoaderReadMode read_mode) {
    global_read_mode = read_mode;
}

SkaAssetFileLoaderReadMode ska_asset_file_loader_get_read_mode() {
    return global_read_mode;
}

SkaArchiveFileAsset ska_asset_file_loader_get_asset(const char* path) {
    void* file_buffer = NULL;
    usize file_buffer_size;
    zip_entry_open(package_archive, path);
    {
        zip_entry_read(package_archive, &file_buffer, &file_buffer_size);
    }
    return (SkaArchiveFileAsset) {
        .buffer = file_buffer,
        .buffer_size = file_buffer_size
    };
}

SkaArchiveFileAsset ska_asset_file_loader_load_asset_from_disk(const char* path) {
    SkaArchiveFileAsset asset = {NULL, 0 };
    if (ska_fs_does_file_exist(path)) {
        asset.buffer = ska_fs_read_file_contents(path, &asset.buffer_size);
    }
    return asset;
}

bool ska_asset_file_loader_is_asset_valid(SkaArchiveFileAsset* file_asset) {
    return file_asset != NULL && file_asset->buffer != NULL;
}

char* ska_asset_file_loader_read_file_contents_as_string(const char* file_path, usize* size) {
    char* file_string = NULL;
    usize len = 0;
    if (global_read_mode == SkaAssetFileLoaderReadMode_DISK) {
        if (ska_fs_does_file_exist(file_path)) {
            file_string = ska_fs_read_file_contents(file_path, &len);
        }
    } else if (global_read_mode == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset file_asset = ska_asset_file_loader_get_asset(file_path);
        if (ska_asset_file_loader_is_asset_valid(&file_asset)) {
            file_string = ska_strdup_from_memory(file_asset.buffer, file_asset.buffer_size);
            len = file_asset.buffer_size;
        }
    }
    if (size) {
        *size = len;
    }
    return file_string;
}

char* ska_asset_file_loader_read_file_contents_as_string_without_raw(const char* file_path, usize* size) {
    char* file_string = NULL;
    usize len = 0;
    if (global_read_mode == SkaAssetFileLoaderReadMode_DISK) {
        if (ska_fs_does_file_exist(file_path)) {
            file_string = ska_fs_read_file_contents_without_raw(file_path, &len);
        }
    } else if (global_read_mode == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset file_asset = ska_asset_file_loader_get_asset(file_path);
        if (ska_asset_file_loader_is_asset_valid(&file_asset)) {
            file_string = ska_strdup_from_memory(file_asset.buffer, file_asset.buffer_size);
            ska_str_remove_char(file_string, '\r');
            len = file_asset.buffer_size;
        }
    }
    if (size) {
        *size = len;
    }
    return file_string;
}

#if SKA_RENDERING
SkaAssetFileImageData* ska_asset_file_loader_load_image_data(const char* file_path) {
    SkaAssetFileImageData* image_data = NULL;
    stbi_set_flip_vertically_on_load(false);
    if (global_read_mode == SkaAssetFileLoaderReadMode_DISK) {
        image_data = SKA_ALLOC(SkaAssetFileImageData);
        image_data->data = stbi_load(file_path, &image_data->width, &image_data->height, &image_data->nr_channels, 0);
    } else if (global_read_mode == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset file_asset = ska_asset_file_loader_get_asset(file_path);
        if (ska_asset_file_loader_is_asset_valid(&file_asset)) {
            image_data = SKA_ALLOC(SkaAssetFileImageData);
            image_data->data = stbi_load_from_memory(file_asset.buffer, (int32)file_asset.buffer_size, &image_data->width, &image_data->height, &image_data->nr_channels, 0);
        }
    }
    return image_data;
}

void ska_asset_file_loader_free_image_data(SkaAssetFileImageData* data) {
    stbi_image_free(data->data);
    SKA_FREE(data);
}
#endif // #if SKA_RENDERING
