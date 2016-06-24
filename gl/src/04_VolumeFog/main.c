#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <GL/glut.h>

#include "mathlib.h"
#include "load3ds.h"
#include "loadjpeg.h"

#define GL_FOG_COORDINATE_SOURCE_EXT     0x8450
#define GL_FOG_COORDINATE_EXT            0x8451

typedef void (APIENTRY * PFNGLFOGCOORDFEXTPROC) (GLfloat coord);
PFNGLFOGCOORDFEXTPROC glFogCoordfEXT = NULL;

float camera[3],dir[3],light[4];
float angle,alpha;
float dist = 200;
float fps;
float *vertex;
int num_vertex;
int texture;
int fog;
float fogcolor[] = { 0.1, 0.1, 0.2, 1.0 };

/*
 *
 */

void init(void) {
	unsigned char *data;
	int width,height;
	
	glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC) wglGetProcAddress("glFogCoordfEXT");

	glClearDepth(1);
	glClearColor(fogcolor[0],fogcolor[1],fogcolor[2],fogcolor[3]);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogfv(GL_FOG_COLOR,fogcolor);
	glFogf(GL_FOG_START,0);
	glFogf(GL_FOG_END,50);
	glFogi(GL_FOG_COORDINATE_SOURCE_EXT,GL_FOG_COORDINATE_EXT);
	
	glEnable(GL_LIGHT0);

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	if((data = LoadJPEG("./data/ground.jpg",&width,&height))) {
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,
			GL_UNSIGNED_BYTE,data);
		free(data);
	}
	
	vertex = Load3DS("./data/ground.3ds",&num_vertex);
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
	int i,j;
	float foglevel;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* set matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,4.0 / 3.0,1,1200);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera[0],camera[1],camera[2],dir[0],dir[1],dir[2],0,0,1);

	/* set light */
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0,GL_POSITION,light);
	
	/* rendering */
	
	if(!fog) glEnable(GL_FOG);
	glEnable(GL_TEXTURE_2D);
	
	glBegin(GL_TRIANGLES);
	for(i = 0; i < num_vertex; i++) {
		j = i << 3;
		glNormal3fv((float*)&vertex[j] + 3);
		glTexCoord2fv((float*)&vertex[j] + 6);
		
		if(vertex[j + 2] < 20.0) foglevel = (-vertex[j + 2] + 20.0) / 2.0;
		else foglevel = 0.0;
		if(vertex[j + 0] > 200) foglevel += vertex[j + 0] - 200;
		else if(vertex[j + 0] < -200) foglevel += -vertex[j + 0] - 200;
		if(vertex[j + 1] > 200) foglevel += vertex[j + 1] - 200;
		else if(vertex[j + 1] < -200) foglevel += -vertex[j + 1] - 200;

		glFogCoordfEXT(foglevel);
		
		glVertex3fv((float*)&vertex[j]);
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);
	if(!fog) glDisable(GL_FOG);
	
	/* all disable */
	glDisable(GL_LIGHTING);
	
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
	
	angle += ifps * 0.1;
	
	VectorSet(dist * cos(angle),dist * sin(angle),
		dist * (cos(alpha) + 1) / 2,camera);
	VectorSet(0,0,0,dir);
	
	VectorSet(0,0,1000,light);
	light[3] = 1.0;
	
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
		case 'a':
			dist -= 10;
			break;
		case 'z':
			dist += 10;
			break;
		case 's':
			alpha += 0.02;
			break;
		case 'x':
			alpha -= 0.02;
			break;
		case 'f':
			fog = !fog;
			break;
	}
}

/*
 *
 */

int main(int argc,char **argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

#if defined(DEMO_FULLSCREEN)
    glutGameModeString("800x600");
    glutEnterGameMode();
    glutSetCursor(GLUT_CURSOR_NONE);
#else
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("04 - VolumeFog");
#endif

	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
