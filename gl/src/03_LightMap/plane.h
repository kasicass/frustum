/*  light mapping demo
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __PLANE_H__
#define __PLANE_H__

typedef struct {
    float v[3][3];
    float t[3][2];
    float plane[4];
} face_t;

#ifdef __cplusplus
extern "C" {
#endif

    face_t *createplaneface(int *num);
    void drawplane(void);

#ifdef __cplusplus
}
#endif
#endif
