/*	shadow map
 *
 *		written by Alexander Zaprjagaev
 *		frustum@public.tsu.ru
 */

#include <stdio.h>
#include <malloc.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "mathlib.h"
#include "shadow.h"

shadow_t *shadow_create(int size) {
	shadow_t *s;
	s = calloc(1,sizeof(shadow_t));
	s->size = size;
	s->shadow = malloc(sizeof(unsigned char) * size * size * 3);
	glGenTextures(1,&s->texture_id);
	glBindTexture(GL_TEXTURE_2D,s->texture_id);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,size,size,0,GL_LUMINANCE,
			GL_UNSIGNED_BYTE,NULL);
	return s;
}

static void shadow_render_triangle(shadow_t *s,float *vertex) {
	int x,y,length;
	float x0,y0,x1,y1,x2,y2;
	float x_start,x_end,dx_start,dx_end;
	x0 = vertex[0];
	y0 = vertex[1];
	x1 = vertex[3];
	y1 = vertex[4];
	x2 = vertex[6];
	y2 = vertex[7];
	if((x1 - x0) * (y2 - y1) - (x2 - x1) * (y1 - y0) < 0) return;
#define SWAP(a,b) { float tmp = a; a = b; b = tmp; }
	if(y0 > y1) { SWAP(x0,x1) SWAP(y0,y1) }
	if(y0 > y2) { SWAP(x0,x2) SWAP(y0,y2) }
	if(y1 > y2) { SWAP(x1,x2) SWAP(y1,y2) }
#undef SWAP
	x_start = x0;
	dx_start = (x2 - x0) / (y2 - y0);
	x_end = x0;
	dx_end = (x1 - x0) / (y1 - y0);
	for(y = y0; y < y1 - 1; y++) {
		unsigned char *dest;
		if(x_start < x_end) { x = x_start; length = x_end - x_start; }
		else { x = x_end; length = x_start - x_end; }
		dest = s->shadow + (s->size * y + x) * 3;
		length++;
		while(length--) {
			*dest++ = 255;
			dest += 2;
		}
		x_start += dx_start;
		x_end += dx_end;
	}
	x_end = x1;
	dx_end = (x2 - x1) / (y2 - y1);
	for(y = y1; y < y2 - 1; y++) {
		unsigned char *dest;
		if(x_start < x_end) { x = x_start; length = x_end - x_start; }
		else { x = x_end; length = x_start - x_end; }
		dest = s->shadow + (s->size * y + x) * 3;
		length++;
		while(length--) {
			*dest++ = 255;
			dest += 2;
		}
		x_start += dx_start;
		x_end += dx_end;
	}
}

static void shadow_blur(shadow_t *s) {
	int x,y,sum,size3 = s->size * 3;
	unsigned char *shadow,*inp,*outp;
	shadow = malloc(s->size * s->size * 3);
	memset(shadow,0,s->size * s->size * 3);
	inp = s->shadow + size3;
	outp = shadow + size3;
	for(y = 1; y < s->size - 1; y++) {
		inp += 3;
		outp += 3;
		for(x = 1; x < s->size - 1; x++) {
			sum = inp[-size3 - 3] + inp[-size3] + inp[-size3 + 3] +
				inp[-3] + inp[0] + inp[3] +
				inp[size3 - 3] + inp[size3] + inp[size3 + 3];
			sum /= 9;
			inp += 3;
			*outp++ = sum;
			*outp++ = sum;
			*outp++ = sum;
		}
		inp += 3;
		outp += 3;
	}
	memcpy(s->shadow,shadow,s->size * s->size * 3);
	free(shadow);
}

static void shadow_clip_plane(shadow_t *s) {
	int i;
	matrix_t m;
	m_inverse(s->matrix,m);
	for(i = 0; i < 4; i++) {
		vec3_t v0,v1,v2,a,b;
		switch(i) {
			case 0:
				v_set(s->min[0],s->max[1],0,v0);
				v_set(s->max[0],s->max[1],0,v1);
				v_set(s->min[0],s->max[1],1,v2);
				break;
			case 1:
				v_set(s->max[0],s->max[1],0,v0);
				v_set(s->max[0],s->min[1],0,v1);
				v_set(s->max[0],s->max[1],1,v2);
				break;
			case 2:
				v_set(s->max[0],s->min[1],0,v0);
				v_set(s->min[0],s->min[1],0,v1);
				v_set(s->max[0],s->min[1],1,v2);
				break;
			case 3:
				v_set(s->min[0],s->min[1],0,v0);
				v_set(s->min[0],s->max[1],0,v1);
				v_set(s->min[0],s->min[1],1,v2);
				break;
		}
		v_transform(v0,m,v0);
		v_transform(v1,m,v1);
		v_transform(v2,m,v2);
		v_sub(v1,v0,a);
		v_sub(v2,v0,b);
		v_cross(a,b,s->plane[i]);
		s->plane[i][3] = -v_dot(s->plane[i],v0);
	}
}

void shadow_render_sw(shadow_t *s,float *light,float *matrix,
		void *vertex,int stride,int num_vertex) {
	int i;
	float size,sx,sy,*tvertex,*tvertex_ptr;
	float *vertex_ptr;
	vec3_t eye,dir,up;
	matrix_t m;
	
	v_normalize(light,eye);
	v_scale(eye,SHADOW_DIST,eye);
	v_set(0,0,0,dir);
	v_set(0,0,1,up);
	
	m_look_at(eye,dir,up,s->matrix);
	m_multiply(s->matrix,matrix,m);
	
	v_set(999999,999999,999999,s->min);
	v_set(-999999,-999999,-999999,s->max);

	tvertex = (float*)malloc(sizeof(float) * 3 * num_vertex);
	tvertex_ptr = tvertex;
	
	for(i = 0, vertex_ptr = vertex, tvertex_ptr = tvertex; i < num_vertex;
			i++, vertex_ptr += stride, tvertex_ptr += 3) {
		vec3_t v;
		v_transform((float*)vertex_ptr,m,v);
		if(s->max[0] < v[0]) s->max[0] = v[0];
		if(s->min[0] > v[0]) s->min[0] = v[0];
		if(s->max[1] < v[1]) s->max[1] = v[1];
		if(s->min[1] > v[1]) s->min[1] = v[1];
		v_copy(v,tvertex_ptr);
	}
	
	size = (s->max[0] - s->min[0]) / (float)s->size * 2.0;
	s->max[0] += size;
	s->min[0] -= size;
	size = (s->max[1] - s->min[1]) / (float)s->size * 2.0;
	s->max[1] += size;
	s->min[1] -= size;
	
	sx = 1.0 / (s->max[0] - s->min[0]) * (float)s->size;
	sy = 1.0 / (s->max[1] - s->min[1]) * (float)s->size;
	for(i = 0, tvertex_ptr = tvertex; i < num_vertex; i++, tvertex_ptr += 3) {
		tvertex_ptr[0] = (tvertex_ptr[0] - s->min[0]) * sx;
		tvertex_ptr[1] = (tvertex_ptr[1] - s->min[1]) * sy;
	}
	
	shadow_clip_plane(s);
	
    glBindTexture(GL_TEXTURE_2D,s->texture_id);
	
	memset(s->shadow,0,s->size * s->size * 3);
	for(i = 0, tvertex_ptr = tvertex; i < num_vertex;
			i += 3, tvertex_ptr += 9) shadow_render_triangle(s,tvertex_ptr);
	shadow_blur(s);
	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,s->size,s->size,GL_RGB,
		GL_UNSIGNED_BYTE,s->shadow);
	
	free(tvertex);
}

void shadow_render_hw(shadow_t *s,float *light,float *matrix,
		void *vertex,int stride,int num_vertex) {
	int i,viewport[4];
	float size;
	float *vertex_ptr;
	vec3_t eye,dir,up;
	matrix_t m;
	
	v_normalize(light,eye);
	v_scale(eye,SHADOW_DIST,eye);
	v_set(0,0,0,dir);
	v_set(0,0,1,up);
	
	m_look_at(eye,dir,up,s->matrix);
	m_multiply(s->matrix,matrix,m);
	
	v_set(999999,999999,999999,s->min);
	v_set(-999999,-999999,-999999,s->max);
	for(i = 0, vertex_ptr = vertex; i < num_vertex; i++, vertex_ptr += stride) {
		vec3_t v;
		v_transform((float*)vertex_ptr,m,v);
		if(s->max[0] < v[0]) s->max[0] = v[0];
		if(s->min[0] > v[0]) s->min[0] = v[0];
		if(s->max[1] < v[1]) s->max[1] = v[1];
		if(s->min[1] > v[1]) s->min[1] = v[1];
	}
	
	size = (s->max[0] - s->min[0]) / (float)s->size * 2.0;
	s->max[0] += size;
	s->min[0] -= size;
	size = (s->max[1] - s->min[1]) / (float)s->size * 2.0;
	s->max[1] += size;
	s->min[1] -= size;
	
	shadow_clip_plane(s);

	glGetIntegerv(GL_VIEWPORT,viewport);
	glViewport(0,0,s->size,s->size);
	glScissor(0,0,s->size,s->size);
	glEnable(GL_SCISSOR_TEST);
	
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(s->min[0],s->max[0],s->min[1],s->max[1],1,SHADOW_DIST * 2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(s->matrix);
	
	glPushMatrix();
	glMultMatrixf(matrix);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,stride,vertex);
	glDrawArrays(GL_TRIANGLES,0,num_vertex);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
	
    glBindTexture(GL_TEXTURE_2D,s->texture_id);
	
	glReadPixels(0,0,s->size,s->size,GL_RGB,GL_UNSIGNED_BYTE,s->shadow);
	shadow_blur(s);
	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,s->size,s->size,GL_RGB,
		GL_UNSIGNED_BYTE,s->shadow);

	glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
	glScissor(viewport[0],viewport[1],viewport[2],viewport[3]);
	glDisable(GL_SCISSOR_TEST);
}

void shadow_project(shadow_t *s,float *matrix,
		void *vertex,int stride,int num_vertex) {
	int i;
	vec4_t plane_s = { 1, 0, 0, 0 };
	vec4_t plane_t = { 0, 1, 0, 0 };
	vec4_t plane_r = { 0, 0, 1, 0 };
	vec4_t plane_q = { 0, 0, 0, 1 };

	glTexGenfv(GL_S,GL_EYE_PLANE,plane_s);
	glTexGenfv(GL_T,GL_EYE_PLANE,plane_t);
	glTexGenfv(GL_R,GL_EYE_PLANE,plane_r);
	glTexGenfv(GL_Q,GL_EYE_PLANE,plane_q);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);
	
	for(i = 0; i < 4; i++) {
		int gl_plane;
		double plane[4];
		if(i == 0) gl_plane = GL_CLIP_PLANE0;
		else if(i == 1) gl_plane = GL_CLIP_PLANE1;
		else if(i == 2) gl_plane = GL_CLIP_PLANE2;
		else gl_plane = GL_CLIP_PLANE3;
		plane[0] = s->plane[i][0];
		plane[1] = s->plane[i][1];
		plane[2] = s->plane[i][2];
		plane[3] = s->plane[i][3];
		glClipPlane(gl_plane,plane);
		glEnable(gl_plane);
	}
	
	glPushMatrix();
	glMultMatrixf(matrix);
	
	glMatrixMode(GL_TEXTURE);
	glTranslatef(0.5,0.5,0);
	glScalef(0.5,0.5,1.0);
	glOrtho(s->min[0],s->max[0],s->min[1],s->max[1],-1,1);
    glMultMatrixf(s->matrix);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,s->texture_id);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,stride,vertex);
	glDrawArrays(GL_TRIANGLES,0,num_vertex);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
	glDisable(GL_CLIP_PLANE2);
	glDisable(GL_CLIP_PLANE3);
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
}
