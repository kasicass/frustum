/*  thing
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
#include "loadime.h"
#include "loadtga.h"
#include "loadjpeg.h"
#include "thing.h"

#include "system.h"

int thing_load_3ds(thing_mesh_t *mesh,char *name) {
    float radius,dist[3];
    int i,j,num_vertex;
    mesh->vertex = Load3DS(name,&num_vertex);   // load 3ds file
    if(!mesh->vertex) return 0;
    mesh->num_vertex = num_vertex;
    VectorSet(1000000,1000000,1000000,mesh->min);
    VectorSet(-1000000,-1000000,-1000000,mesh->max);
    for(i = 0; i < num_vertex * 8; i += 8) {    // get size object
        for(j = 0; j < 3; j++) {
            if(mesh->vertex[i + j] < mesh->min[j]) mesh->min[j] = mesh->vertex[i + j];
            if(mesh->vertex[i + j] > mesh->max[j]) mesh->max[j] = mesh->vertex[i + j];
        }
    }
    VectorAdd(mesh->min,mesh->max,mesh->center);
    VectorScale(mesh->center,0.5,mesh->center);     // center object (min + max) / 2
    mesh->radius = 0;
    for(i = 0; i < num_vertex * 8; i += 8) {    // get radius object
        VectorSub(&mesh->vertex[i],mesh->center,dist);
        radius = sqrt(dist[0] * dist[0] + dist[1] * dist[1] + dist[2] * dist[2]);
        if(radius > mesh->radius) mesh->radius = radius;
    }
    mesh->interpolate = 0;
    return 1;
}

int thing_load_ime(thing_mesh_t *mesh,char *name) {
    float radius,dist[3];
    int i,j,k,num_vertex,num_frame;
    mesh->frame = LoadIME(name,&num_vertex,&num_frame); // load interpolate mesh
    if(!mesh->frame) return 0;
    mesh->num_vertex = num_vertex;
    mesh->num_frame = num_frame;
    mesh->vertex = (float*)malloc(sizeof(float) * 8 * num_vertex);
    if(!mesh->vertex) return 0;
    memcpy(mesh->vertex,mesh->frame[0],sizeof(float) * 8 * num_vertex);
    VectorSet(1000000,1000000,1000000,mesh->min);
    VectorSet(-1000000,-1000000,-1000000,mesh->max);
    for(i = 0; i < num_frame; i++)  // get size object
        for(j = 0; j < num_vertex * 8; j += 8)
            for(k = 0; k < 3; k++) {
                if(mesh->frame[i][j + k] < mesh->min[k]) mesh->min[k] = mesh->frame[i][j + k];
                if(mesh->frame[i][j + k] > mesh->max[k]) mesh->max[k] = mesh->frame[i][j + k];
            }
    VectorAdd(mesh->min,mesh->max,mesh->center);
    VectorScale(mesh->center,0.5,mesh->center);     // center object (min + max) / 2
    mesh->radius = 0;
    for(i = 0; i < num_frame; i++)  // get radius object
        for(j = 0; j < num_vertex * 8; j += 8) {
            VectorSub(&mesh->frame[i][j],mesh->center,dist);
            radius = sqrt(dist[0] * dist[0] + dist[1] * dist[1] + dist[2] * dist[2]);
            if(radius > mesh->radius) mesh->radius = radius;
        }
    mesh->interpolate = 1;
    return 1;
}

thing_mesh_t *thing_mesh_load(thing_mesh_config_t *config) {
    unsigned char *data;
    int i,width,height;
    thing_mesh_t *mesh;
    mesh = (thing_mesh_t*)malloc(sizeof(thing_mesh_t));
    if(!mesh) return NULL;
    memset(mesh,0,sizeof(thing_mesh_t));
    if(strstr(config->meshname,".3ds")) {
        if(!thing_load_3ds(mesh,config->meshname)) return NULL; // load 3ds
        if(config->shader == SHADER_LIGHTMAP) { // load lightmap mesh
            mesh->lightvertex = Load3DS(config->lightname,&i);
            if(!mesh->lightvertex || i != mesh->num_vertex) return NULL;
        }
    } else if(strstr(config->meshname,".ime")) {
        if(!thing_load_ime(mesh,config->meshname)) return NULL;
    } else return NULL;    
    mesh->material = glGenLists(1); // create material
    glNewList(mesh->material,GL_COMPILE);
    glMaterialfv(GL_FRONT,GL_AMBIENT,config->ambient);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,config->diffuse);
    glMaterialfv(GL_FRONT,GL_SPECULAR,config->specular);
    glEndList();
    data = NULL;    // load base texture
    if(strstr(config->basetexture,".jpg")) data = LoadJPEG(config->basetexture,&width,&height);
    else if(strstr(config->basetexture,".tga")) data = LoadTGA(config->basetexture,&width,&height);
    if(data) {
        glGenTextures(1,&mesh->base);
        glBindTexture(GL_TEXTURE_2D,mesh->base);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    if(config->shader == SHADER_SPECULAR || config->shader == SHADER_GLOSS) {   // load specular texture
        data = NULL;
        if(strstr(config->speculartexture,".jpg")) data = LoadJPEG(config->speculartexture,&width,&height);
        else if(strstr(config->speculartexture,".tga")) data = LoadTGA(config->speculartexture,&width,&height);
        if(data) {
            glGenTextures(1,&mesh->specular);
            glBindTexture(GL_TEXTURE_2D,mesh->specular);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
            free(data);
        }
    }
    if(config->shader == SHADER_LIGHTMAP) { // load lightmap texture
        data = NULL;
        if(strstr(config->lighttexture,".jpg")) data = LoadJPEG(config->lighttexture,&width,&height);
        else if(strstr(config->lighttexture,".tga")) data = LoadTGA(config->lighttexture,&width,&height);
        if(data) {
            glGenTextures(1,&mesh->light);
            glBindTexture(GL_TEXTURE_2D,mesh->light);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
            free(data);
        }
    }
    mesh->shader = config->shader;
    mesh->alphatest = config->alphatest;
    strcpy(mesh->name,config->name);
    return mesh;
}

thing_mesh_t *thing_mesh_copy(thing_mesh_t *oldmesh) { // copy mesh
    thing_mesh_t *mesh;
    mesh = (thing_mesh_t*)malloc(sizeof(thing_mesh_t));
    if(!mesh) return NULL;
    memcpy(mesh,oldmesh,sizeof(thing_mesh_t));
    mesh->vertex = (float*)malloc(sizeof(float) * 8 * mesh->num_vertex);
    if(!mesh->vertex) return NULL;  // only vertex chnged
    memcpy(mesh->vertex,oldmesh->vertex,sizeof(float) * 8 * mesh->num_vertex);
    return mesh;
}

void thing_mesh_free(thing_mesh_t *mesh) {
    int i;
    if(mesh->vertex) free(mesh->vertex);
    if(mesh->lightvertex) free(mesh->lightvertex);
    for(i = 0; i < mesh->num_frame; i++)
        if(mesh->frame[i]) free(mesh->frame[i]);
    if(mesh->base) glDeleteTextures(1,&mesh->base);
    if(mesh->specular) glDeleteTextures(1,&mesh->specular);
    free(mesh);
}

void thing_mesh_create(thing_mesh_t *mesh,float frame) {
    int i,cur,next;
    if(!mesh->interpolate || mesh->num_frame == 1) return;  // not interpolated or only one mesh
    cur = frame;
    frame -= cur;
    if(cur > mesh->num_frame - 1) cur %= mesh->num_frame;   // cur mesh
    next = cur + 1;
    if(next > mesh->num_frame - 1) next = 0;    // next mesh
    for(i = 0; i < mesh->num_vertex * 8; i += 8) {  // interpolate
        VectorSub(&mesh->frame[next][i],&mesh->frame[cur][i],&mesh->vertex[i]);
        VectorScale(&mesh->vertex[i],frame,&mesh->vertex[i]);
        VectorAdd(&mesh->vertex[i],&mesh->frame[cur][i],&mesh->vertex[i]);
        VectorSub(&mesh->frame[next][i + 3],&mesh->frame[cur][i + 3],&mesh->vertex[i + 3]);
        VectorScale(&mesh->vertex[i + 3],frame,&mesh->vertex[i + 3]);
        VectorAdd(&mesh->vertex[i + 3],&mesh->frame[cur][i + 3],&mesh->vertex[i + 3]);
    }
}

void thing_mesh_render(thing_mesh_t *mesh) {
    glEnableClientState(GL_VERTEX_ARRAY);   // enable arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3,GL_FLOAT,sizeof(float) * 8,mesh->vertex + 0);
    glNormalPointer(GL_FLOAT,sizeof(float) * 8,mesh->vertex + 3);
    glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
    glDisableClientState(GL_VERTEX_ARRAY);  // disable arrays
    glDisableClientState(GL_NORMAL_ARRAY);
}

void thing_texture_mesh_render(thing_mesh_t *mesh) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3,GL_FLOAT,sizeof(float) * 8,mesh->vertex + 0);
    glNormalPointer(GL_FLOAT,sizeof(float) * 8,mesh->vertex + 3);
    glTexCoordPointer(2,GL_FLOAT,sizeof(float) * 8,mesh->vertex + 6);
    glCallList(mesh->material);
    switch(mesh->shader) {  // switch shaders
        case SHADER_DIFFUSE:
            if(mesh->alphatest) {
                glEnable(GL_ALPHA_TEST);
                glAlphaFunc(GL_GEQUAL,0.9);
                glDisable(GL_CULL_FACE);
            }
            glBindTexture(GL_TEXTURE_2D,mesh->base);
            glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
            if(mesh->alphatest) {
                glEnable(GL_CULL_FACE);
                glDisable(GL_ALPHA_TEST);
            }
            break;
        case SHADER_SPECULAR:
            glBindTexture(GL_TEXTURE_2D,mesh->base);
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glClientActiveTextureARB(GL_TEXTURE1_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_ADD);
            glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
            glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glBindTexture(GL_TEXTURE_2D,mesh->specular);
            glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
            break;
        case SHADER_GLOSS:
            glDisable(GL_LIGHTING);
            glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
            glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glBindTexture(GL_TEXTURE_2D,mesh->specular);
            glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glEnable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE,GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D,mesh->base);
            glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_TEXTURE);
            glDisable(GL_BLEND);
            break;
        case SHADER_TRANSPARENT:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE,GL_SRC_ALPHA);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_EQUAL,1);
            glBindTexture(GL_TEXTURE_2D,mesh->base);
            glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            break;
        case SHADER_LIGHTMAP:
            glBindTexture(GL_TEXTURE_2D,mesh->base);
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glClientActiveTextureARB(GL_TEXTURE1_ARB);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,mesh->light);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2,GL_FLOAT,sizeof(float) * 8,mesh->lightvertex + 6);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
            glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_TEXTURE_2D);
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
            break;
    }
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

thing_t *thing_create(thing_mesh_t *mesh,float *pos,float target) {
    thing_t *thing;
    thing = (thing_t*)malloc(sizeof(thing_t));
    if(!thing) return NULL;
    memset(thing,0,sizeof(thing_t));
    VectorCopy(pos,thing->pos);     // set position
    thing->target = target;
    VectorCopy(pos,thing->center);  // change center coordinate
    VectorAdd(thing->center,mesh->center,thing->center);
    thing->radius = mesh->radius;
    VectorCopy(pos,thing->min);     // change min and max coordinate
    VectorAdd(thing->min,mesh->min,thing->min);
    VectorCopy(pos,thing->max);
    VectorAdd(thing->max,mesh->max,thing->max);
    thing->mesh = mesh;
    return thing;
}

void thing_free(thing_t *thing) {
    free(thing);
}

void thing_move(thing_t *thing,float *pos,float target) {
    VectorCopy(pos,thing->pos);
    thing->target = target;
    VectorCopy(pos,thing->center);  // new position, center and size
    VectorAdd(thing->center,thing->mesh->center,thing->center);
    VectorCopy(pos,thing->min);
    VectorAdd(thing->min,thing->mesh->min,thing->min);
    VectorCopy(pos,thing->max);
    VectorAdd(thing->max,thing->mesh->max,thing->max);
}

void thing_render(thing_t *thing) {
    glPushMatrix();
    glTranslatef(thing->pos[0],thing->pos[1],thing->pos[2]); // move object
    glRotatef(thing->target,0,0,1); // rotate object
    thing_texture_mesh_render(thing->mesh);
    glPopMatrix();
}
