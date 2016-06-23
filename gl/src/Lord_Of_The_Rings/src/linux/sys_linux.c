/*  linux initialization
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include <GL/gl.h>

#include "../common/common.h"

static Display *display;                // display
static Window window,rootwindow;        // window
static int screen;                      // screen
static int screenwidth,screenheight;    // screen width, screen height
static GLXContext glxcontext;           // glx context
static int windowwidth,windowheight;    // window width, window height
static int key;                         // keyboard

/*  system timer
 *
 */

int sys_milliseconds(void) {
    struct timeval tval;
    struct timezone tzone;
    static int secbase = 0;
    gettimeofday(&tval,&tzone);
    if(!secbase) {
        secbase = tval.tv_sec;
        return tval.tv_usec / 1000;
    }
    return (tval.tv_sec - secbase) * 1000 + tval.tv_usec / 1000;
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
    fprintf(stderr,"error: %s\n",buffer);
    exit(1);
}

/*  system quit
 *
 */

void sys_quit(void) {
    glXDestroyContext(display,glxcontext);
    XDestroyWindow(display,window);
    XCloseDisplay(display);
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

/*  system init OpenGL
 *
 */

int sys_initGL(int width,int height,int bpp,int fullscreen,char *title) {
    char *displayname = ":0.0";
    char cursor_data[1] = { 0 };
    int attrib[] = { GLX_RGBA,
                     GLX_RED_SIZE,1,
                     GLX_GREEN_SIZE,1,
                     GLX_BLUE_SIZE,1,
                     GLX_DOUBLEBUFFER,
                     GLX_DEPTH_SIZE,1,None };
    XVisualInfo *visinfo;
    XSizeHints hint;
    XSetWindowAttributes attr;
    unsigned long mask;
    Atom motifhints;
    struct {
        long flags;
        long functions;
        long decorations;
        long input_mode;
    } hints;
    XColor dummy;
    Pixmap blank;
    Cursor cursor;    
    display = XOpenDisplay(displayname);
    if(!display) {
        printf("couldn`t open the x11 display\n");
        return 0;
    }
    screen = DefaultScreen(display);
    rootwindow = RootWindow(display,screen);
    screenwidth = DisplayWidth(display,screen);
    screenheight = DisplayHeight(display,screen);
    visinfo = glXChooseVisual(display,screen,attrib);
    if(!visinfo) {
        printf("couldn`t get RGBA double buffer visual\n");
        return 0;
    }
    hint.x = 0;
    hint.y = 0;
    hint.flags = PPosition | PSize;
    if(fullscreen) {
        hint.width = screenwidth;
        hint.height = screenheight;
    } else {
        hint.width = width;
        hint.height = height;
    }
    windowwidth = hint.width;
    windowheight = hint.height;
    attr.background_pixel = 0;
    attr.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask;
    attr.colormap = XCreateColormap(display,rootwindow,visinfo->visual,AllocNone);
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
    window = XCreateWindow(display,rootwindow,hint.x,hint.y,hint.width,hint.height,0,visinfo->depth,InputOutput,visinfo->visual,mask,&attr);
    if(fullscreen) {
        motifhints = None;
        motifhints = XInternAtom(display,"_MOTIF_WM_HINTS",0);
        if(motifhints != None ) {
            hints.flags = 2;
            hints.decorations = 0;
            XChangeProperty(display,window,motifhints,motifhints,32,PropModeReplace,(unsigned char*)&hints,4);
        }
        blank = XCreateBitmapFromData(display,window,cursor_data,1,1);
        cursor = XCreatePixmapCursor(display,blank,blank,&dummy,&dummy,0,0);
        XDefineCursor(display,window,cursor);
    }
    XSetStandardProperties(display,window,title,title,None,NULL,0,&hint);
    XMapWindow(display,window);
    glxcontext = glXCreateContext(display,visinfo,NULL,True);
    if(!glxcontext) {
        printf("glx context failed\n");
        return 0;
    }
    glXMakeCurrent(display,window,glxcontext);
    return 1;
}

/*  system swap buffers
 *
 */

void sys_swap(void) {
    glXSwapBuffers(display,window);
}

/*  system main
 *
 */

int main(int argc,char **argv) {
    XEvent event;
    char buffer[1];
    common_init(argc,argv);
    while(1) {
        while(XPending(display) > 0) {
            XNextEvent(display,&event);
            switch(event.type) {
                case Expose:
                    break;
                case ConfigureNotify:
                    windowwidth = event.xconfigure.width;
                    windowheight = event.xconfigure.height;
                    break;
                case KeyPress:
                    if(XLookupString(&event.xkey,buffer,sizeof(buffer),NULL,NULL)) {
                        key = buffer[0];
                    }
                    break;
            }
        }
        common_frame();
        glXSwapBuffers(display,window);
    }
    return 0;
}
