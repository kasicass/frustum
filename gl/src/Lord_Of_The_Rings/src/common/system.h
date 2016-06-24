/*  system
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef _WIN32
#include <windows.h>
#include "../win32/sys_glext.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>

int sys_milliseconds(void);
void sys_error(char *error,...);
void sys_quit(void);
int sys_width(void);
int sys_height(void);
int sys_key(void);
int sys_initGL(int width,int height,int bpp,int fullscreen,wchar_t *title);
void sys_swap(void);

#endif /* __SYSTEM_H__ */
