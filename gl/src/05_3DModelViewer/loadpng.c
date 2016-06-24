/*  load PNG file
 *
 *  written by Alexander Zaprjagaev
 *  2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <malloc.h>
#include <png.h>

unsigned char *LoadPNG(char *name,int *width,int *height) {
    FILE *file;
    double gamma;
    unsigned char *data;
    unsigned long w,h;
    int i,j,k,l,bit_depth,color_type;
    png_uint_32 channels,row_bytes;
    png_byte *img,**row,sig[8];
    png_structp png_ptr = 0;
    png_infop info_ptr = 0;
    file = fopen(name,"rb");
    if(!file) return NULL;
    fread(sig,8,1,file);
    if(!png_check_sig(sig,8)) {
        fclose(file);
        return NULL;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if(!png_ptr) {
        fclose(file);
        return NULL;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        png_destroy_read_struct(&png_ptr,0,0);
        fclose(file);
        return NULL;
    }
    png_init_io(png_ptr,file);
    png_set_sig_bytes(png_ptr,8);
    png_read_info(png_ptr,info_ptr);
    png_get_IHDR(png_ptr,info_ptr,&w,&h,&bit_depth,&color_type,0,0,0);
    if(bit_depth == 16) png_set_strip_16(png_ptr);
    if(color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);
    if(bit_depth < 8) png_set_expand(png_ptr);
    if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS)) png_set_expand(png_ptr);
    if(color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
 png_set_gray_to_rgb(png_ptr);
    if(png_get_gAMA(png_ptr,info_ptr,&gamma)) png_set_gamma(png_ptr,(double)2.2,gamma);
    png_read_update_info(png_ptr,info_ptr);
    png_get_IHDR(png_ptr,info_ptr,&w,&h,&bit_depth,&color_type,0,0,0);
    row_bytes = png_get_rowbytes(png_ptr,info_ptr);
    channels = png_get_channels(png_ptr,info_ptr);
    img = (png_byte*)malloc(sizeof(png_byte) * row_bytes * h);
    row = (png_byte**)malloc(sizeof(png_byte*) * h);
    for(i = 0; i < h; i++) row[i] = img + row_bytes * i;
    png_read_image(png_ptr,row);
    png_read_end(png_ptr,NULL);
    data = (unsigned char*)malloc(sizeof(unsigned char) * w * h * 4);
    switch(channels) {
        case 4:
            for(j = 0, k = 0; j < h; j++)
                for(i = 0; i < w; i++, k += 4) {
                    l = row_bytes * j + i * 4;
                    data[k + 0] = img[l + 0];
                    data[k + 1] = img[l + 1];
                    data[k + 2] = img[l + 2];
                    data[k + 3] = img[l + 3];
                }
            break;
        case 3:
            for(j = 0, k = 0; j < h; j++)
                for(i = 0; i < w; i++, k += 4) {
                    l = row_bytes * j + i * 3;
                    data[k + 0] = img[l + 0];
                    data[k + 1] = img[l + 1];
                    data[k + 2] = img[l + 2];
                    data[k + 3] = 255;
                }
            break;
    }
    fclose(file);
    free(img);
    free(row);
    png_destroy_read_struct(&png_ptr,0,0);
    *width = w;
    *height = h;
    return data;
}
