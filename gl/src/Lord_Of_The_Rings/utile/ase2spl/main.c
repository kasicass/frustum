/*  import spline path (spl) from 3dsmax asciiexport 200 (ase)
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

void load_pos(FILE *file,float *pos) {
    fscanf(file,"%f %f %f",&pos[0],&pos[1],&pos[2]);
}

int load_int(FILE *file) {
    int val;
    fscanf(file,"%d",&val);
    return val;
}

int ase2spl(char *asename,char *splname) {
    char buffer[128];
    float *vertex = NULL;
    int i,num_vertex = 0;
    FILE *file;
    file = fopen(asename,"r");
    if(!file) {
        printf("error open ase file\n");
        return 0;
    }
    while(fscanf(file,"%s",buffer) != EOF) {
        if(!strcmp(buffer,"*SHAPE_VERTEXCOUNT")) {
            num_vertex = load_int(file);
            vertex = (float*)malloc(sizeof(float) * 3 * num_vertex);
            if(!vertex) {
                printf("error get memory for vertex buffer\n");
                return 0;
            }
            memset(vertex,0,sizeof(float) * 3 * num_vertex);
        } else if(!strcmp(buffer,"*SHAPE_VERTEX_KNOT") ||
                  !strcmp(buffer,"*SHAPE_VERTEX_INTERP")) {
            if(!vertex) break;
            i = load_int(file);
            load_pos(file,&vertex[i * 3]);
            if(i == num_vertex - 1) break;
        }
    }
    fclose(file);
    if(!vertex || !num_vertex) {
        printf("error load ase file spline not finded\n");
        return 0;
    }
    file = fopen(splname,"w");
    if(!file) {
        printf("error create spl file\n");
        return 0;
    }
    for(i = 0; i < num_vertex * 3; i += 3)
        fprintf(file,"%.4f %.4f %.4f\n",vertex[i + 0],vertex[i + 1],vertex[i + 2]);
    free(vertex);
    fclose(file);
    return 1;
}

int main(int argc,char **argv) {
    int i;
    char asename[256],splname[256],*ptr;
    if(argc == 1) {
        printf("import spline path (spl) from 3dsmax asciiexport 200 (ase)\n");
        return 0;
    }
    for(i = 1; i < argc; i++) {
        strcpy(asename,argv[i]);
        strcpy(splname,argv[i]);
        ptr = strstr(splname,".ase");
        if(ptr) strcpy(ptr,".spl");
        else strcat(splname,".spl");
        if(ase2spl(asename,splname)) printf("%s -> %s\n",asename,splname);
    }
    return 0;
}
