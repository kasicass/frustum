/*  openGL extension initialization
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "sys_glext.h"

#ifdef _WIN32
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
#endif

int sys_initglext(void) {
    char *extensions;
    extensions = (char*)glGetString(GL_EXTENSIONS);
    if(!strstr(extensions,"GL_ARB_multitexture")) return 0;
#ifdef _WIN32
    glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
    glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
    glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
#endif
    return 1;
}
