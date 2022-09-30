// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <cstring> // for memcpy
#include <stdexcept>

#include <png.h>

#include "FilePNG.hpp"

namespace
{

struct SPNGData
{
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

void error_callback(png_structp, png_const_charp message)
{
    throw std::runtime_error{message};
}

void warning_callback(png_structp png_ptr, png_const_charp message)
{
    // TODO: maybe some logging
}

void read_data_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    SPNGData *sdata = (SPNGData*)(png_get_io_ptr(png_ptr));

    if (sdata->LenSouBuf - sdata->SmeBuf < int(length))
    {
        throw std::runtime_error{"Read Error"};
    }

    memcpy(data, ((uint8_t*)(sdata->SouBuf)) + sdata->SmeBuf, length);

    sdata->SmeBuf += length;
}

void write_data_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    SPNGData *sdata = (SPNGData *)(png_get_io_ptr(png_ptr));

    if ((sdata->SmeBuf + int(length)) < sdata->LenSouBuf)
    {
        memcpy(((uint8_t*)sdata->SouBuf) + sdata->SmeBuf, data, length);
    }

    sdata->SmeBuf += length;
}

} // namespace

namespace FilePNG
{

// format: 1-gray 2-rgb 3-rgba 4-palate
uint32_t ReadStart_Buf(void *soubuf, uint32_t soubuflen, uint32_t *lenx, uint32_t *leny, uint32_t *countcolor, uint32_t *format)
{
    SPNGData *data = new SPNGData;
    memset(data, 0, sizeof(SPNGData));
    unsigned int sig_read = 0;

    try
    {
        data->SouBuf = soubuf;
        data->LenSouBuf = soubuflen;

        if ((data->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, error_callback,
                                                    warning_callback)) == NULL)
            throw std::runtime_error{"Error"};

        if ((data->info_ptr = png_create_info_struct(data->png_ptr)) == NULL)
            throw std::runtime_error{"Error"};

        png_set_read_fn(data->png_ptr, (void *)data, read_data_callback);

        png_set_sig_bytes(data->png_ptr, sig_read);

        png_read_info(data->png_ptr, data->info_ptr);

        png_set_strip_16(data->png_ptr);
        png_set_packing(data->png_ptr);

        png_read_update_info(data->png_ptr, data->info_ptr);

        uint32_t coltype = png_get_color_type(data->png_ptr, data->info_ptr);
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
            throw std::runtime_error{"Error"};

        data->LenX = *lenx = png_get_image_width(data->png_ptr,data->info_ptr);
        data->LenY = *leny = png_get_image_height(data->png_ptr,data->info_ptr);
    }
    catch (const std::exception&)
    {
        if (data->png_ptr != NULL)
            png_destroy_read_struct(&(data->png_ptr), &(data->info_ptr), (png_infopp)NULL);
        delete data;
        return 0;
    }
    return (uint32_t)data;
}

uint32_t Read(uint32_t id, void *buf, uint32_t lenline, uint32_t *arraycolor) {
    SPNGData *data = (SPNGData *)id;
    png_bytep *row_pointers = NULL;
    uint8_t* tbuf;
    int i;

    uint32_t result = 1;

    try
    {
        if (arraycolor != NULL && data->num_palette > 0)
        {
            for (i = 0; i < data->num_palette; i++)
            {
                arraycolor[i] =
                    (uint32_t(data->palette[i].red)) |
                    ((uint32_t(data->palette[i].green)) << 8) |
                    ((uint32_t(data->palette[i].blue)) << 16);
            }
        }

        if (png_get_rowbytes(data->png_ptr, data->info_ptr) > lenline)
            throw std::runtime_error{"Error"};

        row_pointers = (png_bytep *)malloc((data->LenY) * sizeof(png_bytep));
        for (i = 0, tbuf = (uint8_t*)buf; i < data->LenY; i++, tbuf += lenline)
            row_pointers[i] = tbuf;

        png_read_image(data->png_ptr, row_pointers);
        png_read_end(data->png_ptr, data->info_ptr);
    }
    catch (const std::exception&)
    {
        result = 0;
    }

    if (row_pointers != NULL)
        free(row_pointers);
    if (data->png_ptr != NULL)
        png_destroy_read_struct(&(data->png_ptr), &(data->info_ptr), (png_infopp)NULL);
    if (data->fFree != 0 && data->SouBuf != NULL)
        free(data->SouBuf);
    delete data;

    return result;
}

int Write(void *bufout, int bufoutlen, void *buf, uint32_t ll, uint32_t lx, uint32_t ly, uint32_t bytepp, int rgb_to_bgr) {
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    SPNGData data;
    uint8_t** rows = NULL;
    int i;

    try
    {
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, error_callback,
                                          warning_callback);
        if (png_ptr == NULL)
            throw std::runtime_error{"Error"};

        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL)
            throw std::runtime_error{"Error"};

        data.SouBuf = bufout;
        data.LenSouBuf = bufoutlen;
        data.SmeBuf = 0;

        png_set_write_fn(png_ptr, (void *)&data, write_data_callback, NULL);

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
            throw std::runtime_error{"Error"};

        png_set_IHDR(png_ptr, info_ptr, lx, ly, 8, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);

        if (rgb_to_bgr)
            png_set_bgr(png_ptr);

        rows = (uint8_t**)malloc(ly * sizeof(uint8_t*));
        for (i = 0; i < int(ly); i++)
            rows[i] = ((uint8_t*)buf) + i * ll;

        png_write_image(png_ptr, rows);

        png_write_end(png_ptr, info_ptr);

        if (png_ptr != NULL)
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        if (rows != NULL)
            free(rows);
    }
    catch (const std::exception&)
    {
        if (png_ptr != NULL)
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        if (rows != NULL)
            free(rows);
        return 0;
    }
    return data.SmeBuf;
}

} // namespace FilePNG