#ifndef CITY_H
#define CITY_H

/* This generated file contains includes for project dependencies */
#include "city/bake_config.h"

#undef ECS_META_IMPL
#ifndef FLECS_CITY_IMPL
#define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#endif

#ifdef __cplusplus
extern "C" {
#endif

ECS_STRUCT(CityBuilding, {
    int16_t size;
    int16_t height;
});

ECS_STRUCT(CityBlock, {
    int16_t size;
    int16_t building_size;
    int16_t building_height;
});

ECS_STRUCT(City, {
    int16_t blocks_x;
    int16_t blocks_y;
    int16_t block_size;
    int16_t road_width;
    int16_t building_size;
    int16_t min_building_height;
    int16_t max_building_height;
});

void FlecsCityImport(
    ecs_world_t *world);

#ifdef __cplusplus
}
#endif

#endif
