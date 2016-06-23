/*  landscape thing-map creater
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include "land.h"
#include "config.h"
#include "loadtga.h"

int main(int argc,char **argv) {
    int i,j,k,l;
    FILE *file;
    land_t *land;
    main_config_t *config;
    int num_object;
    object_config_t *object;
    unsigned char *heightmap,*objectmap;
    int width,height;
    float point[3],target;
    
    if(argc < 2) config = main_config("map.cfg");
    else config = main_config(argv[1]);    
    if(!config) {
        printf("parse error\nfile: %s\n",(argc < 2) ? "map.cfg" : argv[1]);
        return 1;
    }
    
    object = object_config(config->objectconfig,&num_object);
    if(!object) {
        printf("parse error\nfile: %s\n",config->objectconfig);
        return 1;
    }
    
    heightmap = LoadTGA(config->heightmap,&width,&height);
    if(!heightmap) {
        printf("load error\nfile: %s\n",config->heightmap);
        return 1;
    }
    land = land_create(heightmap,width,height,config->step,config->altitude);
    free(heightmap);
    
    objectmap = LoadTGA(config->objectmap,&width,&height);
    if(!objectmap) {
        printf("load error\nfile: %s\n",config->objectmap);
        return 1;
    }

    file = fopen(config->fileout,"w");
    if(!file) {
        printf("error create\nfile: %s\n",config->fileout);
        return 1;
    }

    srand((long)time(0));

    for(j = 0, k = 0; j < height; j++)
        for(i = 0; i < width; i++, k += 4) {
            for(l = 0; l < num_object; l++)
                if(objectmap[k + 0] == object[l].color[0] &&
                   objectmap[k + 1] == object[l].color[1] &&
                   objectmap[k + 2] == object[l].color[2]) {
                    point[0] = (float)i / (float)width * land->step * (float)land->width;
                    point[1] = (float)j / (float)height * land->step * (float)land->height;
                    point[2] = land_height(land,point);
                    if(object[l].flag == FRANDOM) target = (float)rand() / RAND_MAX * 360.0;
                    else target = object[l].target;
                    fprintf(file,"%s : %.3f %.3f %.3f %.3f\n",
                        object[l].name,point[0],point[1],point[2],target);
                    l = -1;
                    break;
                }
            if(l != -1 && (objectmap[k + 0] != 0 ||
                           objectmap[k + 1] != 0 ||
                           objectmap[k + 2] != 0)) {
                printf("in %5u,%-5u unknow color #%02x%02x%02x\n",i,j,
                    objectmap[k + 0],
                    objectmap[k + 1],
                    objectmap[k + 2]);
            }
        }
    
    fclose(file);
    printf("done\n");

    return 0;
}
