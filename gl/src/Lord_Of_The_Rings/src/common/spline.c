/*  spline
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "mathlib.h"
#include "spline.h"

void spline_pos(spline_t *spline,float t,float *pos) {
    int link;
    float t_2,t_3;
    t *= ((float)spline->num_key / spline->totaltime);
    link = (int)t;
    t -= link;
    if(link > spline->num_key - 1) {
        if(spline->close) link = link % spline->num_key;
        else {
            link = spline->num_key - 1;
            t = 1.0;
        }
    }
    link *= 4;
    t_2 = t * t;
    t_3 = t_2 * t;
    pos[0] = spline->x[link + 0] +
             spline->x[link + 1] * t +
             spline->x[link + 2] * t_2 +
             spline->x[link + 3] * t_3;
    pos[1] = spline->y[link + 0] +
             spline->y[link + 1] * t +
             spline->y[link + 2] * t_2 +
             spline->y[link + 3] * t_3;
    pos[2] = spline->z[link + 0] +
             spline->z[link + 1] * t +
             spline->z[link + 2] * t_2 +
             spline->z[link + 3] * t_3;
}

float spline_target(spline_t *spline,float t) {
    int link;
    float t_2,consern[3];
    t *= ((float)spline->num_key / spline->totaltime);
    link = (int)t;
    t -= link;
    if(link > spline->num_key - 1) {
        if(spline->close) link = link % spline->num_key;
        else {
            link = spline->num_key - 1;
            t = 1.0;
        }
    }
    link *= 4;
    t_2 = t * t;
    consern[0] = spline->x[link + 1] +
                 2.0 * spline->x[link + 2] * t +
                 3.0 * spline->x[link + 3] * t_2;
    consern[1] = spline->y[link + 1] +
                 2.0 * spline->y[link + 2] * t +
                 3.0 * spline->y[link + 3] * t_2;
    consern[2] = 0;
    VectorNormalize(consern,consern);
    if(consern[0] > 0) return -acos(consern[1]) * RAD2DEG;
    return acos(consern[1]) * RAD2DEG;
}

spline_t *spline_create(float *key,int num_key,float totaltime,float tension,float bias,float continuity) {
    int i,j;
    float prev,cur,next,p0,p1,r0,r1,*ptr;
    spline_t *spline;
    spline = (spline_t*)malloc(sizeof(spline_t));
    if(!spline) return NULL;
    memset(spline,0,sizeof(spline_t));
    spline->x = (float*)malloc(sizeof(float) * 4 * num_key);
    if(!spline->x) return NULL;
    spline->y = (float*)malloc(sizeof(float) * 4 * num_key);
    if(!spline->y) return NULL;
    spline->z = (float*)malloc(sizeof(float) * 4 * num_key);
    if(!spline->z) return NULL;
    for(i = 0; i < 3; i++) {    // for all axis
        if(i == 0) ptr = spline->x;
        else if(i == 1) ptr = spline->y;
        else ptr = spline->z;
        for(j = 0; j < num_key; j++) {  // calculate df/dx
            if(j == 0) {
                prev = key[i + j * 3];
                cur = key[i + j * 3];
                next = key[i + j * 3 + 3];
            } else if(j == num_key - 1) {
                prev = key[i + j * 3 - 3];
                cur = key[i + j * 3];
                next = key[i + j * 3];
            } else {
                prev = key[i + j * 3 - 3];
                cur = key[i + j * 3];
                next = key[i + j * 3 + 3];
            }
            p0 = (cur - prev) * (1.0 + bias);
            p1 = (next - cur) * (1.0 - bias);
            r0 = (1.0 - tension) * 0.5 * (p0 + p1) * (1.0 + continuity);
            r1 = (1.0 - tension) * 0.5 * (p0 + p1) * (1.0 - continuity);
            ptr[j * 4 + 0] = cur;
            ptr[j * 4 + 1] = next;
            ptr[j * 4 + 2] = r0;
            if(j != 0) ptr[(j - 1) * 4 + 3] = r1;
        }
        for(j = 0; j < num_key; j++) {
            p0 = ptr[j * 4 + 0];
            p1 = ptr[j * 4 + 1];
            r0 = ptr[j * 4 + 2];
            r1 = ptr[j * 4 + 3];
            ptr[j * 4 + 0] = p0;
            ptr[j * 4 + 1] = r0;
            ptr[j * 4 + 2] = -3.0 * p0 + 3.0 * p1 - 2.0 * r0 - r1;
            ptr[j * 4 + 3] = 2.0 * p0 - 2.0 * p1 + r0 + r1;
        }
    }
    spline->close = 0;
    spline->num_key = num_key;
    spline->totaltime = totaltime;
    return spline;
}


spline_t *spline_close_create(float *key,int num_key,float totaltime,float tension,float bias,float continuity) {
    int i,j;
    float prev,cur,next,p0,p1,r0,r1,*ptr;
    spline_t *spline;
    spline = (spline_t*)malloc(sizeof(spline_t));
    if(!spline) return NULL;
    memset(spline,0,sizeof(spline_t));
    spline->x = (float*)malloc(sizeof(float) * 4 * num_key);
    if(!spline->x) return NULL;
    spline->y = (float*)malloc(sizeof(float) * 4 * num_key);
    if(!spline->y) return NULL;
    spline->z = (float*)malloc(sizeof(float) * 4 * num_key);
    if(!spline->z) return NULL;
    for(i = 0; i < 3; i++) {    // for all axis
        if(i == 0) ptr = spline->x;
        else if(i == 1) ptr = spline->y;
        else ptr = spline->z;
        for(j = 0; j < num_key; j++) {  // calculate df/dx
            if(j == 0) {
                prev = key[i + (num_key - 1) * 3];
                cur = key[i + j * 3];
                next = key[i + j * 3 + 3];
            } else if(j == num_key - 1) {
                prev = key[i + j * 3 - 3];
                cur = key[i + j * 3];
                next = key[i];
            } else {
                prev = key[i + j * 3 - 3];
                cur = key[i + j * 3];
                next = key[i + j * 3 + 3];
            }
            p0 = (cur - prev) * (1.0 + bias);
            p1 = (next - cur) * (1.0 - bias);
            r0 = (1.0 - tension) * 0.5 * (p0 + p1) * (1.0 + continuity);
            r1 = (1.0 - tension) * 0.5 * (p0 + p1) * (1.0 - continuity);
            ptr[j * 4 + 0] = cur;
            ptr[j * 4 + 1] = next;
            ptr[j * 4 + 2] = r0;
            if(j != 0) ptr[(j - 1) * 4 + 3] = r1;
            else ptr[(num_key - 1) * 4 + 3] = r1;
        }
        for(j = 0; j < num_key; j++) {
            p0 = ptr[j * 4 + 0];
            p1 = ptr[j * 4 + 1];
            r0 = ptr[j * 4 + 2];
            r1 = ptr[j * 4 + 3];
            ptr[j * 4 + 0] = p0;
            ptr[j * 4 + 1] = r0;
            ptr[j * 4 + 2] = -3.0 * p0 + 3.0 * p1 - 2.0 * r0 - r1;
            ptr[j * 4 + 3] = 2.0 * p0 - 2.0 * p1 + r0 + r1;
        }
    }
    spline->close = 1;
    spline->num_key = num_key;
    spline->totaltime = totaltime;
    return spline;
}

float *spline_load_key(char *name,int *num_key) {
    int i;
    float *key;
    FILE *file;
    char buffer[256];
    file = fopen(name,"r");
    if(!file) return 0;
    i = 0;
    while(fscanf(file,"%s",buffer) != EOF) i++;
    i /= 3;
    key = (float*)malloc(sizeof(float) * 3 * i);
    if(!key) {
        fclose(file);
        return NULL;
    }
    *num_key = i;
    fseek(file,0,SEEK_SET);
    for(i = 0; i < *num_key * 3; i++) {
        fscanf(file,"%s",buffer);
        key[i] = atof(buffer);
    }
    fclose(file);
    return key;
}

spline_t *spline_load(char *name,float totaltime,float tension,float bias,float continuity) {
    float *key;
    int num_key;
    spline_t *spline;
    key = spline_load_key(name,&num_key);
    if(!key) return NULL;
    spline = spline_create(key,num_key,totaltime,tension,bias,continuity);
    free(key);
    return spline;
}

spline_t *spline_close_load(char *name,float totaltime,float tension,float bias,float continuity) {
    float *key;
    int num_key;
    spline_t *spline;
    key = spline_load_key(name,&num_key);
    if(!key) return NULL;
    spline = spline_close_create(key,num_key,totaltime,tension,bias,continuity);
    free(key);
    return spline;
}

void spline_free(spline_t *spline) {
    free(spline->x);
    free(spline->y);
    free(spline->z);
    free(spline);
}
