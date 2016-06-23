/*  project texture demo
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include <GL/glut.h>
#include "mathlib.h"

unsigned char *LoadJPEG(char*,int*,int*);
float *Load3DS(char*,int*);

int findcrosspoint(const float *a,const float *b,float *point);

#define SHADOWSIZE 256
#define MODELSIZE 32

int groundlist;
int modellist;
int lightlist;

float lightpos[4];
float objpos[3];
float camerapos[3];
float cameradir[3];

int stop;
int view;
float dist = 100;
float alpha = 60;

float modelangle;
float modelsize;
float *vertexmodel;
int num_vertexmodel;

float *vertexground;
float *planeground;
int num_vertexground;

int textureground;
int textureshadow;
int texturemodel;

void init(void) {
    float tmp,xmin,xmax,ymin,ymax,zmin,zmax,a[3],b[3];
    int i,j,k,width,height;
    unsigned char *data;
    float plane_S[] = { 1.0, 0.0, 0.0, 0.0 };
    float plane_T[] = { 0.0, 1.0, 0.0, 0.0 };
    float plane_R[] = { 0.0, 0.0, 1.0, 0.0 };
    float plane_Q[] = { 0.0, 0.0, 0.0, 1.0 };

    glClearDepth(1.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_LIGHT0);
    
    modelsize = 0;
    vertexmodel = Load3DS("data/model.3ds",&num_vertexmodel);
    printf("./data/model.3ds %d face\n",num_vertexmodel / 3);
    
    xmin = xmax = ymin = ymax = zmin = zmax = 0;
    
    for(i = 0; i < num_vertexmodel; i++) {
        if(vertexmodel[(i << 3) + 0] < xmin) xmin = vertexmodel[(i << 3) + 0];
        if(vertexmodel[(i << 3) + 0] > xmax) xmax = vertexmodel[(i << 3) + 0];
        if(vertexmodel[(i << 3) + 1] < ymin) ymin = vertexmodel[(i << 3) + 1];
        if(vertexmodel[(i << 3) + 1] > ymax) ymax = vertexmodel[(i << 3) + 1];
        if(vertexmodel[(i << 3) + 2] < zmin) zmin = vertexmodel[(i << 3) + 2];
        if(vertexmodel[(i << 3) + 2] > zmax) zmax = vertexmodel[(i << 3) + 2];
    }
    
    for(i = 0; i < num_vertexmodel; i++) {
        vertexmodel[(i << 3) + 0] -= (xmax + xmin) / 2.0;
        vertexmodel[(i << 3) + 1] -= (ymax + ymin) / 2.0;
        vertexmodel[(i << 3) + 2] -= (zmax + zmin) / 2.0;
    }
    
    for(i = 0; i < num_vertexmodel; i++) {
        tmp = sqrt(vertexmodel[(i << 3) + 0] * vertexmodel[(i << 3) + 0] +
                    vertexmodel[(i << 3) + 1] * vertexmodel[(i << 3) + 1] +
                    vertexmodel[(i << 3) + 2] * vertexmodel[(i << 3) + 2]);
        if(tmp > modelsize) modelsize = tmp;
    }
    
    tmp = MODELSIZE / modelsize;
    modelsize = MODELSIZE;
    for(i = 0; i < num_vertexmodel; i++) {
        vertexmodel[(i << 3) + 0] *= tmp;
        vertexmodel[(i << 3) + 1] *= tmp;
        vertexmodel[(i << 3) + 2] *= tmp;
    }
    
    vertexground = Load3DS("data/ground.3ds",&num_vertexground);
    printf("./data/ground.3ds %d face\n",num_vertexground / 3);
    
    planeground = (float*)malloc(sizeof(float) * 4 * num_vertexground / 3);
    for(i = 0, j = 0, k = 0; i < num_vertexground; i += 3, j += 24, k += 4) {
        VectorSub(&vertexground[j + 8],&vertexground[j],a);
        VectorSub(&vertexground[j + 16],&vertexground[j],b);
        VectorCrossProduct(a,b,&planeground[k]);
        VectorNormalize(&planeground[k],&planeground[k]);
        planeground[k + 3] = -VectorDotProduct(&planeground[k],&vertexground[j]);
    }
    
    glGenTextures(1,&textureground);
    glBindTexture(GL_TEXTURE_2D,textureground);
    if((data = LoadJPEG("data/ground.jpg",&width,&height))) {
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    
    glGenTextures(1,&texturemodel);
    glBindTexture(GL_TEXTURE_2D,texturemodel);
    if((data = LoadJPEG("data/model.jpg",&width,&height))) {
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    
    glGenTextures(1,&textureshadow);
    glBindTexture(GL_TEXTURE_2D,textureshadow);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D,0,4,SHADOWSIZE,SHADOWSIZE,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    glTexGenfv(GL_S,GL_EYE_PLANE,plane_S);
    glTexGenfv(GL_T,GL_EYE_PLANE,plane_T);
    glTexGenfv(GL_R,GL_EYE_PLANE,plane_R);
    glTexGenfv(GL_Q,GL_EYE_PLANE,plane_Q);

    modellist = glGenLists(1);
    glNewList(modellist,GL_COMPILE);
    glPushMatrix();
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertexmodel; i++) {
        glNormal3fv((float*)&vertexmodel[i << 3] + 3);
        glTexCoord2fv((float*)&vertexmodel[i << 3] + 6);
        glVertex3fv((float*)&vertexmodel[i << 3]);
    }
    glEnd();
    glPopMatrix();
    glEndList();
    
    groundlist = glGenLists(1);
    glNewList(groundlist,GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < num_vertexground; i++) {
        glNormal3fv((float*)&vertexground[i << 3] + 3);
        glTexCoord2fv((float*)&vertexground[i << 3] + 6);
        glVertex3fv((float*)&vertexground[i << 3]);
    }
    glEnd();
    glEndList();
    
    lightlist = glGenLists(1);
    glNewList(lightlist,GL_COMPILE);
    glutSolidSphere(0.6,16,16);
    glEndList();
}

void shadowmap(void) {
    glViewport(0,0,SHADOWSIZE,SHADOWSIZE);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-modelsize,modelsize,-modelsize,modelsize,1,10000);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(lightpos[0],lightpos[1],lightpos[2], objpos[0],objpos[1],objpos[2], 0,1,0);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    glColor4f(0.5,0.5,0.5,1);

    glPushMatrix();
    glTranslatef(objpos[0],objpos[1],objpos[2]);
    glRotatef(modelangle,0,0,1);
    glCallList(modellist);
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D,textureshadow);
    glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,SHADOWSIZE,SHADOWSIZE);
    
    glColor4f(1,1,1,1);
    
    glViewport(0,0,800,600);
}

float getfps() {
    static int start,end,count;
    static float fps;
    if(count > 10) {
        end = start;
        start = glutGet(GLUT_ELAPSED_TIME);
        fps = count * 1000.0 / (start - end);
        count = 0;
    }
    count++;
    return fps;
}

void    drawstring(float x,float y,char *string) {
    int i,l;
    glRasterPos2f(x,y);
    l = strlen(string);
    for(i = 0; i < l; i++)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13,string[i]);
}

void display(void) {
    float mview[16],imview[16];
    char buffer[128];

    shadowmap();

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,4.0 / 3.0,1,2048);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camerapos[0],camerapos[1],camerapos[2], cameradir[0],cameradir[1],cameradir[2], 0,0,1);

    glLightfv(GL_LIGHT0,GL_POSITION,lightpos);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
    glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);                        // draw model
    glBindTexture(GL_TEXTURE_2D,texturemodel);
    glPushMatrix();
    glTranslatef(objpos[0],objpos[1],objpos[2]);
    glRotatef(modelangle,0,0,1);
    glCallList(modellist);
    glPopMatrix();
    
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D,textureground);     // draw ground
    glCallList(groundlist);

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();                                 // light sources
    glTranslatef(lightpos[0],lightpos[1],lightpos[2]);
    glColor4f(0,1,0,1);
    glCallList(lightlist);
    glColor4f(1,1,1,1);
    glPopMatrix();
    
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);
    glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
    glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
    glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
    glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);

    glGetFloatv(GL_MODELVIEW_MATRIX,mview);
    MatrixInverse(mview,imview);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(0.5,0.5,0);
    glScalef(0.5,0.5,1.0);
    glOrtho(-modelsize,modelsize,-modelsize,modelsize,-1,1);
    gluLookAt(lightpos[0],lightpos[1],lightpos[2], objpos[0],objpos[1],objpos[2], 0,1,0);
    glMultMatrixf(imview);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);                             // shadow
    glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
    glBindTexture(GL_TEXTURE_2D,textureshadow);
    glCallList(groundlist);
    glDisable(GL_BLEND);
    
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    sprintf(buffer,"fps: %.2f",getfps());
    drawstring(-0.95,0.95,buffer);
    glEnable(GL_DEPTH_TEST);
    
    glutSwapBuffers();
}

int findcrosspoint(const float *a,const float *b,float *point) {
    register int i,j,k;
    float dot0,dot1,angle,p0[3],p1[3],p2[3];
    for(i = 0, j = 0, k = 0; i < num_vertexground; i += 3, j += 4, k += 24) {
        dot0 = -VectorDotProduct(a,&planeground[j]);
        dot1 = -VectorDotProduct(b,&planeground[j]);
        VectorSub(b,a,point);
        VectorScale(point,(planeground[j + 3] - dot0) / (dot1 - dot0),point);
        VectorAdd(point,a,point);
        VectorSub(point,&vertexground[k],p0);
        VectorSub(point,&vertexground[k + 8],p1);
        VectorSub(point,&vertexground[k + 16],p2);
        VectorNormalize(p0,p0);
        VectorNormalize(p1,p1);
        VectorNormalize(p2,p2);
        angle = acos(VectorDotProduct(p0,p1)) +
                acos(VectorDotProduct(p1,p2)) +
                acos(VectorDotProduct(p2,p0));
        if(angle > 6.28) return 1;
    }
    return 0;
}

int testpolygon(const float *v1,const float *v2,const float *v3,const float *p) {
    if((v2[0] - v1[0]) * (p[1] - v1[1]) - (v2[1] - v1[1]) * (p[0] - v1[0]) < 0.0 ||
        (v3[0] - v2[0]) * (p[1] - v2[1]) - (v3[1] - v2[1]) * (p[0] - v2[0]) < 0.0 ||
        (v1[0] - v3[0]) * (p[1] - v3[1]) - (v1[1] - v3[1]) * (p[0] - v3[0]) < 0.0) return 0;
    return 1;
}

float heightground(const float *point) {
    register int i,j;
    float k,dot0,dot1,a[3],b[3],plane[4];
    for(i = 0, j = 0; i < num_vertexground; i += 3, j += 24)
        if(testpolygon(&vertexground[j],&vertexground[j + 8],&vertexground[j + 16],point)) break;
    VectorCopy(&planeground[j / 6],plane);
    plane[3] = planeground[j / 6 + 3];
    VectorCopy(point,a);
    VectorCopy(point,b);
    b[2] += 1.0;
    dot0 = -VectorDotProduct(plane,a);
    dot1 = -VectorDotProduct(plane,b);
    k = (plane[3] - dot0) / (dot1 - dot0);
    return a[2] + (b[2] - a[2]) * k;
}

void idle(void) {
    static float angle0,angle1;
    objpos[0] = 100 * cos(angle0);
    objpos[1] = 100 * sin(angle0);
    objpos[2] = heightground(objpos) + 60;
    modelangle = angle0 * RAD2DEG;
    lightpos[0] = objpos[0] + 10 * cos(angle1);
    lightpos[1] = objpos[1] + 10 * sin(angle1);
    lightpos[2] = objpos[2] + 40;
    lightpos[3] = 1.0;
    if(view) {
        VectorSet(0,0,0,cameradir);
    } else {
        VectorCopy(objpos,cameradir);
    }
    camerapos[0] = cameradir[0] + dist * sin(3.14 * alpha / 180.0);
    camerapos[1] = cameradir[1] + 10;
    camerapos[2] = cameradir[2] + dist * cos(3.14 * alpha / 180.0);
    switch(stop) {
        case 0:
            angle0 += 0.004;
            angle1 -= 0.016;
            break;
        case 1:
            angle1 += 0.016;
            break;
        case 2:
            angle0 += 0.004;
            break;
        case 3:
            break;
    }
    glutPostRedisplay();
}

void keyboard(unsigned char c,int x,int y) {
    switch(c) {
        case 0x1b:
            exit(1);
            break;
        case 'm':
            stop++;
            if(stop == 4) stop = 0;
            break;
        case 'a':
            dist -= 5;
            break;
        case 'z':
            dist += 5;
            break;
        case 's':
            alpha--;
            break;
        case 'x':
            alpha++;
            break;
        case 'v':
            view++;
            if(view == 2) view = 0;
            break;
    }
}

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
	glutCreateWindow("01 - Projection Shadow");
#endif

    init();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
