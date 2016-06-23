/*  load 3ds file
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <malloc.h>

#define CHUNK_MAIN      0x4D4D   
#define CHUNK_OBJMESH   0x3D3D
#define CHUNK_OBJBLOCK  0x4000
#define CHUNK_TRIMESH   0x4100
#define CHUNK_VERTLIST  0x4110
#define CHUNK_FACELIST  0x4120
#define CHUNK_MAPLIST   0x4140
 
static struct {
    int num_vertex;
    int num_uvmap;
    int num_face;
    float *vertex;
    float *normal;
    float *uvmap;
    int *indices;
    int pos;
} object_l;

static int skeepstr(FILE *file) {
    int i = 0;
    char c = 1;
    while((c = fgetc(file)) != 0x00) i++;
    object_l.pos += ++i;
    return i;
}

static void statfile3ds(FILE *file,int size) {
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

static void loadfile3ds(FILE *file,int size) {
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
                    j = i * 6;
                    fread(&object_l.indices[j],2,1,file);
                    fread(&object_l.indices[j + 1],2,1,file);
                    fread(&object_l.indices[j + 2],2,1,file);
                    object_l.indices[j] += startvertex;
                    object_l.indices[j + 1] += startvertex;
                    object_l.indices[j + 2] += startvertex;
                    object_l.indices[j + 3] = object_l.indices[j];
                    object_l.indices[j + 4] = object_l.indices[j + 1];
                    object_l.indices[j + 5] = object_l.indices[j + 2];
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

static void optimizemesh(void) {
    int *indices = (int*)malloc(sizeof(int) * object_l.num_vertex);
    int i,j,num_vertex;
    for(i = 0, num_vertex = 0; i < object_l.num_vertex; i++) {
        j = num_vertex - 1;
        while(j >= 0 && (object_l.vertex[i * 3] != object_l.vertex[j * 3] ||
                         object_l.vertex[i * 3 + 1] != object_l.vertex[j * 3 + 1] ||
                         object_l.vertex[i * 3 + 2] != object_l.vertex[j * 3 + 2])) j--;
        if(j < 0) {
            indices[i] = num_vertex;
            object_l.vertex[num_vertex * 3] = object_l.vertex[i * 3];
            object_l.vertex[num_vertex * 3 + 1] = object_l.vertex[i * 3 + 1];
            object_l.vertex[num_vertex * 3 + 2] = object_l.vertex[i * 3 + 2];
            num_vertex++;
        }
        else indices[i] = j;
    }
    for(i = 0; i < object_l.num_face * 6; i += 6) {
        object_l.indices[i] = indices[object_l.indices[i]];
        object_l.indices[i + 1] = indices[object_l.indices[i + 1]];
        object_l.indices[i + 2] = indices[object_l.indices[i + 2]];
    }
    object_l.num_vertex = num_vertex;
    free(indices);
}

static void calcnormals(void) {
    float *vertex = object_l.vertex;
    float *normal = object_l.normal;
    int *indices = object_l.indices;
    float a[3],b[3],n[3],length,ilength;
    int i,j,k,l;
    for(i = 0; i < object_l.num_vertex; i++)
        normal[i * 3] = normal[i * 3 + 1] = normal[i * 3 + 2] = 0;
    for(i = 0; i < object_l.num_face * 6; i += 6) {
        j = indices[i] * 3;
        k = indices[i + 1] * 3;
        l = indices[i + 2] * 3;
        a[0] = vertex[k] - vertex[j];
        b[0] = vertex[l] - vertex[j];
        a[1] = vertex[k + 1] - vertex[j + 1];
        b[1] = vertex[l + 1] - vertex[j + 1];
        a[2] = vertex[k + 2] - vertex[j + 2];
        b[2] = vertex[l + 2] - vertex[j + 2];
        n[0] = a[1] * b[2] - a[2] * b[1];
        n[1] = a[2] * b[0] - a[0] * b[2];
        n[2] = a[0] * b[1] - a[1] * b[0];
        length = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        if(length != 0.0) {
            ilength = 1.0 / length;
            n[0] *= ilength;
            n[1] *= ilength;
            n[2] *= ilength;
        }
        normal[j] += n[0];
        normal[k] += n[0];
        normal[l] += n[0];
        normal[j + 1] += n[1];
        normal[k + 1] += n[1];
        normal[l + 1] += n[1];
        normal[j + 2] += n[2];
        normal[k + 2] += n[2];
        normal[l + 2] += n[2];
    }
    for(i = 0, j = 0; i < object_l.num_vertex; i++, j += 3) {
        length = sqrt(normal[j] * normal[j] +
                      normal[j + 1] * normal[j + 1] +
                      normal[j + 2] * normal[j + 2]);
        if(length != 0.0) {
            ilength = 1.0 / length;
            normal[j] *= ilength;
            normal[j + 1] *= ilength;
            normal[j + 2] *= ilength;
        }
    }
}

float *Load3DS(char *name,int *num_vertex) {
    int i,j,k,l,size;
    float *vertex;
    FILE *file;
    if(!(file = fopen(name,"rb"))) return 0;
    fseek(file,0,SEEK_END);
    size = ftell(file);
    fseek(file,0,SEEK_SET); 
    object_l.num_vertex = 0;
    object_l.num_face = 0;
    object_l.pos = 0;
    statfile3ds(file,size);
    object_l.vertex = (float*)malloc(sizeof(float) * object_l.num_vertex * 3);
    memset(object_l.vertex,0,sizeof(float) * object_l.num_vertex * 3);
    object_l.normal = (float*)malloc(sizeof(float) * object_l.num_vertex * 3);
    memset(object_l.normal,0,sizeof(float) * object_l.num_vertex * 3);
    object_l.uvmap = (float*)malloc(sizeof(float) * object_l.num_vertex * 2);
    memset(object_l.uvmap,0,sizeof(float) * object_l.num_vertex * 2);
    object_l.indices = (int*)malloc(sizeof(int) * object_l.num_face * 6);
    memset(object_l.indices,0,sizeof(int) * object_l.num_face * 6);
    fseek(file,0,SEEK_SET);
    object_l.num_vertex = 0;
    object_l.num_face = 0;    
    object_l.pos = 0;
    loadfile3ds(file,size);
    fclose(file);
    optimizemesh();
    calcnormals();
    vertex = (float*)malloc(sizeof(float) * 8 * 3 * object_l.num_face);
    for(i = 0; i < object_l.num_face; i++)
        for(j = 0; j < 3; j++) {
            k = i * 6 + j;
            l = (i * 3 + j) * 8;
            vertex[l] = object_l.vertex[object_l.indices[k] * 3];
            vertex[l + 1] = object_l.vertex[object_l.indices[k] * 3 + 1];
            vertex[l + 2] = object_l.vertex[object_l.indices[k] * 3 + 2];
            vertex[l + 3] = object_l.normal[object_l.indices[k] * 3];
            vertex[l + 4] = object_l.normal[object_l.indices[k] * 3 + 1];
            vertex[l + 5] = object_l.normal[object_l.indices[k] * 3 + 2];
            vertex[l + 6] = object_l.uvmap[object_l.indices[k + 3] * 2];
            vertex[l + 7] = object_l.uvmap[object_l.indices[k + 3] * 2 + 1];
        }
    *num_vertex = object_l.num_face * 3;
    free(object_l.vertex);
    free(object_l.normal);
    free(object_l.uvmap);
    free(object_l.indices);
    return vertex;
}
