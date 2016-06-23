/*  spline
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __SPLINE_H__
#define __SPLINE_H__

typedef struct {
    float *x;       // 4 parameter spline for x
    float *y;       // 4 parameter spline for y
    float *z;       // 4 parameter spline for z
    int close;      // close spline or not (1 - close)
    int num_key;    // num key points
    float totaltime;    // total time
} spline_t;

void spline_pos(spline_t *spline,float t,float *pos);
float spline_target(spline_t *spline,float t);
spline_t *spline_create(float *key,int num_key,float totaltime,float tension,float bias,float continuity);
spline_t *spline_close_create(float *key,int num_key,float totaltime,float tension,float bias,float continuity);
float *spline_load_key(char *name,int *num_key);
spline_t *spline_load(char *name,float totaltime,float tension,float bias,float continuity);
spline_t *spline_close_load(char *name,float totaltime,float tension,float bias,float continuity);
void spline_free(spline_t *spline);

#endif /* __SPLINE_H__ */
