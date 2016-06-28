/*	font
 *
 *			written by Alexander Zaprjagaev
 *			frustum@public.tsu.ru
 */

#ifndef __FONT_H__
#define __FONT_H__

typedef struct {
	int texture_id;
	int list_id;
	int step;
} font_t;

font_t *font_load_tga(char *name);
void font_printf(font_t *font,float width,float height,float x,float y,
	char *string,...);

#endif /* __FONT_H__ */
