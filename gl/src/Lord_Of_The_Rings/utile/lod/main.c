#include <stdio.h>
#include <malloc.h>

#define LAND_NODE_SIZE 17

void lod0_create(unsigned short *indices,int *num_indices);
void lod1_create(unsigned short *indices,int *num_indices);

void fprint_indices(char *name,unsigned short *indices,int num_indices,int lod) {
    int i,j,k;
    FILE *file;
    file = fopen(name,"w");
    fprintf(file,"unsigned short indices_lod%u[%u] = {\n",lod,num_indices);
    for(j = 0; j <= num_indices / 16; j++) {
        fprintf(file,"    ");
        for(i = 0; i < 16; i++) {
            k = j * 16 + i;
            if(k < num_indices) fprintf(file,"%u, ",indices[k]);
        }
        fseek(file,-1,SEEK_CUR);
        fprintf(file,"\n");
    }
    fseek(file,-2,SEEK_CUR);
    fprintf(file," };\n");
    fclose(file);
}

int main(int ergc,char **argc) {
    int num_indices;
    unsigned short *indices;
    indices = (unsigned short*)malloc(sizeof(unsigned short) * (LAND_NODE_SIZE - 1) * (LAND_NODE_SIZE - 1) * 6);
    if(!indices) return 1;
    lod0_create(indices,&num_indices);
    fprint_indices("lod0.c",indices,num_indices,0);
    lod1_create(indices,&num_indices);
    fprint_indices("lod1.c",indices,num_indices,1);
    return 0;
}

void lod0_create(unsigned short *indices,int *num_indices) {
    int i,j,k,l;
    for(j = 0, k = 0, l = 0; j < LAND_NODE_SIZE - 1; j++)
        for(i = 0; i < LAND_NODE_SIZE - 1; i++, k += 6) {
            l = LAND_NODE_SIZE * j + i;
            indices[k + 0] = l;
            indices[k + 1] = l + 1;
            indices[k + 2] = l + LAND_NODE_SIZE;
            indices[k + 3] = l + LAND_NODE_SIZE + 1;
            indices[k + 4] = l + LAND_NODE_SIZE;
            indices[k + 5] = l + 1;
        }
    *num_indices = k;
}

void lod1_create(unsigned short *indices,int *num_indices) {
    int i,j,k,l;
    for(j = 0, k = 0, l = 0; j < LAND_NODE_SIZE - 1; j += 2)
        for(i = 0; i < LAND_NODE_SIZE - 1; i += 2) {
            l = LAND_NODE_SIZE * j + i;
            if(i != 0 && i != LAND_NODE_SIZE - 3 && j != 0 && j != LAND_NODE_SIZE - 3) {
                indices[k + 0] = l;
                indices[k + 1] = l + 2;
                indices[k + 2] = l + LAND_NODE_SIZE * 2;
                indices[k + 3] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 4] = l + LAND_NODE_SIZE * 2;
                indices[k + 5] = l + 2;
                k += 6;
            } else if(j == 0 && i != 0 && i != LAND_NODE_SIZE - 3) {
                indices[k + 0] = l;
                indices[k + 1] = l + 1;
                indices[k + 2] = l + LAND_NODE_SIZE * 2;
                indices[k + 3] = l + 1;
                indices[k + 4] = l + 2;
                indices[k + 5] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 6] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 7] = l + LAND_NODE_SIZE * 2;
                indices[k + 8] = l + 1;
                k += 9;
            } else if(j == LAND_NODE_SIZE - 3 && i != 0 && i != LAND_NODE_SIZE - 3) {
                indices[k + 0] = l;
                indices[k + 1] = l + LAND_NODE_SIZE * 2 + 1;
                indices[k + 2] = l + LAND_NODE_SIZE * 2;
                indices[k + 3] = l + 2;
                indices[k + 4] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 5] = l + LAND_NODE_SIZE * 2 + 1;
                indices[k + 6] = l;
                indices[k + 7] = l + 2;
                indices[k + 8] = l + LAND_NODE_SIZE * 2 + 1;
                k += 9;
            } else if(i == 0 && j != 0 && j != LAND_NODE_SIZE - 3) {
                indices[k + 0] = l;
                indices[k + 1] = l + 2;
                indices[k + 2] = l + LAND_NODE_SIZE * 1;
                indices[k + 3] = l + LAND_NODE_SIZE * 1;
                indices[k + 4] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 5] = l + LAND_NODE_SIZE * 2;
                indices[k + 6] = l + 2;
                indices[k + 7] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 8] = l + LAND_NODE_SIZE * 1;
                k += 9;
            } else if(i == LAND_NODE_SIZE - 3 && j != 0 && j != LAND_NODE_SIZE - 3) {
                indices[k + 0] = l;
                indices[k + 1] = l + 2;
                indices[k + 2] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 3] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 4] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 5] = l + LAND_NODE_SIZE * 2;
                indices[k + 6] = l;
                indices[k + 7] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 8] = l + LAND_NODE_SIZE * 2;
                k += 9;
            } else if(i == 0 && j == 0) {
                indices[k + 0] = l;
                indices[k + 1] = l + 1;
                indices[k + 2] = l + LAND_NODE_SIZE * 1;
                indices[k + 3] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 4] = l + LAND_NODE_SIZE * 1;
                indices[k + 5] = l + 1;
                indices[k + 6] = l + 1;
                indices[k + 7] = l + 2;
                indices[k + 8] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 9] = l + LAND_NODE_SIZE * 1;
                indices[k + 10] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 11] = l + LAND_NODE_SIZE * 2;
                indices[k + 12] = l + 2;
                indices[k + 13] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 14] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 15] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 16] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 17] = l + LAND_NODE_SIZE * 2;
                k += 18;
            } else if(i == LAND_NODE_SIZE - 3 && j == 0) {
                indices[k + 0] = l + 1;
                indices[k + 1] = l + 2;
                indices[k + 2] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 3] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 4] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 5] = l + 2;
                indices[k + 6] = l;
                indices[k + 7] = l + 1;
                indices[k + 8] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 9] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 10] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 11] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 12] = l;
                indices[k + 13] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 14] = l + LAND_NODE_SIZE * 2;
                indices[k + 15] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 16] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 17] = l + LAND_NODE_SIZE * 2;
                k += 18;
            } else if(i == 0 && j == LAND_NODE_SIZE - 3) {
                indices[k + 0] = l;
                indices[k + 1] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 2] = l + LAND_NODE_SIZE * 1;
                indices[k + 3] = l + LAND_NODE_SIZE * 1;
                indices[k + 4] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 5] = l + LAND_NODE_SIZE * 2;
                indices[k + 6] = l + LAND_NODE_SIZE * 2;
                indices[k + 7] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 8] = l + LAND_NODE_SIZE * 2 + 1;
                indices[k + 9] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 10] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 11] = l + LAND_NODE_SIZE * 2 + 1;
                indices[k + 12] = l;
                indices[k + 13] = l + 2;
                indices[k + 14] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 15] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 16] = l + 2;
                indices[k + 17] = l + LAND_NODE_SIZE * 2 + 2;
                k += 18;
            } else if(i == LAND_NODE_SIZE - 3 && j == LAND_NODE_SIZE - 3) {
                indices[k + 0] = l + 2;
                indices[k + 1] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 2] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 3] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 4] = l + LAND_NODE_SIZE * 2 + 1;
                indices[k + 5] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 6] = l + LAND_NODE_SIZE * 1 + 2;
                indices[k + 7] = l + LAND_NODE_SIZE * 2 + 2;
                indices[k + 8] = l + LAND_NODE_SIZE * 2 + 1;
                indices[k + 9] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 10] = l + LAND_NODE_SIZE * 2 + 1;
                indices[k + 11] = l + LAND_NODE_SIZE * 2;
                indices[k + 12] = l;
                indices[k + 13] = l + 2;
                indices[k + 14] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 15] = l;
                indices[k + 16] = l + LAND_NODE_SIZE * 1 + 1;
                indices[k + 17] = l + LAND_NODE_SIZE * 2;
                k += 18;
            }
        }
    *num_indices = k;
}
