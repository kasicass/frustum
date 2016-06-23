/*  landscape
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include "land.h"

void VectorCrossProduct(const float *v1,const float *v2,float *out) {
    out[0] = v1[1] * v2[2] - v1[2] * v2[1];
    out[1] = v1[2] * v2[0] - v1[0] * v2[2];
    out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

land_t *land_create(unsigned char *heightmap,int width,int height,float step,float altitude) {
    int i,j,k,l;
    land_t *land;
    land = (land_t*)malloc(sizeof(land_t));
    if(!land) return NULL;
    memset(land,0,sizeof(land_t));
    land->vertex = (land_vertex_t*)malloc(sizeof(land_vertex_t) * width * height);
    if(!land->vertex) return NULL;
    for(j = 0, k = 0, l = 0; j < height; j++)
        for(i = 0; i < width; i++, k++, l += 4)
            VectorSet((float)i * step,(float)j * step,(float)heightmap[l] / 255.0 * altitude,land->vertex[k].v);
    land->width = width;
    land->height = height;
    land->step = step;
    return land;
}

float land_height(land_t *land,float *point) {
    int i,j,k;
    float x,y,dot0,dot1,p00[3],p10[3],p01[3],v10[3],v01[3],point0[3],point1[3],plane[4];
    x = point[0] / land->step;
    y = point[1] / land->step;
    i = (int)x;
    j = (int)y;
    if(i < 0) i = 0;
    if(i > land->width - 1) i = land->width - 1;
    if(j < 0) j = 0;
    if(j > land->height - 1) j = land->height - 1;
    k = land->width * j + i;
    if(x + y < 1 + (int)x + (int)y) {
        VectorCopy(land->vertex[k].v,p00);
        VectorCopy(land->vertex[k + 1].v,p10);
        VectorCopy(land->vertex[k + land->width].v,p01);
    } else {
        VectorCopy(land->vertex[k + land->width + 1].v,p00);
        VectorCopy(land->vertex[k + land->width].v,p10);
        VectorCopy(land->vertex[k + 1].v,p01);
    }
    VectorSub(p10,p00,v10);
    VectorSub(p01,p00,v01);
    VectorCrossProduct(v10,v01,plane);
    plane[3] = -VectorDotProduct(plane,p00);
    VectorCopy(point,point0);
    VectorCopy(point,point1);
    point0[2] = 0;
    point1[2] = 1;
    dot0 = -VectorDotProduct(plane,point0);
    dot1 = -VectorDotProduct(plane,point1);
    return (point0[2] + (point1[2] - point0[2]) *
        (plane[3] - dot0) / (dot1 - dot0));
}
