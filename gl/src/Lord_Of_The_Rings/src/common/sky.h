/*  sky
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __SKY_H__
#define __SKY_H__

#include "camera.h"

#define SKY_MAX_LAYER  4
#define SKY_MAX_FLARE  7

typedef struct {
    char mesh[256];                 // mesh name
    int num_layer;                  // num layer
    float target;                   // target
    float height[SKY_MAX_LAYER];    // heights
    float time[SKY_MAX_LAYER];      // times
    char texture[SKY_MAX_LAYER][256];   // texture name
    int texture_mode;               // texture mode
} sky_config_t;

typedef struct {
    float *vertex;  // mesh
    int num_vertex; // num vertex
    float radius;   // radius
    int num_layer;  // num layer
    float target;   // target rotaton
    float height[SKY_MAX_LAYER];// height layer
    float time[SKY_MAX_LAYER];  // time rotaion
    float angle[SKY_MAX_LAYER]; // current angle
    int texture[SKY_MAX_LAYER]; // texture id
} sky_t;

typedef struct {
    float pos[3];                       // sun position
    float color[4];                     // color
    int num_flare;                      // num flare
    float radius[SKY_MAX_FLARE];        // radius
    float position[SKY_MAX_FLARE];      // position
    float opacity[SKY_MAX_FLARE];       // opacity
    char texture[SKY_MAX_FLARE][256];   // texture name
    int texture_mode;                   // texture mode
} sky_sun_config_t;

typedef struct {
    float pos[3];
    float color[4];
    int num_flare;
    float radius[SKY_MAX_FLARE];
    float position[SKY_MAX_FLARE];
    float opacity[SKY_MAX_FLARE];
    int texture[SKY_MAX_FLARE];
} sky_sun_t;

sky_t *sky_load(sky_config_t *config);
void sky_free(sky_t *sky);
void sky_render(sky_t *sky,camera_t *camera,float ifps);

sky_sun_t *sky_sun_load(sky_sun_config_t *config);
void sky_sun_free(sky_sun_t *sun);
void sky_sun_render(sky_sun_t *sun,camera_t *camera);

#endif /* __SKY_H__ */
