#pragma once

#include "seika/math/math.h"
#include "seika/rendering/texture.h"

#define SKA_TILE_MAP_MAX_TILES 36

typedef struct SkaTile {
    SkaVector2 position;
} SkaTile;

typedef struct SkaTileMap {
    SkaTexture* tileSpriteSheet;
    SkaVector2 tileSize;
    SkaTile tiles[36];
} SkaTileMap;

SkaTileMap* ska_tile_map_create(SkaTexture* tileSpriteSheet);
void ska_tile_map_add_tile(SkaTileMap* tileMap, SkaVector2 position);
SkaTile* ska_tile_map_get_tile(SkaTileMap* tileMap, SkaVector2 position);
SkaTile* ska_tile_map_has_tile(SkaTileMap* tileMap, SkaVector2 position);
void ska_tile_map_destroy(SkaTileMap* tileMap);
