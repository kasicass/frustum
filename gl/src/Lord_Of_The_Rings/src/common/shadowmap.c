/*  shadow map
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <malloc.h>

#include "mathlib.h"
#include "thing.h"
#include "shadowmap.h"

#include "system.h"

static int shadow_size;
static int shadow_current_texture;
static int shadow_texture[SHADOW_MAX];

void shadow_create_texture(int size) {
    int i;
    shadow_size = size;
    for(i = 0; i < SHADOW_MAX; i++) {
        glGenTextures(1,&shadow_texture[i]);
        glBindTexture(GL_TEXTURE_2D,shadow_texture[i]);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,shadow_size,shadow_size,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    }
}

void shadow_free(void) {
    int i;
    for(i = 0; i < SHADOW_MAX; i++)
        glDeleteTextures(1,&shadow_texture[i]);
}

void shadow_enable_shadowmap(void) {
    glViewport(0,0,shadow_size,shadow_size);
    shadow_current_texture = 0;
}

void shadow_disable_shadowmap(int width,int height) {
    glViewport(0,0,width,height);
}

void shadow_shadowmap(float *lightpos,thing_t *thing,float intensity) {
    float radius,pos[3];
    if(shadow_current_texture > SHADOW_MAX - 1) return;
    thing->shadow = shadow_texture[shadow_current_texture];
    glBindTexture(GL_TEXTURE_2D,shadow_texture[shadow_current_texture]);
    shadow_current_texture++;
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    radius = thing->radius * 1.1;
    glOrtho(-radius,radius,-radius,radius,0.01,radius * 2.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    VectorSub(lightpos,thing->center,pos);
    VectorNormalize(pos,pos);
    VectorScale(pos,radius,pos);
    VectorAdd(pos,thing->center,pos);
    gluLookAt(pos[0],pos[1],pos[2],thing->center[0],thing->center[1],thing->center[2],0,0,1);
    glColor3f(intensity,intensity,intensity);
    glPushMatrix();
    glTranslatef(thing->pos[0],thing->pos[1],thing->pos[2]);
    glRotatef(thing->target,0,0,1);
    thing_mesh_render(thing->mesh);
    glPopMatrix();
    glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,0,0,shadow_size,shadow_size,0);
}

void shadow_enable_project(void) {
    float plane_s[] = { 1, 0, 0, 0 };
    float plane_t[] = { 0, 1, 0, 0 };
    float plane_r[] = { 0, 0, 1, 0 };
    float plane_q[] = { 0, 0, 0, 1 };
    glTexGenfv(GL_S,GL_EYE_PLANE,plane_s);
    glTexGenfv(GL_T,GL_EYE_PLANE,plane_t);
    glTexGenfv(GL_R,GL_EYE_PLANE,plane_r);
    glTexGenfv(GL_Q,GL_EYE_PLANE,plane_q);
    glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
    glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
    glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
    glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);
    glMatrixMode(GL_TEXTURE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
}

void shadow_disable_project(void) {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_BLEND);
}

void shadow_project(float *light,thing_t *thing) {
    float radius;
    glLoadIdentity();
    glTranslatef(0.5,0.5,0);
    glScalef(0.5,0.5,1.0);
    radius = thing->radius * 1.1;
    glOrtho(-radius,radius,-radius,radius,-1,1);
    gluLookAt(light[0],light[1],light[2],thing->center[0],thing->center[1],thing->center[2],0,0,1);
    glBindTexture(GL_TEXTURE_2D,thing->shadow);
}
