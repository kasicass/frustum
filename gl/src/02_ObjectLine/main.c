#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <GL/glut.h>

#include "mathlib.h"
#include "loadjpeg.h"
#include "load3ds.h"

float camera[3],dir[3],light[4];
float view;
float angle;
float fps;
int list,texture;
int pause;

/*
 *
 */

void init(void) {
	unsigned char *data;
	float *model;
	int i,width,height,num_face;
	
	glClearDepth(1);
	glClearColor(0,0,0,0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHT0);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	
	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	if((data = LoadJPEG("./data/base.jpg",&width,&height))) {
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,
			GL_UNSIGNED_BYTE,data);
		free(data);
	}
	
	model = Load3DS("./data/model.3ds",&num_face);
	list = glGenLists(1);
	glNewList(list,GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for(i = 0; i < num_face; i++) {
		glNormal3fv((float*)&model[i << 3] + 3);
		glVertex3fv((float*)&model[i << 3]);
	}
	glEnd();
	glEndList();
	free(model);
}

/*
 *
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
	for(i = 0; i < l; i++)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13,buffer[i]);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/*
 *
 */

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* set matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,4.0 / 3.0,1,1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera[0],camera[1],camera[2],dir[0],dir[1],dir[2],0,0,1);

	/* set light */
	glLightfv(GL_LIGHT0,GL_POSITION,light);
	
	/* rendering */
	
	glPushMatrix();
	glRotatef(angle,0,0,1);

	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS,1,-1);
	glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glCallList(list);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glStencilFunc(GL_NOTEQUAL,1,-1);
	glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
	glLineWidth(8.0);
	glPolygonMode(GL_FRONT,GL_LINE);
	glColor3f(0,1,0);
	glCallList(list);
	glColor3f(1,1,1);
	glPolygonMode(GL_FRONT,GL_FILL);
	glDisable(GL_STENCIL_TEST);
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	
	glPopMatrix();
	
	dprintf(-0.95,0.95,"fps: %.2f",fps);
	
	glutSwapBuffers();
}

/*
 *
 */

void idle(void) {
	float ifps;
	
	fps = getfps();
	ifps = 1.0 / fps;
	
	if(!pause) view += ifps;
	
	VectorSet(0,120,240 * cos(view / 3),camera);
	VectorSet(0,0,0,dir);
	VectorCopy(camera,light);
	light[3] = 1.0;
	
	angle -= ifps * 360.0 * 0.2;
	
	glutPostRedisplay();
}

/*
 *
 */

void keyboard(unsigned char c,int x,int y) {
	switch(c) {
		case 27:
			exit(1);
			break;
		case ' ':
			pause = !pause;
			break;
	}
}

/*
 *
 */

int main(int argc,char **argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);

#if defined(DEMO_FULLSCREEN)
    glutGameModeString("800x600");
    glutEnterGameMode();
    glutSetCursor(GLUT_CURSOR_NONE);
#else
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("02 - Object Line");
#endif

	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
