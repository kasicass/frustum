/*  font
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __FONT_H__
#define __FONT_H__

#define NUM_LETTER 37 // '.' + '0'-'9' + 'A'-'Z'

typedef struct {
    int letter[NUM_LETTER]; // font texture id
} font_t;

font_t *font_load(char *name,char *path);
void font_free(font_t *font);
void font_print(font_t *font,float x,float y,float size,char *string,...);

#endif /* __FONT_H__ */
