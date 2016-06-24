#include <math.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "loadtga.h"
#include "mathlib.h"
#include "camera.h"
#include "terrain.h"

/*
 */
#define WIDTH	800.0
#define HEIGHT	600.0

terrain_t *terrain;
camera_t *camera;

float fps;
int small;

struct {
	float phi,psi,dist;
	float speed[3];
	float pos[3],dir[3],up[3];
	int key_up,key_down,key_left,key_right;
} control;

/*
 */
int texture_create(unsigned char *data,int width,int height) {
	int id;
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
	return id;
}

int texture_load(char *name) {
	int id,width,height;
	unsigned char *data;
	data = load_tga(name,&width,&height);
	if(!data) return -1;
	id = texture_create(data,width,height);
	free(data);
	return id;
}

/*
 */
int init(void) {
	glClearDepth(1);
	glClearColor(0,0,0.4,0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	terrain = terrain_load_tga("data/height.tga",2,30);
	terrain_load_texture(terrain,"data/ground0/ground.tga",0);
	terrain_load_texture(terrain,"data/ground1/ground.tga",1);
	terrain_load_texture(terrain,"data/grass0/grass.tga",2);
	terrain_load_texture(terrain,"data/grass1/grass.tga",3);
	
	camera = camera_create(45,WIDTH / HEIGHT,0.1,100);
	
	{
	float light[3] = { 500, 500, 500 };
	unsigned char *data;
	terrain->shadow_id = texture_load("data/shadow.tga");
	if(terrain->shadow_id == -1) {
		data = terrain_light(terrain,light,0.4,256,256);
		save_tga("data/shadow.tga",data,256,256);
		terrain->shadow_id = texture_create(data,256,256);
		free(data);
	}
	}
	
	v_set(0,0,0,control.speed);
	v_set(64,64,0,control.dir);
	control.dist = 20;
	control.psi = 90;
	control.phi = 45;
	
	return 0;
}

/* get fps
 */
float getfps(void) {
	static float fps = 60;
	static int starttime,endtime,counter;
	if(counter == 10) {
		endtime = starttime;
		starttime = glutGet(GLUT_ELAPSED_TIME);
		fps = counter * 1000.0 / (float)(starttime - endtime);
		counter = 0;
	}
	counter++;
	return fps;
}

void dprintf(float x,float y,char *string,...) {
	int i,l;
	char buffer[256];
	va_list argptr;
	va_start(argptr,string);
	l = vsprintf(buffer,string,argptr);
	va_end(argptr);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1,1,-1,1,-1,1);
	glRasterPos2f(x,y);
	for(i = 0; i < l; i++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,buffer[i]);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void render(void) {
	
	terrain_render(terrain,camera);

	{
	float p[3],n[3];
	static float alpha;
	glColor3f(0,0,1);
	glPushMatrix();
	glTranslatef(control.dir[0],control.dir[1],control.dir[2] - 1.5);
	glutSolidSphere(1,16,16);
	terrain_normal(terrain,control.dir,n);
	v_scale(n,3,n);
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3fv(n);
	glEnd();
	glColor3f(1,1,1);
	glPopMatrix();
	alpha += 1.0 / fps * 360 / 20 * DEG2RAD;
	v_set(sin(alpha) * 40,cos(alpha) * 40,0,p);
	v_scale(p,sin(alpha / 3),p);
	p[0] += 64;
	p[1] += 64;
	p[2] = terrain_height(terrain,p);
	if(camera_check_sphere(camera,p,1) == 0) {
		glPushMatrix();
		glTranslatef(p[0],p[1],p[2]);
		glutSolidSphere(1,16,16);
		terrain_normal(terrain,p,n);
		v_scale(n,4,n);
		glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3fv(n);
		glEnd();
		glPopMatrix();
	}
	}
}

void small_render(void) {
	glViewport(WIDTH - 320,HEIGHT - 240,320,240);
	glScissor(WIDTH - 320,HEIGHT - 240,320,240);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0,0.4,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0.4,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,320.0 / 240.0,1,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,0,100, 64,64,0, 0,0,1);
	render();
	glViewport(0,0,WIDTH,HEIGHT);
	glDisable(GL_SCISSOR_TEST);
}

void render_axis(float x,float y) {
	float v[3];
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(x,y,0);
	gluPerspective(45,WIDTH / HEIGHT,1,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0,0,-30);
	glBegin(GL_LINES);
	v_set(1,0,0,v);
	glColor3fv(v);
	v_transform_normal(v,camera->mview,v);
	glVertex3f(0,0,0);
	glVertex3fv(v);
	v_set(0,1,0,v);
	glColor3fv(v);
	v_transform_normal(v,camera->mview,v);
	glVertex3f(0,0,0);
	glVertex3fv(v);
	v_set(0,0,1,v);
	glColor3fv(v);
	v_transform_normal(v,camera->mview,v);
	glVertex3f(0,0,0);
	glVertex3fv(v);
	glEnd();
	glPopMatrix();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	camera_look_at(camera,control.pos,control.dir,control.up);

	render();
	if(small) small_render();
	
	render_axis(-0.9,0.8);
	
	glColor3f(0,1,0);
	dprintf(-0.95,0.95,"fps: %.2f",fps);
	dprintf(-0.95,0.90,"%.2f %.2f %.2f",
		camera->dir[0],camera->dir[1],camera->dir[2]);
	glColor3f(1,1,1);
	glutSwapBuffers();
}

/*
 */
void idle(void) {
	float ifps;
	float rad,v[3],q0[4],q1[4],q2[4],m0[16];

	fps = getfps();
	ifps = 1.0 / fps;
	
	if(control.key_up) control.speed[0] -= 2;
	if(control.key_down) control.speed[0] += 2;
	if(control.key_left) control.speed[1] -= 2;
	if(control.key_right) control.speed[1] += 2;
	control.speed[0] -= control.speed[0] * 0.04;
	control.speed[1] -= control.speed[1] * 0.04;
	if(control.speed[0] > 40) control.speed[0] = 40;
	if(control.speed[0] < -40) control.speed[0] = -40;
	if(control.speed[1] > 40) control.speed[1] = 40;
	if(control.speed[1] < -40) control.speed[1] = -40;
	
	v_copy(control.dir,v);
	rad = control.psi * DEG2RAD;
	v[0] += (control.speed[0] * cos(rad) - control.speed[1] * sin(rad)) * ifps;
	v[1] += (control.speed[0] * sin(rad) + control.speed[1] * cos(rad)) * ifps;
	if(terrain_boundary(terrain,v) == 0) v_copy(v,control.dir);
	
	control.dir[2] = terrain_height(terrain,control.dir) + 1.5;
	
	v_set(0,0,1,v);
	q_set(v,control.psi,q0);
	v_set(0,1,0,v);
	q_set(v,-control.phi,q1);
	q_multiply(q0,q1,q2);
	q_to_matrix(q2,m0);
	v_set(control.dist,0,0,control.pos);
	v_transform(control.pos,m0,control.pos);
	v_add(control.pos,control.dir,control.pos);
	v_set(0,0,1,control.up);
	
	glutWarpPointer(WIDTH / 2,HEIGHT / 2);
	glutPostRedisplay();
}

/*
 */
void keyboard(unsigned char c,int x,int y) {
	static int wareframe;
	switch(c) {
		case 27:
			exit(0);
			break;
		case 'w':
		case 'W':
			wareframe = !wareframe;
			glPolygonMode(GL_FRONT_AND_BACK,wareframe ? GL_LINE : GL_FILL);
			break;
		case 's':
		case 'S':
			small = !small;
			break;
	}
}

void keyboard_down(int key,int x,int y) {
	switch(key) {
		case GLUT_KEY_UP: control.key_up = 1; break;
		case GLUT_KEY_DOWN: control.key_down = 1; break;
		case GLUT_KEY_LEFT: control.key_left = 1; break;
		case GLUT_KEY_RIGHT: control.key_right = 1; break;
	}
}

void keyboard_up(int key,int x,int y) {
	switch(key) {
		case GLUT_KEY_UP: control.key_up = 0; break;
		case GLUT_KEY_DOWN: control.key_down = 0; break;
		case GLUT_KEY_LEFT: control.key_left = 0; break;
		case GLUT_KEY_RIGHT: control.key_right = 0; break;
	}
}

void mouse(int button,int state,int x,int y) {
	if(button == 3) control.dist -= 1;
	if(button == 4) control.dist += 1;
	if(control.dist < 1) control.dist = 1;
	if(control.dist > 50) control.dist = 50;
}

void motion(int x,int y) {
	control.psi += (x - WIDTH / 2) * 0.2;
	control.phi += (y - HEIGHT / 2) * 0.2;
	if(control.phi < -89) control.phi = -89;
	if(control.phi > 89) control.phi = 89;
}

int main(int argc,char **argv) {
	GLenum err;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

#if defined(DEMO_FULLSCREEN)
    glutGameModeString("800x600");
    glutEnterGameMode();
    glutSetCursor(GLUT_CURSOR_NONE);
#else
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("09 - Terrain");
#endif
	
	err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "glewInit() error: %s\n", glewGetErrorString(err));
	}

	if(init() != 0) return 1;
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_down);
	glutSpecialUpFunc(keyboard_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutMainLoop();
	return 0;
}
