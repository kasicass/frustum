/*	skinnedmesh
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
#include "skinnedmesh.h"

#define WIDTH	800
#define HEIGHT	600

int my_pause,my_animation,my_gun = 1,my_head = 1;
float phi = -30,psi,dist = 100,time;
vec4 camera = { 0,0,0,1 };
vec3 dir;

sm_t *sm;
int gun_id,helmet_id;

float *load_mdc(char *name,int *num_vertex);

/* init
 */
int init(void) {
	int i,num_vertex;
	float *vertex;
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

	//sm = sm_load_ascii("data/mesh.txt");
	//sm_save("data/mesh.sm",sm);
	sm = sm_load("data/mesh.sm");
	
	vertex = load_mdc("data/gun.mdc",&num_vertex);
	if(!vertex) return -1;
	
    gun_id = glGenLists(1);
	glNewList(gun_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
		glTexCoord2fv((float*)&vertex[i << 3] + 6);
		glNormal3fv((float*)&vertex[i << 3] + 3);
        glVertex3fv((float*)&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	free(vertex);
	
	vertex = load_mdc("data/helmet.mdc",&num_vertex);
	if(!vertex) return -1;
	
    helmet_id = glGenLists(1);
	glNewList(helmet_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
		glTexCoord2fv((float*)&vertex[i << 3] + 6);
		glNormal3fv((float*)&vertex[i << 3] + 3);
        glVertex3fv((float*)&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	free(vertex);
	
	printf("\
' ' - pause\n\
'g' - gun toggle\n\
'h' - head toggle\n\
'1' - '6' animations\n");
	
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
void render(void) {
	float m[16],im[16];
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(45,4.0 / 3.0,0.5,1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(camera[0],camera[1],camera[2], dir[0],dir[1],dir[2], 0,0,1);
    glLightfv(GL_LIGHT0,GL_POSITION,camera);
	
	switch(my_animation) {
		case 0: sm_frame(sm,0,131,time * 15); break;
		case 1: sm_frame(sm,131,218,time * 20); break;
		case 2: sm_frame(sm,218,260,time * 20); break;
		case 3: sm_frame(sm,260,310,time * 20); break;
		case 4: sm_frame(sm,310,367,time * 20); break;
		case 5: sm_frame(sm,367,378,time * 15); break;
	}
	
	glEnable(GL_LIGHTING);
	
	sm_render(sm);

	if(my_gun) {
		glPushMatrix();
		sm_bone_transform(sm,sm_bone(sm,"tag_weapon"),m);
		m_transpose_rotation(m,im);
		glMultMatrixf(im);
		glCallList(gun_id);
		glPopMatrix();
	}
	if(my_head) {
		glPushMatrix();
		sm_bone_transform(sm,sm_bone(sm,"tag_head"),m);
		m_transpose_rotation(m,im);
		glMultMatrixf(im);
		glCallList(helmet_id);
		glPopMatrix();
	}
	
	glDisable(GL_LIGHTING);
	
	SDL_GL_SwapBuffers();
}

/*
 */
void idle(void) {
	float ifps;
	vec3 v;
	vec4 q0,q1,q2;
	matrix m;

	ifps = 1.0 / getfps();
	
	if(!my_pause) time += ifps;
	
	v_set(0,0,1,v);
	q_set(v,psi,q0);
	v_set(0,1,0,v);
	q_set(v,phi,q1);
	q_multiply(q0,q1,q2);
	q_to_matrix(q2,m);
	v_set(dist,0,0,camera);
	v_transform(camera,m,camera);
	v_set(0,0,50,dir);
	v_add(camera,dir,camera);
	
}

/*
 */
void keyboard(int key) {
	switch(key) {
		case SDLK_ESCAPE:
			SDL_Quit();
			exit(0);
			break;
		case SDLK_SPACE:
			my_pause = !my_pause;
			break;
		case SDLK_g:
			my_gun = !my_gun;
			break;
		case SDLK_h:
			my_head = !my_head;
			break;
		case SDLK_1:
			my_animation = 0;
			break;
		case SDLK_2:
			my_animation = 1;
			break;
		case SDLK_3:
			my_animation = 2;
			break;
		case SDLK_4:
			my_animation = 3;
			break;
		case SDLK_5:
			my_animation = 4;
			break;
		case SDLK_6:
			my_animation = 5;
			break;
	}
}

/*
 */
void mouse(int button,int state,int x,int y) {
	if(button == 4) dist -= 10;
	if(button == 5) dist += 10;
	if(dist < 10) dist = 10;
	if(dist > 200) dist = 200;
	psi += (x - WIDTH / 2) * 0.2;
	phi += (y - HEIGHT / 2) * 0.2;
	if(phi < -89) phi = -89;
	if(phi > 89) phi = 89;
}

/*
 */
int main(int argc,char **argv) {
	SDL_Surface *screen;
	int done;
	
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_OPENGL);
	if(!screen) {
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
