#ifndef __CAMERA_H__
#define __CAMERA_H__

typedef struct {
	float pos[3],dir[3],up[3];
	float fov,aspect;
	float mview[16],imview[16];
	float clipnear,clipfar;
	float frustum[6][4];
} camera_t;

camera_t *camera_create(float fov,float aspect,float clipnear,float clipfar);
void camera_look_at(camera_t *c,float *pos,float *dir,float *up);
int camera_check_point(camera_t *c,float *p);
int camera_check_sphere(camera_t *c,float *p,float r);
int camera_check_box(camera_t *c,float *min,float *max);

#endif /* __CAMERA_H__ */
