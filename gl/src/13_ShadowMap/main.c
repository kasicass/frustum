/*	shadows
 *
 *		written by Alexander Zaprjagaev
 *		frustum@public.tsu.ru
 */

#include <math.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "mathlib.h"
#include "load3ds.h"
#include "loadjpeg.h"
#include "shadow.h"
#include "skinnedmesh.h"
#include "loadmdc.h"
#include "font.h"

#define WIDTH	1024
#define HEIGHT	768

int my_pause,my_shadow,my_mode,my_drop,my_animation,my_help,my_size;
int mesh_id,texture_id;
float fps,time,stage,phi = -30,psi,dist = 100,angle;
vec3_t camera;

int size[3] = { 64, 128, 256 };

vec3_t mesh = { 0, 0, 20 };

vec4_t light_g = { 100, 0, 1000, 1 };
vec4_t light = { 0, 0, 0, 0 };
vec4_t light_ = { 0, 0, 0, 0 };
vec4_t light__ = { 0, 0, 0, 0 };

shadow_t *shadow,*shadow_,*shadow__;
shadow_t *shadow64,*shadow_64,*shadow__64;
shadow_t *shadow128,*shadow_128,*shadow__128;
shadow_t *shadow256,*shadow_256,*shadow__256;

font_t *font;

sm_t *sm;
float *helmet_v,*gun_v;
int helmet_nv,gun_nv,helmet_id,gun_id;
matrix_t matrix;

float *sphere_v;
int sphere_nv,sphere_id;
matrix_t matrix_s;

float *ground_v;
int ground_nv,ground_id;
matrix_t matrix_g;

char *help = "\
shadow map & skinned mesh demo\n\
written by Alexander Zaprjagaev\n\
frustum@public.tsu.ru\n\
h - help\n\
f - full screen\n\
s - software / hardware shadows\n\
d - drop shadow frame\n\
z - shadow size 64/128/256\n\
space - pause\n\
n - next animation\n\
p - prev animation\n\
0 - shadow off\n\
1 - one shadow\n\
2 - -//- shadow\n\
3 - -//- shadow";

/* init
 */
int init(void) {
	int i,num_vertex,width,height;
	float *vertex;
	unsigned char *data;
	GLenum err;

	err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "glewInit() error: %s\n", glewGetErrorString(err));
	}

	glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHT0);
	glPolygonOffset(-1.0,0.1);
	
	/* font */
	font = font_load_tga("data/font.tga");

	/* sphere */
	vertex = load_3ds("data/sphere.3ds",&num_vertex);
	sphere_v = vertex;
	sphere_nv = num_vertex;
    sphere_id = glGenLists(1);
	glNewList(sphere_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
		glTexCoord2fv(&vertex[i << 3] + 6);
		glNormal3fv(&vertex[i << 3] + 3);
		v_scale(&vertex[i << 3],4,&vertex[i << 3]);
        glVertex3fv(&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	
	/* ground */
	vertex = load_3ds("data/ground.3ds",&num_vertex);
	ground_v = vertex;
	ground_nv = num_vertex;
    ground_id = glGenLists(1);
	glNewList(ground_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
		glTexCoord2fv(&vertex[i << 3] + 6);
		glNormal3fv(&vertex[i << 3] + 3);
		v_scale(&vertex[i << 3],50,&vertex[i << 3]);
        glVertex3fv(&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	
	/* texture */
	glGenTextures(1,&texture_id);
	glBindTexture(GL_TEXTURE_2D,texture_id);
	if((data = load_jpeg("data/ground.jpg",&width,&height))) {
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,
			GL_UNSIGNED_BYTE,data);
		free(data);
	}

	/* model */
	
	vertex = load_mdc("data/helmet.mdc",&num_vertex);
	helmet_v = vertex;
	helmet_nv = num_vertex;
    helmet_id = glGenLists(1);
	glNewList(helmet_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
		glTexCoord2fv(&vertex[i << 3] + 6);
		glNormal3fv(&vertex[i << 3] + 3);
        glVertex3fv(&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	
	vertex = load_mdc("data/gun.mdc",&num_vertex);
	gun_v = vertex;
	gun_nv = num_vertex;
    gun_id = glGenLists(1);
	glNewList(gun_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
		glTexCoord2fv(&vertex[i << 3] + 6);
		glNormal3fv(&vertex[i << 3] + 3);
        glVertex3fv(&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	
	sm = sm_load("data/mesh.sm");
	
	/* shadow */
	shadow = shadow64 = shadow_create(64);
	shadow_ = shadow_64 = shadow_create(64);
	shadow__ = shadow__64 = shadow_create(64);
	shadow128 = shadow_create(128);
	shadow_128 = shadow_create(128);
	shadow__128 = shadow_create(128);
	shadow256 = shadow_create(256);
	shadow_256 = shadow_create(256);
	shadow__256 = shadow_create(256);
	
	return 0;
}

/* get fps
 */
float getfps(void) {
	static float fps = 60;
	static int starttime,endtime,counter;
	if(counter == 10) {
		endtime = starttime;
		starttime = SDL_GetTicks();
		fps = counter * 1000.0 / (float)(starttime - endtime);
		counter = 0;
	}
	counter++;
	return fps;
}

/*
 */
void create_shadow(sm_t *sm) {
	static int drop = 0;
	int i,num_vertex;
	float *vertex,*vertex_ptr;
	matrix_t m,im;
	
	if(my_drop && drop < 1) {
		drop++;
		return;
	} else {
		drop = 0;
	}
	
	for(i = 0, num_vertex = 0; i < sm->num_surface; i++) {
		num_vertex += sm->surface[i]->num_face * 3;
	}
	num_vertex += gun_nv;
	num_vertex += helmet_nv;
	vertex = malloc(sizeof(float) * 3 * num_vertex);
	for(i = 0, vertex_ptr = vertex; i < sm->num_surface; i++) {
		int j;
		sm_surface_t *s = sm->surface[i];
		for(j = 0; j < s->num_face; j++, vertex_ptr += 9) {
			v_copy(s->vertex[s->face[j].v0].xyz,vertex_ptr + 0);
			v_copy(s->vertex[s->face[j].v1].xyz,vertex_ptr + 3);
			v_copy(s->vertex[s->face[j].v2].xyz,vertex_ptr + 6);
		}
	}
	sm_bone_transform(sm,sm_bone(sm,"tag_weapon"),m);
	m_transpose_rotation(m,im);
	for(i = 0; i < gun_nv; i++, vertex_ptr += 3) {
		vec3_t v;
		v_transform(&gun_v[i << 3],im,v);
		v_copy(v,vertex_ptr);
	}
	sm_bone_transform(sm,sm_bone(sm,"tag_head"),m);
	m_transpose_rotation(m,im);
	for(i = 0; i < helmet_nv; i++, vertex_ptr += 3) {
		vec3_t v;
		v_transform(&helmet_v[i << 3],im,v);
		v_copy(v,vertex_ptr);
	}
	
	if(my_shadow == 1) {
		if(my_mode) {
			shadow_render_hw(shadow,light,matrix,
				vertex,sizeof(float) * 3,num_vertex);
		} else {
			shadow_render_sw(shadow,light,matrix,
				vertex,sizeof(float) * 3,num_vertex);
		}
	} else if(my_shadow == 2) {
		if(my_mode) {
			shadow_render_hw(shadow,light,matrix,
				vertex,sizeof(float) * 3,num_vertex);
			shadow_render_hw(shadow_,light_,matrix,
				vertex,sizeof(float) * 3,num_vertex);
		} else {
			shadow_render_sw(shadow,light,matrix,
				vertex,sizeof(float) * 3,num_vertex);
			shadow_render_sw(shadow_,light_,matrix,
				vertex,sizeof(float) * 3,num_vertex);
		}
	} else if(my_shadow == 3) {
		if(my_mode) {
			shadow_render_hw(shadow,light,matrix,
				vertex,sizeof(float) * 3,num_vertex);
			shadow_render_hw(shadow_,light_,matrix,
				vertex,sizeof(float) * 3,num_vertex);
			shadow_render_hw(shadow__,light__,matrix,
				vertex,sizeof(float) * 3,num_vertex);
		} else {
			shadow_render_sw(shadow,light,matrix,
				vertex,sizeof(float) * 3,num_vertex);
			shadow_render_sw(shadow_,light_,matrix,
				vertex,sizeof(float) * 3,num_vertex);
			shadow_render_sw(shadow__,light__,matrix,
				vertex,sizeof(float) * 3,num_vertex);
		}
	}
	free(vertex);
}

/*
 */
void render(void) {
	matrix_t m,im;
	
	switch(my_animation) {
		case 0: sm_frame(sm,0,131,time * 15); break;
		case 1: sm_frame(sm,131,218,time * 20); break;
		case 2: sm_frame(sm,218,260,time * 20); break;
		case 3: sm_frame(sm,260,310,time * 20); break;
		case 4: sm_frame(sm,310,367,time * 20); break;
		case 5: sm_frame(sm,367,378,time * 15); break;
	}
	
	create_shadow(sm);
	
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(45,4.0 / 3.0,1,1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(camera[0],camera[1],camera[2], mesh[0],mesh[1],mesh[2], 0,0,1);
    glLightfv(GL_LIGHT0,GL_POSITION,light_g);
	
	glEnable(GL_LIGHTING);
	
	glPushMatrix();
	glMultMatrixf(matrix_g);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,texture_id);
	glCallList(ground_id);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	
	glPushMatrix();
	glMultMatrixf(matrix_s);
	glCallList(sphere_id);
	glPopMatrix();
	
	glPushMatrix();
	glMultMatrixf(matrix);

	sm_render(sm);
	glPushMatrix();
	sm_bone_transform(sm,sm_bone(sm,"tag_weapon"),m);
	m_transpose_rotation(m,im);
	glMultMatrixf(im);
	glCallList(gun_id);
	glPopMatrix();
	glPushMatrix();
	sm_bone_transform(sm,sm_bone(sm,"tag_head"),m);
	m_transpose_rotation(m,im);
	glMultMatrixf(im);
	glCallList(helmet_id);
	glPopMatrix();

	glPopMatrix();
	glDisable(GL_LIGHTING);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_FALSE);
	glColor3f(0.5,0.5,0.5);
	if(my_shadow == 1) {
		shadow_project(shadow,matrix_g,ground_v,sizeof(float) * 8,ground_nv);
		shadow_project(shadow,matrix_s,sphere_v,sizeof(float) * 8,sphere_nv);
	} else if(my_shadow == 2) {
		shadow_project(shadow,matrix_g,ground_v,sizeof(float) * 8,ground_nv);
		shadow_project(shadow,matrix_s,sphere_v,sizeof(float) * 8,sphere_nv);
		shadow_project(shadow_,matrix_g,ground_v,sizeof(float) * 8,ground_nv);
		shadow_project(shadow_,matrix_s,sphere_v,sizeof(float) * 8,sphere_nv);
	} else if(my_shadow == 3) {
		shadow_project(shadow,matrix_g,ground_v,sizeof(float) * 8,ground_nv);
		shadow_project(shadow,matrix_s,sphere_v,sizeof(float) * 8,sphere_nv);
		shadow_project(shadow_,matrix_g,ground_v,sizeof(float) * 8,ground_nv);
		shadow_project(shadow_,matrix_s,sphere_v,sizeof(float) * 8,sphere_nv);
		shadow_project(shadow__,matrix_g,ground_v,sizeof(float) * 8,ground_nv);
		shadow_project(shadow__,matrix_s,sphere_v,sizeof(float) * 8,sphere_nv);
	}
	glColor3f(1,1,1);
	glDepthMask(GL_TRUE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	font_printf(font,1600,1200,10,10,
		"fps %.2f\nshadow %s\ndrop %s\nanimation %d\nsize %d",
		fps,my_mode ? "hard" : "soft",my_drop ? "yes" : "no",
		my_animation,size[my_size]);
	
	if(my_help || stage > 0.0) {
		if(stage != 1.0) {
			glColor4f(0.1,1.0,0.95,stage / 2.0);
			font_printf(font,800,600,200 - (1.0 - stage) * 300,100,help);
			font_printf(font,800,600,200 + (1.0 - stage) * 300,100,help);
		} else {
			glColor4f(0.1,1.0,0.95,stage);
			font_printf(font,800,600,200,100,help);
		}
		glColor4f(1,1,1,1);
	}
	
	SDL_GL_SwapBuffers();
}

/*
 */
void idle(void) {
	float ifps;
	vec3_t v;
	vec4_t q0,q1,q2;
	matrix_t m0,m1;
	
	fps = getfps();
	ifps = 1.0 / fps;
		time += ifps;
	
	if(!my_pause) angle += ifps * 360.0 / 4.0;
	
	if(my_help) {
		stage += ifps * 4.0;
		if(stage > 1.0) stage = 1.0;
	} else {
		stage -= ifps * 2.0;
		if(stage < 0.0) stage = 0.0;
	}
	
	v_set(0,0,0,v);
	m_translate(v,matrix);
	
	v_set(0,0,-1,v);
	m_translate(v,matrix_g);
	
	v_set(60,0,20,v);
	m_translate(v,m0);
	m_rotation_z(angle / 7,m1);
	m_multiply(m1,m0,matrix_s);
	
	v_set(sin(angle * DEG2RAD / 3),cos(angle * DEG2RAD / 3),1,light)
	v_normalize(light,light);
	
	v_set(sin(-angle * DEG2RAD / 5),cos(-angle * DEG2RAD / 5),0.5,light_)
	v_normalize(light_,light_);
	
	v_set(sin(angle * DEG2RAD / 7),cos(angle * DEG2RAD / 7),2,light__)
	v_normalize(light__,light__);
	
	v_set(0,0,1,v);
	q_set(v,psi,q0);
	v_set(0,1,0,v);
	q_set(v,phi,q1);
	q_multiply(q0,q1,q2);
	q_to_matrix(q2,m0);
	v_set(dist,0,0,camera);
	v_transform(camera,m0,camera);
	v_add(camera,mesh,camera);
}

/*
 */
void keyboard(int key) {
	static int flag;
	switch(key) {
		case SDLK_ESCAPE:
			SDL_Quit();
			exit(0);
			break;
		case SDLK_f:
			if(flag == SDL_FULLSCREEN) flag = 0;
			else flag = SDL_FULLSCREEN;
			SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_OPENGL | flag);
			break;
		case SDLK_SPACE:
			my_pause = !my_pause;
			break;
		case SDLK_0:
			my_shadow = 0;
			break;
		case SDLK_1:
			my_shadow = 1;
			break;
		case SDLK_2:
			my_shadow = 2;
			break;
		case SDLK_3:
			my_shadow = 3;
			break;
		case SDLK_s:
			my_mode = !my_mode;
			break;
		case SDLK_d:
			my_drop = !my_drop;
			break;
		case SDLK_n:
			my_animation++;
			if(my_animation > 5) my_animation = 0;
			break;
		case SDLK_p:
			my_animation--;
			if(my_animation < 0) my_animation = 5;
			break;
		case SDLK_h:
			my_help = !my_help;
			break;
		case SDLK_z:
			my_size++;
			if(my_size > 2) my_size = 0;
			if(my_size == 0) {
				shadow = shadow64;
				shadow_ = shadow_64;
				shadow__ = shadow__64;
			} else if(my_size == 1) {
				shadow = shadow128;
				shadow_ = shadow_128;
				shadow__ = shadow__128;
			} else {
				shadow = shadow256;
				shadow_ = shadow_256;
				shadow__ = shadow__256;
			}
			break;
	}
}

/*
 */
void mouse(int button,int state,int x,int y) {
	if(button == 4) dist -= 10;
	if(button == 5) dist += 10;
	if(dist < 10) dist = 10;
	if(dist > 500) dist = 500;
	psi += (x - WIDTH / 2) * 0.2;
	phi += (y - HEIGHT / 2) * 0.2;
	if(phi < -89) phi = -89;
	if(phi > 89) phi = 89;
}

/*
 */
int main(int argc,char **argv) {
	int done;
	
	SDL_Init(SDL_INIT_VIDEO);
	if(!SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_OPENGL)) {
		printf("couldn`t set video mode: %s\n",SDL_GetError());
		SDL_Quit();
		exit(1);
	}
	
	if(init() != 0) return 1;
	
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WarpMouse(WIDTH / 2,HEIGHT / 2);
	
	done = 0;
	while(!done) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					done = 1;
					break;
				case SDL_KEYDOWN:
					keyboard(event.key.keysym.sym);
					break;
				case SDL_MOUSEMOTION:
					mouse(0,0,event.motion.x,event.motion.y);
					break;
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					mouse(event.button.button,event.button.state,
						event.button.x,event.button.y);
					break;
			}
		}
		SDL_WarpMouse(WIDTH / 2,HEIGHT / 2);
		idle();
		render();
	}
	
	SDL_Quit();
	return 0;
}
