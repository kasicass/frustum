/*  camera
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <malloc.h>

#include "mathlib.h"
#include "camera.h"

#include "system.h"

camera_t *camera_create(camera_config_t *config) {
    camera_t *camera;
    camera = (camera_t*)malloc(sizeof(camera_t));
    if(!camera) return NULL;
    memset(camera,0,sizeof(camera_t));
    camera->fov = config->fov;
    camera->aspect = config->aspect;
    camera->clipnear = config->clipnear;
    camera->clipfar = config->clipfar;
    camera->fogcolor[0] = config->fogcolor[0];
    camera->fogcolor[1] = config->fogcolor[1];
    camera->fogcolor[2] = config->fogcolor[2];
    camera->fogcolor[3] = config->fogcolor[3];
    camera->fogstart = config->fogstart;
    camera->fogend = config->fogend;
    return camera;
}

void camera_free(camera_t *camera) {
    free(camera);
}

void camera_extract_frustum(camera_t *camera) {
    float ilength,clip[16],projection[16],modelview[16];
    glGetFloatv(GL_PROJECTION_MATRIX,projection);
    glGetFloatv(GL_MODELVIEW_MATRIX,modelview);
    MatrixMultiply(projection,modelview,clip);
    camera->frustum[0][0] = clip[3] - clip[0];
    camera->frustum[0][1] = clip[7] - clip[4];
    camera->frustum[0][2] = clip[11] - clip[8];
    camera->frustum[0][3] = clip[15] - clip[12];
    ilength = 1.0 / VectorLength(camera->frustum[0]);
    camera->frustum[0][0] *= ilength;
    camera->frustum[0][1] *= ilength;
    camera->frustum[0][2] *= ilength;
    camera->frustum[0][3] *= ilength;
    camera->frustum[1][0] = clip[3] + clip[0];
    camera->frustum[1][1] = clip[7] + clip[4];
    camera->frustum[1][2] = clip[11] + clip[8];
    camera->frustum[1][3] = clip[15] + clip[12];
    ilength = 1.0 / VectorLength(camera->frustum[1]);
    camera->frustum[1][0] *= ilength;
    camera->frustum[1][1] *= ilength;
    camera->frustum[1][2] *= ilength;
    camera->frustum[1][3] *= ilength;
    camera->frustum[2][0] = clip[3] + clip[1];
    camera->frustum[2][1] = clip[7] + clip[5];
    camera->frustum[2][2] = clip[11] + clip[9];
    camera->frustum[2][3] = clip[15] + clip[13];
    ilength = 1.0 / VectorLength(camera->frustum[2]);
    camera->frustum[2][0] *= ilength;
    camera->frustum[2][1] *= ilength;
    camera->frustum[2][2] *= ilength;
    camera->frustum[2][3] *= ilength;
    camera->frustum[3][0] = clip[3] - clip[1];
    camera->frustum[3][1] = clip[7] - clip[5];
    camera->frustum[3][2] = clip[11] - clip[9];
    camera->frustum[3][3] = clip[15] - clip[13];
    ilength = 1.0 / VectorLength(camera->frustum[3]);
    camera->frustum[3][0] *= ilength;
    camera->frustum[3][1] *= ilength;
    camera->frustum[3][2] *= ilength;
    camera->frustum[3][3] *= ilength;
    camera->frustum[4][0] = clip[3] - clip[2];
    camera->frustum[4][1] = clip[7] - clip[6];
    camera->frustum[4][2] = clip[11] - clip[10];
    camera->frustum[4][3] = clip[15] - clip[14];
    ilength = 1.0 / VectorLength(camera->frustum[4]);
    camera->frustum[4][0] *= ilength;
    camera->frustum[4][1] *= ilength;
    camera->frustum[4][2] *= ilength;
    camera->frustum[4][3] *= ilength;
    camera->frustum[5][0] = clip[3] + clip[2];
    camera->frustum[5][1] = clip[7] + clip[6];
    camera->frustum[5][2] = clip[11] + clip[10];
    camera->frustum[5][3] = clip[15] + clip[14];
    ilength = 1.0 / VectorLength(camera->frustum[5]);
    camera->frustum[5][0] *= ilength;
    camera->frustum[5][1] *= ilength;
    camera->frustum[5][2] *= ilength;
    camera->frustum[5][3] *= ilength;
}

void camera_view(camera_t *camera) {
    float modelview[16];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera->fov,camera->aspect,camera->clipnear,camera->clipfar);
    glMatrixMode(GL_MODELVIEW);
    MatrixLookAt(camera->pos,camera->dir,camera->up,modelview);
    MatrixInverse(modelview,camera->inverse);
    glLoadMatrixf(modelview);
    camera_extract_frustum(camera);
}

int camera_check_point(camera_t *camera,float *pos) {
    int i;
    for(i = 0; i < 6; i++)
        if(-VectorDotProduct(camera->frustum[i],pos) >= camera->frustum[i][3]) return 0;
    return 1;
}

int camera_check_sphere(camera_t *camera,float *pos,float radius) {
    int i;
    for(i = 0; i < 6; i++)
        if(-VectorDotProduct(camera->frustum[i],pos) >= camera->frustum[i][3] + radius) return 0;
    return 1;
}

int camera_check_box(camera_t *camera,float *min,float *max) {
    int i;
    float point[3];
    for(i = 0; i < 6; i++) {
        VectorSet(min[0],min[1],min[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        VectorSet(max[0],min[1],min[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        VectorSet(min[0],max[1],min[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        VectorSet(max[0],max[1],min[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        VectorSet(min[0],min[1],max[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        VectorSet(max[0],min[1],max[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        VectorSet(min[0],max[1],max[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        VectorSet(max[0],max[1],max[2],point);
        if(-VectorDotProduct(camera->frustum[i],point) < camera->frustum[i][3]) continue;
        return 0;
    }
    return 1;
}
