/*  sky
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "mathlib.h"
#include "load3ds.h"
#include "camera.h"
#include "loadtga.h"
#include "loadjpeg.h"
#include "sky.h"

#include "system.h"

sky_t *sky_load(sky_config_t *config) {
    unsigned char *data;
    int i,width,height;
    sky_t *sky;
    sky = (sky_t*)malloc(sizeof(sky_t));
    memset(sky,0,sizeof(sky_t));
    if(!sky) return NULL;
    sky->vertex = Load3DS(config->mesh,&sky->num_vertex);   // load mesh
    if(!sky->vertex) return NULL;
    sky->radius = 0;
    for(i = 0; i < sky->num_vertex * 8; i += 8)     // get radius max z coordinate
        if(sky->vertex[i + 2] > sky->radius) sky->radius = sky->vertex[i + 2];
    for(i = 0; i < config->num_layer; i++) {
        sky->height[i] = config->height[i];
        sky->time[i] = config->time[i];
        data = NULL;
        if(strstr(config->texture[i],".jpg")) data = LoadJPEG(config->texture[i],&width,&height);
        else if(strstr(config->texture[i],".tga")) data = LoadTGA(config->texture[i],&width,&height);
        if(data) {
            glGenTextures(1,&sky->texture[i]);
            glBindTexture(GL_TEXTURE_2D,sky->texture[i]);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
            free(data);
        }
    }
    sky->num_layer = config->num_layer;
    sky->target = config->target;
    return sky;
}

void sky_free(sky_t *sky) {
    int i;
    if(sky->vertex) free(sky->vertex);
    for(i = 0; i < sky->num_layer; i++)
        if(sky->texture[i]) glDeleteTextures(1,&sky->texture[i]);
    free(sky);
}

void sky_render(sky_t *sky,camera_t *camera,float ifps) {
    int i;
    float dist,black[4] = { 0, 0, 0, 0 };
    glClearColor(camera->fogcolor[0],camera->fogcolor[1],camera->fogcolor[2],camera->fogcolor[3]);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera->fov,camera->aspect,camera->clipnear,sky->radius);
    glMatrixMode(GL_MODELVIEW);
    glEnableClientState(GL_VERTEX_ARRAY);   // enable vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3,GL_FLOAT,sizeof(float) * 8,sky->vertex + 0);  // set arrays
    glNormalPointer(GL_FLOAT,sizeof(float) * 8,sky->vertex + 3);
    glTexCoordPointer(2,GL_FLOAT,sizeof(float) * 8,sky->vertex + 6);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glPushMatrix();
    glTranslatef(camera->pos[0],camera->pos[1],camera->pos[2] - sky->radius + sky->height[0]);
    glRotatef(sky->target,0,0,1);
    glRotatef(sky->angle[0],1,0,0);
    glFogfv(GL_FOG_COLOR,camera->fogcolor);
    dist = sqrt(sky->radius * sky->radius - (sky->radius - sky->height[0]) * (sky->radius - sky->height[0]));
    glFogf(GL_FOG_START,dist - sky->height[0]);
    glFogf(GL_FOG_END,dist);
    glBindTexture(GL_TEXTURE_2D,sky->texture[0]);
    glDrawArrays(GL_TRIANGLES,0,sky->num_vertex);
    glPopMatrix();
    sky->angle[0] += ifps / sky->time[0] * 360.0;
    glFogfv(GL_FOG_COLOR,black);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    for(i = 1; i < sky->num_layer; i++) {
        glPushMatrix();
        glTranslatef(camera->pos[0],camera->pos[1],camera->pos[2] - sky->radius + sky->height[i]);
        glRotatef(sky->target,0,0,1);
        glRotatef(sky->angle[i],1,0,0);
        dist = sqrt(sky->radius * sky->radius - (sky->radius - sky->height[i]) * (sky->radius - sky->height[i]));
        glFogf(GL_FOG_START,dist - sky->height[i]);
        glFogf(GL_FOG_END,dist);
        glBindTexture(GL_TEXTURE_2D,sky->texture[i]);
        glDrawArrays(GL_TRIANGLES,0,sky->num_vertex);
        glPopMatrix();
        sky->angle[i] += ifps / sky->time[i] * 360.0;
    }
    glDisable(GL_BLEND);
    glDisable(GL_FOG);
    glEnable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera->fov,camera->aspect,camera->clipnear,camera->clipfar);
    glMatrixMode(GL_MODELVIEW);
}

sky_sun_t *sky_sun_load(sky_sun_config_t *config) {
    unsigned char *data;
    int i,width,height;
    sky_sun_t *sun;
    sun = (sky_sun_t*)malloc(sizeof(sky_sun_t));
    if(!sun) return NULL;
    memset(sun,0,sizeof(sky_sun_t));
    sun->num_flare = config->num_flare;
    VectorCopy(config->pos,sun->pos);
    sun->color[0] = config->color[0];
    sun->color[1] = config->color[1];
    sun->color[2] = config->color[2];
    sun->color[3] = config->color[3];
    for(i = 0; i < config->num_flare; i++) {
        sun->radius[i] = config->radius[i];
        sun->position[i] = config->position[i];
        sun->opacity[i] = config->opacity[i];
        data = NULL;
        if(strstr(config->texture[i],".jpg"))
            data = LoadJPEG(config->texture[i],&width,&height);
        else if(strstr(config->texture[i],".tga"))
            data = LoadTGA(config->texture[i],&width,&height);
        if(data) {
            glGenTextures(1,&sun->texture[i]);
            glBindTexture(GL_TEXTURE_2D,sun->texture[i]);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
            free(data);
        }
    }
    return sun;
}

void sky_sun_free(sky_sun_t *sun) {
    int i;
    for(i = 0; i < sun->num_flare; i++)
        if(sun->texture[i]) glDeleteTextures(1,&sun->texture[i]);
    free(sun);
}

void sky_sun_render(sky_sun_t *sun,camera_t *camera) {
    int i;
    float pos[3],dir[3],light[3],center[3],axis[3];
    float dx[3],dy[3],x[3],y[3],dot;
    VectorSet(camera->inverse[12],camera->inverse[13],camera->inverse[14],pos);
    VectorSet(0,0,-1,dir);
    VectorTransformNormal(dir,camera->inverse,dir);
    VectorNormalize(dir,dir);
    VectorCopy(sun->pos,light);
    VectorNormalize(light,light);
    dot = VectorDotProduct(light,dir);
    if(dot < 0.0) return;
    VectorScale(dir,camera->clipfar - camera->clipnear,center);
    VectorAdd(center,pos,center);
    VectorScale(light,(camera->clipfar - camera->clipnear) / dot,light);
    VectorAdd(light,pos,light);
    VectorSub(light,center,axis);
    VectorNormalize(axis,dx);
    VectorCrossProduct(dx,dir,dy);
    VectorSet(1,0,0,x);
    VectorTransformNormal(x,camera->inverse,x);
    VectorNormalize(x,x);
    VectorCrossProduct(x,dir,y);
    VectorScale(x,sun->radius[0],x);
    VectorScale(y,sun->radius[0],y);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glColor3f(sun->opacity[0],sun->opacity[0],sun->opacity[0]);
    glBindTexture(GL_TEXTURE_2D,sun->texture[0]);
    glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0,1);
        glVertex3f(light[0] - x[0] - y[0],light[1] - x[1] - y[1],light[2] - x[2] - y[2]);
        glTexCoord2f(1,1);
        glVertex3f(light[0] + x[0] - y[0],light[1] + x[1] - y[1],light[2] + x[2] - y[2]);
        glTexCoord2f(0,0);
        glVertex3f(light[0] - x[0] + y[0],light[1] - x[1] + y[1],light[2] - x[2] + y[2]);
        glTexCoord2f(1,0);
        glVertex3f(light[0] + x[0] + y[0],light[1] + x[1] + y[1],light[2] + x[2] + y[2]);
    glEnd();
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    for(i = 1; i < sun->num_flare; i++) {
        VectorScale(axis,sun->position[i],light);
        VectorAdd(light,center,light);
        VectorScale(dx,sun->radius[i],x);
        VectorScale(dy,sun->radius[i],y);
        glColor3f(sun->opacity[i],sun->opacity[i],sun->opacity[i]);
        glBindTexture(GL_TEXTURE_2D,sun->texture[i]);
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0,1);
            glVertex3f(light[0] - x[0] - y[0],light[1] - x[1] - y[1],light[2] - x[2] - y[2]);
            glTexCoord2f(1,1);
            glVertex3f(light[0] + x[0] - y[0],light[1] + x[1] - y[1],light[2] + x[2] - y[2]);
            glTexCoord2f(0,0);
            glVertex3f(light[0] - x[0] + y[0],light[1] - x[1] + y[1],light[2] - x[2] + y[2]);
            glTexCoord2f(1,0);
            glVertex3f(light[0] + x[0] + y[0],light[1] + x[1] + y[1],light[2] + x[2] + y[2]);
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glColor4f(1,1,1,1);
}
