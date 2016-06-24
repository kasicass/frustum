#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <malloc.h>

#define MD3_IDENT	(('3' << 24) + ('P' << 16) + ('D' << 8) + 'I')
#define MD3_VERSION	15

typedef struct {
	int ident;
	char name[64];
	int flags;
	int num_frames;
	int num_shaders;
	int num_verts;
	int num_triangles;
	int ofs_triangles;
	int ofs_shaders;
	int ofs_st;
	int ofs_xyz_normals;
	int ofs_end;
} md3_surface_t;

typedef struct {
	int indexes[3];
} md3_triangle_t;

typedef struct {
	float st[2];
} md3_st_t;

typedef struct {
	short xyz[3];
	short normal;
} md3_xyz_normal_t;

typedef struct {
	int ident;
	int version;
	char name[64];
	int flags;
	int num_frame;
	int num_tags;
	int num_surfaces;
	int num_skin;
	int ofs_frames;
	int ofs_tags;
	int ofs_surfaces;
	int ofs_end;
} md3_header_t;

static int load_surface(FILE *file,int ofs_surface,
	float **vertex,int *num_vertex) {
	int i;
	md3_surface_t surface;
	md3_triangle_t *triangle;
	md3_st_t *st;
	md3_xyz_normal_t *xyz_normal;
	fseek(file,ofs_surface,SEEK_SET);
	fread(&surface,1,sizeof(md3_surface_t),file);
	triangle = (md3_triangle_t*)malloc(sizeof(md3_triangle_t) *
		surface.num_triangles);
	st = (md3_st_t*)malloc(sizeof(md3_st_t) *
		surface.num_verts);
	xyz_normal = (md3_xyz_normal_t*)malloc(sizeof(md3_xyz_normal_t) *
		surface.num_verts);
	fseek(file,ofs_surface + surface.ofs_triangles,SEEK_SET);
	fread(triangle,surface.num_triangles,sizeof(md3_triangle_t),file);
	fseek(file,ofs_surface + surface.ofs_st,SEEK_SET);
	fread(st,surface.num_verts,sizeof(md3_st_t),file);
	fseek(file,ofs_surface + surface.ofs_xyz_normals,SEEK_SET);
	fread(xyz_normal,surface.num_verts,sizeof(md3_xyz_normal_t),file);
	*vertex = (float*)malloc(sizeof(float) * surface.num_triangles * 24);
	for(i = 0; i < surface.num_triangles; i++) {
		int j;
		for(j = 0; j < 3; j++) {
			int k = i * 24 + j * 8;
			int l = triangle[i].indexes[j];
			float alpha = 2.0 * M_PI / 255.0 * (xyz_normal[l].normal & 0xff);
			float beta = 2.0 * M_PI / 255.0 * (xyz_normal[l].normal >> 8);
			(*vertex)[k + 1] = xyz_normal[l].xyz[0] / 64.0;
			(*vertex)[k + 0] = xyz_normal[l].xyz[1] / 64.0;
			(*vertex)[k + 2] = xyz_normal[l].xyz[2] / 64.0;
			(*vertex)[k + 4] = cos(beta) * sin(alpha);
			(*vertex)[k + 3] = sin(beta) * sin(alpha);
			(*vertex)[k + 5] = cos(alpha);
			(*vertex)[k + 6] = st[l].st[0];
			(*vertex)[k + 7] = st[l].st[1];
		}
	}
	free(triangle);
	free(st);
	free(xyz_normal);
	*num_vertex = surface.num_triangles * 3;
	return surface.ofs_end;
}

static float *load_header(FILE *file,int *num_vertex) {
	int i;
	float **surface,*vertex,*ptr;
	int *surface_num_vertex;
	md3_header_t header;
	fread(&header,1,sizeof(md3_header_t),file);
	if(header.ident != MD3_IDENT) {
		fprintf(stderr,"wrong ident of md3 file\n");
		return NULL;
	}
	if(header.version != MD3_VERSION) {
		fprintf(stderr,"wrong version of md3 file\n");
		return NULL;
	}
	surface = (float**)malloc(sizeof(float*) * header.num_surfaces);
	surface_num_vertex = (int*)malloc(sizeof(int) * header.num_surfaces);
	for(i = 0, *num_vertex = 0; i < header.num_surfaces; i++) {
		header.ofs_surfaces += load_surface(file,header.ofs_surfaces,
			&surface[i],&surface_num_vertex[i]);
		*num_vertex += surface_num_vertex[i];
	}
	vertex = (float*)malloc(sizeof(float) * *num_vertex * 8);
	for(i = 0, ptr = vertex; i < header.num_surfaces; i++) {
		memcpy(ptr,surface[i],sizeof(float) * surface_num_vertex[i] * 8);
		ptr += surface_num_vertex[i] * 8;
		free(surface[i]);
	}
	free(surface);
	free(surface_num_vertex);
	return vertex;
}

float *LoadMD3(char *name,int *num_vertex) {
	FILE *file;
	float *mesh;
	file = fopen(name,"rb");
	if(!file) {
		fprintf(stderr,"error open %s file\n",name);
		return NULL;
	}
	mesh = load_header(file,num_vertex);
	fclose(file);
	return mesh;
}