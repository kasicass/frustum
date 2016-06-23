/*  lightning
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __LIGHTNING_H__
#define __LIGHTNING_H__

#define LIGHTING_TEXTURE 6

#include "land.h"
#include "camera.h"

typedef struct {
    float width;
    float radius;
    int num_texture;
    char texture[LIGHTING_TEXTURE][256];
    char flare[256];
    int texture_mode;
} lightning_config_t;

typedef struct {
    float width;        // lightning width
    float radius;       // light on land radius
    int num_texture;    // num lighting texture
    int texture[LIGHTING_TEXTURE];  // lightning texture id
    int flare;          // flare texture id
} lightning_t;

lightning_t *lightning_load(lightning_config_t *config);
void lightning_free(lightning_t *lightning);
void lightning_render(lightning_t *lightning,camera_t *camera,land_t *land,float *from,float *to,int frame);

#endif /* __LIGHTNING_H__ */
