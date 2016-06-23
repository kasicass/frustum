/*  convert 3ds file to interpolated mesh file .ime
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <malloc.h>
#include "load3ds.h"

#define VectorSet(x,y,z,v) { (v)[0] = x; (v)[1] = y; (v)[2] = z; }
#define VectorScale(a,b,c) { (c)[0] = (a)[0] * b; (c)[1] = (a)[1] * b; (c)[2] = (a)[2] * b; }
#define VectorAdd(a,b,c) { (c)[0] = (a)[0] + (b)[0]; (c)[1] = (a)[1] + (b)[1]; (c)[2] = (a)[2] + (b)[2]; }
#define VectorSub(a,b,c) { (c)[0] = (a)[0] - (b)[0]; (c)[1] = (a)[1] - (b)[1]; (c)[2] = (a)[2] - (b)[2]; }

typedef struct {
    int header;
    int num_frame;
    int num_vertex;
    int offset_vertex;
    int num_uvmap;
    int offset_uvmap;
    int num_face;
    int offset_indices;
} imeheader_t;

void object_scale(object_t *object,float scale) {
    int i;
    for(i = 0; i < object->num_vertex * 3; i += 3)
        VectorScale(&object->vertex[i],scale,&object->vertex[i]);
}

void mesh_optimize(object_t *object,float *move,float *scale) {
    int i,j;
    float max[3],min[3];
    move[0] = move[1] = move[2] = 0;
    scale[0] = scale[1] = scale[2] = 0;
    VectorSet(0,0,0,move);
    VectorSet(1,1,1,scale);
    VectorSet(1000000,1000000,1000000,min);
    VectorSet(-1000000,-1000000,-1000000,max);
    for(i = 0; i < object->num_vertex * 3; i += 3)
        for(j = 0; j < 3; j++) {
            if(object->vertex[i + j] > max[j]) max[j] = object->vertex[i + j];
            if(object->vertex[i + j] < min[j]) min[j] = object->vertex[i + j];
        }
    VectorAdd(min,max,move);
    VectorScale(move,0.5,move);
    for(i = 0; i < object->num_vertex * 3; i += 3)
        VectorSub(&object->vertex[i],move,&object->vertex[i]);
    scale[0] = 2.0 / (max[0] - min[0]);
    scale[1] = 2.0 / (max[1] - min[1]);
    scale[2] = 2.0 / (max[2] - min[2]);
    for(i = 0; i < object->num_vertex * 3; i += 3) {
        object->vertex[i + 0] *= scale[0];
        object->vertex[i + 1] *= scale[1];
        object->vertex[i + 2] *= scale[2];
    }
    scale[0] = 1.0 / scale[0];
    scale[1] = 1.0 / scale[1];
    scale[2] = 1.0 / scale[2];
}

int save_ime(char *name,object_t **object,int num_frame) {
    int i,j;
    short buffer[3];
    float move[3],scale[3];
    FILE *file;
    imeheader_t header;
    file = fopen(name,"wb");
    if(!file) return 0;
    header.header = 'i' | ('m' << 8) | ('e' << 16) | (' ' << 24);
    header.num_frame = num_frame;
    header.num_vertex = object[0]->num_vertex;
    header.offset_vertex = sizeof(imeheader_t);
    header.num_uvmap = object[0]->num_uvmap;
    header.offset_uvmap = header.offset_vertex + sizeof(short) * 3 * header.num_vertex * header.num_frame + sizeof(float) * 6 * header.num_frame;
    header.num_face = object[0]->num_face;
    header.offset_indices = header.offset_uvmap + sizeof(short) * 2 * header.num_uvmap;
    fwrite(&header,sizeof(imeheader_t),1,file);
    for(i = 0; i < num_frame; i++) {
        mesh_optimize(object[i],move,scale);
        fwrite(move,sizeof(float),3,file);
        fwrite(scale,sizeof(float),3,file);
        for(j = 0; j < header.num_vertex * 3; j += 3) {
            buffer[0] = (int)(32767.0 * object[i]->vertex[j + 0]);
            buffer[1] = (int)(32767.0 * object[i]->vertex[j + 1]);
            buffer[2] = (int)(32767.0 * object[i]->vertex[j + 2]);
            fwrite(buffer,sizeof(short),3,file);
        }
    }
    for(i = 0; i < header.num_uvmap * 2; i += 2) {
        buffer[0] = (int)(32767.0 * object[0]->uvmap[i + 0]);
        buffer[1] = (int)(32767.0 * object[0]->uvmap[i + 1]);
        fwrite(buffer,sizeof(short),2,file);
    }
    for(i = 0; i < header.num_face * 3; i += 3) {
        buffer[0] = object[0]->indices[i + 0];
        buffer[1] = object[0]->indices[i + 1];
        buffer[2] = object[0]->indices[i + 2];
        fwrite(buffer,sizeof(short),3,file);
    }
    fclose(file);
    return 1;
}

int main(int argc,char **argv) {
    FILE *file;
    char name[256],meshname[256];
    int i,num_frame;
    float scale;
    object_t **object;
    if(argc == 1) {
        printf("3ds to interpolate mesh\n");
        printf("config example:\n");
        printf("  newfile.ime\n");
        printf("  scale mesh\n");
        printf("  num frame\n");
        printf("  mesh0.3ds\n");
        printf("  mesh1.3ds\n");
        printf("  ...\n");
        return 0;
    }
    file = fopen(argv[1],"r");
    if(!file) {
        printf("error open config file %s\n",argv[1]);
        return 1;
    }
    fscanf(file,"%s",name);
    fscanf(file,"%f",&scale);
    fscanf(file,"%u",&num_frame);
    object = (object_t**)malloc(sizeof(object_t*) * num_frame);
    for(i = 0; i < num_frame; i++) {
        fscanf(file,"%s",meshname);
        object[i] = Load3DS(meshname);
        if(!object[i]) {
            printf("error open mesh %s\n",meshname);
            fclose(file);
            return 1;
        }
        object_scale(object[i],scale);
    }
    fclose(file);
    save_ime(name,object,num_frame);
    return 0;
}
