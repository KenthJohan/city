// Comment out this line when using as DLL
#define flecs_components_graphics_STATIC
#ifndef FLECS_COMPONENTS_GRAPHICS_H
#define FLECS_COMPONENTS_GRAPHICS_H

/*
                                   )
                                  (.)
                                  .|.
                                  | |
                              _.--| |--._
                           .-';  ;`-'& ; `&.
                          \   &  ;    &   &_/
                           |"""---...---"""|
                           \ | | | | | | | /
                            `---.|.|.|.---'

 * This file is generated by bake.lang.c for your convenience. Headers of
 * dependencies will automatically show up in this file. Include bake_config.h
 * in your main project file. Do not edit! */

#ifndef FLECS_COMPONENTS_GRAPHICS_BAKE_CONFIG_H
#define FLECS_COMPONENTS_GRAPHICS_BAKE_CONFIG_H

/* Headers of public dependencies */
#include "flecs.h"
#include "flecs_components_cglm.h"

/* Convenience macro for exporting symbols */
#ifndef flecs_components_graphics_STATIC
#if flecs_components_graphics_EXPORTS && (defined(_MSC_VER) || defined(__MINGW32__))
  #define FLECS_COMPONENTS_GRAPHICS_API __declspec(dllexport)
#elif flecs_components_graphics_EXPORTS
  #define FLECS_COMPONENTS_GRAPHICS_API __attribute__((__visibility__("default")))
#elif defined _MSC_VER
  #define FLECS_COMPONENTS_GRAPHICS_API __declspec(dllimport)
#else
  #define FLECS_COMPONENTS_GRAPHICS_API
#endif
#else
  #define FLECS_COMPONENTS_GRAPHICS_API
#endif

#endif



// Reflection system boilerplate
#undef ECS_META_IMPL
#ifndef FLECS_COMPONENTS_GRAPHICS_IMPL
#define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#endif

#ifdef __cplusplus
extern "C" {
#endif

ECS_STRUCT(EcsCamera, {
    vec3 position;
    vec3 lookat;
    vec3 up;
    float fov;
    float near;
    float far;
    bool ortho;
});

ECS_STRUCT(EcsDirectionalLight, {
    vec3 position;
    vec3 direction;
    vec3 color;
});

ECS_STRUCT(EcsRgb, {
    float r;
    float g;
    float b;
});

typedef EcsRgb ecs_rgb_t;

ECS_STRUCT(EcsRgba, {
    float r;
    float g;
    float b;
    float a;
});

typedef EcsRgba ecs_rgba_t;

ECS_STRUCT(EcsSpecular, {
    float specular_power;
    float shininess;
});

ECS_STRUCT(EcsEmissive, {
    float value;
});

FLECS_COMPONENTS_GRAPHICS_API
void FlecsComponentsGraphicsImport(
    ecs_world_t *world);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#ifndef FLECS_NO_CPP

namespace flecs {
namespace components {

class graphics : FlecsComponentsGraphics {
public:
    struct rgb_t : ecs_rgb_t {
        operator float*() {
            return reinterpret_cast<float*>(this);
        }
    };

    struct rgba_t : ecs_rgba_t {
        operator float*() {
            return reinterpret_cast<float*>(this);
        }
    };

    struct Camera : EcsCamera {
        Camera() {
            this->set_position(0, 0, 0);
            this->set_lookat(0, 1, 1);
            this->set_up(0, -1, 0);
            this->set_fov(30);
            this->near = 0.1f;
            this->far = 100;
        }

        void set_position(float x, float y, float z) {
            this->position[0] = x;
            this->position[1] = y;
            this->position[2] = z;
        }

        void set_lookat(float x, float y, float z) {
            this->lookat[0] = x;
            this->lookat[1] = y;
            this->lookat[2] = z;
        }

        void set_up(float x, float y, float z) {
            this->up[0] = x;
            this->up[1] = y;
            this->up[2] = z;
        }

        void set_fov(float value) {
            this->fov = value;
        }
    };

    struct DirectionalLight : EcsDirectionalLight {
        DirectionalLight() {
            this->set_position(0, 0, 0);
            this->set_direction(0, 1, 1);
            this->set_color(1, 1, 1);
        }

        void set_position(float x, float y, float z) {
            this->position[0] = x;
            this->position[1] = y;
            this->position[2] = z;
        }

        void set_direction(float x, float y, float z) {
            this->direction[0] = x;
            this->direction[1] = y;
            this->direction[2] = z;
        }

        void set_color(float r, float g, float b) {
            this->color[0] = r;
            this->color[1] = g;
            this->color[2] = b;
        }
    };

    using Rgb = EcsRgb;
    using Rgba = EcsRgba;
    using Specular = EcsSpecular;
    using Emissive = EcsEmissive;

    graphics(flecs::world& ecs) {
        FlecsComponentsGraphicsImport(ecs);

        ecs.module<flecs::components::graphics>();
        ecs.pod_component<Camera>("flecs::components::graphics::Camera");
        ecs.pod_component<DirectionalLight>("flecs::components::graphics::DirectionalLight");
        ecs.pod_component<Rgb>("flecs::components::graphics::Rgb");
        ecs.pod_component<Rgba>("flecs::components::graphics::Rgba");
        ecs.pod_component<Specular>("flecs::components::graphics::Specular");
        ecs.pod_component<Emissive>("flecs::components::graphics::Emissive");
    }
};

}
}

#endif
#endif

#endif

