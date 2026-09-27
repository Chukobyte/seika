// Offline dev tool: parses an SDL gamecontrollerdb.txt file and emits seika's own generated
// gamepad mapping table (seika/seika/input/gamepad_db_generated.c). Run manually, output is
// checked into git -- see seika/thirdparty/gamecontrollerdb/README.md.
//
// Usage: seika_gamepad_db_gen <path-to-gamecontrollerdb.txt> <path-to-output.c>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <seika/defines.h>
#include <seika/file_system.h>
#include <seika/string.h>
#include <seika/data_structures/array_list_template.h>
#include <seika/data_structures/hash_map_template.h>
#include <seika/input/input.h>
#include <seika/input/gamepad_db.h>

#define MAX_ENTRIES_PER_CONTROLLER 32
#define MAX_GUID_LEN 64
#define MAX_NAME_LEN 128

typedef enum GenPlatform {
    GenPlatform_WINDOWS,
    GenPlatform_LINUX,
    GenPlatform_MACOS,
    GenPlatform_COUNT,
    GenPlatform_SKIP,
} GenPlatform;

static const char* gen_platform_guard[GenPlatform_COUNT] = {
    "PLATFORM_WINDOWS",
    "PLATFORM_LINUX",
    "PLATFORM_MACOS",
};

typedef struct ParsedController {
    char guid[MAX_GUID_LEN];
    char name[MAX_NAME_LEN];
    SkaGamepadMappingEntry entries[MAX_ENTRIES_PER_CONTROLLER];
    uint32 entry_count;
} ParsedController;

SKA_ARRAY_LIST_DECLARE(ParsedControllerList, ParsedController)
SKA_ARRAY_LIST_IMPLEMENT(ParsedControllerList, ParsedController)

// Fixed-size wrapper so the guid can be used as a SKA_HASH_MAP_* KEY_TYPE: the template's
// default hash/compare operate on raw sizeof(KEY_TYPE) bytes via memcmp, which only gives
// correct string-content equality for a fixed-size, zero-padded buffer like this one (a plain
// 'char*' key would hash/compare pointer values, not string contents).
typedef struct GuidKey {
    char bytes[MAX_GUID_LEN];
} GuidKey;

static GuidKey make_guid_key(const char* guid) {
    GuidKey key = {0};
    memcpy(key.bytes, guid, MAX_GUID_LEN);
    return key;
}

SKA_HASH_MAP_DECLARE(GuidToIndexMap, GuidKey, usize)
SKA_HASH_MAP_IMPLEMENT(GuidToIndexMap, GuidKey, usize)

// Maps an SDL gamecontrollerdb.txt key name to seika's SkaInputKey_GAMEPAD_* enum. Returns
// SkaInputKey_INVALID for names with no seika equivalent.
static SkaInputKey sdl_key_name_to_ska_key(const char* name, usize len) {
    struct { const char* name; SkaInputKey key; } static const table[] = {
        {"a", SkaInputKey_GAMEPAD_FACE_BUTTON_SOUTH},
        {"b", SkaInputKey_GAMEPAD_FACE_BUTTON_EAST},
        {"x", SkaInputKey_GAMEPAD_FACE_BUTTON_WEST},
        {"y", SkaInputKey_GAMEPAD_FACE_BUTTON_NORTH},
        {"back", SkaInputKey_GAMEPAD_BACK},
        {"start", SkaInputKey_GAMEPAD_START},
        {"guide", SkaInputKey_GAMEPAD_GUIDE},
        {"leftshoulder", SkaInputKey_GAMEPAD_LEFT_SHOULDER},
        {"rightshoulder", SkaInputKey_GAMEPAD_RIGHT_SHOULDER},
        {"leftstick", SkaInputKey_GAMEPAD_LEFT_ANALOG_BUTTON},
        {"rightstick", SkaInputKey_GAMEPAD_RIGHT_ANALOG_BUTTON},
        {"dpup", SkaInputKey_GAMEPAD_DPAD_UP},
        {"dpdown", SkaInputKey_GAMEPAD_DPAD_DOWN},
        {"dpleft", SkaInputKey_GAMEPAD_DPAD_LEFT},
        {"dpright", SkaInputKey_GAMEPAD_DPAD_RIGHT},
        {"leftx", SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X},
        {"lefty", SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y},
        {"rightx", SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X},
        {"righty", SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y},
        {"lefttrigger", SkaInputKey_GAMEPAD_LEFT_TRIGGER},
        {"righttrigger", SkaInputKey_GAMEPAD_RIGHT_TRIGGER},
        {"paddle1", SkaInputKey_GAMEPAD_PADDLE1},
        {"paddle2", SkaInputKey_GAMEPAD_PADDLE2},
        {"paddle3", SkaInputKey_GAMEPAD_PADDLE3},
        {"paddle4", SkaInputKey_GAMEPAD_PADDLE4},
        {"misc1", SkaInputKey_GAMEPAD_MISC1},
        {"misc2", SkaInputKey_GAMEPAD_MISC2},
        {"touchpad", SkaInputKey_GAMEPAD_TOUCHPAD},
    };
    for (usize i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (strlen(table[i].name) == len && strncmp(table[i].name, name, len) == 0) {
            return table[i].key;
        }
    }
    return SkaInputKey_INVALID;
}

static GenPlatform sdl_platform_name_to_gen_platform(const char* value, usize len) {
    if (len == 7 && strncmp(value, "Windows", 7) == 0) return GenPlatform_WINDOWS;
    if (len == 5 && strncmp(value, "Linux", 5) == 0) return GenPlatform_LINUX;
    if (len == 8 && strncmp(value, "Mac OS X", 8) == 0) return GenPlatform_MACOS;
    return GenPlatform_SKIP; // Android, iOS, or anything unrecognized
}

// Parses a value like "b3", "a2", "a2~", "+a3", "-a3", "h0.4" into a mapping entry (target_key
// must already be set by the caller). Returns false if the syntax isn't recognized.
static bool parse_value(const char* value, usize len, SkaGamepadMappingEntry* out_entry) {
    if (len == 0) {
        return false;
    }
    if (value[0] == 'b') {
        out_entry->source_type = SkaGamepadMappingSourceType_BUTTON;
        out_entry->index = (uint8)atoi(value + 1);
        out_entry->axis_modifier = SkaGamepadAxisModifier_NONE;
        out_entry->hat_bit = 0;
        return true;
    }
    if (value[0] == 'h') {
        // h<hat_index>.<bit>
        const char* dot = memchr(value, '.', len);
        if (!dot) {
            return false;
        }
        out_entry->source_type = SkaGamepadMappingSourceType_HAT;
        out_entry->index = (uint8)atoi(value + 1);
        out_entry->hat_bit = (uint8)atoi(dot + 1);
        out_entry->axis_modifier = SkaGamepadAxisModifier_NONE;
        return true;
    }
    if (value[0] == 'a') {
        out_entry->source_type = SkaGamepadMappingSourceType_AXIS;
        out_entry->hat_bit = 0;
        if (value[len - 1] == '~') {
            out_entry->axis_modifier = SkaGamepadAxisModifier_INVERTED;
        } else {
            out_entry->axis_modifier = SkaGamepadAxisModifier_NONE;
        }
        out_entry->index = (uint8)atoi(value + 1);
        return true;
    }
    if ((value[0] == '+' || value[0] == '-') && len > 1 && value[1] == 'a') {
        out_entry->source_type = SkaGamepadMappingSourceType_AXIS;
        out_entry->hat_bit = 0;
        out_entry->axis_modifier = value[0] == '+' ? SkaGamepadAxisModifier_POSITIVE_HALF : SkaGamepadAxisModifier_NEGATIVE_HALF;
        out_entry->index = (uint8)atoi(value + 2);
        return true;
    }
    return false;
}

// Parses one gamecontrollerdb.txt line into 'out'. Returns the target GenPlatform, or
// GenPlatform_SKIP if the line is a comment/blank/non-desktop-platform/malformed line.
static GenPlatform parse_line(const char* line, usize line_len, ParsedController* out) {
    if (line_len == 0 || line[0] == '#') {
        return GenPlatform_SKIP;
    }

    const char* comma1 = memchr(line, ',', line_len);
    if (!comma1) {
        return GenPlatform_SKIP;
    }
    const usize guid_len = (usize)(comma1 - line);
    if (guid_len == 0 || guid_len >= MAX_GUID_LEN) {
        return GenPlatform_SKIP;
    }

    const char* remaining = comma1 + 1;
    usize remaining_len = line_len - guid_len - 1;
    const char* comma2 = memchr(remaining, ',', remaining_len);
    if (!comma2) {
        return GenPlatform_SKIP;
    }
    const usize name_len = (usize)(comma2 - remaining);
    if (name_len == 0 || name_len >= MAX_NAME_LEN) {
        return GenPlatform_SKIP;
    }

    memcpy(out->guid, line, guid_len);
    out->guid[guid_len] = '\0';
    memcpy(out->name, remaining, name_len);
    out->name[name_len] = '\0';
    out->entry_count = 0;

    remaining = comma2 + 1;
    remaining_len -= name_len + 1;

    GenPlatform platform = GenPlatform_SKIP;
    bool platform_found = false;

    while (remaining_len > 0) {
        const char* field_end = memchr(remaining, ',', remaining_len);
        const usize field_len = field_end ? (usize)(field_end - remaining) : remaining_len;
        if (field_len > 0) {
            const char* colon = memchr(remaining, ':', field_len);
            if (colon) {
                const usize key_len = (usize)(colon - remaining);
                const char* value = colon + 1;
                const usize value_len = field_len - key_len - 1;

                if (key_len == 8 && strncmp(remaining, "platform", 8) == 0) {
                    platform = sdl_platform_name_to_gen_platform(value, value_len);
                    platform_found = true;
                } else {
                    const SkaInputKey target_key = sdl_key_name_to_ska_key(remaining, key_len);
                    if (target_key != SkaInputKey_INVALID && out->entry_count < MAX_ENTRIES_PER_CONTROLLER) {
                        SkaGamepadMappingEntry entry = {0};
                        entry.target_key = target_key;
                        char value_buf[16];
                        if (value_len < sizeof(value_buf)) {
                            memcpy(value_buf, value, value_len);
                            value_buf[value_len] = '\0';
                            if (parse_value(value_buf, value_len, &entry)) {
                                out->entries[out->entry_count++] = entry;
                            }
                        }
                    }
                }
            }
        }
        if (!field_end) {
            break;
        }
        remaining_len -= field_len + 1;
        remaining = field_end + 1;
    }

    return platform_found ? platform : GenPlatform_SKIP;
}

static int compare_parsed_controller_guid(const void* a, const void* b) {
    return strcmp(((const ParsedController*)a)->guid, ((const ParsedController*)b)->guid);
}

// ska_input_key_to_string() returns a human-readable label ("Gamepad Face Button South"), not
// a C identifier, so it can't be used to emit source code -- this returns the literal
// SkaInputKey_GAMEPAD_* enum name for the subset sdl_key_name_to_ska_key() can produce.
static const char* input_key_c_name(SkaInputKey key) {
    switch (key) {
        case SkaInputKey_GAMEPAD_FACE_BUTTON_SOUTH: return "SkaInputKey_GAMEPAD_FACE_BUTTON_SOUTH";
        case SkaInputKey_GAMEPAD_FACE_BUTTON_EAST: return "SkaInputKey_GAMEPAD_FACE_BUTTON_EAST";
        case SkaInputKey_GAMEPAD_FACE_BUTTON_WEST: return "SkaInputKey_GAMEPAD_FACE_BUTTON_WEST";
        case SkaInputKey_GAMEPAD_FACE_BUTTON_NORTH: return "SkaInputKey_GAMEPAD_FACE_BUTTON_NORTH";
        case SkaInputKey_GAMEPAD_BACK: return "SkaInputKey_GAMEPAD_BACK";
        case SkaInputKey_GAMEPAD_START: return "SkaInputKey_GAMEPAD_START";
        case SkaInputKey_GAMEPAD_LEFT_SHOULDER: return "SkaInputKey_GAMEPAD_LEFT_SHOULDER";
        case SkaInputKey_GAMEPAD_RIGHT_SHOULDER: return "SkaInputKey_GAMEPAD_RIGHT_SHOULDER";
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_BUTTON: return "SkaInputKey_GAMEPAD_LEFT_ANALOG_BUTTON";
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_BUTTON: return "SkaInputKey_GAMEPAD_RIGHT_ANALOG_BUTTON";
        case SkaInputKey_GAMEPAD_DPAD_UP: return "SkaInputKey_GAMEPAD_DPAD_UP";
        case SkaInputKey_GAMEPAD_DPAD_DOWN: return "SkaInputKey_GAMEPAD_DPAD_DOWN";
        case SkaInputKey_GAMEPAD_DPAD_LEFT: return "SkaInputKey_GAMEPAD_DPAD_LEFT";
        case SkaInputKey_GAMEPAD_DPAD_RIGHT: return "SkaInputKey_GAMEPAD_DPAD_RIGHT";
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X: return "SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X";
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y: return "SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y";
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X: return "SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X";
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y: return "SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y";
        case SkaInputKey_GAMEPAD_LEFT_TRIGGER: return "SkaInputKey_GAMEPAD_LEFT_TRIGGER";
        case SkaInputKey_GAMEPAD_RIGHT_TRIGGER: return "SkaInputKey_GAMEPAD_RIGHT_TRIGGER";
        case SkaInputKey_GAMEPAD_GUIDE: return "SkaInputKey_GAMEPAD_GUIDE";
        case SkaInputKey_GAMEPAD_PADDLE1: return "SkaInputKey_GAMEPAD_PADDLE1";
        case SkaInputKey_GAMEPAD_PADDLE2: return "SkaInputKey_GAMEPAD_PADDLE2";
        case SkaInputKey_GAMEPAD_PADDLE3: return "SkaInputKey_GAMEPAD_PADDLE3";
        case SkaInputKey_GAMEPAD_PADDLE4: return "SkaInputKey_GAMEPAD_PADDLE4";
        case SkaInputKey_GAMEPAD_MISC1: return "SkaInputKey_GAMEPAD_MISC1";
        case SkaInputKey_GAMEPAD_MISC2: return "SkaInputKey_GAMEPAD_MISC2";
        case SkaInputKey_GAMEPAD_TOUCHPAD: return "SkaInputKey_GAMEPAD_TOUCHPAD";
        default: return "SkaInputKey_INVALID";
    }
}

static void emit_entry(SkaString* out, const SkaGamepadMappingEntry* entry) {
    char line[192];
    snprintf(line, sizeof(line), "        { %s, %s, %u, %s, %u },\n",
        input_key_c_name(entry->target_key),
        entry->source_type == SkaGamepadMappingSourceType_BUTTON ? "SkaGamepadMappingSourceType_BUTTON" :
            entry->source_type == SkaGamepadMappingSourceType_AXIS ? "SkaGamepadMappingSourceType_AXIS" : "SkaGamepadMappingSourceType_HAT",
        (unsigned)entry->index,
        entry->axis_modifier == SkaGamepadAxisModifier_NONE ? "SkaGamepadAxisModifier_NONE" :
            entry->axis_modifier == SkaGamepadAxisModifier_INVERTED ? "SkaGamepadAxisModifier_INVERTED" :
            entry->axis_modifier == SkaGamepadAxisModifier_POSITIVE_HALF ? "SkaGamepadAxisModifier_POSITIVE_HALF" : "SkaGamepadAxisModifier_NEGATIVE_HALF",
        (unsigned)entry->hat_bit);
    ska_string_append(out, line, strlen(line));
}

// Parses the whole file, dedupes per platform (keeping the last occurrence in file order via a
// guid->slot hash map), sorts each platform's controllers by guid, and emits one #if-guarded
// section per platform into 'out'.
static void generate(const char* input_path, SkaString* out) {
    usize file_size = 0;
    char* file_contents = ska_fs_read_file_contents(input_path, &file_size);
    if (!file_contents) {
        fprintf(stderr, "Failed to read '%s'\n", input_path);
        exit(1);
    }

    ParsedControllerList platform_lists[GenPlatform_COUNT];
    GuidToIndexMap* platform_guid_to_index[GenPlatform_COUNT];
    for (int p = 0; p < GenPlatform_COUNT; p++) {
        platform_lists[p] = ParsedControllerList_init_default_capacity();
        platform_guid_to_index[p] = GuidToIndexMap_init(64);
    }

    usize line_count = 0, skipped_count = 0;
    const char* cursor = file_contents;
    const char* file_end = file_contents + file_size;
    while (cursor < file_end) {
        const char* newline = memchr(cursor, '\n', (usize)(file_end - cursor));
        usize line_len = newline ? (usize)(newline - cursor) : (usize)(file_end - cursor);
        if (line_len > 0 && cursor[line_len - 1] == '\r') {
            line_len--;
        }

        ParsedController parsed = {0};
        const GenPlatform platform = parse_line(cursor, line_len, &parsed);
        if (platform != GenPlatform_SKIP) {
            line_count++;
            const GuidKey key = make_guid_key(parsed.guid);
            usize* existing_index_ptr = GuidToIndexMap_get(platform_guid_to_index[platform], key);
            if (existing_index_ptr) {
                *ParsedControllerList_get(&platform_lists[platform], *existing_index_ptr) = parsed;
            } else {
                const usize new_index = platform_lists[platform].size;
                ParsedControllerList_append(&platform_lists[platform], parsed);
                GuidToIndexMap_add(platform_guid_to_index[platform], key, new_index);
            }
        } else if (line_len > 0 && cursor[0] != '#') {
            skipped_count++;
        }

        cursor = newline ? newline + 1 : file_end;
    }

    fprintf(stderr, "Parsed %zu usable lines, skipped %zu malformed/unrecognized lines\n", line_count, skipped_count);

    const char* header =
        "// Generated by seika_gamepad_db_gen (seika/tools/gen_gamepad_db.c) from\n"
        "// seika/thirdparty/gamecontrollerdb/gamecontrollerdb.txt. Do not hand-edit --\n"
        "// re-run the generator and commit the result instead.\n\n"
        "#include \"gamepad_db.h\"\n\n"
        "#if SKA_INPUT\n\n";
    ska_string_append(out, header, strlen(header));

    for (int p = 0; p < GenPlatform_COUNT; p++) {
        char guard_line[64];
        snprintf(guard_line, sizeof(guard_line), "%s defined(%s)\n", p == 0 ? "#if" : "#elif", gen_platform_guard[p]);
        ska_string_append(out, guard_line, strlen(guard_line));

        ParsedController* sorted = ParsedControllerList_to_array(&platform_lists[p]);
        const usize count = platform_lists[p].size;
        qsort(sorted, count, sizeof(ParsedController), compare_parsed_controller_guid);

        for (usize i = 0; i < count; i++) {
            char entries_var_line[64];
            snprintf(entries_var_line, sizeof(entries_var_line), "static const SkaGamepadMappingEntry k_entries_%zu[] = {\n", i);
            ska_string_append(out, entries_var_line, strlen(entries_var_line));
            for (uint32 e = 0; e < sorted[i].entry_count; e++) {
                emit_entry(out, &sorted[i].entries[e]);
            }
            const char* close = "};\n\n";
            ska_string_append(out, close, strlen(close));
        }

        const char* table_open = "static const SkaGamepadMapping k_gamepad_db_table[] = {\n";
        ska_string_append(out, table_open, strlen(table_open));
        for (usize i = 0; i < count; i++) {
            char table_line[256];
            snprintf(table_line, sizeof(table_line), "    { \"%s\", \"%s\", k_entries_%zu, %u },\n",
                sorted[i].guid, sorted[i].name, i, sorted[i].entry_count);
            ska_string_append(out, table_line, strlen(table_line));
        }
        const char* table_close = "};\n";
        ska_string_append(out, table_close, strlen(table_close));

        char count_line[128];
        snprintf(count_line, sizeof(count_line), "static const uint32 k_gamepad_db_table_count = %zu;\n\n", count);
        ska_string_append(out, count_line, strlen(count_line));
    }

    const char* else_branch =
        "#else\n"
        "static const SkaGamepadMapping k_gamepad_db_table[1] = {0};\n"
        "static const uint32 k_gamepad_db_table_count = 0;\n\n"
        "#endif\n\n"
        "const SkaGamepadMapping* ska_gamepad_db_get_table(void) { return k_gamepad_db_table; }\n"
        "usize ska_gamepad_db_get_table_count(void) { return k_gamepad_db_table_count; }\n\n"
        "#endif // #if SKA_INPUT\n";
    ska_string_append(out, else_branch, strlen(else_branch));

    for (int p = 0; p < GenPlatform_COUNT; p++) {
        ParsedControllerList_deinit(&platform_lists[p]);
        GuidToIndexMap_deinit(platform_guid_to_index[p]);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <path-to-gamecontrollerdb.txt> <path-to-output.c>\n", argv[0]);
        return 1;
    }

    SkaString out;
    ska_string_init_empty(&out);
    generate(argv[1], &out);

    // ska_fs_write_to_file() writes via fprintf("%s", ...), which requires a null terminator,
    // but SkaString does not null-terminate heap-backed buffers (ska_string_ensure_null_terminated
    // is a no-op once a string has spilled to the heap, which this ~1MB buffer always has) --
    // without this, fprintf reads past the buffer into adjacent heap memory until it happens to
    // hit a stray zero byte.
    ska_string_append(&out, "\0", 1);

    if (!ska_fs_write_to_file(argv[2], ska_string_as_slice(&out))) {
        fprintf(stderr, "Failed to write '%s'\n", argv[2]);
        return 1;
    }

    fprintf(stderr, "Wrote '%s'\n", argv[2]);
    ska_string_deinit(&out);
    return 0;
}
