/*  win32 initialization
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <GL/gl.h>

#include "sys_glext.h"
#include "../common/common.h"

#define MAX_ARGV    128

static HGLRC wglcontext;                // wgl context
static int fullscreen;                  // fullscreen
static int argc;                        // argc
static char *argv[MAX_ARGV];            // argv
static int windowwidth,windowheight;    // window width, window height
static int key;                         // keyboard

/*  system timer
 *
 */

int sys_milliseconds(void) {
    static int base;
    static int initialized = 0;
    if(!initialized) {
        base = timeGetTime();
        initialized = 1;
    }
    return timeGetTime() - base;
}

/*  system error
 *
 */

void sys_error(char *error,...) {
    va_list argptr;
    char buffer[256];
    va_start(argptr,error);
    vsprintf(buffer,error,argptr);
    va_end(argptr);
    if(fullscreen) ChangeDisplaySettings(NULL,0);
    MessageBox(NULL,buffer,"error",0);
    exit(1);
}

/*  system quit
 *
 */

void sys_quit(void) {
    if(fullscreen) ChangeDisplaySettings(NULL,0);
    wglMakeCurrent(NULL,NULL);
    wglDeleteContext(wglcontext);
    exit(0);
}

/*  system window width
 *
 */

int sys_width(void) {
    return windowwidth;
}

/*  system window height
 *
 */

int sys_height(void) {
    return windowheight;
}

/*  system key
 *
 */

int sys_key(void) {
    int old_key;
    old_key = key;
    key = 0;
    return old_key;
}

/*  window proc
 *
 */

LRESULT CALLBACK sys_wndproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam) {
    switch(message) {
        case WM_SIZE:
            windowwidth = LOWORD(lparam);
            windowheight = HIWORD(lparam);
            break;
        case WM_CHAR:
            key = wparam;
            break;
        case WM_DESTROY:
            if(fullscreen) ChangeDisplaySettings(NULL,0);
            wglMakeCurrent(NULL,NULL);
            wglDeleteContext(wglcontext);
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd,message,wparam,lparam);
}

/*  system init OpenGL
 *
 */

int sys_initGL(int width,int height,int bpp,int fs,char *title) {
    WNDCLASS wc;
    HWND hwnd;
    HDC hdc;
    int style;
    HINSTANCE hInstance;
    DEVMODE settings;
    int pixelformat;
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        24,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        32,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0 };
    hInstance = GetModuleHandle(NULL);
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)sys_wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = 0;
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "lotr";
    if(!RegisterClass(&wc)) return 0;
    if(fs) {
        memset(&settings,0,sizeof(DEVMODE));
        settings.dmSize = sizeof(DEVMODE);
        settings.dmPelsWidth = width;
        settings.dmPelsHeight = height;
        settings.dmBitsPerPel = bpp;
        settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
        if(ChangeDisplaySettings(&settings,CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) return 0;
        ShowCursor(FALSE);
        style = WS_POPUP;
    } else {
        style = WS_OVERLAPPED | WS_THICKFRAME;
    }
    windowwidth = width;
    windowheight = height;
    fullscreen = fs;
    hwnd = CreateWindowEx(0,"lotr",title,style,0,0,width,height,NULL,NULL,hInstance,NULL);
    if(!hwnd) return 0;
    if(!(hdc = GetDC(hwnd))) return 0;
    if(!(pixelformat = ChoosePixelFormat(hdc,&pfd))) return 0;
    if(!(SetPixelFormat(hdc,pixelformat,&pfd))) return 0;
    if(!(wglcontext = wglCreateContext(hdc))) return 0;
    if(!wglMakeCurrent(hdc,wglcontext)) return 0;
    ShowWindow(hwnd,SW_SHOW);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    if(!sys_initglext()) sys_error("init extension failed");
    return 1;
}

/*  system swap buffer
 *
 */

void sys_swap(void) {
    HDC hdc;
    hdc = wglGetCurrentDC();
    SwapBuffers(hdc);
}

/*  system main
 *
 */

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
    MSG msg;
    HDC hdc;
    argc = 1;
    argv[0] = "none";
    while(*lpCmdLine && argc < MAX_ARGV) {
        while(*lpCmdLine && *lpCmdLine <= ' ') lpCmdLine++;
        if(*lpCmdLine) {
            argv[argc++] = lpCmdLine;
            while(*lpCmdLine && *lpCmdLine > ' ') lpCmdLine++;
            if(*lpCmdLine) *(lpCmdLine++) = 0;
        }
    }
    common_init(argc,argv);
    while(1) {
        int key;
        if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) {
            if(!GetMessage(&msg,NULL,0,0)) return msg.wParam;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            common_frame();
            hdc = wglGetCurrentDC();
            SwapBuffers(hdc);
        }
    }
    return msg.wParam;
}
