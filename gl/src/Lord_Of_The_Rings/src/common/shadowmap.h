/*  shadow map
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __SHADOWMAP_H__
#define __SHADOWMAP_H__

#include "thing.h"

#define SHADOW_MAX 16

void shadow_create_texture(int size);
void shadow_free(void);
void shadow_enable_shadowmap(void);
void shadow_disable_shadowmap(int width,int height);
void shadow_shadowmap(float *lightpos,thing_t *thing,float intensity);
void shadow_enable_project(void);
void shadow_disable_project(void);
void shadow_project(float *light,thing_t *thing);

#endif /* __SHADOWMAP_H__ */
