// NV Vertex Programm
// runs only on NV video card

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include "mathlib.h"
#include "loadjpeg.h"
#include "load3ds.h"

float camera[3],dir[3],light[4];
float view;
float angle;
float fps;
float stage;
int dstage;
int list;
int texture;
int program; /* vertex program */

int pause,cull,vp;

/*
 *
 */

unsigned char *loadfile(char *name) {
	FILE *file;
	int size;
	unsigned char *data;
	file = fopen(name,"r");
	if(!file) { fprintf(stderr,"error load %s file",name); return NULL; }
	fseek(file,0,SEEK_END);
	size = ftell(file);
	data = (unsigned char*)malloc(size + 1);
	fseek(file,0,SEEK_SET);
	fread(data,1,size,file);
	data[size] = 0;
	fclose(file);
	return data;
}

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
	
	data = loadfile("program.vp");
	
	glGenProgramsNV(1,&program);
	glBindProgramNV(GL_VERTEX_PROGRAM_NV,program);
	glLoadProgramNV(GL_VERTEX_PROGRAM_NV,program,
		strlen((char*)data),data);
	
	glTrackMatrixNV(GL_VERTEX_PROGRAM_NV,0,
		GL_MODELVIEW_PROJECTION_NV,GL_IDENTITY_NV);

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
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0,GL_POSITION,light);
	
	/* rendering */
	
	glPushMatrix();
	glRotatef(angle,0,0,1);
	
	glEnable(GL_TEXTURE_2D);
	
	glCallList(list);

	glDisable(GL_TEXTURE_2D);
	
	if(!vp) {
		if(cull) glCullFace(GL_FRONT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
	
		glColor4f(0,0,0.5,1);
	
		glEnable(GL_VERTEX_PROGRAM_NV);
		glBindProgramNV(GL_VERTEX_PROGRAM_NV,program);
		glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV,5,stage * 4 + 0.1,1,1,1);
		glCallList(list);
		glDisable(GL_VERTEX_PROGRAM_NV);
	
		glColor4f(1,1,1,1);
	
		glDisable(GL_BLEND);
		if(cull) glCullFace(GL_BACK);
	}
	
	glPopMatrix();
	
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
	
	if(!pause) view += ifps;
	
	VectorSet(0,100,200 * cos(view / 3),camera);
	VectorSet(0,0,0,dir);
	VectorCopy(camera,light);
	light[3] = 1.0;
	
	angle -= ifps * 360.0 * 0.2;
	
	if(dstage) stage += ifps * 1.0;
	else stage -= ifps * 1.0;
	if(stage > 1.0 || stage < 0.0) dstage = !dstage;
	
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
		case 'c':
			cull = !cull;
			break;
		case 'v':
			vp = !vp;
			break;
	}
}

/*
 *
 */

int main(int argc,char **argv) {
	GLenum err;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

#if defined(DEMO_FULLSCREEN)
    glutGameModeString("800x600");
    glutEnterGameMode();
    glutSetCursor(GLUT_CURSOR_NONE);
#else
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("08 - VertexProgram");
#endif

	err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "glewInit() error: %s\n", glewGetErrorString(err));
	}

	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
