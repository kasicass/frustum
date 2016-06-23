/*  config
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define FRANDOM 1

typedef struct {
    float step,altitude;
    char heightmap[256];
    char objectconfig[256];
    char objectmap[256];
    char fileout[256];
} main_config_t;

typedef struct {
    char name[32];
    int color[3];
    float target;
    int flag;
} object_config_t;

main_config_t *main_config(char *name);
object_config_t *object_config(char *config,int *num_object);

#endif /* __CONFIG_H__ */
