/*  load ime file (interpolated mesh)
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <malloc.h>

static struct {
    int header;
    int num_frame;
    int num_vertex;
    int offset_vertex;
    int num_uvmap;
    int offset_uvmap;
    int num_face;
    int offset_indices;
} header_l;

static void calcnormals(float *vertex,float *normal,short *indices) {
    float a[3],b[3],n[3],length,ilength;
    int i,j,k,l;
    for(i = 0; i < header_l.num_vertex * 3; i++) normal[i] = 0;
    for(i = 0; i < header_l.num_face * 3; i += 3) {
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
    for(i = 0, j = 0; i < header_l.num_vertex; i++, j += 3) {
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

static void createmesh(float *vertex,float *normal,float *uvmap,short *indices,float *object) {
    int i,j,k,l;
    for(i = 0; i < header_l.num_face; i++)
        for(j = 0; j < 3; j++) {
            k = i * 3 + j;
            l = k * 8;
            object[l + 0] = vertex[indices[k] * 3 + 0];
            object[l + 1] = vertex[indices[k] * 3 + 1];
            object[l + 2] = vertex[indices[k] * 3 + 2];
            object[l + 3] = normal[indices[k] * 3 + 0];
            object[l + 4] = normal[indices[k] * 3 + 1];
            object[l + 5] = normal[indices[k] * 3 + 2];
            object[l + 6] = uvmap[indices[k] * 2 + 0];
            object[l + 7] = uvmap[indices[k] * 2 + 1];
        }
}

static void loadime(FILE *file,float **object) {
    int i,j;
    float move[3],scale[3];
    float *vertex,*normal,*uvmap;
    short *buffer,*indices;
    vertex = (float*)malloc(sizeof(float) * header_l.num_vertex * 3);
    normal = (float*)malloc(sizeof(float) * header_l.num_vertex * 3);
    uvmap = (float*)malloc(sizeof(float) * header_l.num_uvmap * 2);
    buffer = (short*)malloc(sizeof(short) * header_l.num_face * 3 * 3);
    indices = (short*)malloc(sizeof(short) * header_l.num_face * 3);
    fseek(file,header_l.offset_indices,SEEK_SET);   // load indices
    fread(indices,sizeof(short) * header_l.num_face * 3,1,file);
    fseek(file,header_l.offset_uvmap,SEEK_SET);     // load uvmap
    fread(buffer,sizeof(short) * header_l.num_uvmap * 2,1,file);
    for(i = 0; i < header_l.num_uvmap * 2; i += 2) {
        uvmap[i + 0] = (float)buffer[i + 0] / 32767.0;
        uvmap[i + 1] = (float)buffer[i + 1] / 32767.0;
    }
    fseek(file,header_l.offset_vertex,SEEK_SET);    // load vertex and create mesh
    for(i = 0; i < header_l.num_frame; i++) {
        fread(move,sizeof(float),3,file);
        fread(scale,sizeof(float),3,file);
        fread(buffer,sizeof(short) * header_l.num_vertex * 3,1,file);
        for(j = 0; j < header_l.num_vertex * 3; j += 3) {
            vertex[j + 0] = (float)buffer[j + 0] / 32767.0 * scale[0] + move[0];
            vertex[j + 1] = (float)buffer[j + 1] / 32767.0 * scale[1] + move[1];
            vertex[j + 2] = (float)buffer[j + 2] / 32767.0 * scale[2] + move[2];
        }
        calcnormals(vertex,normal,indices);
        createmesh(vertex,normal,uvmap,indices,object[i]);
    }
    free(indices);
    free(buffer);
    free(uvmap);
    free(normal);
    free(vertex);
}

float **LoadIME(char *name,int *num_vertex,int *num_frame) {
    int i;
    float **object;
    FILE *file;
    file = fopen(name,"rb");
    if(!file) return NULL;
    fread(&header_l,sizeof(header_l),1,file);
    object = (float**)malloc(sizeof(float*) * header_l.num_frame);
    if(!object) {
        fclose(file);
        return NULL;
    }
    for(i = 0; i < header_l.num_frame; i++) {
        object[i] = (float*)malloc(sizeof(float) * header_l.num_face * 8 * 3);
        if(!object[i]) {
            fclose(file);
            return NULL;
        }
    }
    loadime(file,object);
    fclose(file);
    *num_vertex = header_l.num_face * 3;
    *num_frame = header_l.num_frame;
    return object;
}
