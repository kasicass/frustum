/*  script
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
#include "spline.h"
#include "sky.h"
#include "thing.h"
#include "particle.h"
#include "dynamiclight.h"
#include "scene.h"

static void skeep_comment(FILE *file) {
    char c;
    while(fread(&c,1,1,file) == 1 && c != '\n');
}

static int skeep_bracket(FILE *file) {
    char c;
    while(fread(&c,1,1,file) == 1) {
        if(c == '{') skeep_bracket(file);
        if(c == '}') return 1;
    }
    return 0;
}

static float load_float(FILE *file) {
    float val;
    fscanf(file,"%f",&val);
    return val;
}

static float load_int(FILE *file) {
    int val;
    fscanf(file,"%d",&val);
    return val;
}

static void load_name(FILE *file,char *path,char *name) {
    char buffer[128];
    fscanf(file,"%s",buffer);
    strcpy(name,path);
    strcat(name,buffer);
}

static void load_pos(FILE *file,float *pos) {
    fscanf(file,"%f %f %f",&pos[0],&pos[1],&pos[2]);
}

static void load_color(FILE *file,float *color) {
    fscanf(file,"%f %f %f %f",&color[0],&color[1],&color[2],&color[3]);
}

static int load_yesno(FILE *file) {
    char buffer[128];
    fscanf(file,"%s",buffer);
    if(!strcmp(buffer,"yes") || !strcmp(buffer,"1")) return 1;
    return 0;
}

/*  load mesh
 *
 */

thing_mesh_t **load_mesh(char *name,char *path,int *num_mesh,int texture_mode) {
    int i;
    FILE *file;
    char buffer[128];
    thing_mesh_config_t config;
    thing_mesh_t **mesh;
    file = fopen(name,"r"); // open config file
    if(!file) return NULL;
    i = 0;
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);   // comment
        else if(!strcmp(buffer,"object")) i++;      // object
        else if(!strcmp(buffer,"{")) {              // braket
            if(!skeep_bracket(file)) return NULL;
        } else return NULL;
    }
    mesh = (thing_mesh_t**)malloc(sizeof(thing_mesh_t*) * i);
    if(!mesh) return NULL;
    i = 0;
    fseek(file,0,SEEK_SET);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);           // comment
        else if(!strcmp(buffer,"object")) {
            fscanf(file,"%s",buffer);                       // skeep '{'
            memset(&config,0,sizeof(thing_mesh_config_t));
            config.texture_mode = texture_mode;
            while(fscanf(file,"%s",buffer) != EOF) {        // object
                if(buffer[0] == '#') skeep_comment(file);   // comment
                else if(!strcmp(buffer,"name")) fscanf(file,"%s",config.name);
                else if(!strcmp(buffer,"mesh")) load_name(file,path,config.meshname);
                else if(!strcmp(buffer,"lightmesh")) load_name(file,path,config.lightname);
                else if(!strcmp(buffer,"base")) load_name(file,path,config.basetexture);
                else if(!strcmp(buffer,"spec")) load_name(file,path,config.speculartexture);
                else if(!strcmp(buffer,"light")) load_name(file,path,config.lighttexture);
                else if(!strcmp(buffer,"ambient")) load_color(file,config.ambient);
                else if(!strcmp(buffer,"diffuse")) load_color(file,config.diffuse);
                else if(!strcmp(buffer,"specular")) load_color(file,config.specular);
                else if(!strcmp(buffer,"shader")) {       // shader
                    fscanf(file,"%s",buffer);
                    if(!strcmp(buffer,"SHADER_DIFFUSE")) config.shader = SHADER_DIFFUSE;
                    else if(!strcmp(buffer,"SHADER_SPECULAR")) config.shader = SHADER_SPECULAR;
                    else if(!strcmp(buffer,"SHADER_GLOSS")) config.shader = SHADER_GLOSS;
                    else if(!strcmp(buffer,"SHADER_TRANSPARENT")) config.shader = SHADER_TRANSPARENT;
                    else if(!strcmp(buffer,"SHADER_LIGHTMAP")) config.shader = SHADER_LIGHTMAP;
                    else return NULL;
                } else if(!strcmp(buffer,"alphatest")) config.alphatest = load_yesno(file);
                else if(!strcmp(buffer,"}")) break;
                else return NULL;
            }
            mesh[i] = thing_mesh_load(&config);
            if(!mesh[i]) return NULL;
            i++;
        } else return NULL;
    }
    fclose(file);
    *num_mesh = i;
    return mesh;
}

/*  load thing
 *
 */

thing_t **load_thing(char *name,thing_mesh_t **mesh,int num_mesh,int *num_thing) {
    int i,j,size;
    FILE *file;
    float pos[3],target;
    char buffer[128],*data;
    thing_t **thing;
    file = fopen(name,"r");
    if(!file) return NULL;
    fseek(file,0,SEEK_END);
    size = ftell(file);
    fseek(file,0,SEEK_SET);
    data = (char*)malloc(sizeof(char) * size);
    fread(data,1,size,file);
    for(j = 0, i = 0; j < size; j++) if(data[j] == ':') i++;
    free(data);
    thing = (thing_t**)malloc(sizeof(thing_t*) * i);
    if(!thing) return NULL;
    i = 0;
    fseek(file,0,SEEK_SET);
    while(fscanf(file,"%s",buffer) != EOF) {
        for(j = 0; j < num_mesh; j++)
            if(!strcmp(buffer,mesh[j]->name)) {
                fscanf(file,"%s",buffer);
                pos[0] = load_float(file);
                pos[1] = load_float(file);
                pos[2] = load_float(file);
                target = load_float(file);
                thing[i] = thing_create(mesh[j],pos,target);
                i++;
                break;
            }
    }
    fclose(file);
    *num_thing = i;
    return thing;
}

/*  load animation
 *
 */

scene_animation_t **load_animation(char *name,char *path,thing_mesh_t **mesh,int num_mesh,int *num_animation) {
    int i,j;
    FILE *file;
    char buffer[128],pathname[256],objectname[32];
    float length = 0;
    thing_mesh_t *newmesh;
    scene_animation_t **animation;
    file = fopen(name,"r"); // open config file
    if(!file) return NULL;
    i = 0;
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"animation")) i++;
        else if(!strcmp(buffer,"{")) {
            if(!skeep_bracket(file)) return NULL;
        } else return NULL;
    }
    animation = (scene_animation_t**)malloc(sizeof(scene_animation_t*) * i);
    if(!animation) return NULL;
    i = 0;
    fseek(file,0,SEEK_SET);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"animation")) {
            animation[i] = (scene_animation_t*)malloc(sizeof(scene_animation_t));
            if(!animation[i]) return 0;
            memset(animation[i],0,sizeof(scene_animation_t));
            fscanf(file,"%s",buffer);
            strcpy(objectname,"");
            while(fscanf(file,"%s",buffer) != EOF) {
                if(buffer[0] == '#') skeep_comment(file);
                else if(!strcmp(buffer,"name")) fscanf(file,"%s",objectname);
                else if(!strcmp(buffer,"start")) animation[i]->start = load_float(file);
                else if(!strcmp(buffer,"end")) animation[i]->end = load_float(file);
                else if(!strcmp(buffer,"time")) animation[i]->time = load_float(file);
                else if(!strcmp(buffer,"pos")) load_pos(file,animation[i]->pos);
                else if(!strcmp(buffer,"target")) animation[i]->target = load_float(file);
                else if(!strcmp(buffer,"onland")) animation[i]->onland = load_yesno(file);
                else if(!strcmp(buffer,"shadow")) animation[i]->shadow = load_float(file);
                else if(!strcmp(buffer,"length")) length = load_float(file);
                else if(!strcmp(buffer,"path")) {
                    load_name(file,path,pathname);
                    if(length <= animation[i]->end - animation[i]->start)
                        animation[i]->path = spline_close_load(pathname,length,0,0,0);
                    else animation[i]->path = spline_load(pathname,length,0,0,0);
                    if(!animation[i]->path) return 0;
                }
                else if(!strcmp(buffer,"}")) break;
                else return NULL;
            }
            for(j = 0; j <= num_mesh; j++) {
                if(j == num_mesh) return 0;
                if(!strcmp(objectname,mesh[j]->name)) {
                    newmesh = thing_mesh_copy(mesh[j]);
                    if(!newmesh) return 0;
                    animation[i]->thing = thing_create(newmesh,animation[i]->pos,0);
                    if(!animation[i]->thing) return 0;
                    break;
                }
            }
            i++;
        } else return NULL;
    }
    fclose(file);
    *num_animation = i;
    return animation;
}

/*  load particle
 *
 */

scene_particle_t **load_particle(char *name,char *path,int *num_particle,int texture_mode) {
    int i;
    FILE *file;
    char buffer[128],pathname[256];
    float length = 0;
    particle_config_t config;
    scene_particle_t **particle;
    file = fopen(name,"r"); // open config file
    if(!file) return NULL;
    i = 0;
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"particle")) i++;
        else if(!strcmp(buffer,"{")) {
            if(!skeep_bracket(file)) return NULL;
        } else return NULL;
    }
    particle = (scene_particle_t**)malloc(sizeof(scene_particle_t*) * i);
    if(!particle) return NULL;
    i = 0;
    fseek(file,0,SEEK_SET);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"particle")) {
            particle[i] = (scene_particle_t*)malloc(sizeof(scene_particle_t));
            if(!particle[i]) return 0;
            memset(particle[i],0,sizeof(scene_particle_t));
            memset(&config,0,sizeof(particle_config_t));
            config.texture_mode = texture_mode;
            fscanf(file,"%s",buffer);
            while(fscanf(file,"%s",buffer) != EOF) {
                if(buffer[0] == '#') skeep_comment(file);
                else if(!strcmp(buffer,"center")) load_pos(file,config.center);
                else if(!strcmp(buffer,"color")) load_color(file,config.color);
                else if(!strcmp(buffer,"num_particle")) config.num_particle = load_int(file);
                else if(!strcmp(buffer,"mass")) config.mass = load_float(file);
                else if(!strcmp(buffer,"speed")) config.speed = load_float(file);
                else if(!strcmp(buffer,"radius")) config.radius = load_float(file);
                else if(!strcmp(buffer,"time")) config.time = load_float(file);
                else if(!strcmp(buffer,"force")) load_pos(file,config.force);
                else if(!strcmp(buffer,"texture")) load_name(file,path,config.texture);
                else if(!strcmp(buffer,"pos")) load_pos(file,particle[i]->pos);
                else if(!strcmp(buffer,"start")) particle[i]->start = load_float(file);
                else if(!strcmp(buffer,"end")) particle[i]->end = load_float(file);
                else if(!strcmp(buffer,"length")) length = load_float(file);
                else if(!strcmp(buffer,"path")) {
                    load_name(file,path,pathname);
                    if(length <= particle[i]->end - particle[i]->start)
                        particle[i]->path = spline_close_load(pathname,length,0,0,0);
                    else particle[i]->path = spline_load(pathname,length,0,0,0);
                    if(!particle[i]->path) return 0;
                }
                else if(!strcmp(buffer,"}")) break;
                else return NULL;
            }
            particle[i]->particle = particle_load(&config);
            if(!particle[i]->particle) return NULL; // radius = (speed + |force| / mass * time) * time
            particle[i]->radius = (particle[i]->particle->maxspeed + VectorLength(particle[i]->particle->force) / particle[i]->particle->mass * particle[i]->particle->time) * particle[i]->particle->time;
            i++;
        } else return NULL;
    }
    fclose(file);
    *num_particle = i;
    return particle;
}

/*  load dynamiclight
 *
 */

scene_dynamiclight_t **load_dynamiclight(char *name,char *path,int *num_dynamiclight,int texture_mode) {
    int i;
    FILE *file;
    char buffer[128],pathname[256];
    float length = 0;
    dynamiclight_config_t config;
    scene_dynamiclight_t **dynamiclight;
    file = fopen(name,"r"); // open config file
    if(!file) return NULL;
    i = 0;
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"dynamiclight")) i++;
        else if(!strcmp(buffer,"{")) {
            if(!skeep_bracket(file)) return NULL;
        } else return NULL;
    }
    dynamiclight = (scene_dynamiclight_t**)malloc(sizeof(scene_dynamiclight_t*) * i);
    if(!dynamiclight) return NULL;
    i = 0;
    fseek(file,0,SEEK_SET);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"dynamiclight")) {
            dynamiclight[i] = (scene_dynamiclight_t*)malloc(sizeof(scene_dynamiclight_t));
            if(!dynamiclight[i]) return 0;
            memset(dynamiclight[i],0,sizeof(scene_dynamiclight_t));
            memset(&config,0,sizeof(dynamiclight_config_t));
            config.texture_mode = texture_mode;
            fscanf(file,"%s",buffer);
            while(fscanf(file,"%s",buffer) != EOF) {
                if(buffer[0] == '#') skeep_comment(file);
                else if(!strcmp(buffer,"center")) load_pos(file,config.center);
                else if(!strcmp(buffer,"color")) load_color(file,config.color);
                else if(!strcmp(buffer,"flareradius")) config.flareradius = load_float(file);
                else if(!strcmp(buffer,"lightradius")) config.lightradius = load_float(file);
                else if(!strcmp(buffer,"flaretexture")) load_name(file,path,config.flaretexture);
                else if(!strcmp(buffer,"lighttexture")) load_name(file,path,config.lighttexture);
                else if(!strcmp(buffer,"opengl")) dynamiclight[i]->opengl = load_yesno(file);
                else if(!strcmp(buffer,"pos")) load_pos(file,dynamiclight[i]->pos);
                else if(!strcmp(buffer,"start")) dynamiclight[i]->start = load_float(file);
                else if(!strcmp(buffer,"end")) dynamiclight[i]->end = load_float(file);
                else if(!strcmp(buffer,"length")) length = load_float(file);
                else if(!strcmp(buffer,"path")) {
                    load_name(file,path,pathname);
                    if(length <= dynamiclight[i]->end - dynamiclight[i]->start)
                        dynamiclight[i]->path = spline_close_load(pathname,length,0,0,0);
                    else dynamiclight[i]->path = spline_load(pathname,length,0,0,0);
                    if(!dynamiclight[i]->path) return 0;
                }
                else if(!strcmp(buffer,"}")) break;
                else return NULL;
            }
            dynamiclight[i]->light = dynamiclight_load(&config);
            if(!dynamiclight[i]->light) return NULL;
            dynamiclight[i]->radius = dynamiclight[i]->light->flareradius;
            if(dynamiclight[i]->light->flareradius < dynamiclight[i]->light->lightradius)
                dynamiclight[i]->radius = dynamiclight[i]->light->lightradius;
            i++;
        } else return NULL;
    }
    fclose(file);
    *num_dynamiclight = i;
    return dynamiclight;
}

/*  load land
 *
 */

static int load_land(scene_t *scene,FILE *file,char *path,int texture_mode) {
    char buffer[128];
    land_config_t config;
    memset(&config,0,sizeof(land_config_t));
    config.num_base = 1;
    config.num_detail = 1;
    config.texture_mode = texture_mode;
    fscanf(file,"%s",buffer);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"land")) load_name(file,path,config.heightmap);
        else if(!strcmp(buffer,"step")) config.step = load_float(file);
        else if(!strcmp(buffer,"altitude")) config.altitude = load_float(file);
        else if(!strcmp(buffer,"lod")) config.lod = load_float(file);
        else if(!strcmp(buffer,"ambient")) load_color(file,config.ambient);
        else if(!strcmp(buffer,"diffuse")) load_color(file,config.diffuse);
        else if(!strcmp(buffer,"specular")) load_color(file,config.specular);
        else if(!strcmp(buffer,"base")) load_name(file,path,config.base);
        else if(!strcmp(buffer,"num_base")) config.num_base = load_int(file);
        else if(!strcmp(buffer,"detail")) load_name(file,path,config.detail);
        else if(!strcmp(buffer,"num_detail")) config.num_detail = load_int(file);
        else if(!strcmp(buffer,"}")) {
            scene->land = land_create(&config);
            break;
        } else return 0;
    }
    if(!scene->land) return 0;
    return 1;
}

/*  load camera
 *
 */

static int load_camera(scene_t *scene,FILE *file,char *path) {
    char buffer[128],pathname[256];
    float length = 0;
    camera_config_t config;
    memset(&config,0,sizeof(camera_config_t));
    config.aspect = 4.0 / 3.0;
    fscanf(file,"%s",buffer);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"start")) scene->start = load_float(file);
        else if(!strcmp(buffer,"end")) scene->end = load_float(file);
        else if(!strcmp(buffer,"length")) length = load_float(file);
        else if(!strcmp(buffer,"pathpos")) {
            load_name(file,path,pathname);
            if(length <= scene->end - scene->start) scene->pathpos = spline_close_load(pathname,length,0,0,0);
            else scene->pathpos = spline_load(pathname,length,0,0,0);
            if(!scene->pathpos) return 0;
        } else if(!strcmp(buffer,"pathdir")) {
            load_name(file,path,pathname);
            if(length <= scene->end - scene->start) scene->pathdir = spline_close_load(pathname,length,0,0,0);
            else scene->pathdir = spline_load(pathname,length,0,0,0);
            if(!scene->pathdir) return 0;
        } else if(!strcmp(buffer,"fov")) config.fov = load_float(file);
        else if(!strcmp(buffer,"aspect")) config.aspect = load_float(file);
        else if(!strcmp(buffer,"clipnear")) config.clipnear = load_float(file);
        else if(!strcmp(buffer,"clipfar")) config.clipfar = load_float(file);
        else if(!strcmp(buffer,"fogcolor")) load_color(file,config.fogcolor);
        else if(!strcmp(buffer,"fogstart")) config.fogstart = load_float(file);
        else if(!strcmp(buffer,"fogend")) config.fogend = load_float(file);
        else if(!strcmp(buffer,"}")) {
            scene->camera = camera_create(&config);
            break;
        } else return 0;
    }
    if(!scene->camera) return 0;
    return 1;
}

/*  load sky and sun
 *
 */

static int load_sky_sun_flare(sky_sun_config_t *config,FILE *file,char *path) {
    char buffer[128];
    int flare = 0;
    fscanf(file,"%s",buffer);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"flare")) flare = load_int(file);
        else if(!strcmp(buffer,"position")) config->position[flare] = load_float(file);
        else if(!strcmp(buffer,"radius")) config->radius[flare] = load_float(file);
        else if(!strcmp(buffer,"opacity")) config->opacity[flare] = load_float(file);
        else if(!strcmp(buffer,"texture")) load_name(file,path,config->texture[flare]);
        else if(!strcmp(buffer,"}")) break;
        else return 0;
    }
    return 1;
}

static int load_sky_layer(sky_config_t *config,FILE *file,char *path) {
    char buffer[128];
    int layer = 0;
    fscanf(file,"%s",buffer);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"layer")) layer = load_int(file);
        else if(!strcmp(buffer,"height")) config->height[layer] = load_float(file);
        else if(!strcmp(buffer,"time")) config->time[layer] = load_float(file);
        else if(!strcmp(buffer,"texture")) load_name(file,path,config->texture[layer]);
        else if(!strcmp(buffer,"}")) break;
        else return 0;
    }
    return 1;
}

static int load_sky_sun(scene_t *scene,FILE *file,char *path,int texture_mode) {
    char buffer[128],pathname[256];
    float length = 0;
    sky_sun_config_t config;
    memset(&config,0,sizeof(sky_sun_config_t));
    config.texture_mode = texture_mode;
    fscanf(file,"%s",buffer);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"pos")) load_pos(file,config.pos);
        else if(!strcmp(buffer,"color")) load_color(file,config.color);
        else if(!strcmp(buffer,"num_flare")) config.num_flare = load_int(file);
        else if(!strcmp(buffer,"flare")) load_sky_sun_flare(&config,file,path);
        else if(!strcmp(buffer,"length")) length = load_float(file);
        else if(!strcmp(buffer,"path")) {
            load_name(file,path,pathname);
            if(length <= scene->end - scene->start)
                scene->pathsun = spline_close_load(pathname,length,0,0,0);
            else scene->pathsun = spline_load(pathname,length,0,0,0);
            if(!scene->pathsun) return 0;
        } else if(!strcmp(buffer,"}")) {
            scene->sun = sky_sun_load(&config);
            break;
        } else return 0;
    }
    if(!scene->sun) return 0;
    return 1;
}

static int load_sky(scene_t *scene,FILE *file,char *path,int texture_mode) {
    char buffer[128];
    sky_config_t config;
    memset(&config,0,sizeof(sky_config_t));
    config.texture_mode = texture_mode;
    fscanf(file,"%s",buffer);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"mesh")) load_name(file,path,config.mesh);
        else if(!strcmp(buffer,"num_layer")) config.num_layer = load_int(file);
        else if(!strcmp(buffer,"target")) config.target = load_float(file);
        else if(!strcmp(buffer,"layer")) load_sky_layer(&config,file,path);
        else if(!strcmp(buffer,"sun")) {
            if(!load_sky_sun(scene,file,path,texture_mode)) return 0;
        } else if(!strcmp(buffer,"}")) {
            scene->sky = sky_load(&config);
            break;
        } else return 0;
    }
    if(!scene->sky) return 0;
    return 1;
}

static int load_object(scene_t *scene,FILE *file,char *path,int texture_mode) {
    char buffer[128],name[256];
    fscanf(file,"%s",buffer);
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"mesh")) {
            load_name(file,path,name);
            scene->mesh = load_mesh(name,path,&scene->num_mesh,texture_mode);
            if(!scene->mesh) return 0;
        } else if(!strcmp(buffer,"thing")) {
            load_name(file,path,name);
            scene->thing = load_thing(name,scene->mesh,scene->num_mesh,&scene->num_thing);
            if(!scene->thing) return 0;
        } else if(!strcmp(buffer,"animation")) {
            load_name(file,path,name);
            scene->animation = load_animation(name,path,scene->mesh,scene->num_mesh,&scene->num_animation);
            if(!scene->animation) return 0;
        } else if(!strcmp(buffer,"particle")) {
            load_name(file,path,name);
            scene->particle = load_particle(name,path,&scene->num_particle,texture_mode);
            if(!scene->particle) return 0;
        } else if(!strcmp(buffer,"dynamiclight")) {
            load_name(file,path,name);
            scene->dynamiclight = load_dynamiclight(name,path,&scene->num_dynamiclight,texture_mode);
            if(!scene->dynamiclight) return 0;
        } else if(!strcmp(buffer,"}")) break;
        else return 0;
    }
    return 1;
}

/*  scene load
 *
 */

scene_t *scene_load(char *name,char *path,int texture_mode) {
    FILE *file;
    char buffer[128];
    scene_t *scene;
    file = fopen(name,"r");
    if(!file) return NULL;
    scene = (scene_t*)malloc(sizeof(scene_t));
    memset(scene,0,sizeof(scene_t));
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"land")) {       // load land
            if(!load_land(scene,file,path,texture_mode)) return NULL;
        } else if(!strcmp(buffer,"camera")) {   // load camera
            if(!load_camera(scene,file,path)) return NULL;
        } else if(!strcmp(buffer,"sky")) {      // load sky
            if(!load_sky(scene,file,path,texture_mode)) return NULL;
        } else if(!strcmp(buffer,"object")) {   // load object
            if(!load_object(scene,file,path,texture_mode)) return NULL;
        } else return NULL;
    }
    fclose(file);
    return scene;
}
