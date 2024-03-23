#pragma once

#include "../math/se_math.h"
#include "../rendering/texture.h"

#define SE_TILE_MAP_MAX_TILES 36

typedef struct SETile {
    SKAVector2 position;
} SETile;

typedef struct SETileMap {
    SETexture* tileSpriteSheet;
    SKAVector2 tileSize;
    SETile tiles[36];
} SETileMap;

SETileMap* se_tile_map_create(SETexture* tileSpriteSheet);
void se_tile_map_add_tile(SETileMap* tileMap, SKAVector2 position);
SETile* se_tile_map_get_tile(SETileMap* tileMap, SKAVector2 position);
SETile* se_tile_map_has_tile(SETileMap* tileMap, SKAVector2 position);
void se_tile_map_destroy(SETileMap* tileMap);
