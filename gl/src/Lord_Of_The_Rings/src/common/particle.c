/*  particle
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "mathlib.h"
#include "camera.h"
#include "loadtga.h"
#include "loadjpeg.h"
#include "particle.h"

#include "system.h"

particle_t *particle_load(particle_config_t *config) {
    unsigned char *data;
    int i,width,height;
    particle_t *particle;
    particle = (particle_t*)malloc(sizeof(particle_t));
    if(!particle) return NULL;
    memset(particle,0,sizeof(particle_t));
    particle->pos = (float*)malloc(sizeof(float) * config->num_particle * 3);
    if(!particle->pos) return NULL;
    particle->speed = (float*)malloc(sizeof(float) * config->num_particle * 3);
    if(!particle->speed) return NULL;
    particle->life = (float*)malloc(sizeof(float) * config->num_particle);
    if(!particle->life) return NULL;
    memset(particle->speed,0,sizeof(float) * config->num_particle * 3);
    for(i = 0; i < config->num_particle; i++) {
        VectorCopy(config->center,&particle->pos[i * 3]);
        particle->life[i] = (float)i / (float)config->num_particle;
    }
    particle->num_particle = config->num_particle;
    particle->center[0] = config->center[0];
    particle->center[1] = config->center[1];
    particle->center[2] = config->center[2];
    particle->color[0] = config->color[0];
    particle->color[1] = config->color[1];
    particle->color[2] = config->color[2];
    particle->color[3] = config->color[3];
    particle->mass = config->mass;
    particle->maxspeed = config->speed;
    particle->radius = config->radius;
    particle->time = config->time;
    particle->force[0] = config->force[0];
    particle->force[1] = config->force[1];
    particle->force[2] = config->force[2];
    data = NULL;
    if(strstr(config->texture,".jpg"))
        data = LoadJPEG(config->texture,&width,&height);
    else if(strstr(config->texture,".tga"))
        data = LoadTGA(config->texture,&width,&height);
    if(data) {
        glGenTextures(1,&particle->texture);
        glBindTexture(GL_TEXTURE_2D,particle->texture);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    return particle;
}

void particle_free(particle_t *particle) {
    if(particle->pos) free(particle->pos);
    if(particle->speed) free(particle->speed);
    if(particle->life) free(particle->life);
    if(particle->texture) glDeleteTextures(1,&particle->texture);
    free(particle);
}

void particle_render(particle_t *particle,camera_t *camera,float ifps) {
    int i,j;
    float dx[3],dy[3],pos[3],speed[3];
    VectorSet(1,0,0,dx);
    VectorSet(0,1,0,dy);
    VectorTransformNormal(dx,camera->inverse,dx);
    VectorTransformNormal(dy,camera->inverse,dy);
    VectorScale(dx,particle->radius,dx);
    VectorScale(dy,particle->radius,dy);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glBindTexture(GL_TEXTURE_2D,particle->texture);
    for(i = 0, j = 0; i < particle->num_particle; i++, j += 3) {
        if(particle->life[i] > 0) {
            VectorScale(particle->force,1.0 / particle->mass * ifps,speed); // force
            speed[2] -= PHYSIC_G * ifps;  // gravity
            VectorAdd(&particle->speed[j],speed,&particle->speed[j]);
            VectorScale(&particle->speed[j],ifps,speed);
            VectorAdd(&particle->pos[j],speed,&particle->pos[j]);
            particle->life[i] -= ifps / particle->time;
        } else {
            particle->life[i] = 1.0;
            VectorCopy(particle->center,&particle->pos[j]);
            VectorSet(((float)rand() / RAND_MAX - 0.5) * particle->maxspeed,
                      ((float)rand() / RAND_MAX - 0.5) * particle->maxspeed,
                      ((float)rand() / RAND_MAX - 0.5) * particle->maxspeed,&particle->speed[j]);
        }
        VectorCopy(&particle->pos[j],pos);
        glColor4f(particle->color[0] * particle->life[i],
                  particle->color[1] * particle->life[i],
                  particle->color[2] * particle->life[i],
                  particle->color[4] * particle->life[i]);
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0,1);
            glVertex3f(pos[0] - dx[0] - dy[0],pos[1] - dx[1] - dy[1],pos[2] - dx[2] - dy[2]);
            glTexCoord2f(1,1);
            glVertex3f(pos[0] + dx[0] - dy[0],pos[1] + dx[1] - dy[1],pos[2] + dx[2] - dy[2]);
            glTexCoord2f(0,0);
            glVertex3f(pos[0] - dx[0] + dy[0],pos[1] - dx[1] + dy[1],pos[2] - dx[2] + dy[2]);
            glTexCoord2f(1,0);
            glVertex3f(pos[0] + dx[0] + dy[0],pos[1] + dx[1] + dy[1],pos[2] + dx[2] + dy[2]);
        glEnd();
    }
    glColor4f(1,1,1,1);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void particle_move(particle_t *particle,float *pos) {
    VectorCopy(pos,particle->center);
}
