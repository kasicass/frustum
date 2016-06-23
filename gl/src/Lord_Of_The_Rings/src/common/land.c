/*  landscape 3d engine
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "mathlib.h"
#include "camera.h"
#include "land.h"
#include "loadtga.h"
#include "loadjpeg.h"

#include "system.h"

land_node_vertex_t *land_create_mesh(land_t *land,land_config_t *config) {
    unsigned char *heightmap;
    int i,j,k,l,width,height;
    land_node_vertex_t *vertex;
    float p00[3],p10[3],p01[3],v10[3],v01[3],n[3];
    heightmap = NULL;
    if(strstr(config->heightmap,".tga") || strstr(config->heightmap,".TGA"))
        heightmap = LoadTGA(config->heightmap,&width,&height);
    else if(strstr(config->heightmap,".jpg") || strstr(config->heightmap,".JPG"))
        heightmap = LoadJPEG(config->heightmap,&width,&height);
    if(!heightmap) return NULL;
    vertex = (land_node_vertex_t*)malloc(sizeof(land_node_vertex_t) * width * height);
    if(!vertex) return NULL;
    land->vertex = (land_vertex_t*)malloc(sizeof(land_vertex_t) * width * height);
    if(!land->vertex) return NULL;
    for(j = 0, k = 0, l = 0; j < height; j++)   // create mesh
        for(i = 0; i < width; i++, k++, l += 4) {
            VectorSet((float)i * config->step,(float)j * config->step,(float)heightmap[l] / 255.0 * config->altitude,vertex[k].v);
            VectorSet(0,0,0,vertex[k].n);
            vertex[k].t0[0] = vertex[k].t1[0] = (float)i / (float)(width - 1);
            vertex[k].t0[1] = vertex[k].t1[1] = (float)j / (float)(height - 1);
            vertex[k].t0[0] *= (float)config->num_base;
            vertex[k].t0[1] *= (float)config->num_base;
            vertex[k].t1[0] *= (float)config->num_detail;
            vertex[k].t1[1] *= (float)config->num_detail;
            VectorCopy(vertex[k].v,land->vertex[k].v);
        }
    for(j = 0, k = 0; j < height - 1; j++, k++) // calculate normals
        for(i = 0; i < width - 1; i++, k++) {
            VectorCopy(vertex[k].v,p00);
            VectorCopy(vertex[k + 1].v,p10);
            VectorCopy(vertex[k + width].v,p01);
            VectorSub(p10,p00,v10);
            VectorSub(p01,p00,v01);
            VectorCrossProduct(v10,v01,n);
            VectorNormalize(n,n);
            VectorAdd(vertex[k].n,n,vertex[k].n);
            VectorAdd(vertex[k + 1].n,n,vertex[k + 1].n);
            VectorAdd(vertex[k + width].n,n,vertex[k + width].n);
            VectorCopy(vertex[k + width + 1].v,p00);
            VectorCopy(vertex[k + width].v,p10);
            VectorCopy(vertex[k + 1].v,p01);
            VectorSub(p10,p00,v10);
            VectorSub(p01,p00,v01);
            VectorCrossProduct(v10,v01,n);
            VectorNormalize(n,n);
            VectorAdd(vertex[k + width + 1].n,n,vertex[k + width + 1].n);
            VectorAdd(vertex[k + width].n,n,vertex[k + width].n);
            VectorAdd(vertex[k + 1].n,n,vertex[k + 1].n);
        }
    for(i = 0; i < width * height; i++) // normalize normals
        VectorNormalize(vertex[i].n,vertex[i].n);
    land->width = width;
    land->height = height;
    land->step = config->step;
    land->lod = config->lod;
    free(heightmap);
    return vertex;
}

int land_create_texture(land_t *land,land_config_t *config) {
    int i,j,k,l,m,width,height,imagewidth,imageheight,offset;
    unsigned char *data,*image;
    land->material = glGenLists(1); // create material
    glNewList(land->material,GL_COMPILE);
    glMaterialfv(GL_FRONT,GL_AMBIENT,config->ambient);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,config->diffuse);
    glMaterialfv(GL_FRONT,GL_SPECULAR,config->specular);
    glEndList();
    land->texture = (int*)malloc(sizeof(int) * config->num_base * config->num_base + 1);
    if(!land->texture) return 0;
    memset(land->texture,0,sizeof(int) * config->num_base * config->num_base + 1);    
    land->num_texture = config->num_base * config->num_base + 1;
    data = NULL;
    if(strstr(config->detail,".jpg")) data = LoadJPEG(config->detail,&width,&height);
    else if(strstr(config->detail,".tga")) data = LoadTGA(config->detail,&width,&height);
    if(data) {  // create detail texture
        glGenTextures(1,&land->texture[0]);
        glBindTexture(GL_TEXTURE_2D,land->texture[0]);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }    
    data = NULL;
    if(strstr(config->base,".jpg")) data = LoadJPEG(config->base,&width,&height);
    else if(strstr(config->base,".tga")) data = LoadTGA(config->base,&width,&height);
    if(data) {
        imagewidth = width / config->num_base;
        imageheight = height / config->num_base;
        image = (unsigned char*)malloc(imagewidth * imageheight * 4);
        if(!image) return 0;
        for(j = 0; j < config->num_base; j++)   // create base textures
            for(i = 0; i < config->num_base; i++) {
                for(l = 0, m = 0; l < imageheight; l++) {
                    offset = (width * (imageheight * j + l) + imagewidth * i) * 4;
                    for(k = 0; k < imagewidth * 4; k += 4, m += 4) {
                        image[m + 0] = data[offset + k + 0];
                        image[m + 1] = data[offset + k + 1];
                        image[m + 2] = data[offset + k + 2];
                        image[m + 3] = data[offset + k + 3];
                    }
                }
                glGenTextures(1,&land->texture[config->num_base * j + i + 1]);
                glBindTexture(GL_TEXTURE_2D,land->texture[config->num_base * j + i + 1]);
                glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,config->texture_mode);
                glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,config->texture_mode);
                glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
                glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
                gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,imagewidth,imageheight,GL_RGBA,GL_UNSIGNED_BYTE,image);
            }
        free(image);
        free(data);
    }
    return 1;
}

void land_node_size(land_node_t *node) {
    int i,j;
    VectorSet(1000000,1000000,1000000,node->min);
    VectorSet(-1000000,-1000000,-1000000,node->max);
    for(i = 0; i < node->width * node->height; i++)
        for(j = 0; j < 3; j++) {
            if(node->vertex[i].v[j] < node->min[j]) node->min[j] = node->vertex[i].v[j];
            if(node->vertex[i].v[j] > node->max[j]) node->max[j] = node->vertex[i].v[j];
        }
}

int land_create_div_plane(land_node_t *node) {
    float point[3];
    if(node->width <= LAND_NODE_SIZE && node->height <= LAND_NODE_SIZE) return 0;
    VectorSet((node->min[0] + node->max[0]) / 2.0,(node->min[1] + node->max[1]) / 2.0,0,point);
    VectorSet(0,0,0,node->plane);
    if(node->width > node->height) node->plane[0] = 1;
    else node->plane[1] = 1;
    node->plane[3] = -VectorDotProduct(node->plane,point);
    return 1;
}

int land_div_node(land_node_t *node) {
    int i,j,k,width,height;
    node->left = (land_node_t*)malloc(sizeof(land_node_t));
    if(!node->left) return 0;
    memset(node->left,0,sizeof(land_node_t));
    node->right = (land_node_t*)malloc(sizeof(land_node_t));
    if(!node->right) return 0;
    memset(node->right,0,sizeof(land_node_t));
    if(node->plane[0] == 1) {
        width = node->width / 2 + 1;
        height = node->height;
    } else {
        width = node->width;
        height = node->height / 2 + 1;
    }
    node->left->vertex = (land_node_vertex_t*)malloc(sizeof(land_node_vertex_t) * width * height);
    if(!node->left->vertex) return 0;
    node->right->vertex = (land_node_vertex_t*)malloc(sizeof(land_node_vertex_t) * width * height);
    if(!node->right->vertex) return 0;
    if(node->plane[0] == 1) {
        for(j = 0, k = 0; j < node->height; j++)
            for(i = 0; i < node->width; i++, k++) {
                if(i < width) node->left->vertex[width * j + i] = node->vertex[k];
                if(i >= width - 1) node->right->vertex[width * j + i - width + 1] = node->vertex[k];
            }
    } else {
        for(j = 0, k = 0; j < node->height; j++)
            for(i = 0; i < node->width; i++, k++) {
                if(j < height) node->left->vertex[width * j + i] = node->vertex[k];
                if(j >= height - 1) node->right->vertex[width * (j - height + 1) + i] = node->vertex[k];
            }
    }
    node->left->width = node->right->width = width;
    node->left->height = node->right->height = height;
    return 1;
}

void land_create_bsp_tree_process(land_node_t *node) {
    land_node_size(node);
    if(!land_create_div_plane(node)) return;
    land_div_node(node);
    free(node->vertex);
    node->vertex = 0;
    node->width = node->height = 0;
    land_create_bsp_tree_process(node->left);   // left
    land_create_bsp_tree_process(node->right);  // right
}

void land_create_bsp_tree_texture_process(land_node_t *node,int *texture,int num_base) {
    int i,s,t;
    if(!node->left && !node->right) {
        node->base = texture[num_base * (int)node->vertex->t0[1] + (int)node->vertex->t0[0] + 1];
        node->detail = texture[0];
        s = (int)node->vertex->t0[0];
        t = (int)node->vertex->t0[1];
        for(i = 0; i < node->width * node->height; i++) {
            node->vertex[i].t0[0] -= s;
            node->vertex[i].t0[1] -= t;
        }
        return;
    }
    land_create_bsp_tree_texture_process(node->left,texture,num_base);
    land_create_bsp_tree_texture_process(node->right,texture,num_base);
}

void land_create_bsp_tree_indices_process(land_node_t *node,float lod) {
    if(!node->left && !node->right) {
        node->lod = lod;
        node->indices[0] = indices_lod0;
        node->indices[1] = indices_lod1;
        node->num_indices[0] = LAND_LOD0_NUM_INDICES;
        node->num_indices[1] = LAND_LOD1_NUM_INDICES;
        return;
    }
    land_create_bsp_tree_indices_process(node->left,lod);
    land_create_bsp_tree_indices_process(node->right,lod);
}

int land_create_bsp_tree(land_t *land,land_config_t *config,land_node_vertex_t *vertex) {
    land->root = (land_node_t*)malloc(sizeof(land_node_t));
    if(!land->root) return 0;
    memset(land->root,0,sizeof(land_node_t));
    land->root->vertex = vertex;
    land->root->width = land->width;
    land->root->height = land->height;
    land_create_bsp_tree_process(land->root);
    land_create_bsp_tree_texture_process(land->root,land->texture,config->num_base);
    land_create_bsp_tree_indices_process(land->root,config->lod);
    return 1;
}

land_t *land_create(land_config_t *config) {
    land_t *land;
    land_node_vertex_t *vertex;
    land = (land_t*)malloc(sizeof(land_t));
    if(!land) return NULL;
    memset(land,0,sizeof(land_t));
    vertex = land_create_mesh(land,config);
    if(!vertex) return NULL;
    if(!land_create_texture(land,config)) return NULL;
    if(!land_create_bsp_tree(land,config,vertex)) return NULL;
    return land;
}

void land_free_node_process(land_node_t *node) {
    if(!node->left && !node->right) {
        free(node->vertex); // free vertex
        free(node);
        return;
    }
    land_free_node_process(node->left);
    land_free_node_process(node->right);
    free(node);
}

void land_free(land_t *land) {
    int i;
    for(i = 0; i < land->num_texture; i++)  // free texture
        glDeleteTextures(1,&land->texture[i]);
    free(land->texture);
    free(land->vertex); // free vertex
    land_free_node_process(land->root);
    free(land);
}

float land_height(land_t *land,float *point) {
    int i,j,k;
    float x,y,dot0,dot1,p00[3],p10[3],p01[3],v10[3],v01[3],point0[3],point1[3],plane[4];
    x = point[0] / land->step;
    y = point[1] / land->step;
    i = (int)x;
    j = (int)y;
    if(i < 0) i = 0;
    else if(i > land->width - 2) i = land->width - 2;
    if(j < 0) j = 0;
    else if(j > land->height - 2) j = land->height - 2;
    k = land->width * j + i;
    if(x + y >= 1 + (int)x + (int)y) {  // 1st or 2nd triangle
        VectorCopy(land->vertex[k + land->width + 1].v,p00);
        VectorCopy(land->vertex[k + land->width].v,p10);
        VectorCopy(land->vertex[k + 1].v,p01);
    } else {
        VectorCopy(land->vertex[k].v,p00);
        VectorCopy(land->vertex[k + 1].v,p10);
        VectorCopy(land->vertex[k + land->width].v,p01);
    }
    VectorSub(p10,p00,v10);
    VectorSub(p01,p00,v01);
    VectorCrossProduct(v10,v01,plane);
    plane[3] = -VectorDotProduct(plane,p00);
    VectorCopy(point,point0);
    VectorCopy(point,point1);
    point0[2] = 0;
    point1[2] = 1;
    dot0 = -VectorDotProduct(plane,point0);
    dot1 = -VectorDotProduct(plane,point1);
    return (point0[2] + (point1[2] - point0[2]) *
        (plane[3] - dot0) / (dot1 - dot0));
}

void land_crossline(land_t *land,float *line_start,float *line_end,float *point) {
    int tryes;
    float scale,height,old_height,dir[3];
    VectorSub(line_end,line_start,dir);
    VectorNormalize(dir,dir);
    old_height = line_end[2] - land_height(land,line_end);
    scale = old_height;
    VectorScale(dir,scale,point);
    VectorAdd(point,line_end,point);
    height = point[2] - land_height(land,point);
    tryes = LAND_CROSS_TRYES;
    while(fabs(height) + fabs(old_height) > land->step && tryes--) {
        if(height > 0) scale += (height + fabs(old_height)) / 2.0;
        else scale += (height - fabs(old_height)) / 2.0;
        old_height = height;
        VectorScale(dir,scale,point);
        VectorAdd(point,line_end,point);
        height = point[2] - land_height(land,point);
    }
}

void land_render_node(land_node_t *node,int lod) {
    glBindTexture(GL_TEXTURE_2D,node->base);
    glVertexPointer(3,GL_FLOAT,sizeof(land_node_vertex_t),node->vertex->v);
    glNormalPointer(GL_FLOAT,sizeof(land_node_vertex_t),node->vertex->n);
    glTexCoordPointer(2,GL_FLOAT,sizeof(land_node_vertex_t),node->vertex->t0);
    if(node->detail) {  // if detail texture define
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glClientActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,node->detail);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2,GL_FLOAT,sizeof(land_node_vertex_t),node->vertex->t1);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
        glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_ADD_SIGNED_EXT);
    }
    glDrawElements(GL_TRIANGLES,node->num_indices[lod],GL_UNSIGNED_SHORT,node->indices[lod]);
    if(node->detail) {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glClientActiveTextureARB(GL_TEXTURE0_ARB);
    }
}

void land_render_process(land_node_t *node,camera_t *camera) {
    int lod;
    float dist,sub[3];
    if(!node->left && !node->right) {
        if(camera_check_box(camera,node->min,node->max)) {
            VectorAdd(node->min,node->max,sub);
            VectorScale(sub,0.5,sub);
            VectorSub(sub,camera->pos,sub);
            dist = sqrt(sub[0] * sub[0] + sub[1] * sub[1] + sub[2] * sub[2]);
            if(dist < node->lod) lod = 0;
            else lod = 1;
            land_render_node(node,lod);
        }
        return;
    }
    if(-VectorDotProduct(camera->pos,node->plane) > node->plane[3]) {
        land_render_process(node->left,camera);
        if(camera_check_box(camera,node->right->min,node->right->max))
            land_render_process(node->right,camera);
    } else {
        land_render_process(node->right,camera);
        if(camera_check_box(camera,node->left->min,node->left->max))
            land_render_process(node->left,camera);
    }
}

void land_render(land_t *land,camera_t *camera) {
    glCallList(land->material);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    land_render_process(land->root,camera);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void land_dynamiclight(land_t *land,float *pos,float *color,float radius) {
    int i,j,k,x0,x1,y0,y1;
    float iradius,height,intensity,S,s,t,delta;
    height = pos[2] - land_height(land,pos);
    intensity = 1.0 - height / radius;
    if(intensity < 0) return;
    radius = sqrt(radius * radius - height * height);
    iradius = 0.5 / radius;
    x0 = (int)((pos[0] - radius) / land->step);
    y0 = (int)((pos[1] - radius) / land->step);
    x1 = (int)((pos[0] + radius) / land->step) + 1;
    y1 = (int)((pos[1] + radius) / land->step) + 1;
    if(x0 < 0) x0 = 0;
    if(x0 > land->width - 2) x0 = land->width - 2;
    if(y0 < 0) y0 = 0;
    if(y0 > land->height - 2) y0 = land->height - 2;
    if(x1 < 0) x1 = 0;
    if(x1 > land->width - 2) x1 = land->width - 2;
    if(y1 < 0) y1 = 0;
    if(y1 > land->height - 2) y1 = land->height - 2;
    S = (land->vertex[land->width * y0 + x0].v[0] - pos[0]) * iradius + 0.5;
    t = (land->vertex[land->width * y0 + x0].v[1] - pos[1]) * iradius + 0.5;
    delta = land->step * iradius;
    glColor3f(color[0] * intensity,color[1] * intensity,color[2] * intensity);
    glBegin(GL_TRIANGLES);
    for(j = y0; j < y1; j++, t += delta)
        for(i = x0, s = S; i < x1; i++, s += delta) {
            k = land->width * j + i;
            glTexCoord2f(s,t);
            glVertex3fv(land->vertex[k].v);
            glTexCoord2f(s + delta,t);
            glVertex3fv(land->vertex[k + 1].v);
            glTexCoord2f(s,t + delta);
            glVertex3fv(land->vertex[k + land->width].v);
            glTexCoord2f(s + delta,t + delta);
            glVertex3fv(land->vertex[k + land->width + 1].v);
            glTexCoord2f(s,t + delta);
            glVertex3fv(land->vertex[k + land->width].v);
            glTexCoord2f(s + delta,t);
            glVertex3fv(land->vertex[k + 1].v);
        }
    glEnd();
}

void land_dynamicshadow(land_t *land,float *light,thing_t *thing) {
    int i,j,k,x0,x1,y0,y1;
    float dir[3],up[3],dx[3],dy[3],blight[3],bpos[3],point[3],min[3],max[3];
    VectorSub(thing->center,light,dir);
    VectorNormalize(dir,dir);
    VectorSet(0,0,1,up);
    VectorCrossProduct(up,dir,dx);
    VectorCrossProduct(dir,dx,dy);
    VectorNormalize(dx,dx);
    VectorNormalize(dy,dy);
    VectorScale(dx,thing->radius,dx);
    VectorScale(dy,thing->radius,dy);
    VectorSet(1000000,1000000,1000000,min);
    VectorSet(-1000000,-1000000,-1000000,max);
    for(i = 0; i < 4; i++) {
        if(i == 0 || i == 1) {
            VectorAdd(light,dx,blight);
            VectorAdd(thing->center,dx,bpos);
        } else {
            VectorSub(light,dx,blight);
            VectorSub(thing->center,dx,bpos);
        }
        if(i == 0 || i == 2) {
            VectorSub(blight,dy,blight);
            VectorSub(bpos,dy,bpos);
        } else {
            VectorAdd(blight,dy,blight);
            VectorAdd(bpos,dy,bpos);
        }
        land_crossline(land,blight,bpos,point);
        for(j = 0; j < 2; j++) {
            if(point[j] < min[j]) min[j] = point[j];
            if(point[j] > max[j]) max[j] = point[j];
        }
    }
    x0 = (int)(min[0] / land->step);
    y0 = (int)(min[1] / land->step);
    x1 = (int)(max[0] / land->step) + 1;
    y1 = (int)(max[1] / land->step) + 1;
    if(x0 < 0) x0 = 0;
    if(x0 > land->width - 1) x0 = land->width - 1;
    if(y0 < 0) y0 = 0;
    if(y0 > land->height - 1) y0 = land->height - 1;
    if(x1 < 0) x1 = 0;
    if(x1 > land->width - 1) x1 = land->width - 1;
    if(y1 < 0) y1 = 0;
    if(y1 > land->height - 1) y1 = land->height - 1;
    glBegin(GL_TRIANGLES);
    for(j = y0; j < y1; j++)
        for(i = x0; i < x1; i++) {
            k = land->width * j + i;
            glVertex3fv(land->vertex[k].v);
            glVertex3fv(land->vertex[k + 1].v);
            glVertex3fv(land->vertex[k + land->width].v);
            glVertex3fv(land->vertex[k + land->width + 1].v);
            glVertex3fv(land->vertex[k + land->width].v);
            glVertex3fv(land->vertex[k + 1].v);
        }
    glEnd();
}
