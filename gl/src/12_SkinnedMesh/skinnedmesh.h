#ifndef __SKINNEDMESH_H__
#define __SKINNEDMESH_H__

#define SM_IDENT		('s' | ('m' << 8) | ('0' << 16) | ('1' << 24))
#define SM_NAME_LENGTH	64
#define SM_MAX_SURFACES	64

typedef struct {
	char name[SM_NAME_LENGTH];
	float matrix[16];
} sm_bone_t;

typedef struct {
	float xyz[3];
	float rot[4];
} sm_frame_t;

typedef struct {
	int bone;
	float weight;
	float xyz[3];
	float normal[3];
} sm_weight_t;

typedef struct {
	float xyz[3];
	float normal[3];
	float st[2];
	int num_weight;
	sm_weight_t *weight;
} sm_vertex_t;

typedef struct {
	int v0,v1,v2;
} sm_face_t;

typedef struct {
	char name[SM_NAME_LENGTH];
	int num_vertex;
	sm_vertex_t *vertex;
	int num_face;
	sm_face_t *face;
} sm_surface_t;

typedef struct {
	int num_bone;
	sm_bone_t *bone;
	int num_frame;
	sm_frame_t **frame;
	int num_surface;
	sm_surface_t *surface[SM_MAX_SURFACES];
} sm_t;

typedef struct {
	char name[SM_NAME_LENGTH];
} sm_file_bone_t;

typedef struct {
	float xyz[3];
	float rot[4];
} sm_file_frame_t;

typedef struct {
	int bone;
	float weight;
	float xyz[3];
	float normal[3];
} sm_file_weight_t;

typedef struct {
	float st[2];
	int num_weight;
} sm_file_vertex_t;

typedef struct {
	int v0,v1,v2;
} sm_file_face_t;

typedef struct {
	char name[SM_NAME_LENGTH];
	int num_vertex;
	int num_face;
} sm_file_surface_t;

typedef struct {
	int ident;
	int num_bone;
	int num_frame;
	int num_surface;
} sm_file_header_t;

sm_t *sm_load_ascii(char *name);
sm_t *sm_load(char *name);
int sm_save(char *name,sm_t *sm);
void sm_frame(sm_t *sm,int from,int to,float frame);
int sm_bone(sm_t *sm,char *name);
void sm_bone_transform(sm_t *sm,int bone,float *matrix);
int sm_surface(sm_t *sm,char *name);
void sm_render_surface(sm_t *sm,int surface);
void sm_render(sm_t *sm);

#endif /* __SKINNEDMESH_H__ */
