/* last update 20040614
 * thanks to Andreas Stenglein <A.Stenglein@gmx.net>
 * and Brian Pail http://mesa3d.sourceforge.net/
 * for the fix of three errors
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include "mathlib.h"
#include "loadjpeg.h"
#include "load3ds.h"

#define WIDTH 1024
#define HEIGHT 768

#define MAP_SIZE 128
#define PARTICLE_TIME 8
#define PARTICLE_SPEED 30

typedef struct {
	float *vertex;
	int num_vertex;
	int base;
	int specular;
} mesh_t;

typedef struct {
	float pos[3];
	float phi,psi;
	mesh_t *mesh;
} mesh_object_t;

typedef struct {
	float *particle;
	int num_particle;
	float radius;
	float center[3];
	float speed[3];
	float maxspeed;
	float color[4];
	int texture_id;
} particle_t;

mesh_t *plane_m,*mesh0_m,*mesh1_m,*mesh2_m;
mesh_object_t object[7];
particle_t *particle[5];
float particle_center[5][3] = {
	{   0,   0, 60 },
	{ 120,   0, 40 },
	{-120,   0, 40 },
	{   0, 120, 40 },
	{   0,-120, 40 },
};
float particle_color[2][4] = {
	{ 0.35, 0.35, 0.70, 1.0 },
	{ 0.73, 0.72, 0.20, 1.0 },
};
int cubemap_id;
int cubemap_define[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
};
float sphere[3];
int sphere_list;
float phi = 40;
float psi = 30;
float dist = 380;
float modelview[16],imodelview[16];
float time,fps;
int pause = 0;
int look = 0;
int drop = 0;
int mode = 0;
int cube = 0;
int particle_on = 1;

/*	simple mesh object
 *
 */

mesh_t *mesh_load(char *name,char *base,char *specular) {
	int width,height;
	unsigned char *data;
	mesh_t *mesh;
	mesh = (mesh_t*)malloc(sizeof(mesh_t));
	if(!mesh) return NULL;
	memset(mesh,0,sizeof(mesh_t));
	mesh->vertex = Load3DS(name,&mesh->num_vertex);
	if(!mesh->vertex) return NULL;
	if(base) {
		glGenTextures(1,&mesh->base);
		glBindTexture(GL_TEXTURE_2D,mesh->base);
		if(base && (data = LoadJPEG(base,&width,&height))) {
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,
				GL_UNSIGNED_BYTE,data);
			free(data);
		}
	}
	if(specular) {
		glGenTextures(1,&mesh->specular);
		glBindTexture(GL_TEXTURE_2D,mesh->specular);
		if((data = LoadJPEG(specular,&width,&height))) {
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,
				GL_UNSIGNED_BYTE,data);
			free(data);
		}
	}
	return mesh;
}

void mesh_render(mesh_t *mesh) {
	if(mesh == NULL) return;
	if(mesh->base) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,mesh->base);
	} else glDisable(GL_TEXTURE_2D);
	if(mesh->specular) {
		glActiveTexture(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,mesh->specular);
		glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
		glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3,GL_FLOAT,sizeof(float) * 8,mesh->vertex + 0);
	glNormalPointer(GL_FLOAT,sizeof(float) * 8,mesh->vertex + 3);
	glTexCoordPointer(2,GL_FLOAT,sizeof(float) * 8,mesh->vertex + 6);
	glDrawArrays(GL_TRIANGLES,0,mesh->num_vertex);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(mesh->specular) {
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0_ARB);
	}
	if(mesh->base) glDisable(GL_TEXTURE_2D);
}

void object_render(mesh_object_t *object) {
	glPushMatrix();
	glTranslatef(object->pos[0],object->pos[1],object->pos[2]);
	glRotatef(object->psi,0,0,1);
	glRotatef(object->phi,0,1,0);
	mesh_render(object->mesh);
	glPopMatrix();
}

/*	particle
 *
 */

float noise(float a) {
	return sqrt(-2 * log((float)rand() / RAND_MAX)) * sin(2 * PI * (float)rand() / RAND_MAX) * a;
}

particle_t *particle_create(float *center,int num_particle,float radius,
	float maxspeed,float *color,int texture_id) {
	int i;
	particle_t *particle;
	particle = (particle_t*)malloc(sizeof(particle_t));
	particle->particle = (float*)malloc(sizeof(float) * 8 * num_particle);
	memset(particle->particle,0,sizeof(float) * 8 * num_particle);
	for(i = 0; i < num_particle; i++) {
		int j = i << 3;
		VectorCopy(center,&particle->particle[j]);
		VectorSet(noise(maxspeed),noise(maxspeed),noise(maxspeed),
		  &particle->particle[j + 3]);
		particle->particle[j + 5] += PARTICLE_SPEED;
		particle->particle[j + 7] =
			(float)i / (float)num_particle * PARTICLE_TIME;
	}
	VectorCopy(center,particle->center);
	particle->num_particle = num_particle;
	particle->radius = radius;
	particle->maxspeed = maxspeed;
	particle->color[0] = color[0];
	particle->color[1] = color[1];
	particle->color[2] = color[2];
	particle->color[3] = color[3];
	particle->texture_id = texture_id;
	return particle;
}

void particle_render(particle_t *particle,float *im) {
	int i;
	float dx[3],dy[3];
	if(!particle || !im || !particle_on) return;
	VectorSet(1,0,0,dx);
	VectorSet(0,1,0,dy);
	VectorTransformNormal(dx,im,dx);
	VectorTransformNormal(dy,im,dy);
	VectorScale(dx,particle->radius,dx);
	VectorScale(dy,particle->radius,dy);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,particle->texture_id);
	for(i = 0; i < particle->num_particle; i++) {
		int j = i << 3;
		float life = 1 - particle->particle[j + 7] / PARTICLE_TIME;
		float pos[3];
		VectorCopy(&particle->particle[j],pos);
		glColor4f(particle->color[0] * life,
				  particle->color[1] * life,
				  particle->color[2] * life,1);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0,1);
		glVertex3f(pos[0]-dx[0]-dy[0],pos[1]-dx[1]-dy[1],pos[2]-dx[2]-dy[2]);
		glTexCoord2f(1,1);
		glVertex3f(pos[0]+dx[0]-dy[0],pos[1]+dx[1]-dy[1],pos[2]+dx[2]-dy[2]);
		glTexCoord2f(0,0);
		glVertex3f(pos[0]-dx[0]+dy[0],pos[1]-dx[1]+dy[1],pos[2]-dx[2]+dy[2]);
		glTexCoord2f(1,0);
		glVertex3f(pos[0]+dx[0]+dy[0],pos[1]+dx[1]+dy[1],pos[2]+dx[2]+dy[2]);
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

float particle_force(float *pos,float *center,float level,float *force) {
	float a[3],b[3],angle,length;
	VectorCopy(pos,a);
	VectorSub(a,center,a);
	a[2] = 0;
	length = VectorNormalize(a,a);
	VectorSet(1,0,0,b);
	angle = acos(VectorDotProduct(a,b));
	if(a[1] < 0) angle *= -1;
	VectorSet(cos(angle + PI / 2),sin(angle + PI / 2),0,force);
	if(length > 1) level *= length / 20;
	VectorScale(force,level,force);
	return angle * RAD2DEG;
}

void particle_update(particle_t *particle,float ifps) {
	int i;
	if(!particle) return;
	for(i = 0; i <particle->num_particle; i++) {
		int j = i << 3;
		particle->particle[j + 7] += ifps;
		if(particle->particle[j + 7] > PARTICLE_TIME) {
			float maxspeed = particle->maxspeed;
			VectorCopy(particle->center,&particle->particle[j]);
			VectorSet(noise(maxspeed),noise(maxspeed),noise(maxspeed),
				&particle->particle[j + 3]);
			particle->particle[j + 5] += PARTICLE_SPEED;
			particle->particle[j + 7] -= PARTICLE_TIME;
		} else {
			float speed[3],force[3];
			VectorScale(&particle->particle[j + 3],ifps,speed);
			VectorAdd(&particle->particle[j],speed,&particle->particle[j]);
			particle->particle[j + 2] += -9.8 * ifps;
			particle_force(&particle->particle[j],particle->center,
				20 * ifps,force);
			VectorAdd(&particle->particle[j],force,&particle->particle[j]);
		}
	}
}

/*	initialization
 *
 */

void init(void) {
	unsigned char *data;
	int i,width,height;
	float ambient[] = { 0.3, 0.3, 0.3, 1.0 };

	/* set openGL */
	
	glClearDepth(1);
	glClearColor(0,0,0,0);
	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHT0);
	glMaterialfv(GL_FRONT,GL_EMISSION,ambient);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogf(GL_FOG_START,500);
	glFogf(GL_FOG_END,1500);

	/* create cubemap */
	
	glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	glGenTextures(1,&cubemap_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB,cubemap_id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
	for(i = 0; i < 6; i++) {
		glTexImage2D(cubemap_define[i],0,GL_RGBA,MAP_SIZE,MAP_SIZE,0,GL_RGBA,
			GL_UNSIGNED_BYTE,NULL);
	}
	glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	
	/* create particle */
	
	glGenTextures(1,&i);
	glBindTexture(GL_TEXTURE_2D,i);
	if((data = LoadJPEG("data/particle.jpg",&width,&height))) {
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,
			GL_UNSIGNED_BYTE,data);
		free(data);
	}

	particle[0] = particle_create(particle_center[0],400,6,20,
		particle_color[0],i);
	particle[1] = particle_create(particle_center[1],200,6,10,
		particle_color[1],i);
	particle[2] = particle_create(particle_center[2],200,6,10,
		particle_color[1],i);
	particle[3] = particle_create(particle_center[3],200,6,10,
		particle_color[1],i);
	particle[4] = particle_create(particle_center[4],200,6,10,
		particle_color[1],i);

	/* load objects */
	
	plane_m = mesh_load("data/plane.3ds","data/plane.jpg",NULL);
	mesh0_m = mesh_load("data/mesh0.3ds","data/mesh0.jpg","data/env.jpg");
	mesh1_m = mesh_load("data/mesh1.3ds","data/mesh1.jpg","data/env.jpg");
	mesh2_m = mesh_load("data/mesh2.3ds",NULL,"data/env.jpg");

	VectorSet(0,0,0,object[0].pos);
	object[0].mesh = plane_m;
	VectorSet(0,0,0,object[1].pos);
	object[1].mesh = mesh0_m;
	VectorSet(0,120,0,object[2].pos);
	object[2].mesh = mesh1_m;
	VectorSet(0,-120,0,object[3].pos);
	object[3].mesh = mesh1_m;
	VectorSet(120,0,0,object[4].pos);
	object[4].mesh = mesh1_m;
	VectorSet(-120,0,0,object[5].pos);
	object[5].mesh = mesh1_m;
	VectorSet(0,0,80,object[6].pos);
	object[6].mesh = mesh2_m;

	sphere_list = glGenLists(1);
	glNewList(sphere_list,GL_COMPILE);
	glutSolidSphere(20,64,64);
	glEndList();
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

void render_scene(float *im) {
	int i;
	float light[] = { 0, 1000, 1000, 0 };
	glLightfv(GL_LIGHT0,GL_POSITION,light);
	glEnable(GL_LIGHTING);
	for(i = 0; i < 7; i++) object_render(&object[i]);
	glDisable(GL_LIGHTING);
	if(im) for(i = 0; i < 5; i++) particle_render(particle[i],im);
}

void create_cubemap(float *pos) {
	int i;
	float m[16],im[16];
	glViewport(0,0,MAP_SIZE,MAP_SIZE);
	glScissor(0,0,MAP_SIZE,MAP_SIZE);
	glEnable(GL_SCISSOR_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90,1,1,2000);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	for(i = 0; i < 6; i++) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		switch(i) {
			case 0:
				glRotatef(90,0,1,0);
				glRotatef(180,1,0,0);
				break;
			case 1:
				glRotatef(-90,0,1,0);
				glRotatef(180,1,0,0);
				break;
			case 2:
				glRotatef(-90,1,0,0);
				break;
			case 3:
				glRotatef(90,1,0,0);
				break;
			case 4:
				glRotatef(180,1,0,0);
				break;
			case 5:
				glRotatef(180,0,0,1);
				break;
		}
		glGetFloatv(GL_MODELVIEW_MATRIX,m);
		glTranslatef(-pos[0],-pos[1],-pos[2]);
		glGetFloatv(GL_MODELVIEW_MATRIX,m);
		MatrixTranspose(m,im);
		render_scene(im);
		glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB,cubemap_id);
		glCopyTexSubImage2D(cubemap_define[i],0,0,0,0,0,MAP_SIZE,MAP_SIZE);
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	}
	glDisable(GL_TEXTURE_2D);
	glViewport(0,0,WIDTH,HEIGHT);
	glScissor(0,0,WIDTH,HEIGHT);
	glDisable(GL_SCISSOR_TEST);
}

void display(void) {
	static int counter;

	if(!cube) {
		if(drop) {
			if(counter == 1) {
				create_cubemap(sphere);
				counter = 0;
			} else counter++;
		} else create_cubemap(sphere);
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/* set matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,4.0 / 3.0,1,2000);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelview);

	/* rendering */

	if(!cube) {
		glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		if(mode) {
			glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP_ARB);
			glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP_ARB);
			glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP_ARB);
		} else {
			glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB);
			glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB);
			glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB);
		}
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB,cubemap_id);
		glPushMatrix();
		glTranslatef(sphere[0],sphere[1],sphere[2]);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		
		imodelview[3] = 0;
		imodelview[7] = 0;
		imodelview[11] = 0;
		imodelview[15] = 1;
		glLoadMatrixf(imodelview);
		
		if(mode) glScalef(1,1,-1);
		glCallList(sphere_list);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
	
		glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP_ARB);
		glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP_ARB);
		glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP_ARB);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB,cubemap_id);
		glPushMatrix();
		glTranslatef(sphere[0],sphere[1],sphere[2]);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadMatrixf(imodelview);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
		glScalef(1,1,-1);
		glCallList(sphere_list);
		glDisable(GL_BLEND);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	}
	
	render_scene(imodelview);
	
	dprintf(-0.95,0.95,"fps: %.2f",fps);
	dprintf(-0.95,0.90,"drop %s look %s",
		drop ? "yes" : "no",look ? "sphere" : "center");
	dprintf(-0.95,0.85,"mode %s",mode ? "refract" : "reflect");
	
	{
		GLenum error;
		while((error = glGetError()) != GL_NO_ERROR) {
			fprintf(stderr,"OpenGL error 0x%04X: %s\n",error,gluErrorString(error));
		}
	}
	
	glutSwapBuffers();
}

/*
 *
 */

void idle(void) {
	int i;
	float ifps;
	float camera[3],dir[4],up[3];
	float q0[4],q1[4],q2[4],m0[16];
	
	fps = getfps();
	ifps = 1.0 / fps;
	if(!pause) time += ifps;
	
	for(i = 0; i < 5; i++) particle_update(particle[i],ifps);
	
	object[6].psi += ifps * 100;
	object[6].phi = sin(time) * 30;
	VectorSet(70 * sin(time / 5),70 * cos(time / 5),
		70 + 45 * sin(time / 3),sphere);
	
	VectorSet(0,0,1,dir);
	QuaternionSet(dir,psi,q0);
	VectorSet(0,1,0,dir);
	QuaternionSet(dir,-phi,q1);
	QuaternionMultiply(q0,q1,q2);
	QuaternionToMatrix(q2,m0);
	VectorSet(dist,0,0,camera);
	VectorTransform(camera,m0,camera);
	if(look) { VectorCopy(sphere,dir); }
	else VectorSet(0,0,0,dir);
	VectorSet(0,0,1,up);
	VectorAdd(camera,dir,camera);
	MatrixLookAt(camera,dir,up,modelview);
	MatrixTranspose(modelview,imodelview);
	
	glutWarpPointer(WIDTH / 2,HEIGHT / 2);
	glutPostRedisplay();
}

/*
 *
 */

void keyboard(unsigned char c,int x,int y) {
	static int wareframe;
	switch(c) {
		case 27:
			exit(1);
			break;
		case 'w':
		case 'W':
			wareframe = !wareframe;
			glPolygonMode(GL_FRONT_AND_BACK,wareframe ? GL_LINE : GL_FILL);
			break;
		case ' ':
			pause = !pause;
			break;
		case 's':
		case 'S':
			look = !look;
			break;
		case 'd':
		case 'D':
			drop = !drop;
			break;
		case 'c':
		case 'C':
			cube = !cube;
			break;
		case 'm':
		case 'M':
			mode = !mode;
			break;
		case 'p':
		case 'P':
			particle_on = !particle_on;
			break;
	}
}

void mouse(int button,int state,int x,int y) {
	if(button == 3) dist -= 10;
	if(button == 4) dist += 10;
	if(dist < 30) dist = 30;
	if(dist > 1000) dist = 1000;
}

void motion(int x,int y) {
	psi += (x - WIDTH / 2) * 0.2;
	phi += (y - HEIGHT / 2) * 0.2;
	if(phi < -89) phi = -89;
	if(phi > 89) phi = 89;
}

int main(int argc,char **argv) {
	GLenum err;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	//glutGameModeString("800x600@32");
	//glutEnterGameMode();
	glutCreateWindow("05 - CubeMap");
	glutReshapeWindow(WIDTH,HEIGHT);

	err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "glewInit() error: %s\n", glewGetErrorString(err));
	}

	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutWarpPointer(WIDTH / 2,HEIGHT / 2);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutMainLoop();
	return 0;
}
