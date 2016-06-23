/*  config
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "config.h"

static int skeep_bracket(FILE *file) {
    char c;
    while(fread(&c,1,1,file) == 1) {
        if(c == '{') skeep_bracket(file);
        if(c == '}') return 1;
    }
    return 0;
}

static void skeep_comment(FILE *file) {
    char c;
    while(fread(&c,1,1,file) == 1 && c != '\n');
}

main_config_t *main_config(char *name) {
    FILE *file;
    char buffer[128];
    main_config_t *config;
    config = (main_config_t*)malloc(sizeof(main_config_t));
    if(!config) return NULL;
    file = fopen(name,"r");
    if(!file) return NULL;
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"heightmap")) {  // height map name
            fscanf(file,"%s",config->heightmap);
        } else if(!strcmp(buffer,"step")) {     // step landscape
            fscanf(file,"%s",buffer);
            config->step = atof(buffer);
        } else if(!strcmp(buffer,"altitude")) { // altitude landscape
            fscanf(file,"%s",buffer);
            config->altitude = atof(buffer);
        } else if(!strcmp(buffer,"object")) {   // object config name
            fscanf(file,"%s",config->objectconfig);
        } else if(!strcmp(buffer,"objectmap")) {// object map
            fscanf(file,"%s",config->objectmap);
        } else if(!strcmp(buffer,"fileout")) {  // file out
            fscanf(file,"%s",config->fileout);
        } else {
            fclose(file);
            return NULL;
        }
    }
    fclose(file);
    return config;
}

object_config_t *object_config(char *name,int *num_object) {
    int i,j;
    char buffer[128],*ptr;
    FILE *file;
    object_config_t *object;
    file = fopen(name,"r");
    if(!file) return NULL;
    i = 0;
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"object")) i++;
        else if(!strcmp(buffer,"{")) {
            if(!skeep_bracket(file)) {
                fclose(file);
                return NULL;
            }
        } else {
            fclose(file);
            return NULL;
        }
    }
    if(i == 0) {
        fclose(file);
        return NULL;
    }
    object = (object_config_t*)malloc(sizeof(object_config_t) * i);
    if(!object) {
        fclose(file);
        return NULL;
    }
    memset(object,0,sizeof(object_config_t) * i);
    fseek(file,0,SEEK_SET);
    i = 0;
    while(fscanf(file,"%s",buffer) != EOF) {
        if(buffer[0] == '#') skeep_comment(file);
        else if(!strcmp(buffer,"object")) {
            fscanf(file,"%s",buffer);   // skeep '{'
            while(fscanf(file,"%s",buffer) != EOF) {
                if(buffer[0] == '#') skeep_comment(file);
                else if(!strcmp(buffer,"name")) { // read name
                    fscanf(file,"%s",object[i].name);
                } else if(!strcmp(buffer,"color")) {  // color
                    fscanf(file,"%s",buffer);   // #0a11ff format
                    if(buffer[0] != '#') {
                        fclose(file);
                        return NULL;
                    }
                    ptr = buffer + 1;
                    for(j = 0; j < 3; j++) {
                        ptr[0] -= 48;
                        ptr[1] -= 48;
                        if(ptr[0] > 9) ptr[0] -= 7;
                        if(ptr[1] > 9) ptr[1] -= 7;
                        if(ptr[0] > 15) ptr[0] -= 32;
                        if(ptr[1] > 15) ptr[1] -= 32;
                        object[i].color[j] = ptr[0] * 16 + ptr[1];
                        ptr += 2;
                    }
                } else if(!strcmp(buffer,"target")) {   // target
                    fscanf(file,"%s",buffer);
                    if(!strcmp(buffer,"random")) object[i].flag = FRANDOM;
                    else object[i].target = atof(buffer);
                } else if(!strcmp(buffer,"}")) {
                    break;
                } else {
                    fclose(file);
                    return NULL;
                }
            }
            i++;
        } else {
            fclose(file);
            return NULL;
        }
    }
    fclose(file);
    *num_object = i;
    return object;
}
