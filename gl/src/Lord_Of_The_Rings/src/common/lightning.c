/*  lightning
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
#include "land.h"
#include "camera.h"
#include "loadtga.h"
#include "loadjpeg.h"
#include "lightning.h"

#include "system.h"

lightning_t *lightning_load(lightning_config_t *config) {
    unsigned char *data;
    int i,width,height;
    lightning_t *lightning;
    lightning = (lightning_t*)malloc(sizeof(lightning_t));
    if(!lightning) return NULL;
    memset(lightning,0,sizeof(lightning_t));
    lightning->width = config->width;
    lightning->radius = config->radius;
    lightning->num_texture = config->num_texture;
    data = NULL;    // loading flare
    if(strstr(config->flare,".jpg")) data = LoadJPEG(config->flare,&width,&height);
    else if(strstr(config->flare,".tga")) data = LoadTGA(config->flare,&width,&height);
    if(data) {
        glGenTextures(1,&lightning->flare);
        glBindTexture(GL_TEXTURE_2D,lightning->flare);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    for(i = 0; i < config->num_texture; i++) {  // loading texture
        data = NULL;
        if(strstr(config->texture[i],".jpg")) data = LoadJPEG(config->texture[i],&width,&height);
        else if(strstr(config->texture[i],".tga")) data = LoadTGA(config->texture[i],&width,&height);
        if(data) {
            glGenTextures(1,&lightning->texture[i]);
            glBindTexture(GL_TEXTURE_2D,lightning->texture[i]);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
            free(data);
        }
    }
    return lightning;
}

void lightning_free(lightning_t *lightning) {
    int i;
    for(i = 0; i < lightning->num_texture; i++)
        if(lightning->texture[i]) glDeleteTextures(1,&lightning->texture[i]);
    free(lightning);
}

void lightning_render(lightning_t *lightning,camera_t *camera,land_t *land,float *from,float *to,int frame) {
    float color[4] = { 1.0, 1.0, 1.0, 1.0 };
    float dir[3],x[3],y[3],dx[3],dy[3],dz[3],length;
    VectorSub(from,to,dir); // lightning length
    length = sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]) / lightning->width;
    VectorNormalize(dir,dir);
    VectorSet(1,0,0,dx);    // vectors for 2d sprite
    VectorTransformNormal(dx,camera->inverse,dx);
    VectorSet(0,1,0,dy);
    VectorTransformNormal(dy,camera->inverse,dy);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glDepthMask(GL_FALSE);
    glColor4fv(color);
    glBindTexture(GL_TEXTURE_2D,lightning->flare);  // flare on land
    land_dynamiclight(land,to,color,lightning->radius);
    VectorScale(dx,lightning->radius,x);       // flare
    VectorScale(dy,lightning->radius,y);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,1);
    glVertex3f(to[0] - x[0] - y[0],to[1] - x[1] - y[1],to[2] - x[2] - y[2]);
    glTexCoord2f(1,1);
    glVertex3f(to[0] + x[0] - y[0],to[1] + x[1] - y[1],to[2] + x[2] - y[2]);
    glTexCoord2f(0,0);
    glVertex3f(to[0] - x[0] + y[0],to[1] - x[1] + y[1],to[2] - x[2] + y[2]);
    glTexCoord2f(1,0);
    glVertex3f(to[0] + x[0] + y[0],to[1] + x[1] + y[1],to[2] + x[2] + y[2]);
    glEnd();
    VectorCrossProduct(dx,dy,dz);                   // ligtning
    VectorCrossProduct(dir,dz,x);
    VectorNormalize(x,x);
    VectorScale(x,lightning->width / 2.0,x);
    if(frame > lightning->num_texture - 1) frame %= lightning->num_texture;
    glBindTexture(GL_TEXTURE_2D,lightning->texture[frame]);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0);
    glVertex3f(to[0] - x[0],to[1] - x[1],to[2] - x[2]);
    glTexCoord2f(0,1);
    glVertex3f(to[0] + x[0],to[1] + x[1],to[2] + x[2]);
    glTexCoord2f(length,0);
    glVertex3f(from[0] - x[0],from[1] - x[1],from[2] - x[2]);
    glTexCoord2f(length,1);
    glVertex3f(from[0] + x[0],from[1] + x[1],from[2] + x[2]);
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
