/*  camera
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __CAMERA_H__
#define __CAMERA_H__

typedef struct {
    float fov;
    float aspect;
    float clipnear;
    float clipfar;
    float fogcolor[4];
    float fogstart;
    float fogend;
} camera_config_t;

typedef struct {
    float pos[3];           // camera position
    float dir[3];           // camera direction
    float up[3];            // camera up
    float inverse[16];      // inverse view matrix
    float fov,aspect;       // fov and aspect camera
    float clipnear,clipfar; // clip bounds
    float frustum[6][4];    // frustum
    float fogcolor[4];      // fog color
    float fogstart;         // fog start
    float fogend;           // fog end
} camera_t;

camera_t *camera_create(camera_config_t *config);
void camera_free(camera_t *camera);
void camera_view(camera_t *camera);
int camera_check_point(camera_t *camera,float *pos);
int camera_check_sphere(camera_t *camera,float *pos,float radius);
int camera_check_box(camera_t *camera,float *min,float *max);

#endif /* __CAMERA_H__ */
