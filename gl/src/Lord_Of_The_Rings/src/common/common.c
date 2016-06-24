/*  lord of the rings demo
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <stdlib.h>

#include "shadowmap.h"
#include "font.h"
#include "script.h"
#include "scene.h"
#include "intro.h"
#include "outtro.h"

#include "system.h"

scene_t *scene; // scene
font_t *font;   // font

int starttime,loadingtime;  // for fps calculate
float time,fps,ifps;    // current time, fps, inverse fps
int showfps,pause,loop; // flags show fps, demo pause, loop demo
int texture_mode;   // texture mode bilinear or trilinear
int part;   // demo part

/*  part 0 - intro
 *  part 1 - moriya
 *  part 2 - mordor
 *  part 3 - outtru
 */

/*  calculate frame per second
 *
 */

float fps_get(void) {
    static float fps = 50;
    static int counter;
    int endtime;
    if(counter > 10) {
        endtime = starttime;
        starttime = sys_milliseconds();
        fps = (float)counter * 1000.0 / (float)(starttime - endtime);
        counter = 0;
    }
    counter++;
    return fps;
}

void fps_stop(void) {
    loadingtime = sys_milliseconds();
}

void fps_start(void) {
    loadingtime = sys_milliseconds() - loadingtime;
    starttime += loadingtime;
}

/*  fade screen 0.0 - 1.0 black, 1.0 - 2.0 - white, 1.0 - fade off
 *
 */

void render_fade(float alpha) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    if(alpha <= 1.0) glColor4f(0,0,0,alpha);
    else {
        alpha -= 1.0;
        glColor4f(alpha,alpha,alpha,1.0);
    }
    glBlendFunc(GL_ONE,GL_SRC_ALPHA);
    glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(-1,-1);
        glVertex2f(1,-1);
        glVertex2f(-1,1);
        glVertex2f(1,1);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

/*  main rendering function
 *
 */

void render(void) {
    
    switch(part) {  // render part demo
        case 0:     // intro
            intro_render(ifps);
            if(time < 4.0) render_fade(time / 4.0); // 4 second start fade
            else if(time > INTRO_LENGTH - 1.0) render_fade(INTRO_LENGTH - time);
            break;
        case 1:     // moriya
            scene_render(scene,ifps);
            if(time < 2.0) render_fade(time / 2.0); // 2 second start fade
            else if(time > scene->end - 1.0) render_fade(scene->end - time);
            break;
        case 2:     // mordor
            scene_render(scene,ifps);
            if(time < 2.0) render_fade(time / 2.0); // 2 second start fade
            else if(time > scene->end - 1.0) render_fade(scene->end - time);
            break;
        case 3:     // outtro
            outtro_render(ifps);
            if(time < 1.0) render_fade(time / 1.0); // 1 second start fade
            else if(time > OUTTRO_LENGTH - 1.0) render_fade(OUTTRO_LENGTH - time);
            break;
    }
    
    if(showfps) {   // show fps
        switch(part) {
            case 0: glColor4f(1,1,1,1); break;
            case 1: glColor4f(0,0,0,1); break;
            case 2: glColor4f(1,1,1,1); break;
            case 3: glColor4f(1,1,1,1); break;
        }
        font_print(font,-1,1,0.05,"fps %.1f",fps);
    }
    
    switch(part) {  // demo part switcher
        case 0:
            if(time > INTRO_LENGTH) {
                fps_stop();
                intro_free();
                render_fade(0.0);   // black screen
                glColor4f(1,1,1,1);
                font_print(font,-0.8,0.05,0.1,"please wait Loading...");
                sys_swap(); // swap buffers
                scene = scene_load("data/moriya/scene.cfg","data/moriya/",texture_mode);
                if(!scene) sys_error("load Moriya error");
                time = 0;
                part = 1;
                fps_start();
            }
            break;
        case 1:
            if(time > scene->end) {
                fps_stop();
                scene_free(scene);
                render_fade(0.0);   // black screen
                glColor4f(1,1,1,1);
                font_print(font,-0.8,0.05,0.1,"please wait Loading...");
                sys_swap(); // swap buffers                
                scene = scene_load("data/mordor/scene.cfg","data/mordor/",texture_mode);
                if(!scene) sys_error("load Mordor error");
                time = 0;
                part = 2;
                fps_start();
            }
            break;
        case 2:
            if(time > scene->end) {
                fps_stop();
                scene_free(scene);
                if(!outtro_load("data/outtro/",texture_mode)) sys_error("load outtro error");
                time = 0;
                part = 3;
                fps_start();
            }
            break;
        case 3:
            if(time > OUTTRO_LENGTH && loop) {  // loop demo
                fps_stop();
                outtro_free();
                if(!intro_load("data/intro/",texture_mode)) sys_error("load intro error");
                time = 0;
                part = 0;
                fps_start();
            }
            if(time > OUTTRO_LENGTH) sys_quit();    // end demo
    }
    
    fps = fps_get();    // get current fps
    ifps = 1.0 / fps;   // calculate inverse fps
    if(pause) ifps = 0; // pause
    time += ifps;       // new time
}

/*  keyboard handler
 *
 */

void keyboard() {
    int key = sys_key();
    switch(key) {
        case 0x1b:      // exit 'ESCAPE'
            sys_quit();
            break;
        case ' ':       // pause
            pause++;
            if(pause == 2) pause = 0;
            break;
        case 'f':       // show fps
        case 'F':
            showfps++;
            if(showfps == 2) showfps = 0;
            break;
    }
}

/*
 *
 */

void common_init(int argc,char **argv) {
    FILE *file;
    char buffer[128],*string;
    int i,width,height,bpp,fullscreen;
    
    width = 640;    // default settings
    height = 480;
    bpp = 32;       // 32 bit color
    fullscreen = 0; // window mode
    loop = 0;       // loop demo off
    texture_mode = GL_LINEAR_MIPMAP_NEAREST;    // bilinear default
    
    file = fopen("lotr.cfg","r");   // open config
    if(file) {  // parse config
        while(fscanf(file,"%s",buffer) != EOF) {
            if(!strcmp(buffer,"width")) fscanf(file,"%d",&width);
            else if(!strcmp(buffer,"height")) fscanf(file,"%d",&height);
            else if(!strcmp(buffer,"bpp")) fscanf(file,"%d",&bpp);
            else if(!strcmp(buffer,"fullscreen")) fscanf(file,"%d",&fullscreen);
            else if(!strcmp(buffer,"loop")) fscanf(file,"%d",&loop);
            else if(!strcmp(buffer,"texture")) {
                fscanf(file,"%s",buffer);
                if(!strcmp(buffer,"GL_LINEAR_MIPMAP_NEAREST")) texture_mode = GL_LINEAR_MIPMAP_NEAREST;
                else if(!strcmp(buffer,"GL_LINEAR_MIPMAP_LINEAR")) texture_mode = GL_LINEAR_MIPMAP_LINEAR;
                else sys_error("config unknow texture format %c%s%c",'"',buffer,'"');
            } else sys_error("config unknow option %c%s%c",'"',buffer,'"');
        }
        fclose(file);
    }
    
    for(i = 1; i < argc; i++) { // command line
        string = argv[i];
        if(*string == '-') while(*++string == '-'); // skeep '-'
        if(!strcmp(string,"width")) width = atoi(argv[++i]);
        else if(!strcmp(string,"height")) height = atoi(argv[++i]);
        else if(!strcmp(string,"bpp")) bpp = atoi(argv[++i]);
        else if(!strcmp(string,"fullscreen")) fullscreen = atoi(argv[++i]);
        else if(!strcmp(string,"loop")) loop = atoi(argv[++i]);
        else if(!strcmp(string,"texture")) {
            i++;
            if(!strcmp(argv[i],"GL_LINEAR_MIPMAP_NEAREST")) texture_mode = GL_LINEAR_MIPMAP_NEAREST;
            else if(!strcmp(argv[i],"GL_LINEAR_MIPMAP_LINEAR")) texture_mode = GL_LINEAR_MIPMAP_LINEAR;
            else sys_error("command line unknow texture format %c%s%c",'"',argv[i],'"');
        } else sys_error("command line unknow option %c%s%c",'"',string,'"');
    }
    
    if(!sys_initGL(width,height,bpp,fullscreen,L"Lord of the rings")) sys_error("error set mode");
    
    glClearDepth(1.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glFogi(GL_FOG_MODE,GL_LINEAR);
    
    shadow_create_texture(128);     // texture for shadows
    font = font_load("default","data/font/");    // load font
    if(!font) sys_error("load font error");
    
    if(!intro_load("data/intro/",texture_mode)) sys_error("load intro error");
    
    part = 0;   // part 0 - intro
}

void common_frame(void) {
    render();
    keyboard();
}
