/*  scene
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __SCENE_H__
#define __SCENE_H__

#define FRAME_PER_SHADOW 3  // one shadow for three frame

#include "land.h"
#include "camera.h"
#include "spline.h"
#include "sky.h"
#include "thing.h"
#include "particle.h"
#include "dynamiclight.h"

typedef struct {
    thing_t *thing;     // thing
    float pos[3];       // start coordinate
    float target;       // start target
    spline_t *path;     // spline position
    float length;       // spline length
    int onland;         // thing on land
    float time;         // time for transform to next mesh
    float frame;        // current frame
    float start,end;    // start and end animation
    float shadow;       // shadow level
    int view;           // object in frustum
} scene_animation_t;

typedef struct {
    particle_t *particle;   // particle system
    float pos[3];           // start coordinate
    spline_t *path;         // particle start coordinate
    float start,end;        // start and end emissivity
    float radius;           // max radius (speed + |force| / mass * time) * time
    int view;               // partcile system in frustum and activate
} scene_particle_t;

typedef struct {
    dynamiclight_t *light;  // dynamic light
    float pos[3];           // start coordinate
    spline_t *path;         // dynamic light start coordinate
    float start,end;        // start and end lighting
    float radius;           // max radius
    int view;               // dynamic light in frustum and activate
    int opengl;             // use OpenGL light
} scene_dynamiclight_t;

typedef struct {
    float time;         // current time
    float start,end;    // start and end camera motion
    land_t *land;       // land
    camera_t *camera;   // camera
    spline_t *pathpos;  // camera position path
    spline_t *pathdir;  // camera direction path
    sky_t *sky;         // sky
    sky_sun_t *sun;     // sun
    spline_t *pathsun;  // sun position path
    thing_mesh_t **mesh;// mesh
    int num_mesh;       // num mesh
    thing_t **thing;    // thing
    int num_thing;      // num thing
    scene_animation_t **animation;  // animation thing
    int num_animation;              // num animation thing
    scene_particle_t **particle;    // particle
    int num_particle;               // num particle
    scene_dynamiclight_t **dynamiclight;// dynamic light
    int num_dynamiclight;               // num dynamic light
} scene_t;

void scene_render(scene_t *scene,float ifps);
void scene_free(scene_t *scene);

#endif /* __SCENE_H__ */
