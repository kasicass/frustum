/*  landscape
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __LAND_H__
#define __LAND_H__

#define VectorSet(x,y,z,v) { (v)[0] = x; (v)[1] = y; (v)[2] = z; }
#define VectorCopy(a,b) { (b)[0] = (a)[0]; (b)[1] = (a)[1]; (b)[2] = (a)[2]; }
#define VectorSub(a,b,c) { (c)[0] = (a)[0] - (b)[0]; (c)[1] = (a)[1] - (b)[1]; (c)[2] = (a)[2] - (b)[2]; }
#define VectorDotProduct(a,b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

typedef struct {
    float v[3];
} land_vertex_t;

typedef struct {
    land_vertex_t *vertex;
    int width,height;
    float step;
} land_t;

void VectorCrossProduct(const float *v1,const float *v2,float *out);
land_t *land_create(unsigned char *heightmap,int width,int height,float step,float altitude);
float land_height(land_t *land,float *point);

#endif /* __LAND_H__ */
