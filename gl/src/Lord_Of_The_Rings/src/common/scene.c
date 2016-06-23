/*  scene
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <malloc.h>
#include "land.h"
#include "camera.h"
#include "spline.h"
#include "sky.h"
#include "thing.h"
#include "shadowmap.h"
#include "particle.h"
#include "dynamiclight.h"
#include "mathlib.h"
#include "scene.h"

#include "system.h"

static void scene_update(scene_t *scene,float ifps) {
    float t;
    scene->time += ifps;
    t = scene->time;
    if(t < scene->start) t = scene->start;
    if(t > scene->end) t = scene->end;
    t -= scene->start;
    VectorSet(0,0,1,scene->camera->up); // set camera
    spline_pos(scene->pathpos,t,scene->camera->pos);
    spline_pos(scene->pathdir,t,scene->camera->dir);
    if(scene->pathsun) spline_pos(scene->pathsun,t,scene->sun->pos);    // sun pos
}

static void scene_animation(scene_t *scene) {
    int i;
    float pos[3],target,t;
    for(i = 0; i < scene->num_animation; i++) {
        t = scene->time;
        if(t < scene->animation[i]->start) t = scene->animation[i]->start;
        else if(t > scene->animation[i]->end) t = scene->animation[i]->end;
        t -= scene->animation[i]->start;
        if(scene->animation[i]->path) { // if spline define
            spline_pos(scene->animation[i]->path,t,pos);
            VectorAdd(pos,scene->animation[i]->pos,pos);
            target = spline_target(scene->animation[i]->path,t) + scene->animation[i]->target;
        } else {    // only pos and target
            VectorCopy(scene->animation[i]->pos,pos);
            target = scene->animation[i]->target;
        }   // object on land
        if(scene->animation[i]->onland) pos[2] = land_height(scene->land,pos);
        thing_move(scene->animation[i]->thing,pos,target);  // move thing
        if(camera_check_sphere(scene->camera,scene->animation[i]->thing->center,scene->animation[i]->thing->radius) &&
           camera_check_box(scene->camera,scene->animation[i]->thing->min,scene->animation[i]->thing->max)) {
            thing_mesh_create(scene->animation[i]->thing->mesh,t / scene->animation[i]->time);  // create mesh
            scene->animation[i]->view = 1;
        } else {
            scene->animation[i]->view = 0;
        }
    }
}

static void scene_particle(scene_t *scene) {
    int i;
    float pos[3];
    for(i = 0; i < scene->num_particle; i++) { // particle system sphere in frustum
        if(camera_check_sphere(scene->camera,scene->particle[i]->particle->center,scene->particle[i]->radius)) {
            if(scene->time > scene->particle[i]->start && scene->time < scene->particle[i]->end) {
                if(scene->particle[i]->path) {
                    spline_pos(scene->particle[i]->path,scene->time - scene->particle[i]->start,pos);
                    VectorAdd(pos,scene->particle[i]->pos,pos);
                    particle_move(scene->particle[i]->particle,pos);
                }
                scene->particle[i]->view = 1;
                continue;
            }
        }
        scene->particle[i]->view = 0;
    }
}

static void scene_dynamiclight(scene_t *scene) {
    int i;
    float pos[3];
    for(i = 0; i < scene->num_dynamiclight; i++) {
        if(camera_check_sphere(scene->camera,scene->dynamiclight[i]->light->center,scene->dynamiclight[i]->radius)) {
            if(scene->time > scene->dynamiclight[i]->start && scene->time < scene->dynamiclight[i]->end) {
                if(scene->dynamiclight[i]->path) {
                    spline_pos(scene->dynamiclight[i]->path,scene->time - scene->dynamiclight[i]->start,pos);
                    VectorAdd(pos,scene->dynamiclight[i]->pos,pos);
                    dynamiclight_move(scene->dynamiclight[i]->light,pos);
                }
                scene->dynamiclight[i]->view = 1;
                continue;
            }
        }
        scene->dynamiclight[i]->view = 0;
    }
}

void scene_render(scene_t *scene,float ifps) {
    int i,j,light;
    static int frame;
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    scene_update(scene,ifps);   // update scene
    scene_animation(scene); // create mesh and check view animated object
    scene_particle(scene);  // check view and move particle system
    scene_dynamiclight(scene);  // check view and move dynamiclight
    if(frame == FRAME_PER_SHADOW) {
        shadow_enable_shadowmap();  // create shadow maps
        for(i = 0; i < scene->num_animation; i++)
            if(scene->animation[i]->view && scene->animation[i]->shadow > 0)
                shadow_shadowmap(scene->sun->pos,scene->animation[i]->thing,scene->animation[i]->shadow);
        shadow_disable_shadowmap(sys_width(),sys_height());
        frame = 0;
    }
    frame++;
    glColor4f(1.0,1.0,1.0,1.0);
    glEnable(GL_TEXTURE_2D);
    camera_view(scene->camera); // set camera
    sky_render(scene->sky,scene->camera,ifps);  // render sky
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);        // sun light
    glLightfv(GL_LIGHT0,GL_POSITION,scene->sun->pos);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,scene->sun->color);
    glEnable(GL_FOG);
    for(i = 0, j = 1, light = 0; i < scene->num_dynamiclight; i++)
        if(scene->dynamiclight[i]->opengl) {    // add dynamic lights
            switch(j) {
                case 1: light = GL_LIGHT1; break;
                case 2: light = GL_LIGHT2; break;
                case 3: light = GL_LIGHT3; break;
                case 4: light = GL_LIGHT4; break;
                case 5: light = GL_LIGHT5; break;
                case 6: light = GL_LIGHT6; break;
                case 7: light = GL_LIGHT7; break;
            }
            glEnable(light);
            glLightfv(light,GL_POSITION,scene->dynamiclight[i]->light->center);
            glLightfv(light,GL_DIFFUSE,scene->dynamiclight[i]->light->color);
            j++;
            if(j == 8) break;
        }
    glFogfv(GL_FOG_COLOR,scene->camera->fogcolor);
    glFogf(GL_FOG_START,scene->camera->fogstart);
    glFogf(GL_FOG_END,scene->camera->fogend);
    land_render(scene->land,scene->camera); // render landscape
    for(i = 0; i < scene->num_thing; i++)   // render thing
        if(camera_check_sphere(scene->camera,scene->thing[i]->center,scene->thing[i]->radius) &&
           camera_check_box(scene->camera,scene->thing[i]->min,scene->thing[i]->max))
            thing_render(scene->thing[i]);
    for(i = 0; i < scene->num_animation; i++)   // render animated object
        if(scene->animation[i]->view)
            thing_render(scene->animation[i]->thing);
    glDisable(GL_LIGHTING);
    shadow_enable_project();
    for(i = 0; i < scene->num_animation; i++)   // dynamic shadow on land
        if(scene->animation[i]->view && scene->animation[i]->shadow > 0) {
            shadow_project(scene->sun->pos,scene->animation[i]->thing);
            land_dynamicshadow(scene->land,scene->sun->pos,scene->animation[i]->thing);
        }
    shadow_disable_project();
    for(i = 0; i < scene->num_particle; i++)    // particle
        if(scene->particle[i]->view)
            particle_render(scene->particle[i]->particle,scene->camera,ifps);
    for(i = 0; i < scene->num_dynamiclight; i++)// dynamiclight
        if(scene->dynamiclight[i]->view)
            dynamiclight_render(scene->dynamiclight[i]->light,scene->camera,scene->land);
    glDisable(GL_FOG);
    sky_sun_render(scene->sun,scene->camera);   // render sun and flare
    glDisable(GL_TEXTURE_2D);
}

/*  free scene
 *
 */

void scene_free(scene_t *scene) {
    int i;
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);
    if(scene->land) land_free(scene->land);
    if(scene->camera) camera_free(scene->camera);
    if(scene->pathpos) spline_free(scene->pathpos);
    if(scene->pathdir) spline_free(scene->pathdir);
    if(scene->sky) sky_free(scene->sky);
    if(scene->sun) sky_sun_free(scene->sun);
    if(scene->pathsun) spline_free(scene->pathsun);
    for(i = 0; i < scene->num_mesh; i++) thing_mesh_free(scene->mesh[i]);
    if(scene->mesh) free(scene->mesh);
    for(i = 0; i < scene->num_thing; i++) thing_free(scene->thing[i]);
    if(scene->thing) free(scene->thing);
    for(i = 0; i < scene->num_animation; i++) {
        thing_free(scene->animation[i]->thing);
        if(scene->animation[i]->path) spline_free(scene->animation[i]->path);
        free(scene->animation[i]);
    }
    if(scene->animation) free(scene->animation);
    for(i = 0; i < scene->num_particle; i++) {
        particle_free(scene->particle[i]->particle);
        if(scene->particle[i]->path) spline_free(scene->particle[i]->path);
        free(scene->particle[i]);
    }
    if(scene->particle) free(scene->particle);
    for(i = 0; i < scene->num_dynamiclight; i++) {
        dynamiclight_free(scene->dynamiclight[i]->light);
        if(scene->dynamiclight[i]->path) spline_free(scene->dynamiclight[i]->path);
        free(scene->dynamiclight[i]);
    }
    if(scene->dynamiclight) free(scene->dynamiclight);
    free(scene);
}
