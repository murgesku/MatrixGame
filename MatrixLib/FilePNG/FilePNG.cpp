// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <windows.h>
#include "FilePNG.hpp"
#include "png.h"

struct SPNGData {
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;
    int num_palette;

    void *SouBuf;
    int SmeBuf;
    int LenSouBuf;
    int fFree;

    int LenX, LenY;
};

static void FilePNG_png_default_error(png_structp png_ptr, png_const_charp message) {
    throw "Error";
}

static void FilePNG_png_default_warning(png_structp png_ptr, png_const_charp message) {}

static void FilePNG_png_default_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    SPNGData *sdata = (SPNGData *)(png_get_io_ptr(png_ptr));

    if (sdata->LenSouBuf - sdata->SmeBuf < int(length))
        throw "Read Error";

    CopyMemory(data, ((BYTE *)(sdata->SouBuf)) + sdata->SmeBuf, length);

    sdata->SmeBuf += length;
}

static void FilePNG_png_default_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    SPNGData *sdata = (SPNGData *)(png_get_io_ptr(png_ptr));

    if ((sdata->SmeBuf + int(length)) < sdata->LenSouBuf) {
        CopyMemory(((BYTE *)sdata->SouBuf) + sdata->SmeBuf, data, length);
    }

    sdata->SmeBuf += length;
}

// format: 1-gray 2-rgb 3-rgba 4-palate
DWORD FilePNG_ReadStart_Buf(void *soubuf, DWORD soubuflen, DWORD *lenx, DWORD *leny, DWORD *countcolor, DWORD *format) {
    SPNGData *data = new SPNGData;
    ZeroMemory(data, sizeof(SPNGData));
    unsigned int sig_read = 0;

    try {
        data->SouBuf = soubuf;
        data->LenSouBuf = soubuflen;

        if ((data->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, FilePNG_png_default_error,
                                                    FilePNG_png_default_warning)) == NULL)
            throw "Error";

        if ((data->info_ptr = png_create_info_struct(data->png_ptr)) == NULL)
            throw "Error";

        png_set_read_fn(data->png_ptr, (void *)data, FilePNG_png_default_read_data);

        png_set_sig_bytes(data->png_ptr, sig_read);

        png_read_info(data->png_ptr, data->info_ptr);

        png_set_strip_16(data->png_ptr);
        png_set_packing(data->png_ptr);

        png_read_update_info(data->png_ptr, data->info_ptr);

        DWORD coltype = png_get_color_type(data->png_ptr, data->info_ptr);
        *countcolor = 0;
        if (coltype == PNG_COLOR_TYPE_GRAY)
            *format = 1;
        else if (coltype == PNG_COLOR_TYPE_RGB)
            *format = 2;
        else if (coltype == PNG_COLOR_TYPE_RGB_ALPHA)
            *format = 3;
        else if (coltype == PNG_COLOR_TYPE_PALETTE) {
            *format = 4;
            png_get_PLTE(data->png_ptr, data->info_ptr, &(data->palette), &(data->num_palette));
            *countcolor = data->num_palette;
        }
        else
            throw "Error";

        data->LenX = *lenx = png_get_image_width(data->png_ptr,data->info_ptr);
        data->LenY = *leny = png_get_image_height(data->png_ptr,data->info_ptr);
    }
    catch (...) {
        if (data->png_ptr != NULL)
            png_destroy_read_struct(&(data->png_ptr), &(data->info_ptr), (png_infopp)NULL);
        delete data;
        return 0;
    }
    return (DWORD)data;
}

DWORD FilePNG_Read(DWORD id, void *buf, DWORD lenline, DWORD *arraycolor) {
    SPNGData *data = (SPNGData *)id;
    png_bytep *row_pointers = NULL;
    BYTE *tbuf;
    int i;

    try {
        if (arraycolor != NULL && data->num_palette > 0) {
            for (i = 0; i < data->num_palette; i++) {
                arraycolor[i] = (DWORD(data->palette[i].red)) | ((DWORD(data->palette[i].green)) << 8) |
                                ((DWORD(data->palette[i].blue)) << 16);
            }
        }

        if (png_get_rowbytes(data->png_ptr, data->info_ptr) > lenline)
            throw "Error";

        row_pointers = (png_bytep *)malloc((data->LenY) * sizeof(png_bytep));
        for (i = 0, tbuf = (BYTE *)buf; i < data->LenY; i++, tbuf += lenline)
            row_pointers[i] = tbuf;

        png_read_image(data->png_ptr, row_pointers);
        png_read_end(data->png_ptr, data->info_ptr);
    }
    catch (...) {
        if (row_pointers != NULL)
            free(row_pointers);
        if (data->png_ptr != NULL)
            png_destroy_read_struct(&(data->png_ptr), &(data->info_ptr), (png_infopp)NULL);
        if (data->fFree != NULL && data->SouBuf != NULL)
            free(data->SouBuf);
        delete data;
        return 0;
    }

    if (row_pointers != NULL)
        free(row_pointers);
    if (data->png_ptr != NULL)
        png_destroy_read_struct(&(data->png_ptr), &(data->info_ptr), (png_infopp)NULL);
    if (data->fFree != NULL && data->SouBuf != NULL)
        free(data->SouBuf);
    delete data;
    return 1;
}

int FilePNG_Write(void *bufout, int bufoutlen, void *buf, DWORD ll, DWORD lx, DWORD ly, DWORD bytepp, int rgb_to_bgr) {
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    SPNGData data;
    BYTE **rows = NULL;
    int i;

    try {
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, FilePNG_png_default_error,
                                          FilePNG_png_default_warning);
        if (png_ptr == NULL)
            throw "Error";

        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL)
            throw "Error";

        data.SouBuf = bufout;
        data.LenSouBuf = bufoutlen;
        data.SmeBuf = 0;

        png_set_write_fn(png_ptr, (void *)&data, FilePNG_png_default_write_data, NULL);

        int type;
        if (bytepp == 1)
            type = PNG_COLOR_TYPE_GRAY;
        else if (bytepp == 2)
            type = PNG_COLOR_TYPE_GRAY_ALPHA;
        else if (bytepp == 3)
            type = PNG_COLOR_TYPE_RGB;
        else if (bytepp == 4)
            type = PNG_COLOR_TYPE_RGB_ALPHA;
        else
            throw "Error";

        png_set_IHDR(png_ptr, info_ptr, lx, ly, 8, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);

        if (rgb_to_bgr)
            png_set_bgr(png_ptr);

        rows = (BYTE **)malloc(ly * sizeof(BYTE *));
        for (i = 0; i < int(ly); i++)
            rows[i] = ((BYTE *)buf) + i * ll;

        png_write_image(png_ptr, rows);

        png_write_end(png_ptr, info_ptr);

        if (png_ptr != NULL)
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        if (rows != NULL)
            free(rows);
    }
    catch (...) {
        if (png_ptr != NULL)
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        if (rows != NULL)
            free(rows);
        return 0;
    }
    return data.SmeBuf;
}
