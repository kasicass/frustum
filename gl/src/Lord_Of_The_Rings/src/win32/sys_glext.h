/*  openGL extension initialization
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __SYS_GLEXT_H__
#define __SYS_GLEXT_H__

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef EXT_texture_env_combine
#define GL_COMBINE_EXT                  0x8570
#define GL_COMBINE_RGB_EXT              0x8571
#define GL_COMBINE_ALPHA_EXT            0x8572
#define GL_RGB_SCALE_EXT                0x8573
#define GL_ADD_SIGNED_EXT               0x8574
#define GL_INTERPOLATE_EXT              0x8575
#define GL_CONSTANT_EXT                 0x8576
#define GL_PRIMARY_COLOR_EXT            0x8577
#define GL_PREVIOUS_EXT                 0x8578
#define GL_SOURCE0_RGB_EXT              0x8580
#define GL_SOURCE1_RGB_EXT              0x8581
#define GL_SOURCE2_RGB_EXT              0x8582
#define GL_SOURCE0_ALPHA_EXT            0x8588
#define GL_SOURCE1_ALPHA_EXT            0x8589
#define GL_SOURCE2_ALPHA_EXT            0x858A
#define GL_OPERAND0_RGB_EXT             0x8590
#define GL_OPERAND1_RGB_EXT             0x8591
#define GL_OPERAND2_RGB_EXT             0x8592
#define GL_OPERAND0_ALPHA_EXT           0x8598
#define GL_OPERAND1_ALPHA_EXT           0x8599
#define GL_OPERAND2_ALPHA_EXT           0x859A
#endif

#ifndef GL_ARB_multitexture
#define GL_ACTIVE_TEXTURE_ARB           0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB    0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB        0x84E2
#define GL_TEXTURE0_ARB                 0x84C0
#define GL_TEXTURE1_ARB                 0x84C1
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC)(GLenum target);
typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum target);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC)(GLenum target,GLfloat s,GLfloat t);
#endif

#ifdef _WIN32
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
#endif

int sys_initglext(void);

#endif /* __SYS_GLEXT_H__ */
