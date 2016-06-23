/*  load ime file (interpolated mesh)
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __LOADIME_H__
#define __LOADIME_H__

// total 8 floats
// 1,2,3 - vertex
// 3,4,5 - normal
// 6,7 - texture coordinate

float **LoadIME(char *name,int *num_vertex,int *num_frame);

#endif /* __LOADIME_H__ */
