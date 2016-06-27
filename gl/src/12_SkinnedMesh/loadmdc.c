/*	load mdc (rtcw) file
 *
 *		written by Alexander Zaprjagaev
 *		frustum@public.tsu.ru
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <malloc.h>

#define MDC_IDENT	(('C' << 24) + ('P' << 16) + ('D' << 8) + 'I')
#define MDC_VERSION	2

typedef struct {
	int ident;
	char name[64];
	int flags;
	int num_comp_frames;
	int num_base_frames;
	int num_shaders;
	int num_verts;
	int num_triangles;
	int ofs_triangles;
	int ofs_shaders;
	int ofs_st;
	int ofs_base_verts;
	int ofs_comp_verts;
	int ofs_frame_base_frames;
	int ofs_frame_comp_frames;
	int ofs_end;
} mdc_surface_t;

typedef struct {
	int indexes[3];
} mdc_triangle_t;

typedef struct {
	float st[2];
} mdc_st_t;

typedef struct {
	short xyz[3];
	short normal;
} mdc_base_verts_t;

typedef struct {
	unsigned char xyz[3];
	unsigned char normal;
} mdc_comp_verts_t;

typedef struct {
	int ident;
	int version;
	char name[64];
	int flags;
	int num_frames;
	int num_tags;
	int num_surfaces;
	int num_skin;
	int ofs_border_frames;
	int ofs_tag_names;
	int ofs_tag_frames;
	int ofs_surfaces;
	int ofs_end;
} mdc_header_t;

static int load_surface(FILE *file,int ofs_surface,
	float **vertex,int *num_vertex) {
	int i;
	mdc_surface_t surface;
	mdc_triangle_t *triangle;
	mdc_st_t *st;
	mdc_base_verts_t *verts;
	fseek(file,ofs_surface,SEEK_SET);
	fread(&surface,1,sizeof(mdc_surface_t),file);
	triangle = (mdc_triangle_t*)malloc(sizeof(mdc_triangle_t) *
		surface.num_triangles);
	st = (mdc_st_t*)malloc(sizeof(mdc_st_t) *
		surface.num_verts);
	verts = (mdc_base_verts_t*)malloc(sizeof(mdc_base_verts_t) *
		surface.num_verts);
	fseek(file,ofs_surface + surface.ofs_triangles,SEEK_SET);
	fread(triangle,surface.num_triangles,sizeof(mdc_triangle_t),file);
	fseek(file,ofs_surface + surface.ofs_st,SEEK_SET);
	fread(st,surface.num_verts,sizeof(mdc_st_t),file);
	fseek(file,ofs_surface + surface.ofs_base_verts,SEEK_SET);
	fread(verts,surface.num_verts,sizeof(mdc_base_verts_t),file);
	*vertex = (float*)malloc(sizeof(float) * surface.num_triangles * 24);
	for(i = 0; i < surface.num_triangles; i++) {
		int j;
		for(j = 0; j < 3; j++) {
			int k = i * 24 + j * 8;
			int l = triangle[i].indexes[j];
			float alpha = 2.0 * M_PI / 255.0 * (verts[l].normal & 0xff);
			float beta = 2.0 * M_PI / 255.0 * (verts[l].normal >> 8);
			(*vertex)[k + 0] = verts[l].xyz[0] / 64.0;
			(*vertex)[k + 1] = verts[l].xyz[1] / 64.0;
			(*vertex)[k + 2] = verts[l].xyz[2] / 64.0;
			(*vertex)[k + 3] = cos(beta) * sin(alpha);
			(*vertex)[k + 4] = sin(beta) * sin(alpha);
			(*vertex)[k + 5] = cos(alpha);
			(*vertex)[k + 6] = st[l].st[0];
			(*vertex)[k + 7] = st[l].st[1];
		}
	}
	free(triangle);
	free(st);
	free(verts);
	*num_vertex = surface.num_triangles * 3;
	return surface.ofs_end;
}

static float *load_header(FILE *file,int *num_vertex) {
	int i;
	float **surface,*vertex,*ptr;
	int *surface_num_vertex;
	mdc_header_t header;
	fread(&header,1,sizeof(mdc_header_t),file);
	if(header.ident != MDC_IDENT) {
		fprintf(stderr,"wrong ident of mdc file\n");
		return NULL;
	}
	if(header.version != MDC_VERSION) {
		fprintf(stderr,"wrong version of mdc file\n");
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

float *load_mdc(char *name,int *num_vertex) {
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
