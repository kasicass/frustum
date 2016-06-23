/*  dynamic light
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "mathlib.h"
#include "land.h"
#include "camera.h"
#include "loadtga.h"
#include "loadjpeg.h"
#include "dynamiclight.h"

#include "system.h"

dynamiclight_t *dynamiclight_load(dynamiclight_config_t *config) {
    unsigned char *data;
    int width,height;
    dynamiclight_t *light;
    light = (dynamiclight_t*)malloc(sizeof(dynamiclight_t));
    if(!light) return NULL;
    memset(light,0,sizeof(dynamiclight_t));
    light->center[0] = config->center[0];
    light->center[1] = config->center[1];
    light->center[2] = config->center[2];
    light->center[3] = 1.0; // opengl point light
    light->color[0] = config->color[0];
    light->color[1] = config->color[1];
    light->color[2] = config->color[2];
    light->color[3] = config->color[3];
    light->flareradius = config->flareradius;
    light->lightradius = config->lightradius;
    data = NULL;
    if(strstr(config->flaretexture,".jpg"))
        data = LoadJPEG(config->flaretexture,&width,&height);
    else if(strstr(config->flaretexture,".tga"))
        data = LoadTGA(config->flaretexture,&width,&height);
    if(data) {
        glGenTextures(1,&light->flaretexture);
        glBindTexture(GL_TEXTURE_2D,light->flaretexture);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    data = NULL;
    if(strstr(config->lighttexture,".jpg"))
        data = LoadJPEG(config->lighttexture,&width,&height);
    else if(strstr(config->lighttexture,".tga"))
        data = LoadTGA(config->lighttexture,&width,&height);
    if(data) {
        glGenTextures(1,&light->lighttexture);
        glBindTexture(GL_TEXTURE_2D,light->lighttexture);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    return light;
}

void dynamiclight_free(dynamiclight_t *light) {
    if(light->flaretexture) glDeleteTextures(1,&light->flaretexture);
    if(light->lighttexture) glDeleteTextures(1,&light->lighttexture);
    free(light);
}

void dynamiclight_render(dynamiclight_t *light,camera_t *camera,land_t *land) {
    float dx[3],dy[3];
    VectorSet(1,0,0,dx);
    VectorSet(0,1,0,dy);
    VectorTransformNormal(dx,camera->inverse,dx);
    VectorTransformNormal(dy,camera->inverse,dy);
    VectorScale(dx,light->flareradius,dx);
    VectorScale(dy,light->flareradius,dy);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    if(light->flaretexture) {   // if flare texture loaded
        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D,light->flaretexture);
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0,1);
            glVertex3f(light->center[0] - dx[0] - dy[0],light->center[1] - dx[1] - dy[1],light->center[2] - dx[2] - dy[2]);
            glTexCoord2f(1,1);
            glVertex3f(light->center[0] + dx[0] - dy[0],light->center[1] + dx[1] - dy[1],light->center[2] + dx[2] - dy[2]);
            glTexCoord2f(0,0);
            glVertex3f(light->center[0] - dx[0] + dy[0],light->center[1] - dx[1] + dy[1],light->center[2] - dx[2] + dy[2]);
            glTexCoord2f(1,0);
            glVertex3f(light->center[0] + dx[0] + dy[0],light->center[1] + dx[1] + dy[1],light->center[2] + dx[2] + dy[2]);
        glEnd();
        glDepthMask(GL_TRUE);
    }
    if(light->lighttexture) {   // if light texture loaded
        glColor4fv(light->color);
        glBindTexture(GL_TEXTURE_2D,light->lighttexture);
        land_dynamiclight(land,light->center,light->color,light->lightradius);
        glColor4f(1,1,1,1);
    }
    glDisable(GL_BLEND);
}

void dynamiclight_move(dynamiclight_t *light,float *pos) {
    VectorCopy(pos,light->center);
}
