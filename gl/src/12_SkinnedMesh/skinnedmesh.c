#include <stdio.h>
#include <malloc.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "mathlib.h"
#include "skinnedmesh.h"

/*
 */
static void sm_read_name(FILE *file,char *name) {
	int i = -1;
	char c;
	while(fread(&c,1,1,file) == 1) {
		if(i == -1) {
			if(c == '"') i = 0;
			else if(c != ' ' && c != '\t' && c != '\n' && c != '\r') {
				*name++ = c;
				i = 1;
			}
			continue;
		}
		if(i == 0 && c == '"') break;
		if(i == 1 && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) break;
		*name++ = c;
	}
	*name = '\0';
}

/*
 */
static sm_surface_t *sm_load_surface_ascii(FILE *file) {
	char buf[256];
	sm_surface_t *surface;
	surface = calloc(1,sizeof(sm_surface_t));
	sm_read_name(file,surface->name);
	fscanf(file,"%s",buf);
	while(fscanf(file,"%s",buf) != EOF) {
		if(!strcmp(buf,"vertex")) {
			int i;
			fscanf(file,"%d %s",&surface->num_vertex,buf);
			surface->vertex = malloc(sizeof(sm_vertex_t) * surface->num_vertex);
			for(i = 0; i < surface->num_vertex; i++) {
				int j;
				sm_vertex_t *v = &surface->vertex[i];
				fscanf(file,"%f %f",&v->st[0],&v->st[1]);
				fscanf(file,"%s %d",buf,&v->num_weight);
				v->weight = malloc(sizeof(sm_weight_t) * v->num_weight);
				fscanf(file,"%s",buf);
				for(j = 0; j < v->num_weight; j++) {
					sm_weight_t *w = &v->weight[j];
					fscanf(file,"%d %f",&w->bone,&w->weight);
					fscanf(file,"%f %f %f",
						&w->xyz[0],&w->xyz[1],&w->xyz[2]);
					fscanf(file,"%f %f %f",
						&w->normal[0],&w->normal[1],&w->normal[2]);
				}
				fscanf(file,"%s",buf);
			}
			fscanf(file,"%s",buf);
		} else if(!strcmp(buf,"face")) {
			int i;
			fscanf(file,"%d %s",&surface->num_face,buf);
			surface->face = malloc(sizeof(sm_face_t) * surface->num_face);
			for(i = 0; i < surface->num_face; i++) {
				sm_face_t *f = &surface->face[i];
				fscanf(file,"%d %d %d",&f->v0,&f->v1,&f->v2);
			}
			fscanf(file,"%s",buf);
		} else if(!strcmp(buf,"}")) {
			return surface;
		} else {
			fprintf(stderr,"unknown string: \"%s\"\n",buf);
			return NULL;
		}
	}
	return NULL;
}

/*
 */
sm_t *sm_load_ascii(char *name) {
	char buf[256];
	FILE *file;
	sm_t *sm;
	file = fopen(name,"rb");
	if(!file) {
		fprintf(stderr,"error open \"%s\" file\n",name);
		return NULL;
	}
	sm = calloc(1,sizeof(sm_t));
	while(fscanf(file,"%s",buf) != EOF) {
		if(!strcmp(buf,"bones")) {
			int i;
			fscanf(file,"%d %s",&sm->num_bone,buf);
			sm->bone = malloc(sizeof(sm_bone_t) * sm->num_bone);
			for(i = 0; i < sm->num_bone; i++) {
				sm_read_name(file,sm->bone[i].name);
			}
			fscanf(file,"%s",buf);
		} else if(!strcmp(buf,"surface")) {
			sm->surface[sm->num_surface++] = sm_load_surface_ascii(file);
		} else if(!strcmp(buf,"animation")) {
			int i;
			fscanf(file,"%d %s",&sm->num_frame,buf);
			sm->frame = malloc(sizeof(sm_frame_t*) * sm->num_frame);
			for(i = 0; i < sm->num_frame; i++) {
				int j;
				sm->frame[i] = malloc(sizeof(sm_frame_t) * sm->num_bone);
				for(j = 0; j < sm->num_bone; j++) {
					sm_frame_t *f = &sm->frame[i][j];
					fscanf(file,"%f %f %f",
						&f->xyz[0],&f->xyz[1],&f->xyz[2]);
					fscanf(file,"%f %f %f %f",
						&f->rot[0],&f->rot[1],&f->rot[2],&f->rot[3]);
				}
			}
			fscanf(file,"%s",buf);
		} else {
			fprintf(stderr,"unknown string: \"%s\"\n",buf);
			return NULL;
		}
	}
	fclose(file);
	return sm;
}

/*
 */
sm_t *sm_load(char *name) {
	int i;
	FILE *file;
	sm_file_header_t header;
	sm_t *sm;
	sm = calloc(1,sizeof(sm_t));
	file = fopen(name,"rb");
	if(!file) {
		fprintf(stderr,"error open \"%s\" file\n",name);
		return NULL;
	}
	fread(&header,sizeof(sm_file_header_t),1,file);
	if(header.ident != SM_IDENT) {
		fprintf(stderr,"wrong vesion skinned mesh file\n");
		fclose(file);
		return NULL;
	}
	sm->num_bone = header.num_bone;
	sm->bone = malloc(sizeof(sm_bone_t) * sm->num_bone);
	for(i = 0; i < header.num_bone; i++) {
		sm_file_bone_t bone;
		fread(&bone,sizeof(sm_file_bone_t),1,file);
		strcpy(sm->bone[i].name,bone.name);
	}
	sm->num_frame = header.num_frame;
	sm->frame = malloc(sizeof(sm_frame_t*) * sm->num_frame);
	for(i = 0; i < header.num_frame; i++) {
		int j;
		sm->frame[i] = malloc(sizeof(sm_frame_t) * sm->num_bone);
		for(j = 0; j < sm->num_bone; j++) {
			sm_file_frame_t frame;
			fread(&frame,sizeof(sm_file_frame_t),1,file);
			sm->frame[i][j].xyz[0] = frame.xyz[0];
			sm->frame[i][j].xyz[1] = frame.xyz[1];
			sm->frame[i][j].xyz[2] = frame.xyz[2];
			sm->frame[i][j].rot[0] = frame.rot[0];
			sm->frame[i][j].rot[1] = frame.rot[1];
			sm->frame[i][j].rot[2] = frame.rot[2];
			sm->frame[i][j].rot[3] = frame.rot[3];
		}
	}
	sm->num_surface = header.num_surface;
	for(i = 0; i < header.num_surface; i++) {
		int j;
		sm_file_surface_t surface;
		sm->surface[i] = malloc(sizeof(sm_surface_t));
		fread(&surface,sizeof(sm_surface_t),1,file);
		strcpy(sm->surface[i]->name,surface.name);
		sm->surface[i]->num_vertex = surface.num_vertex;
		sm->surface[i]->vertex = malloc(sizeof(sm_vertex_t) *
			surface.num_vertex);
		sm->surface[i]->num_face = surface.num_face;
		sm->surface[i]->face = malloc(sizeof(sm_face_t) *
			surface.num_face);
		for(j = 0; j < surface.num_vertex; j++) {
			int k;
			sm_file_vertex_t vertex;
			fread(&vertex,sizeof(sm_vertex_t),1,file);
			sm->surface[i]->vertex[j].st[0] = vertex.st[0];
			sm->surface[i]->vertex[j].st[1] = vertex.st[1];
			sm->surface[i]->vertex[j].num_weight = vertex.num_weight;
			sm->surface[i]->vertex[j].weight = malloc(sizeof(sm_weight_t) *
				vertex.num_weight);
			for(k = 0; k < vertex.num_weight; k++) {
				sm_file_weight_t weight;
				fread(&weight,sizeof(sm_file_weight_t),1,file);
				sm->surface[i]->vertex[j].weight[k].bone = weight.bone;
				sm->surface[i]->vertex[j].weight[k].weight = weight.weight;
				sm->surface[i]->vertex[j].weight[k].xyz[0] = weight.xyz[0];
				sm->surface[i]->vertex[j].weight[k].xyz[1] = weight.xyz[1];
				sm->surface[i]->vertex[j].weight[k].xyz[2] = weight.xyz[2];
				sm->surface[i]->vertex[j].weight[k].normal[0]=weight.normal[0];
				sm->surface[i]->vertex[j].weight[k].normal[1]=weight.normal[1];
				sm->surface[i]->vertex[j].weight[k].normal[2]=weight.normal[2];
			}
		}
		for(j = 0; j < surface.num_face; j++) {
			sm_file_face_t face;
			fread(&face,sizeof(sm_file_face_t),1,file);
			sm->surface[i]->face[j].v0 = face.v0;
			sm->surface[i]->face[j].v1 = face.v1;
			sm->surface[i]->face[j].v2 = face.v2;
		}
	}
	fclose(file);
	return sm;
}

/*
 */
int sm_save(char *name,sm_t *sm) {
	int i;
	FILE *file;
	sm_file_header_t header;
	file = fopen(name,"wb");
	if(!file) {
		fprintf(stderr,"error create \"%s\" file\n",name);
		return -1;
	}
	header.ident = SM_IDENT;
	header.num_bone = sm->num_bone;
	header.num_frame = sm->num_frame;
	header.num_surface = sm->num_surface;
	fwrite(&header,sizeof(sm_file_header_t),1,file);
	for(i = 0; i < header.num_bone; i++) {
		sm_file_bone_t bone;
		strcpy(bone.name,sm->bone[i].name);
		fwrite(&bone,sizeof(sm_file_bone_t),1,file);
	}
	for(i = 0; i < header.num_frame; i++) {
		int j;
		for(j = 0; j < sm->num_bone; j++) {
			sm_file_frame_t frame;
			frame.xyz[0] = sm->frame[i][j].xyz[0];
			frame.xyz[1] = sm->frame[i][j].xyz[1];
			frame.xyz[2] = sm->frame[i][j].xyz[2];
			frame.rot[0] = sm->frame[i][j].rot[0];
			frame.rot[1] = sm->frame[i][j].rot[1];
			frame.rot[2] = sm->frame[i][j].rot[2];
			frame.rot[3] = sm->frame[i][j].rot[3];
			fwrite(&frame,sizeof(sm_file_frame_t),1,file);
		}
	}
	for(i = 0; i < header.num_surface; i++) {
		int j;
		sm_file_surface_t surface;
		strcpy(surface.name,sm->surface[i]->name);
		surface.num_vertex = sm->surface[i]->num_vertex;
		surface.num_face = sm->surface[i]->num_face;
		fwrite(&surface,sizeof(sm_surface_t),1,file);
		for(j = 0; j < surface.num_vertex; j++) {
			int k;
			sm_file_vertex_t vertex;
			vertex.st[0] = sm->surface[i]->vertex[j].st[0];
			vertex.st[1] = sm->surface[i]->vertex[j].st[1];
			vertex.num_weight = sm->surface[i]->vertex[j].num_weight;
			fwrite(&vertex,sizeof(sm_vertex_t),1,file);
			for(k = 0; k < vertex.num_weight; k++) {
				sm_file_weight_t weight;
				weight.bone = sm->surface[i]->vertex[j].weight[k].bone;
				weight.weight = sm->surface[i]->vertex[j].weight[k].weight;
				weight.xyz[0] = sm->surface[i]->vertex[j].weight[k].xyz[0];
				weight.xyz[1] = sm->surface[i]->vertex[j].weight[k].xyz[1];
				weight.xyz[2] = sm->surface[i]->vertex[j].weight[k].xyz[2];
				weight.normal[0]=sm->surface[i]->vertex[j].weight[k].normal[0];
				weight.normal[1]=sm->surface[i]->vertex[j].weight[k].normal[1];
				weight.normal[2]=sm->surface[i]->vertex[j].weight[k].normal[2];
				weight.normal[3]=sm->surface[i]->vertex[j].weight[k].normal[3];
				fwrite(&weight,sizeof(sm_file_weight_t),1,file);
			}
		}
		for(j = 0; j < surface.num_face; j++) {
			sm_file_face_t face;
			face.v0 = sm->surface[i]->face[j].v0;
			face.v1 = sm->surface[i]->face[j].v1;
			face.v2 = sm->surface[i]->face[j].v2;
			fwrite(&face,sizeof(sm_file_face_t),1,file);
		}
	}
	fclose(file);
	return 0;
}

/*
 */ 
void sm_frame(sm_t *sm,int from,int to,float frame) {
	int i,frame0,frame1;
	if(from == -1) from = 0;
	if(to == -1) to = sm->num_frame;
	frame0 = (int)frame;
	frame -= frame0;
	frame0 += from;
	if(frame0 >= to) frame0 = ((frame0 - from) % (to - from)) + from;
	frame1 = frame0 + 1;
	if(frame1 >= to) frame1 = from;
	for(i = 0; i < sm->num_bone; i++) {
		float m0[16],m1[16];
		float xyz0[3],xyz1[3],xyz[3],rot[4];
		v_scale(sm->frame[frame0][i].xyz,1.0 - frame,xyz0);
		v_scale(sm->frame[frame1][i].xyz,frame,xyz1);
		v_add(xyz0,xyz1,xyz);
		q_slerp(sm->frame[frame0][i].rot,sm->frame[frame1][i].rot,frame,rot);
		m_translate(xyz,m0);
		q_to_matrix(rot,m1);
		m_multiply(m0,m1,sm->bone[i].matrix);
	}
	for(i = 0; i < sm->num_surface; i++) {
		int j;
		sm_surface_t *s = sm->surface[i];
		for(j = 0; j < s->num_vertex; j++) {
			int k;
			v_set(0,0,0,s->vertex[j].xyz);
			v_set(0,0,0,s->vertex[j].normal);
			for(k = 0; k < s->vertex[j].num_weight; k++) {
				float v[3];
				sm_weight_t *w = &s->vertex[j].weight[k];
				v_transform(w->xyz,sm->bone[w->bone].matrix,v);
				v_scale(v,w->weight,v);
				v_add(s->vertex[j].xyz,v,s->vertex[j].xyz);
				v_transform_normal(w->normal,sm->bone[w->bone].matrix,v);
				v_scale(v,w->weight,v);
				v_add(s->vertex[j].normal,v,s->vertex[j].normal);
			}
		}
	}
}

/*
 */
int sm_bone(sm_t *sm,char *name) {
	int i;
	for(i = 0; i < sm->num_bone; i++) {
		if(!strcmp(name,sm->bone[i].name)) return i;
	}
	return -1;
}

/*
 */
void sm_bone_transform(sm_t *sm,int num,float *matrix) {
	if(num < 0) m_identity(matrix);
	else memcpy(matrix,sm->bone[num].matrix,sizeof(float) * 16);
}

/*
 */
int sm_surface(sm_t *sm,char *name) {
	int i;
	for(i = 0; i < sm->num_surface; i++) {
		if(!strcmp(name,sm->surface[i]->name)) return i;
	}
	return -1;
}

/*
 */
void sm_render_surface(sm_t *sm,int num) {
	sm_surface_t *s = sm->surface[num];
	if(num < 0) return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3,GL_FLOAT,sizeof(sm_vertex_t),s->vertex->xyz);
	glNormalPointer(GL_FLOAT,sizeof(sm_vertex_t),s->vertex->normal);
	glTexCoordPointer(2,GL_FLOAT,sizeof(sm_vertex_t),s->vertex->st);
	glDrawElements(GL_TRIANGLES,s->num_face * 3,GL_UNSIGNED_INT,s->face);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

/*
 */
void sm_render(sm_t *sm) {
	int i;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for(i = 0; i < sm->num_surface; i++) {
		sm_surface_t *s = sm->surface[i];
		glVertexPointer(3,GL_FLOAT,sizeof(sm_vertex_t),s->vertex->xyz);
		glNormalPointer(GL_FLOAT,sizeof(sm_vertex_t),s->vertex->normal);
		glTexCoordPointer(2,GL_FLOAT,sizeof(sm_vertex_t),s->vertex->st);
		glDrawElements(GL_TRIANGLES,s->num_face * 3,GL_UNSIGNED_INT,s->face);
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
