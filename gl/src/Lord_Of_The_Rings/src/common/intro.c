/*  intro
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "mathlib.h"
#include "camera.h"
#include "thing.h"
#include "particle.h"
#include "loadjpeg.h"
#include "intro.h"

#include "system.h"

#define NUM_FLAME 9

static float time,stage;
static camera_t *camera;
static thing_mesh_t *ring;
static particle_t *particle[NUM_FLAME];
static int messagetexture,modulatetexture;
static int messagewidth,messageheight;

int intro_load(char *path,int texture_mode) {
    char buffer[256];
    unsigned char *data;
    int i,width,height;
    camera_config_t cameraconfig;
    thing_mesh_config_t thingconfig;
    particle_config_t particleconfig;
    
    memset(&cameraconfig,0,sizeof(camera_config_t));    // set camera
    cameraconfig.fov = 45;
    cameraconfig.aspect = (float)sys_width() / (float)sys_height();
    cameraconfig.clipnear = 0.1;
    cameraconfig.clipfar = 10;
    camera = camera_create(&cameraconfig);
    if(!camera) return 0;
    VectorSet(0,-4,0,camera->pos);
    VectorSet(0,0,0,camera->dir);
    VectorSet(0,0,1,camera->up);
    
    memset(&thingconfig,0,sizeof(thing_mesh_config_t)); // load ring
    strcpy(thingconfig.meshname,path);
    strcat(thingconfig.meshname,"ring.3ds");
    thingconfig.ambient[0] = 0;
    thingconfig.ambient[1] = 0;
    thingconfig.ambient[2] = 0;
    thingconfig.ambient[3] = 1;
    thingconfig.diffuse[0] = 1;
    thingconfig.diffuse[1] = 1;
    thingconfig.diffuse[2] = 1;
    thingconfig.diffuse[3] = 1;
    thingconfig.specular[0] = 0;
    thingconfig.specular[1] = 0;
    thingconfig.specular[2] = 0;
    thingconfig.specular[3] = 1;
    strcpy(thingconfig.basetexture,path);
    strcat(thingconfig.basetexture,"ring.tga");
    strcpy(thingconfig.speculartexture,path);
    strcat(thingconfig.speculartexture,"specular.jpg");
    thingconfig.shader = SHADER_GLOSS;
    thingconfig.texture_mode = texture_mode;
    ring = thing_mesh_load(&thingconfig);
    if(!ring) return 0;
    
    memset(&particleconfig,0,sizeof(particle_config_t));    // load particle for flame
    particleconfig.color[0] = 1;
    particleconfig.color[1] = 1;
    particleconfig.color[2] = 1;
    particleconfig.color[3] = 1;
    particleconfig.num_particle = 50;
    particleconfig.mass = 1;
    particleconfig.speed = 0.8;
    particleconfig.radius = 0.25;
    particleconfig.time = 1.0;
    strcpy(particleconfig.texture,path);
    strcat(particleconfig.texture,"flame.jpg");
    particleconfig.texture_mode = texture_mode;
    VectorSet(0,0,11,particleconfig.force);
    for(i = 0; i < NUM_FLAME; i++) {
        VectorSet(-2.0 + 4.0 / (NUM_FLAME  - 1) * i,0,-1.8,particleconfig.center);
        particle[i] = particle_load(&particleconfig);
        if(!particle[i]) return 0;
    }
    
    strcpy(buffer,path);    // load message texture
    strcat(buffer,"message.jpg");
    data = LoadJPEG(buffer,&messagewidth,&messageheight);
    if(data) {
        glGenTextures(1,&messagetexture);
        glBindTexture(GL_TEXTURE_2D,messagetexture);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,messagewidth,messageheight,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    
    strcpy(buffer,path);    // load modulate texture
    strcat(buffer,"modulate.jpg");
    data = LoadJPEG(buffer,&width,&height);
    if(data) {
        glGenTextures(1,&modulatetexture);
        glBindTexture(GL_TEXTURE_2D,modulatetexture);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    
    time = 0;
    stage = 0;
    return 1;
}

void intro_free(void) {
    int i;
    camera_free(camera);
    thing_mesh_free(ring);
    for(i = 0; i < NUM_FLAME; i++) particle_free(particle[i]);
    glDeleteTextures(1,&messagetexture);
    glDeleteTextures(1,&modulatetexture);
}

void intro_render_message(float pos[3],float width,float height,float stage) {
    float from,to;
    from = stage;
    to = stage + (height / width) / ((float)messageheight / (float)messagewidth);
    if(to > 1.0) from = to = 1.0;
    glBindTexture(GL_TEXTURE_2D,messagetexture);    // message texture
    glActiveTextureARB(GL_TEXTURE1_ARB);            // modulate texture
    glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,modulatetexture);
    glBegin(GL_TRIANGLE_STRIP);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0,to);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,0,1);
        glVertex3f(pos[0] - width / 2.0,pos[1],pos[2] - height / 2.0);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1,to);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,1,1);
        glVertex3f(pos[0] + width / 2.0,pos[1],pos[2] - height / 2.0);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0,from);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,0,0);
        glVertex3f(pos[0] - width / 2.0,pos[1],pos[2] + height / 2.0);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1,from);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,1,0);
        glVertex3f(pos[0] + width / 2.0,pos[1],pos[2] + height / 2.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
}

void intro_render(float ifps) {
    int i;
    float messagepos[3] = { 0, 1, 0 };
    float lightpos[4] = { 0, -3, 0, 1 };
    float lightcolor[4] = { 1, 1, 1, 1 };
    static float angle;

    glClearColor(0,0,0,0);      // clear screen
    glViewport(0,0,sys_width(),sys_height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    camera_view(camera);        // set camera
    
    glColor4f(1,1,1,1);         // color
    glEnable(GL_TEXTURE_2D);    // texture on
    
    intro_render_message(messagepos,6,4,stage);
    
    glEnable(GL_LIGHTING);      // set light
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0,GL_POSITION,lightpos);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,lightcolor);
    
    glPushMatrix();
    glRotatef(-angle / 5.0,0,0,1);
    glRotatef(-angle / 3.0,0,1,0);
    glRotatef(270,1,0,0);
    thing_texture_mesh_render(ring);
    glPopMatrix();
    
    glDisable(GL_LIGHTING);     // disable light
    
    for(i = 0; i < NUM_FLAME; i++)
        particle_render(particle[i],camera,ifps);
    
    glDisable(GL_TEXTURE_2D);   // texture off
    
    time += ifps;
    angle += ifps * 180.0;      // rotate ring
    if(time > 4.0) stage += ifps / (INTRO_LENGTH - 4.0);// scroll message
}
