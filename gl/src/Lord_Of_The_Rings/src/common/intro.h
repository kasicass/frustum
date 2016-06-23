/*  intro
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __INTRO_H__
#define __INTRO_H__

#define INTRO_LENGTH 20

int intro_load(char *path,int texture_mode);
void intro_free(void);
void intro_render(float ifps);

#endif /* __INTRO_H__ */
