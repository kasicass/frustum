/*	shadow map
 *
 *		written by Alexander Zaprjagaev
 *		frustum@public.tsu.ru
 */

#ifndef __SHADOW_H__
#define __SHADOW_H__

#define SHADOW_DIST 1000

typedef struct {
	int texture_id;
	int size;
	unsigned char *shadow;
	vec3_t min,max;
	vec4_t plane[4];
	matrix_t matrix;
} shadow_t;

shadow_t *shadow_create(int size);
void shadow_render_hw(shadow_t *s,float *light,float *matrix,
		void *vertex,int stride,int num_vertex);
void shadow_render_sw(shadow_t *s,float *light,float *matrix,
		void *vertex,int stride,int num_vertex);
void shadow_project(shadow_t *s,float *matrix,
		void *vertex,int stride,int num_vertex);

#endif /* __SHADOW_H__ */
