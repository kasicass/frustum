/*  landscape 3d engine
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __LAND_H__
#define __LAND_H__

#define LAND_NODE_SIZE 17
#define LAND_LOD0_NUM_INDICES 1536
#define LAND_LOD1_NUM_INDICES 504
#define LAND_CROSS_TRYES 16

#include "camera.h"
#include "thing.h"

extern unsigned short indices_lod0[LAND_LOD0_NUM_INDICES];
extern unsigned short indices_lod1[LAND_LOD1_NUM_INDICES];

typedef struct {
    float v[3];     // vertex
} land_vertex_t;

typedef struct {
    float v[3];     // vertex
    float n[3];     // normal
    float t0[2];    // base texture coordinate
    float t1[2];    // detail texture coordinate
} land_node_vertex_t;

typedef struct land_node_s {
    struct land_node_s *left,*right;    // child nodes
    land_node_vertex_t *vertex;         // vertex
    int width,height;                   // width and height node
    unsigned short *indices[2];         // indices for lods
    int num_indices[2];                 // num indices for lods
    float lod;                          // lod distance
    int base;                           // id for base texture
    int detail;                         // id for detail texture
    float plane[4];                     // div plane
    float min[3],max[3];                // size this node
} land_node_t;

typedef struct {
    land_vertex_t *vertex;  // vertex
    int width,height;       // width and height all mesh
    float step;             // mesh step
    float lod;              // lod distance
    int material;           // material list
    int *texture;           // all texture (texture[0] - detail texture)
    int num_texture;        // num texture
    land_node_t *root;      // root node
} land_t;

typedef struct {
    char heightmap[256];    // heightmap image
    float step;             // mesh ster
    float altitude;         // mesh altitude
    float lod;              // lod distance
    float ambient[4];       // ambient
    float diffuse[4];       // diffuse
    float specular[4];      // specular
    char base[256];         // base image
    int num_base;           // num sub images for base image
    char detail[256];       // detail image
    int num_detail;         // num detail
    int texture_mode;       // texture filter
} land_config_t;

land_t *land_create(land_config_t *config);
void land_free(land_t *land);
float land_height(land_t *land,float *point);
void land_crossline(land_t *land,float *line_start,float *line_end,float *point);
void land_render(land_t *land,camera_t *camera);
void land_dynamiclight(land_t *land,float *pos,float *color,float radius);
void land_dynamicshadow(land_t *land,float *light,thing_t *thing);

#endif /* __LAND_H__ */
