/*  outtro
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#ifndef __OUTTRO_H__
#define __OUTTRO_H__

#define OUTTRO_LENGTH 20

int outtro_load(char *path,int texture_mode);
void outtro_free(void);
void outtro_render(float ifps);

#endif /* __OUTTRO_H__ */
