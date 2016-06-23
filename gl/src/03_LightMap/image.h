/*  light mapping demo
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

    unsigned char *LoadTGA(char*,int*,int*);
    int SaveTGA(char*,unsigned char*,int,int);
    unsigned char *LoadJPEG(char*,int*,int*);

#ifdef __cplusplus
}
#endif

#endif
   