/*	load & save TGA file
 *
 *		written by Alexander Zaprjagaev
 */

#ifndef __LOADTGA_H__
#define __LOADTGA_H__

unsigned char *load_tga(char *name,int *width,int *height);
int save_tga(char *name,unsigned char *data,int width,int height);

#endif /* __LOADTGA_H__ */
