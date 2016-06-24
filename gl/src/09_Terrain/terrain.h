#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "loadtga.h"
#include "mathlib.h"
#include "camera.h"

#define TERRAIN_MAX_TEXTURE	64

typedef struct {
	int id[15];
} terrain_texture_t;

typedef struct {
	float xyz[3];
	float normal[3];
	float st[2];
	int texture;
} terrain_vertex_t;

typedef struct terrain_node_s {
	struct terrain_node_s *left,*right;
	int width,height;
	terrain_vertex_t **vertex;
	float plane[4];
	float min[3],max[3];
} terrain_node_t;

typedef struct {
	float step;
	int width,height;
	terrain_vertex_t *vertex;
	terrain_node_t *root;
	terrain_texture_t texture[TERRAIN_MAX_TEXTURE];
	int shadow_id;
} terrain_t;

terrain_t *terrain_load_tga(char *name,float step,float height);
int terrain_load_texture(terrain_t *t,char *name,int n);
void terrain_render(terrain_t *t,camera_t *c);
float terrain_height(terrain_t *t,float *p);
int terrain_normal(terrain_t *t,float *p,float *n);
int terrain_boundary(terrain_t *t,float *p);
int terrain_cross_line(terrain_t *t,float *l0,float *l1);
unsigned char *terrain_light(terrain_t *t,float *light,float ambient,
	int width,int height);

#endif /* __TERRAIN_H__ */
