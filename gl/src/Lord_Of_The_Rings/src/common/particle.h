/*  particle
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "camera.h"

#define PHYSIC_G 9.8

typedef struct {
    float center[3];
    float color[4];
    int num_particle;
    float mass;
    float speed;
    float radius;
    float time;
    float force[3];
    char texture[256];
    int texture_mode;
} particle_config_t;

typedef struct {
    float center[3];    // center emissivity
    float *pos;         // coordinate
    float *speed;       // speed
    float *life;        // life
    int num_particle;   // num particle
    float mass;         // mass
    float maxspeed;     // max start speed
    float time;         // time life
    float radius;       // radius
    float force[3];     // force
    float color[4];     // color
    int texture;        // texture id
} particle_t;

particle_t *particle_load(particle_config_t *config);
void particle_free(particle_t *particle);
void particle_render(particle_t *particle,camera_t *camera,float ifps);
void particle_move(particle_t *particle,float *pos);

#endif /* _PARTICLE_H__ */
