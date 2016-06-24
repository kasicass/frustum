#include "terrain.h"

static void terrain_calc_normal(terrain_t *t) {
	int i,x,y;
	float p00[3],p10[3],p01[3],v10[3],v01[3],n[3];
	terrain_vertex_t *v = t->vertex;
	for(y = 0, i = 0; y < t->height - 1; y++, i++)
		for(x = 0; x < t->width - 1; x++, i++) {
			v_copy(v[i].xyz,p00);
			v_copy(v[i + 1].xyz,p10);
			v_copy(v[i + t->width].xyz,p01);
			v_sub(p10,p00,v10);
			v_sub(p01,p00,v01);
			v_cross(v10,v01,n);
			v_normalize(n,n);
			v_add(v[i].normal,n,v[i].normal);
			v_add(v[i + 1].normal,n,v[i + 1].normal);
			v_add(v[i + t->width].normal,n,v[i + t->width].normal);
			v_copy(v[i + t->width + 1].xyz,p00);
			v_copy(v[i + t->width].xyz,p10);
			v_copy(v[i + 1].xyz,p01);
			v_sub(p10,p00,v10);
			v_sub(p01,p00,v01);
			v_cross(v10,v01,n);
			v_normalize(n,n);
			v_add(v[i + t->width + 1].normal,n,v[i + t->width + 1].normal);
			v_add(v[i + t->width].normal,n,v[i + t->width].normal);
			v_add(v[i + 1].normal,n,v[i + 1].normal);
		}
	for(i = 0; i < t->width * t->height; i++)
		v_normalize(v[i].normal,v[i].normal);
}

/* размер субсектора
 */
static void terrain_node_size(terrain_node_t *n) {
	int i,j;
	v_set(999999,999999,999999,n->min);
	v_set(-999999,-999999,-999999,n->max);
	for(i = 0; i < n->width * n->height; i++)
		for(j = 0; j < 3; j++) {
			if(n->vertex[i]->xyz[j] < n->min[j])
				n->min[j] = n->vertex[i]->xyz[j];
			if(n->vertex[i]->xyz[j] > n->max[j])
				n->max[j] = n->vertex[i]->xyz[j];
		}
}

/* секуща?плоскост? */
static int terrain_div_plane(terrain_node_t *n) {
	float p[3];
	if(n->width == 2 && n->height == 2) return -1;
	v_set((n->max[0] + n->min[0]) / 2.0,(n->max[1] + n->min[1]) / 2.0,0,p);
	v_set(0,0,0,n->plane);
	if(n->width > n->height) n->plane[0] = 1;
	else n->plane[1] = 1;
	n->plane[3] = -v_dot(n->plane,p);
	return 0;
}

/* делени?субсектора
 */
static void terrain_div_node(terrain_node_t *n) {
	int i,x,y,w,h;
	n->left = calloc(1,sizeof(terrain_node_t));
	n->right = calloc(1,sizeof(terrain_node_t));
	if(n->plane[0] == 1) {
		w = n->width / 2 + 1;
		h = n->height;
	} else {
		w = n->width;
		h = n->height / 2 + 1;
	}
	n->left->width = n->right->width = w;
	n->left->height = n->right->height = h;
	n->left->vertex = malloc(sizeof(terrain_vertex_t*) * w * h);
	n->right->vertex = malloc(sizeof(terrain_vertex_t*) * w * h);
	if(n->plane[0] == 1) {
		for(y = 0, i = 0; y < n->height; y++)
			for(x = 0; x < n->width; x++, i++) {
				if(x < w) n->left->vertex[w * y + x] = n->vertex[i];
				if(x > w - 2) n->right->vertex[w * y + x - w + 1] =
					n->vertex[i];
			}
	} else {
		for(y = 0, i = 0; y < n->height; y++)
			for(x = 0; x < n->width; x++, i++) {
				if(y < h) n->left->vertex[w * y + x] = n->vertex[i];
				if(y > h - 2) n->right->vertex[w * (y - h + 1) + x] =
					n->vertex[i];
			}
	}
}

/* создание дерева
 */
static void terrain_tree_process(terrain_node_t *n) {
	terrain_node_size(n);
	if(terrain_div_plane(n) == -1) return;
	terrain_div_node(n);
	free(n->vertex);
	n->vertex = NULL;
	n->width = n->height = 0;
	terrain_tree_process(n->left);
	terrain_tree_process(n->right);
}

/* начало создания дерева
 */
static void terrain_create_tree(terrain_t *t) {
	int i;
	t->root = calloc(1,sizeof(terrain_node_t));
	t->root->width = t->width;
	t->root->height = t->height;
	t->root->vertex = malloc(sizeof(terrain_vertex_t*) * t->width * t->height);
	for(i = 0; i < t->width * t->height; i++)
		t->root->vertex[i] = &t->vertex[i];
	terrain_tree_process(t->root);
}

#include "map.h"

/* загрузка из tga файл? */
terrain_t *terrain_load_tga(char *name,float step,float height) {
	int i,x,y,w,h;
	unsigned char *map;
	terrain_t *t;
	map = load_tga(name,&w,&h);
	if(!map) return NULL;
	t = malloc(sizeof(terrain_t));
	t->width = w;
	t->height = h;
	t->step = step;
	t->vertex = malloc(sizeof(terrain_vertex_t) * w * h);
	for(y = 0, i = 0; y < h; y++) {
		for(x = 0; x < w; x++, i++) {
			t->vertex[i].xyz[0] = (float)x * step;
			t->vertex[i].xyz[1] = (float)y * step;
			t->vertex[i].xyz[2] = (float)map[i << 2] / 255.0 * height;
			t->vertex[i].st[0] = (float)x / (float)w;
			t->vertex[i].st[1] = (float)y / (float)h;
			t->vertex[i].texture = my_map[i];
		}
	}
	terrain_calc_normal(t);
	terrain_create_tree(t);
	return t;
}

/* загрузка текстуры
 */
static int terrain_load_texture_file(char *name) {
	int id,width,height;
	unsigned char *data;
	data = load_tga(name,&width,&height);
	if(!data) return -1;
	/*for(id = 0; id < width * height * 4; id += 4)
		if(data[id + 3] > 1) data[id + 3] = 255;
	save_tga(name,data,width,height);*/
	glGenTextures(1,&id);
	glBindTexture(GL_TEXTURE_2D,id);
	glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,
		0,GL_RGBA,GL_UNSIGNED_BYTE,data);
	free(data);
	return id;
}

int terrain_load_texture(terrain_t *t,char *name,int n) {
	int i;
	char *ptr,buf[256],path[256];
	t->texture[n].id[0] = terrain_load_texture_file(name);
	strcpy(buf,name);
	ptr = strrchr(buf,'.');
	*ptr++ = '\0';
	for(i = 0; i < 14; i++) {
		sprintf(path,"%s_%x.%s",buf,i,ptr);
		t->texture[n].id[i + 1] = terrain_load_texture_file(path);
	}
	return 0;
}

/* ?рекурсивны?функци?буде?передаваться меньше параметров
 */ 
static terrain_t *terrain;
static camera_t *camera;

/* рендерин?субсектора
 */
static void terrain_render_triangles(terrain_node_t *n) {
	glBegin(GL_TRIANGLE_STRIP);
	glMultiTexCoord2f(GL_TEXTURE0_ARB,0,0);
	glMultiTexCoord2fv(GL_TEXTURE1_ARB,n->vertex[0]->st);
	glVertex3fv(n->vertex[0]->xyz);
	glMultiTexCoord2f(GL_TEXTURE0_ARB,1,0);
	glMultiTexCoord2fv(GL_TEXTURE1_ARB,n->vertex[1]->st);
	glVertex3fv(n->vertex[1]->xyz);
	glMultiTexCoord2f(GL_TEXTURE0_ARB,0,1);
	glMultiTexCoord2fv(GL_TEXTURE1_ARB,n->vertex[2]->st);
	glVertex3fv(n->vertex[2]->xyz);
	glMultiTexCoord2f(GL_TEXTURE0_ARB,1,1);
	glMultiTexCoord2fv(GL_TEXTURE1_ARB,n->vertex[3]->st);
	glVertex3fv(n->vertex[3]->xyz);
	glEnd();
}

static void terrain_render_node(terrain_node_t *n) {
	register int i,j,min,num;
	int texture[3][2];
	for(i = 1, j = 0, min = n->vertex[0]->texture; i < 4; i++) {
		if(n->vertex[i]->texture != min) j = -1;
		if(n->vertex[i]->texture < min) min = n->vertex[i]->texture;
	}
	if(j == 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,terrain->texture[min].id[0]);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_TEXTURE);
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,terrain->shadow_id);	
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		terrain_render_triangles(n);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_TEXTURE);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		return;
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,terrain->texture[min].id[0]);
	terrain_render_triangles(n);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0);
	for(i = 0, num = 0; i < 4; i++) {
		if(n->vertex[i]->texture != min) {
			for(j = 0; j < num; j++)
				if(n->vertex[i]->texture == texture[j][0]) break;
			if(j == num) {
				texture[j][0] = n->vertex[i]->texture;
				texture[j][1] = 1 << i;
				num++;
			} else texture[j][1] += 1 << i;
		}
	}
	for(i = 0; i < num; i++) {
		if(texture[i][1] == 1) j = 1;
		else if(texture[i][1] == 2) j = 2;
		else if(texture[i][1] == 4) j = 3;
		else if(texture[i][1] == 8) j = 4;
		else if(texture[i][1] == 5) j = 5;
		else if(texture[i][1] == 10) j = 6;
		else if(texture[i][1] == 3) j = 7;
		else if(texture[i][1] == 12) j = 8;
		else if(texture[i][1] == 9) j = 9;
		else if(texture[i][1] == 6) j = 10;
		else if(texture[i][1] == 14) j = 11;
		else if(texture[i][1] == 13) j = 12;
		else if(texture[i][1] == 11) j = 13;
		else j = 14;
		glBindTexture(GL_TEXTURE_2D,terrain->texture[texture[i][0]].id[j]);
		terrain_render_triangles(n);
	}
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,terrain->shadow_id);	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO,GL_SRC_COLOR);
	terrain_render_triangles(n);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

/* рендерин? */
static void terrain_render_process(terrain_node_t *n) {
	if(!n->left && !n->right) {
		if(camera_check_box(camera,n->min,n->max) == 0) terrain_render_node(n);
		return;
	}
	if(-v_dot(camera->pos,n->plane) > n->plane[3]) {
		terrain_render_process(n->left);
		if(camera_check_box(camera,n->right->min,n->right->max) == 0)
			terrain_render_process(n->right);
	} else {
		terrain_render_process(n->right);
		if(camera_check_box(camera,n->left->min,n->left->max) == 0)
			terrain_render_process(n->left);
	}
}

/* начало рендеринга
 */
void terrain_render(terrain_t *t,camera_t *c) {
	terrain = t;
	camera = c;
	terrain_render_process(t->root);
}

/* высота ?точк? */
float terrain_height(terrain_t *t,float *p) {
	int i,j,k;
	float x,y,p00[3],p10[3],p01[3],v10[3],v01[3];
	float dot0,dot1,p0[3],p1[3],plane[4];
	x = p[0] / t->step;
	y = p[1] / t->step;
	i = (int)x;
	j = (int)y;
	if(i < 0 || i > t->width - 2) return 0;
	if(j < 0 || j > t->height - 2) return 0;
	k = t->width * j + i;
	if(x - i + y - j < 1) {
		v_copy(t->vertex[k].xyz,p00);
		v_copy(t->vertex[k + 1].xyz,p10);
		v_copy(t->vertex[k + t->width].xyz,p01);
	} else {
		v_copy(t->vertex[k + t->width + 1].xyz,p00);
		v_copy(t->vertex[k + t->width].xyz,p10);
		v_copy(t->vertex[k + 1].xyz,p01);
	}
	v_sub(p10,p00,v10);
	v_sub(p01,p00,v01);
	v_cross(v10,v01,plane);
	plane[3] = -v_dot(plane,p00);
	v_set(p[0],p[1],0,p0);
	v_set(p[0],p[1],1,p1);
	dot0 = -v_dot(plane,p0);
	dot1 = -v_dot(plane,p1);
	return (plane[3] - dot0) / (dot1 - dot0);
}

/* нормал??точк? */
int terrain_normal(terrain_t *t,float *p,float *n) {
	int i,j,k;
	float x,y,n00[3],n10[3],n01[3],n11[3];
	x = p[0] / t->step;
	y = p[1] / t->step;
	i = (int)x;
	j = (int)y;
	if(i < 0 || i > t->width - 2) return -1;
	if(j < 0 || j > t->height - 2) return -1;
	k = t->width * j + i;
	x -= i;
	y -= j;
	v_copy(t->vertex[k].normal,n00);
	v_copy(t->vertex[k + 1].normal,n10);
	v_copy(t->vertex[k + t->width].normal,n01);
	v_copy(t->vertex[k + t->width + 1].normal,n11);
	v_scale(n00,1 - x,n00);
	v_scale(n10,x,n10);
	v_add(n00,n10,n00);
	v_normalize(n00,n00);
	v_scale(n01,1 - x,n01);
	v_scale(n11,x,n11);
	v_add(n01,n11,n01);
	v_normalize(n01,n01);
	v_scale(n00,1 - y,n00);
	v_scale(n01,y,n01);
	v_add(n00,n01,n00);
	v_normalize(n00,n);
	return 0;
}

/* попадает ли точк??участо?земл? */
int terrain_boundary(terrain_t *t,float *p) {
	if(p[0] < 0 || p[1] < 0) return -1;
	if(p[0] > (t->width - 1) * t->step) return -1;
	if(p[1] > (t->height - 1) * t->step) return -1;
	return 0;
}

/* пересекает ли лини?земл? */
int terrain_cross_line(terrain_t *t,float *l0,float *l1) {
	int prev,cur,on_terrain;
	float pos,step,len,l[3];
	len = sqrt(pow(l1[0] - l0[0],2) + pow(l1[1] - l0[1],2));
	step = 1 / len * t->step;
	prev = cur = on_terrain = 0;
	for(pos = 0; pos <= 1; pos += step) {
		v_sub(l1,l0,l);
		v_scale(l,pos,l);
		v_add(l,l0,l);
		if(on_terrain) {
			if(terrain_boundary(t,l) == -1) return -1;
			cur = (l[2] - terrain_height(t,l) < -1e-6);
			if(cur != prev) return 0;
		} else {
			if(terrain_boundary(t,l) == 0) {
				cur = (l[2] - terrain_height(t,l) < -1e-6);
				on_terrain = 1;
			}
		}
		prev = cur;
	}
	return -1;
}

/* размывание тени
 */
static unsigned char *terrain_blur(unsigned char *in,int w,int h,int n) {
	int x,y,x1,y1,n2,rsum,gsum,bsum,count;
	unsigned char *out,*inp,*outp;
	out = malloc(sizeof(unsigned char) * w * h * 4);
	n2 = n / 2;
	for(y = 0; y < h; y++) {
		outp = out + ((y * w) << 2);
		for(x = 0; x < w; x++) {
			rsum = gsum = bsum = count = 0;
			for(y1 = y - n2; y1 <= y + n2; y1++) {
				if(y1 > 0 && y1 < h) {
					inp = in + ((y1 * w + x - n2) << 2);
					for(x1 = x - n2; x1 <= x + n2; x1++) {
						if(x1 > 0 && x1 < w) {
							rsum += *inp++;
							gsum += *inp++;
							bsum += *inp++;
							inp++;
							count++;
						} else inp += 4;
					}
				}
			}
			rsum /= count;
			gsum /= count;
			bsum /= count;
			*outp++ = rsum;
			*outp++ = gsum;
			*outp++ = bsum;
			*outp++ = in[((y * w + x) << 2) + 3];
		}
	}
	free(in);
	return out;
}

/* расчет освещени? */
unsigned char *terrain_light(terrain_t *t,float *light,float ambient,
	int width,int height) {
	int i,j,k,color;
	float p[3],n[3],dir[3];
	unsigned char *map;
	map = malloc(sizeof(unsigned char) * width * height * 4);
	for(i = 0, k = 0; i < height; i++)
		for(j = 0; j < width; j++, k += 4) {
			p[0] = (float)j / (float)width * t->width * t->step;
			p[1] = (float)i / (float)height * t->height * t->step;
			p[2] = terrain_height(t,p);
			terrain_normal(t,p,n);
			v_sub(light,p,dir);
			v_normalize(dir,dir);
			color = v_dot(n,dir) * 255.0;
			if(terrain_cross_line(t,p,light) == 0) color = ambient * 255.0;
			if(color < ambient * 255.0) color = ambient * 255.0;
			map[k] = color;
			map[k + 1] = color;
			map[k + 2] = color;
			map[k + 3] = 255;
		}
	map = terrain_blur(map,width,height,3);
	return map;
}
