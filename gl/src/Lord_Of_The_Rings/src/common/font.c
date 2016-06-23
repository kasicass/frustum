/*  font
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

#include "loadtga.h"
#include "font.h"

#include "system.h"

font_t *font_load(char *name,char *path) {
    unsigned char *data;
    int i,width,height;
    char buffer[256];
    font_t *font;
    font = (font_t*)malloc(sizeof(font_t));
    for(i = 0; i < NUM_LETTER; i++) {
        sprintf(buffer,"%s%s%04u.tga",path,name,i);   // path + name + number
        data = LoadTGA(buffer,&width,&height);
        if(data) {
            glGenTextures(1,&font->letter[i]);
            glBindTexture(GL_TEXTURE_2D,font->letter[i]);   // bilinear filter
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_NEAREST);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
            gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
            free(data);
        }
    }
    return font;
}

void font_free(font_t *font) {
    free(font);
}

void font_print_letter(font_t *font,float x,float y,float width,float height,char c) {
    int symbol;
    if(c == '.') symbol = 0;
    else if(c >= '0' && c <= '9') symbol = c - 48 + 1;
    else if(c >= 'A' && c <= 'Z') symbol = c - 65 + 11;
    else if(c >= 'a' && c <= 'z') symbol = c - 97 + 11;
    else return;
    glBindTexture(GL_TEXTURE_2D,font->letter[symbol]);
    glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0,1);
        glVertex2f(x,y - height);
        glTexCoord2f(1,1);
        glVertex2f(x + width,y - height);
        glTexCoord2f(0,0);
        glVertex2f(x,y);
        glTexCoord2f(1,0);
        glVertex2f(x + width,y);
    glEnd();
}

void font_print(font_t *font,float x,float y,float size,char *string,...) {
    int i,j;
    float width,height;
    char buffer[256];
    va_list argptr;
    va_start(argptr,string);
    j = vsprintf(buffer,string,argptr);
    va_end(argptr);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    height = size;
    width = height * (float)sys_height() / (float)sys_width();
    for(i = 0; i < j; i++, x += width)
        font_print_letter(font,x,y,width,height,buffer[i]);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
}
