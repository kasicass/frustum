/*  load 3ds file
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __LOAD3DS_H__
#define __LOAD3DS_H__

typedef struct {
    float *vertex;
    float *uvmap;
    int *indices;
    int num_face;
    int num_vertex;
    int num_uvmap;
} object_t;

object_t *Load3DS(char *name);

#endif /* __LOAD3DS_H__ */
