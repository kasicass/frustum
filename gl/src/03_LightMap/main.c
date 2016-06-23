/*  light mapping demo
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <malloc.h>

#include "mathlib.h"
#include "image.h"
#include "plane.h"

#define planelist 1
#define lightlist 2
#define NUM_PARTICLE 50
#define PARTICLE_RADIUS 8
#define NUM_MARKS 1000
#define MARKS_RADIUS 10
#define NUM_FACEMARKS 100
#define ELINE 5
#define ERADIUS 12
#define NUM_EPARTICLE 128
#define EPARTICLE_RADIUS 3

int num_face;
static face_t *face;

int textureplane;
int texturelight;
int textureeffect;
int textureparticle;
int texturemarks;

float lightpos[3];
float lightcolor[3] = { 0.2, 0.4, 1.0 };
float camerapos[3];

float matrixview[16],imatrixview[16];

float fps,spf;
float G = -9.8;
float particlepos[NUM_PARTICLE][3];
float particlevel[NUM_PARTICLE][3];

struct {
    face_t face[NUM_FACEMARKS];
    int num_face;
    float alpha;
    float life;
    int flag;
}   marks[NUM_MARKS];

void init(void) {
    int i,width,height;
    unsigned char *data;
    
    glClearDepth(1.0);
    glClearColor(0,0,0,1);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glDepthFunc(GL_LEQUAL);

    face = createplaneface(&num_face);
    
    for(i = 0; i < NUM_PARTICLE; i++) VectorSet(0,0,-1000,particlepos[i]);
    
    for(i = 0; i < NUM_MARKS; i++) marks[i].flag = 0;
    
    glGenTextures(1,&textureplane);
    glBindTexture(GL_TEXTURE_2D,textureplane);
    if((data = LoadJPEG("data/plane.jpg",&width,&height))) {    
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D,4,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    
    glGenTextures(1,&texturelight);
    glBindTexture(GL_TEXTURE_2D,texturelight);
    if((data = LoadJPEG("data/light.jpg",&width,&height))) {    
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D,0,4,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }

    glGenTextures(1,&textureparticle);
    glBindTexture(GL_TEXTURE_2D,textureparticle);
    if((data = LoadTGA("data/particle.tga",&width,&height))) {    
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D,0,4,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    
    glGenTextures(1,&textureeffect);
    glBindTexture(GL_TEXTURE_2D,textureeffect);
    if((data = LoadTGA("data/effect.tga",&width,&height))) {    
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D,0,4,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }
    
    glGenTextures(1,&texturemarks);
    glBindTexture(GL_TEXTURE_2D,texturemarks);
    if((data = LoadTGA("data/marks.tga",&width,&height))) {    
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D,0,4,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
        free(data);
    }

    glNewList(planelist,GL_COMPILE);
    glColor4f(0.5,0.5,0.5,1);
    drawplane();
    glColor4f(1,1,1,1);
    glEndList();
    
    glNewList(lightlist,GL_COMPILE);
    glColor4f(1,1,1,1);
    glDisable(GL_TEXTURE_2D);
    glutSolidSphere(1,8,8);
    glEnable(GL_TEXTURE_2D);
    glEndList();    
}

int testpolygon(const float *v1,const float *v2,const float *v3,const float *p) {
    if((v2[0] - v1[0]) * (p[1] - v1[1]) - (v2[1] - v1[1]) * (p[0] - v1[0]) < 0.0 ||
       (v3[0] - v2[0]) * (p[1] - v2[1]) - (v3[1] - v2[1]) * (p[0] - v2[0]) < 0.0 ||
       (v1[0] - v3[0]) * (p[1] - v3[1]) - (v1[1] - v3[1]) * (p[0] - v3[0]) < 0.0) return 0;
    return 1;
}

float heightlandscape(const float *point) {
    register int i;
    float k,dot0,dot1,point1[3];
    for(i = 0; i < num_face; i++)
        if(testpolygon(face[i].v[0],face[i].v[1],face[i].v[2],point)) break;
    VectorCopy(point,point1);
    point1[2] += 1.0;
    dot0 = -VectorDotProduct(face[i].plane,point);
    dot1 = -VectorDotProduct(face[i].plane,point1);
    k = (face[i].plane[3] - dot0) / (dot1 - dot0);
    return (point[2] + (point1[2] - point[2]) * k);
}

float getfps() {
    static int start,end,count;
    static float fps = 50;
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

void dynamiclight(float *position,float *color,float radius) {
    int i,j;
    float iradius,u[3],v[3];
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glBindTexture(GL_TEXTURE_2D,texturelight);
    iradius = 1.0 / radius;
    for(i = 0; i < num_face; i++) {
        glColor4f(color[0],color[1],color[2],1);

        for(j = 0; j < 3; j++) {
            u[j] = (face[i].v[j][0] - position[0]) * iradius + 0.5;
            v[j] = (face[i].v[j][1] - position[1]) * iradius + 0.5;
        }

        if(u[0] < 0 && u[1] < 0 && u[2] < 0) continue;
        if(u[0] > 1 && u[1] > 1 && u[2] > 1) continue;
        if(v[0] < 0 && v[1] < 0 && v[2] < 0) continue;
        if(v[0] > 1 && v[1] > 1 && v[2] > 1) continue;

        glBegin(GL_TRIANGLES);
        glTexCoord2f(u[0],v[0]);
        glVertex3fv(face[i].v[0]);
        glTexCoord2f(u[1],v[1]);
        glVertex3fv(face[i].v[1]);
        glTexCoord2f(u[2],v[2]);
        glVertex3fv(face[i].v[2]);
        glEnd();
    }
    glColor4f(1,1,1,1);
    glDisable(GL_BLEND);
}

void drawmarks(float time) {
    int i,j;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D,texturemarks);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < NUM_MARKS; i++)
        if(marks[i].flag) {
            marks[i].life -= time;
            if(marks[i].life < 1) {
                marks[i].alpha -= time;
            }
            if(marks[i].life < 0) {
                marks[i].flag = 0;
                continue;
            }
            glColor4f(1,1,1,marks[i].alpha);
            for(j = 0; j < marks[i].num_face; j++) {
                glTexCoord2f(marks[i].face[j].t[0][0],marks[i].face[j].t[0][1]);
                glVertex3fv(marks[i].face[j].v[0]);
                glTexCoord2f(marks[i].face[j].t[1][0],marks[i].face[j].t[1][1]);
                glVertex3fv(marks[i].face[j].v[1]);
                glTexCoord2f(marks[i].face[j].t[2][0],marks[i].face[j].t[2][1]);
                glVertex3fv(marks[i].face[j].v[2]);
            }
        }
    glEnd();
    glDisable(GL_BLEND);
}

void addmarks(float *position,float radius,float alpha,float life) {
    int i,j,num_facemarks;
    float iradius,u[3],v[3];
    face_t facemarks[NUM_FACEMARKS];
    iradius = 1.0 / radius;
    num_facemarks = 0;
    for(i = 0; i < num_face; i++) {
        for(j = 0; j < 3; j++) {
            u[j] = (face[i].v[j][0] - position[0]) * iradius + 0.5;
            v[j] = (face[i].v[j][1] - position[1]) * iradius + 0.5;
        }
        if(u[0] < 0 && u[1] < 0 && u[2] < 0) continue;
        if(u[0] > 1 && u[1] > 1 && u[2] > 1) continue;
        if(v[0] < 0 && v[1] < 0 && v[2] < 0) continue;
        if(v[0] > 1 && v[1] > 1 && v[2] > 1) continue;
        for(j = 0; j < 3; j++) {
            VectorCopy(face[i].v[j],facemarks[num_facemarks].v[j]);
            facemarks[num_facemarks].t[j][0] = u[j];
            facemarks[num_facemarks].t[j][1] = v[j];
        }
        num_facemarks++;
        if(num_facemarks == NUM_FACEMARKS) break;
    }
    if(num_facemarks)
        for(i = 0; i < NUM_MARKS; i++)
            if(!marks[i].flag) {
                for(j = 0; j < num_facemarks; j++) marks[i].face[j] = facemarks[j];
                marks[i].num_face = num_facemarks;
                marks[i].alpha = alpha;
                marks[i].life = life;
                marks[i].flag = 1;
                return;
            }
}

void particle(float time,float *matrix) {
    int i;
    float g,dx[3],dy[3],vel[3];
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glBindTexture(GL_TEXTURE_2D,textureparticle);
    glDepthMask(GL_FALSE);
    VectorSet(1,0,0,dx);
    VectorTransformNormal(dx,matrix,dx);
    VectorScale(dx,PARTICLE_RADIUS,dx);
    VectorSet(0,1,0,dy);
    VectorTransformNormal(dy,matrix,dy);
    VectorScale(dy,PARTICLE_RADIUS,dy);
    g = G * time;
    for(i = 0; i < NUM_PARTICLE; i++) {
        VectorScale(particlevel[i],time,vel)
        VectorAdd(particlepos[i],vel,particlepos[i]);
        particlevel[i][2] += g * 4.0;
        if(particlevel[i][2] < 0 && particlepos[i][2] < heightlandscape(particlepos[i])) {
            addmarks(particlepos[i],MARKS_RADIUS,1,5);
            VectorCopy(lightpos,particlepos[i]);
            VectorSet((float)(rand() % 400) / 20 - 10,
                      (float)(rand() % 400) / 20 - 10,
                      (float)(rand() % 400) / 7,
                      particlevel[i]);
        } if(particlevel[i][2] < 0 && particlepos[i][2] < -100.0) {
            VectorCopy(lightpos,particlepos[i]);
            VectorSet((float)(rand() % 400) / 20 - 10,
                      (float)(rand() % 400) / 20 - 10,
                      (float)(rand() % 400) / 7,
                      particlevel[i]);
        } else {
            glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0,1);
                glVertex3f(particlepos[i][0] - dx[0] - dy[0],
                           particlepos[i][1] - dx[1] - dy[1],
                           particlepos[i][2] - dx[2] - dy[2]);
                glTexCoord2f(1,1);
                glVertex3f(particlepos[i][0] + dx[0] - dy[0],
                           particlepos[i][1] + dx[1] - dy[1],
                           particlepos[i][2] + dx[2] - dy[2]);
                glTexCoord2f(0,0);
                glVertex3f(particlepos[i][0] - dx[0] + dy[0],
                           particlepos[i][1] - dx[1] + dy[1],
                           particlepos[i][2] - dx[2] + dy[2]);
                glTexCoord2f(1,0);
                glVertex3f(particlepos[i][0] + dx[0] + dy[0],
                           particlepos[i][1] + dx[1] + dy[1],
                           particlepos[i][2] + dx[2] + dy[2]);
            glEnd();
        }
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void effect(float time,float *matrix) {
    int i;
    static float alpha,gamma;
    float dx[3],dy[3],pos[3],phase;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glBindTexture(GL_TEXTURE_2D,textureeffect);
    glDepthMask(GL_FALSE);
    VectorSet(1,0,0,dx);
    VectorTransformNormal(dx,matrix,dx);
    VectorScale(dx,EPARTICLE_RADIUS,dx);
    VectorSet(0,1,0,dy);
    VectorTransformNormal(dy,matrix,dy);
    VectorScale(dy,EPARTICLE_RADIUS,dy);
    alpha += time;
    gamma += time;
    for(i = 0; i < NUM_EPARTICLE; i++) {
        phase = D_PI * 4.0 / NUM_EPARTICLE * (float)i;
        pos[0] = cos(gamma + alpha + phase);
        pos[1] = sin(gamma + alpha + phase);
        pos[2] = 0;
        VectorScale(pos,ERADIUS * (sin(ELINE / 2.0 * (alpha + phase)) + 2) / 3,pos);
        pos[2] = cos(ELINE / 2.0 * (alpha + phase)) * ERADIUS;
        VectorAdd(pos,lightpos,pos);
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0,1);
            glVertex3f(pos[0] - dx[0] - dy[0],
                pos[1] - dx[1] - dy[1],
                pos[2] - dx[2] - dy[2]);
            glTexCoord2f(1,1);
            glVertex3f(pos[0] + dx[0] - dy[0],
                pos[1] + dx[1] - dy[1],
                pos[2] + dx[2] - dy[2]);
            glTexCoord2f(0,0);
            glVertex3f(pos[0] - dx[0] + dy[0],
                pos[1] - dx[1] + dy[1],
                pos[2] - dx[2] + dy[2]);
            glTexCoord2f(1,0);
            glVertex3f(pos[0] + dx[0] + dy[0],
                pos[1] + dx[1] + dy[1],
                pos[2] + dx[2] + dy[2]);
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void display(void) {
    char buffer[128];
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    fps = getfps();
    spf = 1.0 / fps;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,4.0 / 3.0,1,4096);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(matrixview);

    glBindTexture(GL_TEXTURE_2D,textureplane);
    glCallList(planelist);

    dynamiclight(lightpos,lightcolor,80);

    drawmarks(spf);

    glPushMatrix();
    glTranslatef(lightpos[0],lightpos[1],lightpos[2]);
    glCallList(lightlist);
    glPopMatrix();
    
    effect(spf,imatrixview);
    particle(spf,imatrixview);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    sprintf(buffer,"fps: %.2f",fps);
    drawstring(-0.95,0.95,buffer);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    
    glutSwapBuffers();
}

void idle(void) {
    float dir[3],up[3];
    static float angle0;
    static float angle1;
    lightpos[0] = 80 * fabs(cos(angle0 / 3)) * cos(angle0);
    lightpos[1] = 80 * fabs(cos(angle0 / 4)) * sin(angle0);
    lightpos[2] = 40 + heightlandscape(lightpos);
    angle0 -= spf * 0.2;
    camerapos[0] = 140 * cos(angle1);
    camerapos[1] = 140 * sin(angle1);
    camerapos[2] = 100 + 100 * cos(angle1);
    angle1 -= spf * 0.08;
    VectorSet(0,0,0,dir);
    VectorSet(0,0,1,up);
    MatrixLookAt(camerapos,dir,up,matrixview);
    MatrixInverseTranspose(matrixview,imatrixview);
    glutPostRedisplay();
}

void keyboard(unsigned char c,int x,int y) {
    switch(c) {
        case 0x1b:
            exit(1);
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
	glutCreateWindow("03 - LightMap");
#endif

    init();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
