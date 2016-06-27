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

#define SIZE 	128
#define WIDTH	800
#define HEIGHT	600

int my_pause,my_blur = 1;
int shadow_id,mesh_id,texture_id,ground_id;
float phi = -30,psi,dist = 3,angle;
vec3 camera;
vec4 light = { 2, 0, 3, 1 };
vec3 mesh = { 0, 0, 1 };

unsigned char *image;

/* init
 */
int init(void) {
	int i,num_vertex,width,height;
	float *vertex;
	unsigned char *data;
	float rmax;
	vec3 min,max;
    vec4 plane_s = { 1.0, 0.0, 0.0, 0.0 };
    vec4 plane_t = { 0.0, 1.0, 0.0, 0.0 };
    vec4 plane_r = { 0.0, 0.0, 1.0, 0.0 };
    vec4 plane_q = { 0.0, 0.0, 0.0, 1.0 };
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
	glPointSize(4);
	glTexGenfv(GL_S,GL_EYE_PLANE,plane_s);
	glTexGenfv(GL_T,GL_EYE_PLANE,plane_t);
	glTexGenfv(GL_R,GL_EYE_PLANE,plane_r);
	glTexGenfv(GL_Q,GL_EYE_PLANE,plane_q);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,SIZE,SIZE,0,GL_RGB,
		GL_UNSIGNED_BYTE,NULL);
	
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
	
	vertex = load_3ds("data/mesh.3ds",&num_vertex);
	if(!vertex) return -1;
	
	v_set(999999,999999,999999,min);
	v_set(-999999,-999999,-999999,max);
    for(i = 0; i < num_vertex; i++) {
		int j;
		float *v = &vertex[i << 3];
		for(j = 0; j < 3; j++) {
			if(min[j] > v[j]) min[j] = v[j];
			if(max[j] < v[j]) max[j] = v[j];
		}
    }
	v_add(min,max,min);
	v_scale(min,0.5,min);
    for(i = 0; i < num_vertex; i++) {
        v_sub(&vertex[i << 3],min,&vertex[i << 3]);
    }
    for(i = 0, rmax = 0; i < num_vertex; i++) {
        float r = sqrt(v_dot(&vertex[i << 3],&vertex[i << 3]));
		if(r > rmax) rmax = r;
    }
	rmax = 0.8 / rmax;
    for(i = 0; i < num_vertex; i++) {
        v_scale(&vertex[i << 3],rmax,&vertex[i << 3]);
    }
	
    mesh_id = glGenLists(1);
	glNewList(mesh_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
        glNormal3fv((float*)&vertex[i << 3] + 3);
        glVertex3fv((float*)&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	
	vertex = load_3ds("data/ground.3ds",&num_vertex);
	if(!vertex) return -1;
	
    ground_id = glGenLists(1);
	glNewList(ground_id,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertex; i++) {
		glTexCoord2fv((float*)&vertex[i << 3] + 6);
		glNormal3fv((float*)&vertex[i << 3] + 3);
        glVertex3fv((float*)&vertex[i << 3]);
    }
    glEnd();
    glEndList();
	
	image = malloc(SIZE * SIZE * 4);
	
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
void blur(unsigned char *in,int size) {
	int x,y,sum,size3 = size * 3;
	unsigned char *out,*inp,*outp;
	out = malloc(size * size * 3);

	memset(out,255,size * size * 3);
	
	inp = in + size3;
	outp = out + size3;
	
	for(y = 1; y < size - 1; y++) {
		inp += 3;
		outp += 3;
		for(x = 1; x < size - 1; x++) {
			sum = inp[-size3 - 3] + inp[-size3] + inp[-size3 + 3] +
				inp[-3] + inp[0] + inp[3] +
				inp[size3 - 3] + inp[size3] + inp[size3 + 3];
			sum /= 9;
			inp += 3;
			*outp++ = sum;
			*outp++ = sum;
			*outp++ = sum;
		}
		inp += 3;
		outp += 3;
	}
	
	memcpy(in,out,size * size * 3);
	free(out);
}

/*
 */
void create_shadow(void) {
	glViewport(0,0,SIZE,SIZE);
	glScissor(0,0,SIZE,SIZE);
	glEnable(GL_SCISSOR_TEST);
	
    glBindTexture(GL_TEXTURE_2D,shadow_id);
	
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-100,100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(light[0],light[1],light[2], mesh[0],mesh[1],mesh[2], 0,0,1);
    
	glColor3f(0.4,0.4,0.4);
	glTranslatef(mesh[0],mesh[1],mesh[2]);
	glRotatef(angle,0,0,1);
	glRotatef(angle / 3,1,0,0);
	glCallList(mesh_id);
	glColor3f(1,1,1);

	if(my_blur) {
		glReadPixels(0,0,SIZE,SIZE,GL_RGB,GL_UNSIGNED_BYTE,image);
		blur(image,SIZE);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIZE,SIZE,GL_RGB,
			GL_UNSIGNED_BYTE,image);
	} else {
    	glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,SIZE,SIZE);
	}
	
	glDisable(GL_SCISSOR_TEST);
	glViewport(0,0,WIDTH,HEIGHT);
}

/*
 */
void render(void) {
	float m[16],im[16];
	
	create_shadow();
	
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(45,4.0 / 3.0,0.5,100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(camera[0],camera[1],camera[2], mesh[0],mesh[1],mesh[2], 0,0,1);
    glLightfv(GL_LIGHT0,GL_POSITION,light);
	
	/* light */
	glBegin(GL_POINTS);
	glVertex3fv(light);
	glEnd();
	
	/* meshes */
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(mesh[0],mesh[1],mesh[2]);
	glRotatef(angle,0,0,1);
	glRotatef(angle / 3,1,0,0);
	glCallList(mesh_id);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,texture_id);
	glCallList(ground_id);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	
	/* shadow */
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);
	glGetFloatv(GL_MODELVIEW_MATRIX,m);
	m_inverse(m,im);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(0.5,0.5,0);
	glScalef(0.5,0.5,1.0);
	glOrtho(-1,1,-1,1,-1,1);
	gluLookAt(light[0],light[1],light[2], mesh[0],mesh[1],mesh[2], 0,0,1);
    glMultMatrixf(im);
	
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,shadow_id);
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
    glCallList(ground_id);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
	
	SDL_GL_SwapBuffers();
}

/*
 */
void idle(void) {
	static int frames;
	static float time;
	float ifps;
	vec3 v;
	vec4 q0,q1,q2;
	matrix m;

	ifps = 1.0 / getfps();
	
	if(!my_pause) angle += ifps * 360.0 / 4.0;
	
	v_set(sin(angle * DEG2RAD / 3),cos(angle * DEG2RAD / 7),2,light);
	
	v_set(0,0,1,v);
	q_set(v,psi,q0);
	v_set(0,1,0,v);
	q_set(v,phi,q1);
	q_multiply(q0,q1,q2);
	q_to_matrix(q2,m);
	v_set(dist,0,0,camera);
	v_transform(camera,m,camera);
	v_add(camera,mesh,camera);
	
	frames++;
	time += ifps;

	if(time > 1.0) {
		printf("%d frames %.2f fps\n",frames,(float)frames / time);
		frames = 0;
		time = 0;
	}
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
		case SDLK_b:
			my_blur = !my_blur;
			break;
	}
}

/*
 */
void mouse(int button,int state,int x,int y) {
	if(button == 4) dist -= 0.5;
	if(button == 5) dist += 0.5;
	if(dist < 1.5) dist = 1.5;
	if(dist > 10) dist = 10;
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
