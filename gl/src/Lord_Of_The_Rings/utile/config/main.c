/*  lotr config
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "resource.h"

static int width,height,bpp,fullscreen,texture_mode,loop;

void init(HWND hDlg) {
    int i;
    FILE *file;
    char buffer[128];
    width = 640;    // default settings
    height = 480;
    bpp = 32;       // 32 bit color
    fullscreen = 0; // window mode
    loop = 0;       // loop demo off
    texture_mode = 0;   // bilinear default (0 - bilinear, 1 - trilinear)
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
                if(!strcmp(buffer,"GL_LINEAR_MIPMAP_NEAREST")) texture_mode = 0;
                else if(!strcmp(buffer,"GL_LINEAR_MIPMAP_LINEAR")) texture_mode = 1;
                else MessageBox(NULL,"unknow texture mode in lotr.cfg","error",MB_OK);
            } else MessageBox(NULL,"unknow option in lotr.cfg","error",MB_OK);
        }
        fclose(file);
    }
    if(loop) CheckDlgButton(hDlg,IDC_LOOP,MF_CHECKED);
    if(texture_mode) CheckDlgButton(hDlg,IDC_TEXTURE,MF_CHECKED);
    if(fullscreen) CheckDlgButton(hDlg,IDC_FULLSCREEN,MF_CHECKED);
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"640x480 - 16 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"640x480 - 32 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"800x600 - 16 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"800x600 - 32 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"1024x768 - 16 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"1024x768 - 32 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"1280x1024 - 16 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"1280x1024 - 32 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"1600x1200 - 16 bit");
    ComboBox_AddString(GetDlgItem(hDlg,IDC_MODE),"1600x1200 - 32 bit");
    i = -1;
    if(width == 640 && height == 480 && bpp == 16) i = 0;
    else if(width == 640 && height == 480 && bpp == 32) i = 1;
    else if(width == 800 && height == 600 && bpp == 16) i = 2;
    else if(width == 800 && height == 600 && bpp == 32) i = 3;
    else if(width == 1024 && height == 768 && bpp == 16) i = 4;
    else if(width == 1024 && height == 768 && bpp == 32) i = 5;
    else if(width == 1240 && height == 1024 && bpp == 16) i = 6;
    else if(width == 1240 && height == 1024 && bpp == 32) i = 7;
    else if(width == 1600 && height == 1200 && bpp == 16) i = 8;
    else if(width == 1600 && height == 1200 && bpp == 32) i = 9;
    if(i != -1) ComboBox_SetCurSel(GetDlgItem(hDlg,IDC_MODE),i);
}

void save() {
    FILE *file;
    file = fopen("lotr.cfg","w");
    if(!file) {
        MessageBox(NULL,"create lotr.cfg file","error",MB_OK);
        return;
    }
    fprintf(file,"width %u\nheight %u\nbpp %u\nfullscreen %u\nloop %u\n",
        width,height,bpp,fullscreen,loop);
    if(texture_mode == 0) fprintf(file,"texture GL_LINEAR_MIPMAP_NEAREST\n");
    else fprintf(file,"texture GL_LINEAR_MIPMAP_LINEAR\n");
    fclose(file);
}

/*  main dialog
 *
 */

int CALLBACK MainDialog(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam) {
    switch(msg) {
        case WM_INITDIALOG:
            init(hDlg);
            break;
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDC_SAVE:
                    save();
                    break;
                case IDC_EXIT:
                    exit(1);
                    break;
                case IDC_LOOP:
                    loop = (IsDlgButtonChecked(hDlg,IDC_LOOP) == BST_CHECKED);
                    break;
                case IDC_TEXTURE:
                    texture_mode = (IsDlgButtonChecked(hDlg,IDC_TEXTURE) == BST_CHECKED);
                    break;
                case IDC_FULLSCREEN:
                    fullscreen = (IsDlgButtonChecked(hDlg,IDC_FULLSCREEN) == BST_CHECKED);
                    break;
                case IDC_MODE:
                    switch(ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_MODE))) {
                        case 0: width = 640; height = 480; bpp = 16; break;
                        case 1: width = 640; height = 480; bpp = 32; break;
                        case 2: width = 800; height = 600; bpp = 16; break;
                        case 3: width = 800; height = 600; bpp = 32; break;
                        case 4: width = 1024; height = 768; bpp = 16; break;
                        case 5: width = 1024; height = 768; bpp = 32; break;
                        case 6: width = 1240; height = 1024; bpp = 16; break;
                        case 7: width = 1240; height = 1024; bpp = 32; break;
                        case 8: width = 1600; height = 1200; bpp = 16; break;
                        case 9: width = 1600; height = 1200; bpp = 32; break;
                    }
                    break;
            }
    }
    return FALSE;
}

/*  win main
 *
 */

int APIENTRY WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR pCmdLine,INT nCmdShow) {
    DialogBox(hInst,MAKEINTRESOURCE(IDD_MAIN),NULL,MainDialog);
    return TRUE;
}
