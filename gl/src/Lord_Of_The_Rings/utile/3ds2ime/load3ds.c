/*  load 3ds file
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include "load3ds.h"

#define CHUNK_MAIN      0x4D4D   
#define CHUNK_OBJMESH   0x3D3D
#define CHUNK_OBJBLOCK  0x4000
#define CHUNK_TRIMESH   0x4100
#define CHUNK_VERTLIST  0x4110
#define CHUNK_FACELIST  0x4120
#define CHUNK_MAPLIST   0x4140
 
struct {
    int num_vertex;
    int num_uvmap;
    int num_face;
    float *vertex;
    float *uvmap;
    int *indices;
    int pos;
} object_l;

int skeepstr(FILE *file) {
    int i = 0;
    char c = 1;
    while((c = fgetc(file)) != 0x00) i++;
    object_l.pos += ++i;
    return i;
}

void statfile3ds(FILE *file,int size) {
    unsigned short id,num;
    unsigned int length;
    while(object_l.pos < size) {
        fread(&id,2,1,file);
        fread(&length,4,1,file);
        object_l.pos += 6;
        if(length == 0) return;
        length -= 6;
        switch(id) {
            case CHUNK_MAIN:
            case CHUNK_OBJMESH:
            case CHUNK_TRIMESH:
                statfile3ds(file,object_l.pos + length);
                break;
            case CHUNK_OBJBLOCK:
                length -= skeepstr(file);
                statfile3ds(file,object_l.pos + length);
                break;
            case CHUNK_VERTLIST:
                fread(&num,2,1,file);
                fseek(file,num * 12,SEEK_CUR);
                object_l.pos += num * 12 + 2;
                object_l.num_vertex += num;
                break;
            case CHUNK_FACELIST:
                fread(&num,2,1,file);
                fseek(file,num * 8,SEEK_CUR);
                object_l.pos += num * 8 + 2;
                object_l.num_face += num;
                break;
            default:
                fseek(file,length,SEEK_CUR);
                object_l.pos += length;
                break;
        }
    }
}

void loadfile3ds(FILE *file,int size) {
    int i,j,startvertex,startface;
    unsigned short id,num;
    unsigned int length;
    startvertex = object_l.num_vertex;
    startface = object_l.num_face;
    while(object_l.pos < size) {
        fread(&id,2,1,file);
        fread(&length,4,1,file);
        object_l.pos += 6;
        if(length == 0) return;
        length -= 6;
        switch(id) {
            case CHUNK_MAIN:
            case CHUNK_OBJMESH:
            case CHUNK_TRIMESH:
                loadfile3ds(file,object_l.pos + length);
                break;
            case CHUNK_OBJBLOCK:
                length -= skeepstr(file);
                loadfile3ds(file,object_l.pos + length);
                break;
            case CHUNK_VERTLIST:
                fread(&num,2,1,file);
                object_l.num_vertex += num;
                object_l.pos += num * 12 + 2;
                for(i = startvertex; i < object_l.num_vertex; i++) {
                    j = i * 3;
                    fread(&object_l.vertex[j],4,1,file);
                    fread(&object_l.vertex[j + 1],4,1,file);
                    fread(&object_l.vertex[j + 2],4,1,file);
                }
                break;
            case CHUNK_MAPLIST:
                fread(&num,2,1,file);
                object_l.pos += num * 8 + 2;
                for(i = startvertex; i < object_l.num_vertex; i++) {
                    j = i * 2;
                    fread(&object_l.uvmap[j],4,1,file);
                    fread(&object_l.uvmap[j + 1],4,1,file);
                    object_l.uvmap[j + 1] = 1.0 - object_l.uvmap[j + 1];
                }   
                break;
            case CHUNK_FACELIST:
                fread(&num,2,1,file);
                object_l.num_face += num;
                object_l.pos += num * 8 + 2;
                for(i = startface; i < object_l.num_face; i++) {
                    j = i * 3;
                    fread(&object_l.indices[j],2,1,file);
                    fread(&object_l.indices[j + 1],2,1,file);
                    fread(&object_l.indices[j + 2],2,1,file);
                    object_l.indices[j] += startvertex;
                    object_l.indices[j + 1] += startvertex;
                    object_l.indices[j + 2] += startvertex;
                    fseek(file,2,SEEK_CUR);
                }
                break;
            default:
                fseek(file,length,SEEK_CUR);
                object_l.pos += length;
                break;
        }
    }
}

object_t *Load3DS(char *name) {
    int size;
    FILE *file;
    object_t *object;
    if(!(file = fopen(name,"rb"))) return NULL;
    object = (object_t*)malloc(sizeof(object_t));
    fseek(file,0,SEEK_END);
    size = ftell(file);
    object_l.num_vertex = 0;
    object_l.num_face = 0;
    object_l.pos = 0;
    fseek(file,0,SEEK_SET); 
    statfile3ds(file,size);
    object_l.vertex = (float*)malloc(sizeof(float) * object_l.num_vertex * 3);
    memset(object_l.vertex,0,sizeof(float) * object_l.num_vertex * 3);
    object_l.uvmap = (float*)malloc(sizeof(float) * object_l.num_vertex * 2);
    memset(object_l.uvmap,0,sizeof(float) * object_l.num_vertex * 2);
    object_l.indices = (int*)malloc(sizeof(int) * object_l.num_face * 3);
    memset(object_l.indices,0,sizeof(int) * object_l.num_face * 3);
    object_l.num_vertex = 0;
    object_l.num_face = 0;    
    object_l.pos = 0;
    fseek(file,0,SEEK_SET);
    loadfile3ds(file,size);
    object->vertex = object_l.vertex;
    object->uvmap = object_l.uvmap;
    object->indices = object_l.indices;
    object->num_face = object_l.num_face;
    object->num_vertex = object_l.num_vertex;
    object->num_uvmap = object_l.num_vertex;
    fclose(file);
    return object;
}
