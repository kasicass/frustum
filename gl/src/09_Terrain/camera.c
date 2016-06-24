#include <stdio.h>
#include <malloc.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "mathlib.h"
#include "camera.h"

camera_t *camera_create(float fov,float aspect,float clipnear,float clipfar) {
	camera_t *c = malloc(sizeof(camera_t));
	c->fov = fov;
	c->aspect = aspect;
	c->clipnear = clipnear;
	c->clipfar = clipfar;
	return c;
}

static void camera_extract_frustum(camera_t *c) {
	float ilength,clip[16],projection[16];
	glGetFloatv(GL_PROJECTION_MATRIX,projection);
	m_multiply(projection,c->mview,clip);
	c->frustum[0][0] = clip[3] - clip[0];
	c->frustum[0][1] = clip[7] - clip[4];
	c->frustum[0][2] = clip[11] - clip[8];
	c->frustum[0][3] = clip[15] - clip[12];
	ilength = 1.0 / v_length(c->frustum[0]);
	c->frustum[0][0] *= ilength;
	c->frustum[0][1] *= ilength;
	c->frustum[0][2] *= ilength;
	c->frustum[0][3] *= ilength;
	c->frustum[1][0] = clip[3] + clip[0];
	c->frustum[1][1] = clip[7] + clip[4];
	c->frustum[1][2] = clip[11] + clip[8];
	c->frustum[1][3] = clip[15] + clip[12];
	ilength = 1.0 / v_length(c->frustum[1]);
	c->frustum[1][0] *= ilength;
	c->frustum[1][1] *= ilength;
	c->frustum[1][2] *= ilength;
	c->frustum[1][3] *= ilength;
	c->frustum[2][0] = clip[3] - clip[1];
	c->frustum[2][1] = clip[7] - clip[5];
	c->frustum[2][2] = clip[11] - clip[9];
	c->frustum[2][3] = clip[15] - clip[13];
	ilength = 1.0 / v_length(c->frustum[2]);
	c->frustum[2][0] *= ilength;
	c->frustum[2][1] *= ilength;
	c->frustum[2][2] *= ilength;
	c->frustum[2][3] *= ilength;
	c->frustum[3][0] = clip[3] + clip[1];
	c->frustum[3][1] = clip[7] + clip[5];
	c->frustum[3][2] = clip[11] + clip[9];
	c->frustum[3][3] = clip[15] + clip[13];
	ilength = 1.0 / v_length(c->frustum[3]);
	c->frustum[3][0] *= ilength;
	c->frustum[3][1] *= ilength;
	c->frustum[3][2] *= ilength;
	c->frustum[3][3] *= ilength;
	c->frustum[4][0] = clip[3] - clip[2];
	c->frustum[4][1] = clip[7] - clip[6];
	c->frustum[4][2] = clip[11] - clip[10];
	c->frustum[4][3] = clip[15] - clip[14];
	ilength = 1.0 / v_length(c->frustum[4]);
	c->frustum[4][0] *= ilength;
	c->frustum[4][1] *= ilength;
	c->frustum[4][2] *= ilength;
	c->frustum[4][3] *= ilength;
	c->frustum[5][0] = clip[3] + clip[2];
	c->frustum[5][1] = clip[7] + clip[6];
	c->frustum[5][2] = clip[11] + clip[10];
	c->frustum[5][3] = clip[15] + clip[14];
	ilength = 1.0 / v_length(c->frustum[5]);
	c->frustum[5][0] *= ilength;
	c->frustum[5][1] *= ilength;
	c->frustum[5][2] *= ilength;
	c->frustum[5][3] *= ilength;
}

void camera_look_at(camera_t *c,float *pos,float *dir,float *up) {
	v_copy(pos,c->pos);
	v_copy(dir,c->dir);
	v_copy(up,c->up);
	m_look_at(pos,dir,up,c->mview);
	m_inverse(c->mview,c->imview);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(c->fov,c->aspect,c->clipnear,c->clipfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(c->mview);
	camera_extract_frustum(c);
}

int camera_check_point(camera_t *c,float *p) {
	int i;
	for(i = 0; i < 6; i++)
		if(-v_dot(c->frustum[i],p) >= c->frustum[i][3]) return -1;
	return 0;
}

int camera_check_sphere(camera_t *c,float *p,float r) {
	int i;
	for(i = 0; i < 6; i++)
		if(-v_dot(c->frustum[i],p) >= c->frustum[i][3] + r) return -1;
	return 0;
}

int camera_check_box(camera_t *c,float *min,float *max) {
	int i;
	float p[3];
	for(i = 0; i < 6; i++) {
		v_set(min[0],min[1],min[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		v_set(min[0],min[1],max[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		v_set(min[0],max[1],min[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		v_set(min[0],max[1],max[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		v_set(max[0],min[1],min[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		v_set(max[0],min[1],max[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		v_set(max[0],max[1],min[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		v_set(max[0],max[1],max[2],p);
		if(-v_dot(c->frustum[i],p) < c->frustum[i][3]) continue;
		return -1;
	}
	return 0;
}
