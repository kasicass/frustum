/*  dynamic light
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __DYNAMICLIGHT_H__
#define __DYNAMICLIGHT_H__

typedef struct {
    float center[3];
    float color[4];
    float flareradius;
    float lightradius;
    char flaretexture[256];
    char lighttexture[256];
    int texture_mode;
} dynamiclight_config_t;

typedef struct {
    float center[4];    // coordinate and 1 for opengl point light
    float color[4];     // color
    float flareradius;  // flare radius
    float lightradius;  // light radius
    int flaretexture;   // flare texture id
    int lighttexture;   // light texture id
} dynamiclight_t;

dynamiclight_t *dynamiclight_load(dynamiclight_config_t *congig);
void dynamiclight_free(dynamiclight_t *light);
void dynamiclight_render(dynamiclight_t *light,camera_t *camera,land_t *land);
void dynamiclight_move(dynamiclight_t *light,float *pos);

#endif /* __DYNAMICLIGHT_H__ */
