/*  thing
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __THING_H__
#define __THING_H__

#define SHADER_DIFFUSE      0
#define SHADER_SPECULAR     1
#define SHADER_GLOSS        2
#define SHADER_TRANSPARENT  3
#define SHADER_LIGHTMAP     4

typedef struct {
    char name[32];              // object name
    char meshname[256];         // mesh name *.3ds or *.ime
    char lightname[256];        // lightmap mesh name *.3ds
    float ambient[4];           // ambient color
    float diffuse[4];           // diffuse color
    float specular[4];          // specular color
    char basetexture[256];      // base texture
    char speculartexture[256];  // specular texture
    char lighttexture[256];     // lightmap texture
    int shader;                 // shader
    int alphatest;              // alpha test
    int texture_mode;           // texture mode
} thing_mesh_config_t;

typedef struct {
    char name[32];          // object name
    float *vertex;          // vertex
    float *lightvertex;     // vertex for lightmap
    int num_vertex;         // num vertex
    float **frame;          // interpolated mesh
    int num_frame;          // num interpolated mesh
    int interpolate;        // interpolate flag
    float center[3];        // center object
    float radius;           // radius object concerning center
    float min[3];           // max coordinate
    float max[3];           // min coordinate
    int material;           // material id
    int shader;             // shader
    int alphatest;          // alphatest
    int base;               // base texture id
    int specular;           // specular texture id
    int light;              // lightmap texture id
} thing_mesh_t;

typedef struct {
    float pos[3];           // position object
    float target;           // target object
    float center[3];        // center object
    float radius;           // radius
    float min[3];           // size object
    float max[3];
    float frame;            // interpolated mesh frame
    int shadow;             // shadow texture id
    thing_mesh_t *mesh;     // mesh object
} thing_t;

thing_mesh_t *thing_mesh_load(thing_mesh_config_t *config);
thing_mesh_t *thing_mesh_copy(thing_mesh_t *oldmesh);
void thing_mesh_free(thing_mesh_t *mesh);
void thing_mesh_create(thing_mesh_t *mesh,float frame);
void thing_mesh_render(thing_mesh_t *mesh);
void thing_texture_mesh_render(thing_mesh_t *mesh);
thing_t *thing_create(thing_mesh_t *mesh,float *pos,float target);
void thing_free(thing_t *thing);
void thing_move(thing_t *thing,float *pos,float target);
void thing_render(thing_t *thing);

#endif /* __THING_H__ */
