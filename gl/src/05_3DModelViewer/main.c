// viewer.exe <.md3/.3ds/.mdc file>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <GL/glut.h>

float *Load3DS(char*,int*);
float *LoadMD3(char*,int*);
float *LoadMDC(char*,int*);
unsigned char *LoadJPEG(char*,int*,int*);
unsigned char *LoadTGA(char*,int*,int*);
unsigned char *LoadPNG(char*,int*,int*);

#define RADIUS 100.0

int vertex_list;
char *texturename;
int texture_id;
float pos[3];
float phi,psi,theta;
float fov = 45;
float aspect = 4.0 / 3.0;
int mouse_x,mouse_y,mouse_button,mouse_state;
float delta_x,delta_y;

void init(void) {
	glClearDepth(1.0);
	glClearColor(0,0,0,0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glColor4f(1,1,1,1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void load_texture(char *name) {
	unsigned char *data = NULL;
	int width,height;
	if(!name) return;
	if(strstr(name,".jpg") || strstr(name,".JPG"))
		data = LoadJPEG(name,&width,&height);
	else if(strstr(name,".tga") || strstr(name,".TGA"))
		data = LoadTGA(name,&width,&height);
//	else if(strstr(name,".png") || strstr(name,".PNG"))
//		data = LoadPNG(name,&width,&height);
	if(!data) {
		fprintf(stderr,"%s load error\n",name);
		return;
	}
	glEnable(GL_TEXTURE_2D);
	if(texture_id) glDeleteTextures(1,&texture_id);
	glGenTextures(1,&texture_id);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,
		GL_UNSIGNED_BYTE,data);
	free(data);
}

void mesh_size(float *vertex,int num_vertex,float *min,float *max) {
    int i;
    min[0] = min[1] = min[2] = 1000000;
    max[0] = max[1] = max[2] = -1000000;
    for(i = 0; i < num_vertex; i++) {
		int j = i * 8;
		float l;
        l = vertex[j + 0];
        if(l > max[0]) max[0] = l;
        if(l < min[0]) min[0] = l;
		l = vertex[j + 1];
        if(l > max[1]) max[1] = l;
        if(l < min[1]) min[1] = l;
		l = vertex[j + 2];
        if(l > max[2]) max[2] = l;
        if(l < min[2]) min[2] = l;
    }
}

float mesh_radius(float *vertex,int num_vertex) {
    int i;
	float tmp,radius = 0;
    for(i = 0; i < num_vertex; i++) {
		int j = i * 8;
		tmp = sqrt(vertex[j + 0] * vertex[j + 0] +
			vertex[j + 1] * vertex[j + 1] +
			vertex[j + 2] * vertex[j + 2]);
		if(radius < tmp) radius = tmp;
    }
	return radius;
}

void mesh_translate(float *vertex,int num_vertex,float x,float y,float z) {
    int i;
    for(i = 0; i < num_vertex; i++) {
		int j = i * 8;
		vertex[j + 0] += x;
		vertex[j + 1] += y;
		vertex[j + 2] += z;
    }
}

void mesh_scale(float *vertex,int num_vertex,float scale) {
    int i;
    for(i = 0; i < num_vertex; i++) {
		int j = i * 8;
		vertex[j + 0] *= scale;
		vertex[j + 1] *= scale;
		vertex[j + 2] *= scale;
    }
}

int load(int argc,char **argv) {
	char *name;
	int i,num_vertex;
	float *vertex,min[3],max[3];
	for(i = 1; i < argc - 1; i++) {
		if(argv[i][0] == '-') {
			char *s = argv[i];
			while(*++s == '-');
			if(!strcmp(s,"t")) {
				texturename = argv[++i];
				load_texture(texturename);
			} else if(!strcmp(s,"help")) {
				printf("3dview\n");
				printf("written by Alexander Zaprjagaev\n");
				printf("frustum@public.tsu.ru\n");
				printf("-t <texturename> use this texture\n");
				printf("-help print this\n");
				printf("key:\n");
				printf("r - reset coordinate\n");
				printf("w - wareframe\n");
				printf("c - cull face\n");
				printf("g - texgen\n");
				printf("t - reload texture\n");
				return 0;
			} else fprintf(stderr,"unknow option %s\n",argv[i]);
		} else fprintf(stderr,"unknow option %s\n",argv[i]);
	}
	name = argv[argc - 1];
	if(strstr(name,".3ds") || strstr(name,".3DS"))
		vertex = Load3DS(name,&num_vertex);
	else if(strstr(name,".md3") || strstr(name,".MD3"))
		vertex = LoadMD3(name,&num_vertex);
	else if(strstr(name,".mdc") || strstr(name,".MDC"))
		vertex = LoadMDC(name,&num_vertex);
	else if((vertex = Load3DS(name,&num_vertex)));
	else if((vertex = LoadMD3(name,&num_vertex)));
	else if((vertex = LoadMDC(name,&num_vertex)));
	if(!vertex) {
		fprintf(stderr,"%s load error\n",argv[argc - 1]);
		return 0;
	}
	mesh_size(vertex,num_vertex,min,max);
	printf("num face %d\n",num_vertex / 3);
	printf("min %f %f %f\n",min[0],min[1],min[2]);
	printf("max %f %f %f\n",max[0],max[1],max[2]);
	mesh_translate(vertex,num_vertex,-(min[0] + max[0]) / 2.0,
		-(min[1] + max[1]) / 2.0,-(min[2] + max[2]) / 2.0);
	mesh_scale(vertex,num_vertex,RADIUS / mesh_radius(vertex,num_vertex));
	vertex_list = glGenLists(1);
	glNewList(vertex_list,GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for(i = 0; i < num_vertex; i++) {
		float *ptr = &vertex[i << 3];
		glTexCoord2fv(ptr + 6);
		glNormal3fv(ptr + 3);
		glVertex3fv(ptr);
	}
	glEnd();
	glEndList();
	free(vertex);
	return 1;
}

void display(void) {
	float light[] = { 0, 300, 0, 1 };
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov,aspect,1,600);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,300,1, 0,0,0, 0,0,1);
	glLightfv(GL_LIGHT0,GL_POSITION,light);
	
	glTranslatef(pos[0],pos[1],pos[2]);
	glRotatef(theta,1,0,0);
	glRotatef(phi,0,0,1);
	glRotatef(psi,0,1,0);
	
	glCallList(vertex_list);
	
	glutSwapBuffers();
}

void reshape(int w,int h) {
	aspect = (float)w / (float)h;
	glViewport(0,0,w,h);
}

void idle(void) {
	switch(mouse_button) {
		case GLUT_LEFT_BUTTON:
			phi += delta_x / 4.0;
			theta -= delta_y / 4.0;
			break;
		case GLUT_MIDDLE_BUTTON:
			pos[0] -= delta_x / 2.0;
			pos[2] -= delta_y / 2.0;
			break;
		case GLUT_RIGHT_BUTTON:
			psi -= delta_x / 4.0;
			fov -= delta_y / 8.0;
			if(fov < 0.1) fov = 0.1;
			if(fov > 60.0) fov = 60.0;
			break;
	}
	if(mouse_state == GLUT_DOWN) {
		delta_x = 0;
		delta_y = 0;
	}
	glutPostRedisplay();
}

void motion(int x,int y) {
	delta_x = x - mouse_x;
	delta_y = y - mouse_y;
	mouse_x = x;
	mouse_y = y;
	switch(mouse_button) {
		case GLUT_LEFT_BUTTON:
			phi += delta_x / 4.0;
			theta -= delta_y / 4.0;
			break;
		case GLUT_MIDDLE_BUTTON:
			pos[0] -= delta_x / 2.0;
			pos[2] -= delta_y / 2.0;
			break;
		case GLUT_RIGHT_BUTTON:
			psi -= delta_x / 4.0;
			fov -= delta_y / 8.0;
			if(fov < 0.1) fov = 0.1;
			if(fov > 60.0) fov = 60.0;
			break;
	}
	glutPostRedisplay();
}

void mouse(int button,int state,int x,int y) {
	mouse_state = state;
	if(state == GLUT_DOWN) mouse_button = button;
	mouse_x = x;
	mouse_y = y;
}

void keyboard(unsigned char c,int x,int y) {
	static int wareframe = 0;
	static int cullface = 1;
	static int texgen = 0;
	switch(c) {
		case 27:
			exit(1);
			break;
		case 'r':
		case 'R':
			pos[0] = pos[1] = pos[2] = 0.0;
			phi = psi = theta = 0;
			delta_x = delta_y = 0;
			fov = 45;
			break;
		case 't':
		case 'T':
			load_texture(texturename);
			break;
		case 'c':
		case 'C':
			cullface = !cullface;
			if(cullface) glEnable(GL_CULL_FACE);
			else glDisable(GL_CULL_FACE);
			break;
		case 'w':
		case 'W':
			wareframe = !wareframe;
			if(wareframe) glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			break;
		case 'g':
		case 'G':
			texgen = !texgen;
			if(texgen) {
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
			} else {
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
			}
			break;
	}
}

int main(int argc,char **argv) {
	char title[256];
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800,600);
	sprintf(title,"3dview %s",argv[argc - 1]);
	glutCreateWindow(title);
	init();
	if(!load(argc,argv)) return 1;
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();
	return 0;
}