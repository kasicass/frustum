/*  outtro
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
#include "loadjpeg.h"
#include "outtro.h"

#include "system.h"

static float time,stage;
static camera_t *camera;
static int messagetexture,modulatetexture;
static int messagewidth,messageheight;

int outtro_load(char *path,int texture_mode) {
    char buffer[256];
    unsigned char *data;
    int width,height;
    camera_config_t cameraconfig;
    
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

void outtro_free(void) {
    camera_free(camera);
    glDeleteTextures(1,&messagetexture);
    glDeleteTextures(1,&modulatetexture);
}

void outtro_render_message(float pos[3],float width,float height,float stage) {
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

void outtro_render(float ifps) {
    float messagepos[3] = { 0, 1, 0 };

    glClearColor(0,0,0,0);      // clear screen
    glViewport(0,0,sys_width(),sys_height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    camera_view(camera);        // set camera
    
    glColor4f(1,1,1,1);         // color
    glEnable(GL_TEXTURE_2D);    // texture on
    
    outtro_render_message(messagepos,6,4,stage);
    
    glDisable(GL_TEXTURE_2D);   // texture off
    
    time += ifps;
    if(time > 1.0) stage += ifps / (OUTTRO_LENGTH - 1.0);// scroll message
}
