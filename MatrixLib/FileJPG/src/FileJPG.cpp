// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <stdio.h>

#define XMD_H
#define HAVE_BOOLEAN
extern "C" {
#include "jpeglib.h"
}

#include <windows.h>
#include "FileJPG.hpp"


struct SJpegData : jpeg_decompress_struct {
	void * SouBuf; //@field
	int LenSouBuf;//@field
	int fFree;//@field

	jpeg_error_mgr errmgr;//@field
	jpeg_source_mgr jsmgr;//@field
};

void jpeg_fun_error (j_common_ptr cinfo)
{
	throw "Error";
}

//@func
void jpeg_fun_init_source(j_decompress_ptr cinfo)
{
	SJpegData * src = (SJpegData *)cinfo;
}

boolean jpeg_fun_fill_input_buffer (j_decompress_ptr cinfo)
{
	SJpegData * src = (SJpegData *)cinfo;

	return TRUE;
}

void jpeg_fun_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	SJpegData * src = (SJpegData *)cinfo;

	if(num_bytes > 0) {
		src->jsmgr.next_input_byte += (size_t) num_bytes;
		src->jsmgr.bytes_in_buffer -= (size_t) num_bytes;
	}
}

void jpeg_fun_term_source(j_decompress_ptr cinfo)
{
	SJpegData * src = (SJpegData *)cinfo;
}

DWORD FilePNG_ReadStart_Buf(void * soubuf,DWORD soubuflen,DWORD * lenx,DWORD * leny)
{
	SJpegData * data=new SJpegData;
	ZeroMemory(data,sizeof(SJpegData));

	try {
//		if ((cinfo->infile = fopen(filename, "rb")) == NULL) return 0;
		data->SouBuf=soubuf;
		data->LenSouBuf=soubuflen;

		data->err = jpeg_std_error(&(data->errmgr));
		data->errmgr.error_exit = jpeg_fun_error;


		jpeg_create_decompress((jpeg_decompress_struct *)data);
//		jpeg_CreateDecompress(data,JPEG_LIB_VERSION,sizeof(jpeg_decompress_struct));

		data->src=&(data->jsmgr);
		data->jsmgr.init_source = jpeg_fun_init_source;
		data->jsmgr.fill_input_buffer = jpeg_fun_fill_input_buffer;
		data->jsmgr.skip_input_data = jpeg_fun_skip_input_data;
		data->jsmgr.resync_to_restart = jpeg_resync_to_restart;
		data->jsmgr.term_source = jpeg_fun_term_source;
		data->jsmgr.bytes_in_buffer = 0;
		data->jsmgr.next_input_byte = NULL;
		
		data->jsmgr.next_input_byte=(BYTE *)soubuf;
		data->jsmgr.bytes_in_buffer=soubuflen;

		jpeg_read_header(data, TRUE);

		jpeg_start_decompress(data);

		*lenx=data->output_width;
		*leny=data->output_height;
	} catch(...) {
		jpeg_destroy_decompress(data);
		delete data;
		return 0;
	}
	return (DWORD)data;
}

DWORD FilePNG_Read(DWORD id,void * buf,DWORD lenline)
{
	return 0;
}
