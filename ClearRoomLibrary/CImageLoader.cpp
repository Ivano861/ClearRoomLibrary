/*
This file is part of ClearRoomLibrary.

ClearRoomLibrary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

solidity is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ClearRoomLibrary.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include <direct.h>
#include "CImageLoader.h"
#include "jhead.h"
#include "CDecoder.h"
#include "CException.h"
#include "Macro.h"
#include "CAutoFreeMemory.h"
#include "jpeglib.h"
#include "jasper/jasper.h"	/* Decode Red camera movies */
#include "lcms2.h"

#define _USE_MATH_DEFINES // for C++  
#include <math.h> 

#pragma comment(lib,"jpeg.lib") //jpeglib Library
#pragma comment(lib,"libjasper.lib") //libjasper Library
#pragma comment(lib,"lcms2_static.lib") //lcms2 Library

using namespace Unmanaged;

/*
In order to inline this calculation, I make the risky
assumption that all filter patterns can be described
by a repeating pattern of eight rows and two columns

Do not use the FC or BAYER macros with the Leaf CatchLight,
because its pattern is 16x16, not 2x8.

Return values are either 0/1/2/3 = G/M/C/Y or 0/1/2/3 = R/G1/B/G2

PowerShot 600	PowerShot A50	PowerShot Pro70	Pro90 & G1
0xe1e4e1e4:	0x1b4e4b1e:	0x1e4b4e1b:	0xb4b4b4b4:

0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
0 G M G M G M	0 C Y C Y C Y	0 Y C Y C Y C	0 G M G M G M
1 C Y C Y C Y	1 M G M G M G	1 M G M G M G	1 Y C Y C Y C
2 M G M G M G	2 Y C Y C Y C	2 C Y C Y C Y
3 C Y C Y C Y	3 G M G M G M	3 G M G M G M
4 C Y C Y C Y	4 Y C Y C Y C
PowerShot A5	5 G M G M G M	5 G M G M G M
0x1e4e1e4e:	6 Y C Y C Y C	6 C Y C Y C Y
7 M G M G M G	7 M G M G M G
0 1 2 3 4 5
0 C Y C Y C Y
1 G M G M G M
2 C Y C Y C Y
3 M G M G M G

All RGB cameras use one of these Bayer grids:

0x16161616:	0x61616161:	0x49494949:	0x94949494:

0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
0 B G B G B G	0 G R G R G R	0 G B G B G B	0 R G R G R G
1 G R G R G R	1 B G B G B G	1 R G R G R G	1 G B G B G B
2 B G B G B G	2 G R G R G R	2 G B G B G B	2 R G R G R G
3 G R G R G R	3 B G B G B G	3 R G R G R G	3 G B G B G B
*/

#define RAW(row,col) \
	_info->raw_image[(row)*_info->raw_width+(col)]

#define FC(row,col) \
	(_info->filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

#define BAYER(row,col) \
	_info->image[((row) >> _info->shrink)*_info->iwidth + ((col) >> _info->shrink)][FC(row,col)]

#define BAYER2(row,col) \
	_info->image[((row) >> _info->shrink)*_info->iwidth + ((col) >> _info->shrink)][fcol(row,col)]


CImageLoader::CImageLoader(COptions* options, CSimpleInfo* info, CReader* reader) : _options(options), _info(info), _reader(reader), _writer(nullptr)
{
}

CImageLoader::~CImageLoader()
{
	_options = nullptr;
	_info = nullptr;
	_reader = nullptr;
	if (_writer)
	{
		_writer->~CWriter();
		_writer = nullptr;
	}
}

void CImageLoader::LoadImageRaw()
{
	if (_options->user_flip >= 0)
		_info->flip = _options->user_flip;
	switch ((_info->flip + 3600) % 360)
	{
	case 270:
		_info->flip = 5;
		break;
	case 180:
		_info->flip = 3;
		break;
	case  90:
		_info->flip = 6;
	}
	_info->write_fun = WriteThumbType::write_ppm_tiff;

	if (_info->load_raw == LoadRawType::kodak_ycbcr_load_raw)
	{
		_info->height += _info->height & 1;
		_info->width += _info->width & 1;
	}

	if (!_info->is_raw)
		throw CExceptionNoRaw();

	_info->shrink = _info->filters && (_info->half_size || _options->threshold || _options->aber[0] != 1 || _options->aber[2] != 1);
	_info->iheight = (_info->height + _info->shrink) >> _info->shrink;
	_info->iwidth = (_info->width + _info->shrink) >> _info->shrink;

	if (_info->meta_length)
	{
		_info->meta_data = (char *)malloc(_info->meta_length);
		if (!_info->meta_data)
			throw CExceptionMemory("LoadImageRaw()");
	}
	if (_info->filters || _info->colors == 1)
	{
		_info->raw_image = (unsigned short *)calloc((_info->raw_height + 7), _info->raw_width * 2);
		if (!_info->raw_image)
			throw CExceptionMemory("LoadImageRaw()");
	}
	else
	{
		_info->image = (unsigned short(*)[4]) calloc(_info->iheight, _info->iwidth * sizeof *_info->image);
		if (!_info->image)
			throw CExceptionMemory("LoadImageRaw()");
	}

	if (_info->shot_select >= _info->is_raw)
		throw CExceptionInvalidImageRequest();

	_reader->Seek(_info->data_offset, SEEK_SET);
	LoadRaw();

	if (_options->document_mode == 3)
	{
		_info->top_margin = _info->left_margin = _info->fuji_width = 0;
		_info->height = _info->raw_height;
		_info->width = _info->raw_width;
	}
	_info->iheight = (_info->height + _info->shrink) >> _info->shrink;
	_info->iwidth = (_info->width + _info->shrink) >> _info->shrink;
	if (_info->raw_image)
	{
		_info->image = (unsigned short(*)[4]) calloc(_info->iheight, _info->iwidth * sizeof *_info->image);
		if (!_info->image)
			throw CExceptionMemory("LoadImageRaw()");
		crop_masked_pixels();
		free(_info->raw_image);
		_info->raw_image = nullptr;
	}
	if (_info->zero_is_bad)
		remove_zeroes();
	bad_pixels(_options->bpfile);
	if (_options->dark_frame)
		subtract (_options->dark_frame);
	int quality = 2 + !_info->fuji_width;
	if (_options->user_qual >= 0)
		quality = _options->user_qual;
	int i = _info->cblack[3];
	for (size_t c = 0; c < 3; c++)
	{
		if (i > _info->cblack[c])
			i = _info->cblack[c];
	}
	for (size_t c = 0; c < 4; c++)
		_info->cblack[c] -= i;
	_info->black += i;
	i = _info->cblack[6];
	for (size_t c = 0; c < (_info->cblack[4] * _info->cblack[5]); c++)
		if (i > _info->cblack[6+c])
			i = _info->cblack[6+c];
	for (size_t c = 0; c < (_info->cblack[4] * _info->cblack[5]); c++)
		_info->cblack[6+c] -= i;
	_info->black += i;
	if (_options->user_black >= 0)
		_info->black = _options->user_black;
	for (size_t c = 0; c < 4; c++)
		_info->cblack[c] += _info->black;
	if (_options->user_sat > 0)
		_info->maximum = _options->user_sat;
	//#ifdef COLORCHECK
	colorcheck();
	//#endif
	if (_info->is_foveon)
	{
		if (_options->document_mode || _info->load_raw == LoadRawType::foveon_dp_load_raw)
		{
			for (i = 0; i < _info->height*_info->width * 4; i++)
				if ((short)_info->image[0][i] < 0)
					_info->image[0][i] = 0;
		}
		else
		{
			foveon_interpolate();
		}
	}
	else if (_options->document_mode < 2)
	{
		scale_colors();
	}
	pre_interpolate();
	if (_info->filters && !_options->document_mode)
	{
		if (quality == 0)
			lin_interpolate();
		else if (quality == 1 || _info->colors > 3)
			vng_interpolate();
		else if (quality == 2 && _info->filters > 1000)
			ppg_interpolate();
		else if (_info->filters == 9)
			xtrans_interpolate(quality * 2 - 3);
		else
			ahd_interpolate();
	}
	if (_info->mix_green)
	{
		_info->colors = 3;
		for (i = 0; i < _info->height*_info->width; i++)
			_info->image[i][1] = (_info->image[i][1] + _info->image[i][3]) >> 1;
	}
	if (!_info->is_foveon && _info->colors == 3)
		median_filter();
	if (!_info->is_foveon && _options->highlight == 2)
		blend_highlights();
	if (!_info->is_foveon && _options->highlight > 2)
		recover_highlights();
	if (_options->use_fuji_rotate)
		fuji_rotate();
	#ifndef NO_LCMS
	if (_options->cam_profile)
		apply_profile (_options->cam_profile, _options->out_profile);
	#endif
	convert_to_rgb();
	if (_options->use_fuji_rotate)
		stretch();

	const char* write_ext;
	if (_info->write_fun == WriteThumbType::jpeg_thumb)
		write_ext = ".jpg";
	else if (_options->output_tiff && _info->write_fun == WriteThumbType::write_ppm_tiff)
		write_ext = ".tiff";
	else
		write_ext = &".pgm\0.ppm\0.ppm\0.pam"[(_info->colors * 5 - 5)];
	char* ofname = (char *) malloc (strlen(_reader->GetFileName()) + 64);
	CAutoFreeMemory autoFreeOutput(ofname);

	strcpy(ofname, _reader->GetFileName());
	char* cp = strrchr(ofname, '.');
	if (cp)
		*cp = 0;
	if (_options->multi_out)
		sprintf(ofname + strlen(ofname), "_%0*d",
			snprintf(0, 0, "%d", _info->is_raw - 1), _info->shot_select);
	if (_options->thumbnail_only)
		strcat(ofname, ".thumb");
	strcat(ofname, write_ext);
	_writer = new CWriter(ofname);

	switch (_info->write_fun)
	{
	case WriteThumbType::layer_thumb:
		layer_thumb();
		break;
	case WriteThumbType::ppm_thumb:
		ppm_thumb();
		break;
	case WriteThumbType::ppm16_thumb:
		ppm16_thumb();
		break;
	case WriteThumbType::rollei_thumb:
		rollei_thumb();
		break;
	case WriteThumbType::jpeg_thumb:
		jpeg_thumb();
		break;
	case WriteThumbType::foveon_thumb:
		foveon_thumb();
		break;
	case WriteThumbType::write_ppm_tiff:
		write_ppm_tiff();
		break;
	case WriteThumbType::unknown_write:
	default:
		break;
	}

	/*if (_info->image)
	{
		free(_info->image);
		_info->image = nullptr;
	}*/
	/*
	if (multi_out) {
	if (++shot_select < is_raw) arg--;
	else shot_select = 0;
	}
	*/
}

void CImageLoader::LoadRaw()
{
	switch (_info->load_raw)
	{
	case LoadRawType::unknown_load_raw:
		break;
	case LoadRawType::packed_load_raw:
		packed_load_raw();
		break;
	case LoadRawType::unpacked_load_raw:
		unpacked_load_raw();
		break;
	case LoadRawType::lossless_dng_load_raw:
		lossless_dng_load_raw();
		break;
	case LoadRawType::lossy_dng_load_raw:
		lossy_dng_load_raw();
		break;
	case LoadRawType::packed_dng_load_raw:
		packed_dng_load_raw();
		break;
	case LoadRawType::lossless_jpeg_load_raw:
		lossless_jpeg_load_raw();
		break;
	case LoadRawType::canon_load_raw:
		canon_load_raw();
		break;
	case LoadRawType::canon_600_load_raw:
		canon_600_load_raw();
		break;
	case LoadRawType::canon_rmf_load_raw:
		canon_rmf_load_raw();
		break;
	case LoadRawType::canon_sraw_load_raw:
		canon_sraw_load_raw();
		break;
	case LoadRawType::eight_bit_load_raw:
		eight_bit_load_raw();
		break;
	case LoadRawType::foveon_dp_load_raw:
		foveon_dp_load_raw();
		break;
	case LoadRawType::foveon_sd_load_raw:
		foveon_sd_load_raw();
		break;
	case LoadRawType::hasselblad_load_raw:
		hasselblad_load_raw();
		break;
	case LoadRawType::imacon_full_load_raw:
		imacon_full_load_raw();
		break;
	case LoadRawType::kodak_262_load_raw:
		kodak_262_load_raw();
		break;
	case LoadRawType::kodak_65000_load_raw:
		kodak_65000_load_raw();
		break;
	case LoadRawType::kodak_c330_load_raw:
		kodak_c330_load_raw();
		break;
	case LoadRawType::kodak_c603_load_raw:
		kodak_c603_load_raw();
		break;
	case LoadRawType::kodak_dc120_load_raw:
		kodak_dc120_load_raw();
		break;
	case LoadRawType::kodak_jpeg_load_raw:
		kodak_jpeg_load_raw();
		break;
	case LoadRawType::kodak_radc_load_raw:
		kodak_radc_load_raw();
		break;
	case LoadRawType::kodak_rgb_load_raw:
		kodak_rgb_load_raw();
		break;
	case LoadRawType::kodak_ycbcr_load_raw:
		kodak_ycbcr_load_raw();
		break;
	case LoadRawType::leaf_hdr_load_raw:
		leaf_hdr_load_raw();
		break;
	case LoadRawType::minolta_rd175_load_raw:
		minolta_rd175_load_raw();
		break;
	case LoadRawType::nikon_load_raw:
		nikon_load_raw();
		break;
	case LoadRawType::nikon_yuv_load_raw:
		nikon_yuv_load_raw();
		break;
	case LoadRawType::nokia_load_raw:
		nokia_load_raw();
		break;
	case LoadRawType::olympus_load_raw:
		olympus_load_raw();
		break;
	case LoadRawType::panasonic_load_raw:
		panasonic_load_raw();
		break;
	case LoadRawType::pentax_load_raw:
		pentax_load_raw();
		break;
	case LoadRawType::phase_one_load_raw:
		phase_one_load_raw();
		break;
	case LoadRawType::phase_one_load_raw_c:
		phase_one_load_raw_c();
		break;
	case LoadRawType::quicktake_100_load_raw:
		quicktake_100_load_raw();
		break;
	case LoadRawType::redcine_load_raw:
		redcine_load_raw();
		break;
	case LoadRawType::rollei_load_raw:
		rollei_load_raw();
		break;
	case LoadRawType::samsung_load_raw:
		samsung_load_raw();
		break;
	case LoadRawType::samsung2_load_raw:
		samsung2_load_raw();
		break;
	case LoadRawType::samsung3_load_raw:
		samsung3_load_raw();
		break;
	case LoadRawType::sinar_4shot_load_raw:
		sinar_4shot_load_raw();
		break;
	case LoadRawType::smal_v6_load_raw:
		smal_v6_load_raw();
		break;
	case LoadRawType::smal_v9_load_raw:
		smal_v9_load_raw();
		break;
	case LoadRawType::sony_load_raw:
		sony_load_raw();
		break;
	case LoadRawType::sony_arw_load_raw:
		sony_arw_load_raw();
		break;
	case LoadRawType::sony_arw2_load_raw:
		sony_arw2_load_raw();
		break;
	default:
		break;
	}
}

void CImageLoader::LoadImageThumbnail()
{
	if (_options->user_flip >= 0)
		_info->flip = _options->user_flip;
	switch ((_info->flip + 3600) % 360)
	{
	case 270:
		_info->flip = 5;
		break;
	case 180:
		_info->flip = 3;
		break;
	case  90:
		_info->flip = 6;
	}
	_info->write_fun = WriteThumbType::write_ppm_tiff;

	if ((!_info->thumb_offset))
	{
		throw CExceptionNoThumbnail();
	}
	else if (_info->thumb_load_raw != LoadRawType::unknown_load_raw)
	{
		_info->load_raw = _info->thumb_load_raw;
		_info->data_offset = _info->thumb_offset;
		_info->height = _info->thumb_height;
		_info->width = _info->thumb_width;
		_info->filters = 0;
		_info->colors = 3;

		if (!_info->is_raw)
			throw CExceptionNoRaw();

		_info->shrink = _info->filters && (_info->half_size || _options->threshold || _options->aber[0] != 1 || _options->aber[2] != 1);
		_info->iheight = (_info->height + _info->shrink) >> _info->shrink;
		_info->iwidth = (_info->width + _info->shrink) >> _info->shrink;

		if (_info->meta_length)
		{
			_info->meta_data = (char *)malloc(_info->meta_length);
			if (!_info->meta_data)
				throw CExceptionMemory("LoadImageRaw()");
		}
		if (_info->filters || _info->colors == 1)
		{
			_info->raw_image = (unsigned short *)calloc((_info->raw_height + 7), _info->raw_width * 2);
			if (!_info->raw_image)
				throw CExceptionMemory("LoadImageRaw()");
		}
		else
		{
			_info->image = (unsigned short(*)[4]) calloc(_info->iheight, _info->iwidth * sizeof *_info->image);
			if (!_info->image)
				throw CExceptionMemory("LoadImageRaw()");
		}

		if (_info->shot_select >= _info->is_raw)
			throw CExceptionInvalidImageRequest();

		_reader->Seek(_info->data_offset, SEEK_SET);
		LoadRaw();

		if (_options->document_mode == 3)
		{
			_info->top_margin = _info->left_margin = _info->fuji_width = 0;
			_info->height = _info->raw_height;
			_info->width = _info->raw_width;
		}
		_info->iheight = (_info->height + _info->shrink) >> _info->shrink;
		_info->iwidth = (_info->width + _info->shrink) >> _info->shrink;
		if (_info->raw_image)
		{
			_info->image = (unsigned short(*)[4]) calloc(_info->iheight, _info->iwidth * sizeof *_info->image);
			if (!_info->image)
				throw CExceptionMemory("LoadImageRaw()");
			crop_masked_pixels();
			free(_info->raw_image);
			_info->raw_image = nullptr;
		}
		if (_info->zero_is_bad)
			remove_zeroes();
		bad_pixels(_options->bpfile);
		if (_options->dark_frame)
			subtract(_options->dark_frame);
		int quality = 2 + !_info->fuji_width;
		if (_options->user_qual >= 0)
			quality = _options->user_qual;
		int i = _info->cblack[3];
		for (size_t c = 0; c < 3; c++)
		{
			if (i > _info->cblack[c])
				i = _info->cblack[c];
		}
		for (size_t c = 0; c < 4; c++)
			_info->cblack[c] -= i;
		_info->black += i;
		i = _info->cblack[6];
		for (size_t c = 0; c < (_info->cblack[4] * _info->cblack[5]); c++)
			if (i > _info->cblack[6 + c])
				i = _info->cblack[6 + c];
		for (size_t c = 0; c < (_info->cblack[4] * _info->cblack[5]); c++)
			_info->cblack[6 + c] -= i;
		_info->black += i;
		if (_options->user_black >= 0)
			_info->black = _options->user_black;
		for (size_t c = 0; c < 4; c++)
			_info->cblack[c] += _info->black;
		if (_options->user_sat > 0)
			_info->maximum = _options->user_sat;
		//#ifdef COLORCHECK
		colorcheck();
		//#endif
		if (_info->is_foveon)
		{
			if (_options->document_mode || _info->load_raw == LoadRawType::foveon_dp_load_raw)
			{
				for (i = 0; i < _info->height*_info->width * 4; i++)
					if ((short)_info->image[0][i] < 0)
						_info->image[0][i] = 0;
			}
			else
			{
				foveon_interpolate();
			}
		}
		else if (_options->document_mode < 2)
		{
			scale_colors();
		}
		pre_interpolate();
		if (_info->filters && !_options->document_mode)
		{
			if (quality == 0)
				lin_interpolate();
			else if (quality == 1 || _info->colors > 3)
				vng_interpolate();
			else if (quality == 2 && _info->filters > 1000)
				ppg_interpolate();
			else if (_info->filters == 9)
				xtrans_interpolate(quality * 2 - 3);
			else
				ahd_interpolate();
		}
		if (_info->mix_green)
		{
			_info->colors = 3;
			for (i = 0; i < _info->height*_info->width; i++)
				_info->image[i][1] = (_info->image[i][1] + _info->image[i][3]) >> 1;
		}
		if (!_info->is_foveon && _info->colors == 3)
			median_filter();
		if (!_info->is_foveon && _options->highlight == 2)
			blend_highlights();
		if (!_info->is_foveon && _options->highlight > 2)
			recover_highlights();
		if (_options->use_fuji_rotate)
			fuji_rotate();
#ifndef NO_LCMS
		if (_options->cam_profile)
			apply_profile(_options->cam_profile, _options->out_profile);
#endif
		convert_to_rgb();
		if (_options->use_fuji_rotate)
			stretch();
	}
	else
	{
		_reader->Seek(_info->thumb_offset, SEEK_SET);
		_info->write_fun = _info->write_thumb;
	}

	const char* write_ext;
	if (_info->write_fun == WriteThumbType::jpeg_thumb)
		write_ext = ".jpg";
	else if (_options->output_tiff && _info->write_fun == WriteThumbType::write_ppm_tiff)
		write_ext = ".tiff";
	else
		write_ext = &".pgm\0.ppm\0.ppm\0.pam"[(_info->colors * 5 - 5)];
	char* ofname = (char *)malloc(strlen(_reader->GetFileName()) + 64);
	CAutoFreeMemory autoFreeOutput(ofname);

	strcpy(ofname, _reader->GetFileName());
	char* cp = strrchr(ofname, '.');
	if (cp)
		*cp = 0;
	if (_options->multi_out)
		sprintf(ofname + strlen(ofname), "_%0*d",
			snprintf(0, 0, "%d", _info->is_raw - 1), _info->shot_select);
	if (_options->thumbnail_only)
		strcat(ofname, ".thumb");
	strcat(ofname, write_ext);
	_writer = new CWriter(ofname);

	switch (_info->write_fun)
	{
	case WriteThumbType::layer_thumb:
		layer_thumb();
		break;
	case WriteThumbType::ppm_thumb:
		ppm_thumb();
		break;
	case WriteThumbType::ppm16_thumb:
		ppm16_thumb();
		break;
	case WriteThumbType::rollei_thumb:
		rollei_thumb();
		break;
	case WriteThumbType::jpeg_thumb:
		jpeg_thumb();
		break;
	case WriteThumbType::foveon_thumb:
		foveon_thumb();
		break;
	case WriteThumbType::write_ppm_tiff:
		write_ppm_tiff();
		break;
	case WriteThumbType::unknown_write:
	default:
		break;
	}
}

void CImageLoader::packed_load_raw()
{
	int vbits = 0;
	UINT64 bitbuf = 0;

	int bwide = _info->raw_width * _info->tiff_bps / 8;
	bwide += bwide & _info->load_flags >> 7;
	int rbits = bwide * 8 - _info->raw_width * _info->tiff_bps;
	if (_info->load_flags & 1)
		bwide = bwide * 16 / 15;
	int bite = 8 + (_info->load_flags & 24);
	int half = (_info->raw_height + 1) >> 1;
	for (int irow = 0; irow < _info->raw_height; irow++)
	{
		int row = irow;
		if (_info->load_flags & 2 &&
			(row = irow % half * 2 + irow / half) == 1 &&
			_info->load_flags & 4)
		{
			if (vbits = 0, _info->tiff_compress)
			{
				_reader->Seek(_info->data_offset - (-half*bwide & -2048), SEEK_SET);
			}
			else
			{
				_reader->Seek(0, SEEK_END);
				_reader->Seek(_reader->GetPosition() >> 3 << 2, SEEK_SET);
			}
		}
		for (int col = 0; col < _info->raw_width; col++)
		{
			for (vbits -= _info->tiff_bps; vbits < 0; vbits += bite)
			{
				bitbuf <<= bite;
				for (int i = 0; i < bite; i += 8)
					bitbuf |= (unsigned)(_reader->GetChar() << i);
			}
			int val = bitbuf << (64 - _info->tiff_bps - vbits) >> (64 - _info->tiff_bps);
			RAW(row, col ^ (_info->load_flags >> 6 & 1)) = val;
			if (_info->load_flags & 1 && (col % 10) == 9 && _reader->GetChar() &&
				row < _info->height + _info->top_margin && col < _info->width + _info->left_margin)
			{
				throw CExceptionFile();
			}
		}
		vbits -= rbits;
	}
}

void CImageLoader::unpacked_load_raw()
{
	int row, col, bits = 0;

	while (1 << ++bits < _info->maximum);
	_reader->read_shorts(_info->raw_image, _info->raw_width*_info->raw_height);
	for (row = 0; row < _info->raw_height; row++)
	{
		for (col = 0; col < _info->raw_width; col++)
		{
			if ((RAW(row, col) >>= _info->load_flags) >> bits
				&& (unsigned)(row - _info->top_margin) < _info->height
				&& (unsigned)(col - _info->left_margin) < _info->width)
			{
				throw CExceptionFile();
			}
		}
	}
}

void CImageLoader::lossless_dng_load_raw()
{
	unsigned trow = 0;
	unsigned tcol = 0;

	while (trow < _info->raw_height)
	{
		unsigned save = _reader->GetPosition();
		if (_info->tile_length < INT_MAX)
			_reader->Seek(_reader->get4(), SEEK_SET);
		jhead jh(*_reader, *_info, false);
		if (!jh._success)
			break;
		unsigned jwide = jh.jh.wide;
		if (_info->filters)
			jwide *= jh.jh.clrs;
		jwide /= MIN(_info->is_raw, _info->tiff_samples);
		switch (jh.jh.algo)
		{
		case 0xc1:
			jh.jh.vpred[0] = 16384;
			_info->getbits(-1);
			for (unsigned jrow = 0; jrow + 7 < jh.jh.high; jrow += 8)
			{
				for (unsigned jcol = 0; jcol + 7 < jh.jh.wide; jcol += 8)
				{
					jh.ljpeg_idct();
					unsigned short* rp = jh.jh.idct;
					unsigned row = trow + jcol / _info->tile_width + jrow * 2;
					unsigned col = tcol + jcol % _info->tile_width;
					for (unsigned i = 0; i < 16; i += 2)
						for (unsigned j = 0; j < 8; j++)
							adobe_copy_pixel(row + i, col + j, &rp);
				}
			}
			break;
		case 0xc3:
			unsigned row = 0;
			unsigned col = 0;
			for (unsigned jrow = 0; jrow < jh.jh.high; jrow++)
			{
				unsigned short* rp = jh.ljpeg_row(jrow);
				for (unsigned jcol = 0; jcol < jwide; jcol++)
				{
					adobe_copy_pixel(trow + row, tcol + col, &rp);
					if (++col >= _info->tile_width || col >= _info->raw_width)
						row += 1 + (col = 0);
				}
			}
		}
		_reader->Seek(save + 4, SEEK_SET);
		if ((tcol += _info->tile_width) >= _info->raw_width)
			trow += _info->tile_length + (tcol = 0);
	}
}

void CImageLoader::lossy_dng_load_raw()
{
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	JSAMPARRAY buf;
	JSAMPLE(*pixel)[3];
	unsigned short cur[3][256];
	double coeff[9];

	unsigned sorder = _reader->GetOrder();

	unsigned save = _info->data_offset - 4;

	if (_info->meta_offset)
	{
		_reader->Seek(_info->meta_offset, SEEK_SET);
		_reader->SetOrder(0x4d4d);
		unsigned ntags = _reader->get4();
		while (ntags--)
		{
			unsigned opcode = _reader->get4();
			_reader->get4();
			_reader->get4();
			if (opcode != 8)
			{
				_reader->Seek(_reader->get4(), SEEK_CUR);
				continue;
			}
			_reader->Seek(20, SEEK_CUR);
			unsigned c = _reader->get4();
			if (c > 2)
				break;
			_reader->Seek(12, SEEK_CUR);
			unsigned deg = _reader->get4();
			if (deg > 8)
				break;
			for (size_t i = 0; i <= deg && i < 9; i++)
				coeff[i] = _reader->getreal(12);
			for (size_t i = 0; i < 256; i++)
			{
				double tot = 0;
				for (size_t j = 0; j <= deg; j++)
					tot += coeff[j] * pow(i / 255.0, j);
				cur[c][i] = tot * 0xffff;
			}
		}
		_reader->SetOrder(sorder);
	}
	else
	{
		_info->gamma_curve(1 / 2.4, 12.92, 1, 255);
		for (size_t c = 0; c < 3; c++)
			memcpy(cur[c], _info->curve, sizeof cur[0]);
	}
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	unsigned trow = 0;
	unsigned tcol = 0;
	while (trow < _info->raw_height)
	{
		_reader->Seek(save += 4, SEEK_SET);
		if (_info->tile_length < INT_MAX)
			_reader->Seek(_reader->get4(), SEEK_SET);
		jpeg_stdio_src(&cinfo, _reader->GetFILE());
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);
		buf = (*cinfo.mem->alloc_sarray)
			((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * 3, 1);

		unsigned row;
		while (cinfo.output_scanline < cinfo.output_height &&
			(row = trow + cinfo.output_scanline) < _info->height)
		{
			jpeg_read_scanlines(&cinfo, buf, 1);
			pixel = (JSAMPLE(*)[3]) buf[0];
			for (unsigned col = 0; col < cinfo.output_width && tcol + col < _info->width; col++)
			{
				for (size_t c = 0; c < 3; c++)
					_info->image[row*_info->width + tcol + col][c] = cur[c][pixel[col][c]];
			}
		}
		jpeg_abort_decompress(&cinfo);
		if ((tcol += _info->tile_width) >= _info->raw_width)
			trow += _info->tile_length + (tcol = 0);
	}
	jpeg_destroy_decompress(&cinfo);
	_info->maximum = 0xffff;
}

void CImageLoader::packed_dng_load_raw()
{
	unsigned short* pixel = (unsigned short *)calloc(_info->raw_width, _info->tiff_samples * sizeof *pixel);
	CAutoFreeMemory autoFree(pixel);
	for (int row = 0; row < _info->raw_height; row++)
	{
		if (_info->tiff_bps == 16)
		{
			_reader->read_shorts(pixel, _info->raw_width * _info->tiff_samples);
		}
		else
		{
			_info->getbits(-1);
			for (int col = 0; col < _info->raw_width * _info->tiff_samples; col++)
				pixel[col] = _info->getbits(_info->tiff_bps);
		}
		unsigned short *rp = pixel;
		for (int col = 0; col < _info->raw_width; col++)
			adobe_copy_pixel(row, col, &rp);
	}
}

void CImageLoader::lossless_jpeg_load_raw()
{
	jhead jh(*_reader, *_info, false);
	if (!jh._success)
		return;

	int jwide = jh.jh.wide * jh.jh.clrs;
	int row = 0;
	int col = 0;

	for (int jrow = 0; jrow < jh.jh.high; jrow++)
	{
		unsigned short* rp = jh.ljpeg_row(jrow);
		if (_info->load_flags & 1)
			row = jrow & 1 ? _info->height - 1 - jrow / 2 : jrow / 2;
		for (int jcol = 0; jcol < jwide; jcol++)
		{
			int val = _info->curve[*rp++];
			if (_info->cr2_slice[0])
			{
				int jidx = jrow*jwide + jcol;
				int i = jidx / (_info->cr2_slice[1] * _info->raw_height);
				int j = i >= _info->cr2_slice[0];
				if (j)
					i = _info->cr2_slice[0];
				jidx -= i * (_info->cr2_slice[1] * _info->raw_height);
				row = jidx / _info->cr2_slice[1 + j];
				col = jidx % _info->cr2_slice[1 + j] + i*_info->cr2_slice[1];
			}
			if (_info->raw_width == 3984 && (col -= 2) < 0)
				col += (row--, _info->raw_width);
			if ((unsigned)row < _info->raw_height)
				RAW(row, col) = val;
			if (++col >= _info->raw_width)
				col = (row++, 0);
		}
	}
}

void CImageLoader::canon_load_raw()
{
	int diffbuf[64];
	int base[2];
	unsigned short* huff[2];
	int carry = 0;
	int pnum = 0;

	crw_init_tables(_info->tiff_compress, huff);
	CAutoFreeMemory autoFreeHuff0(huff[0]);
	CAutoFreeMemory autoFreeHuff1(huff[1]);

	int lowbits = canon_has_lowbits();
	if (!lowbits)
		_info->maximum = 0x3ff;
	_reader->Seek(540 + lowbits*_info->raw_height*_info->raw_width / 4, SEEK_SET);
	_info->zero_after_ff = 1;

	_info->getbits(-1);
	for (int row = 0; row < _info->raw_height; row += 8)
	{
		unsigned short* pixel = _info->raw_image + row*_info->raw_width;
		int nblocks = MIN(8, _info->raw_height - row) * _info->raw_width >> 6;
		for (int block = 0; block < nblocks; block++)
		{
			memset(diffbuf, 0, sizeof diffbuf);
			for (int i = 0; i < 64; i++)
			{
				int leaf = _info->gethuff(huff[i > 0]);
				if (leaf == 0 && i)
					break;
				if (leaf == 0xff)
					continue;
				i += leaf >> 4;
				int len = leaf & 15;
				if (len == 0)
					continue;
				int diff = _info->getbits(len);
				if ((diff & (1 << (len - 1))) == 0)
					diff -= (1 << len) - 1;
				if (i < 64)
					diffbuf[i] = diff;
			}
			diffbuf[0] += carry;
			carry = diffbuf[0];
			for (int i = 0; i < 64; i++)
			{
				if (pnum++ % _info->raw_width == 0)
					base[0] = base[1] = 512;
				if ((pixel[(block << 6) + i] = base[i & 1] += diffbuf[i]) >> 10)
					throw CExceptionFile();
			}
		}
		if (lowbits)
		{
			int save = _reader->GetPosition();
			_reader->Seek(26 + row*_info->raw_width / 4, SEEK_SET);

			unsigned short* prow = pixel;
			for (unsigned i = 0; i < _info->raw_width * 2; i++)
			{
				int c = _reader->GetChar();
				for (int r = 0; r < 8; r += 2, prow++)
				{
					int val = (*prow << 2) + ((c >> r) & 3);
					if (_info->raw_width == 2672 && val < 512) val += 2;
					*prow = val;
				}
			}
			_reader->Seek(save, SEEK_SET);
		}
	}
}

void CImageLoader::canon_600_load_raw()
{
	unsigned char  data[1120];

	int row = 0;
	for (size_t irow = 0; irow < _info->height; irow++)
	{
		if (_reader->Read(data, 1, 1120) < 1120)
			throw CExceptionFile();

		unsigned short* pix = _info->raw_image + row*_info->raw_width;
		for (unsigned char* dp = data; dp < data + 1120; dp += 10, pix += 8)
		{
			pix[0] = (dp[0] << 2) + (dp[1] >> 6);
			pix[1] = (dp[2] << 2) + (dp[1] >> 4 & 3);
			pix[2] = (dp[3] << 2) + (dp[1] >> 2 & 3);
			pix[3] = (dp[4] << 2) + (dp[1] & 3);
			pix[4] = (dp[5] << 2) + (dp[9] & 3);
			pix[5] = (dp[6] << 2) + (dp[9] >> 2 & 3);
			pix[6] = (dp[7] << 2) + (dp[9] >> 4 & 3);
			pix[7] = (dp[8] << 2) + (dp[9] >> 6);
		}
		if ((row += 2) > _info->height)
			row = 1;
	}
}

void CImageLoader::canon_rmf_load_raw()
{
	for (int row = 0; row < _info->raw_height; row++)
	{
		for (int col = 0; col < _info->raw_width - 2; col += 3)
		{
			int bits = _reader->get4();
			for (int c = 0; c < 3; c++)
			{
				int orow = row;
				int ocol = col + c - 4;
				if (ocol < 0)
				{
					ocol += _info->raw_width;
					if ((orow -= 2) < 0)
						orow += _info->raw_height;
				}
				RAW(orow, ocol) = _info->curve[bits >> (10 * c + 2) & 0x3ff];
			}
		}
	}
	_info->maximum = _info->curve[0x3ff];
}

void CImageLoader::canon_sraw_load_raw()
{
	short *rp = 0;
	short(*ip)[4];
	int jrow = 0;
	int jcol = 0;
	int v[3] = { 0,0,0 };

	jhead jh(*_reader, *_info, 0);
	if (!jh._success || jh.jh.clrs < 4)
		return;
	int jwide = (jh.jh.wide >>= 1) * jh.jh.clrs;

	int ecol = 0;
	for (int slice = 0; slice <= _info->cr2_slice[0]; slice++)
	{
		int scol = ecol;
		ecol += _info->cr2_slice[1] * 2 / jh.jh.clrs;
		if (!_info->cr2_slice[0] || ecol > _info->raw_width - 1) ecol = _info->raw_width & -2;
		for (int row = 0; row < _info->height; row += (jh.jh.clrs >> 1) - 1)
		{
			ip = (short(*)[4]) _info->image + row*_info->width;
			for (int col = scol; col < ecol; col += 2, jcol += jh.jh.clrs)
			{
				if ((jcol %= jwide) == 0)
					rp = (short *)jh.ljpeg_row(jrow++);
				if (col >= _info->width)
					continue;
				for (size_t c = 0; c < (jh.jh.clrs - 2); c++)
					ip[col + (c >> 1)*_info->width + (c & 1)][0] = rp[jcol + c];
				ip[col][1] = rp[jcol + jh.jh.clrs - 2] - 16384;
				ip[col][2] = rp[jcol + jh.jh.clrs - 1] - 16384;
			}
		}
	}

	char *cp;
	for (cp = _info->model2; *cp && !isdigit(*cp); cp++);
	sscanf(cp, "%d.%d.%d", v, v + 1, v + 2);

	int ver = (v[0] * 1000 + v[1]) * 1000 + v[2];
	int hue = (jh.jh.sraw + 1) << 2;
	if (_info->unique_id >= 0x80000281 || (_info->unique_id == 0x80000218 && ver > 1000006))
		hue = jh.jh.sraw << 1;
	ip = (short(*)[4]) _info->image;
	rp = ip[0];
	for (int row = 0; row < _info->height; row++, ip += _info->width)
	{
		if (row & (jh.jh.sraw >> 1))
		{
			for (int col = 0; col < _info->width; col += 2)
			{
				for (size_t c = 1; c < 3; c++)
				{
					if (row == _info->height - 1)
						ip[col][c] = ip[col - _info->width][c];
					else
						ip[col][c] = (ip[col - _info->width][c] + ip[col + _info->width][c] + 1) >> 1;

					for (col = 1; col < _info->width; col += 2)
					{
						for (size_t z = 1; z < 3; z++)
						{
							if (col == _info->width - 1)
								ip[col][z] = ip[col - 1][z];
							else
								ip[col][z] = (ip[col - 1][z] + ip[col + 1][z] + 1) >> 1;
						}
					}
				}
			}
		}
	}

	int pix[3];
	for (; rp < ip[0]; rp += 4)
	{
		if (_info->unique_id == 0x80000218 ||
			_info->unique_id == 0x80000250 ||
			_info->unique_id == 0x80000261 ||
			_info->unique_id == 0x80000281 ||
			_info->unique_id == 0x80000287)
		{
			rp[1] = (rp[1] << 2) + hue;
			rp[2] = (rp[2] << 2) + hue;
			pix[0] = rp[0] + ((50 * rp[1] + 22929 * rp[2]) >> 14);
			pix[1] = rp[0] + ((-5640 * rp[1] - 11751 * rp[2]) >> 14);
			pix[2] = rp[0] + ((29040 * rp[1] - 101 * rp[2]) >> 14);
		}
		else
		{
			if (_info->unique_id < 0x80000218) rp[0] -= 512;
			pix[0] = rp[0] + rp[2];
			pix[2] = rp[0] + rp[1];
			pix[1] = rp[0] + ((-778 * rp[1] - (rp[2] << 11)) >> 12);
		}
		for (size_t c = 0; c < 3; c++)
			rp[c] = CLIP(pix[c] * _info->sraw_mul[c] >> 10);
	}
	_info->maximum = 0x3fff;
}

void CImageLoader::eight_bit_load_raw()
{
	unsigned char* pixel = (unsigned char *)calloc(_info->raw_width, sizeof *pixel);
	CAutoFreeMemory autoFree(pixel);
	for (unsigned row = 0; row < _info->raw_height; row++)
	{
		if (_reader->Read(pixel, 1, _info->raw_width) < _info->raw_width)
			throw CExceptionFile();

		for (unsigned col = 0; col < _info->raw_width; col++)
			RAW(row, col) = _info->curve[pixel[col]];
	}
	_info->maximum = _info->curve[0xff];
}

void CImageLoader::hasselblad_load_raw()
{
	int* back[5];
	int len[2];
	int diff[12];

	jhead jh(*_reader, *_info, false);
	if (!jh._success)
		return;
	_reader->SetOrder(0x4949);
	ph1_bits(-1);
	back[4] = (int *)calloc(_info->raw_width, 3 * sizeof **back);
	CAutoFreeMemory autoFree(back[4]);
	for (int c = 0; c < 3; c++)
		back[c] = back[4] + c*_info->raw_width;
	int sh = _info->tiff_samples > 1;
	_info->cblack[6] >>= sh;
	int shot = LIM(_info->shot_select, 1, _info->tiff_samples) - 1;
	for (int row = 0; row < _info->raw_height; row++)
	{
		for (int c = 0; c < 4; c++)
			back[(c + 3) & 3] = back[c];
		for (int col = 0; col < _info->raw_width; col += 2)
		{
			for (int s = 0; s < _info->tiff_samples * 2; s += 2)
			{
				for (size_t c = 0; c < 2; c++)
					len[c] = ph1_huff(jh.jh.huff[0]);
				for (size_t c = 0; c < 2; c++)
				{
					diff[s + c] = ph1_bits(len[c]);
					if ((diff[s + c] & (1 << (len[c] - 1))) == 0)
						diff[s + c] -= (1 << len[c]) - 1;
					if (diff[s + c] == 65535)
						diff[s + c] = -32768;
				}
			}
			for (int s = col; s < col + 2; s++)
			{
				int pred = 0x8000 + _info->load_flags;
				if (col)
					pred = back[2][s - 2];
				if (col && row > 1)
				{
					switch (jh.jh.psv)
					{
					case 11:
						pred += back[0][s] / 2 - back[0][s - 2] / 2;
						break;
					}
				}
				int f = (row & 1) * 3 ^ ((col + s) & 1);
				for (int c = 0; c < _info->tiff_samples; c++)
				{
					pred += diff[(s & 1)*_info->tiff_samples + c];
					unsigned upix = pred >> sh & 0xffff;
					if (_info->raw_image && c == shot)
						RAW(row, s) = upix;
					if (_info->image)
					{
						unsigned urow = row - _info->top_margin + (c & 1);
						unsigned ucol = col - _info->left_margin - ((c >> 1) & 1);
						unsigned short* ip = &_info->image[urow*_info->width + ucol][f];
						if (urow < _info->height && ucol < _info->width)
							*ip = c < 4 ? upix : (*ip + upix) >> 1;
					}
				}
				back[2][s] = pred;
			}
		}
	}
	if (_info->image)
		_info->mix_green = 1;
}

void CImageLoader::imacon_full_load_raw()
{
	if (!_info->image)
		return;
	for (int row = 0; row < _info->height; row++)
		for (int col = 0; col < _info->width; col++)
			_reader->read_shorts(_info->image[row*_info->width + col], 3);
}

void CImageLoader::kodak_262_load_raw()
{
	static const unsigned char kodak_tree[2][26] =
	{
		{ 0,1,5,1,1,2,0,0,0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7,8,9 },
		{ 0,3,1,1,1,1,1,2,0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7,8,9 }
	};
	unsigned short* huff[2] = {nullptr, nullptr};
	int pi = 0;

	CAutoFreeMemory autoFreeHuff0(huff[0] = CDecoder::make_decoder(kodak_tree[0]));
	CAutoFreeMemory autoFreeHuff1(huff[1] = CDecoder::make_decoder(kodak_tree[1]));

	int ns = (_info->raw_height + 63) >> 5;
	unsigned char* pixel = (unsigned char *)malloc(_info->raw_width * 32 + ns * 4);
	CAutoFreeMemory autoFree(pixel);

	int* strip = (int *)(pixel + _info->raw_width * 32);
	_reader->SetOrder(0x4d4d);
	for (size_t c = 0; c < ns; c++)
		strip[c] = _reader->get4();
	for (int row = 0; row < _info->raw_height; row++)
	{
		if ((row & 31) == 0)
		{
			_reader->Seek(strip[row >> 5], SEEK_SET);
			_info->getbits(-1);
			pi = 0;
		}
		for (int col = 0; col < _info->raw_width; col++)
		{
			int chess = (row + col) & 1;
			int pi1 = chess ? pi - 2 : pi - _info->raw_width - 1;
			int pi2 = chess ? pi - 2 * _info->raw_width : pi - _info->raw_width + 1;
			if (col <= chess)
				pi1 = -1;
			if (pi1 < 0)
				pi1 = pi2;
			if (pi2 < 0)
				pi2 = pi1;
			if (pi1 < 0 && col > 1)
				pi1 = pi2 = pi - 2;
			int pred = (pi1 < 0) ? 0 : (pixel[pi1] + pixel[pi2]) >> 1;
			int val = pred + jhead::ljpeg_diff(huff[chess], *_info);
			pixel[pi] = val;
			if (val >> 8)
				throw CExceptionFile();
			val = _info->curve[pixel[pi++]];
			RAW(row, col) = val;
		}
	}
}

void CImageLoader::kodak_65000_load_raw()
{
	short buf[256];
	int pred[2];

	for (int row = 0; row < _info->height; row++)
	{
		for (int col = 0; col < _info->width; col += 256)
		{
			pred[0] = pred[1] = 0;
			int len = MIN(256, _info->width - col);
			int ret = kodak_65000_decode(buf, len);
			for (int i = 0; i < len; i++)
				if ((RAW(row, col + i) = _info->curve[ret ? buf[i] : (pred[i & 1] += buf[i])]) >> 12)
					throw CExceptionFile();
		}
	}
}

void CImageLoader::kodak_c330_load_raw()
{
	int rgb[3];

	unsigned char* pixel = (unsigned char *)calloc(_info->raw_width, 2 * sizeof *pixel);
	CAutoFreeMemory autoFree(pixel);

	for (int row = 0; row < _info->height; row++)
	{
		if (_reader->Read(pixel, _info->raw_width, 2) < 2)
			throw CExceptionFile();

		if (_info->load_flags && (row & 31) == 31)
			_reader->Seek(_info->raw_width * 32, SEEK_CUR);
		for (int col = 0; col < _info->width; col++)
		{
			int y = pixel[col * 2];
			int cb = pixel[(col * 2 & -4) | 1] - 128;
			int cr = pixel[(col * 2 & -4) | 3] - 128;
			rgb[1] = y - ((cb + cr + 2) >> 2);
			rgb[2] = rgb[1] + cb;
			rgb[0] = rgb[1] + cr;
			for (size_t c = 0; c < 3; c++)
				_info->image[row*_info->width + col][c] = _info->curve[LIM(rgb[c], 0, 255)];
		}
	}
	_info->maximum = _info->curve[0xff];
}

void CImageLoader::kodak_c603_load_raw()
{
	int rgb[3];

	unsigned char* pixel = (unsigned char *)calloc(_info->raw_width, 3 * sizeof *pixel);
	CAutoFreeMemory autoFree(pixel);
	for (int row = 0; row < _info->height; row++)
	{
		if (~row & 1)
			if (_reader->Read(pixel, _info->raw_width, 3) < 3)
				throw CExceptionFile();

		for (int col = 0; col < _info->width; col++)
		{
			int y = pixel[_info->width * 2 * (row & 1) + col];
			int cb = pixel[_info->width + (col & -2)] - 128;
			int cr = pixel[_info->width + (col & -2) + 1] - 128;
			rgb[1] = y - ((cb + cr + 2) >> 2);
			rgb[2] = rgb[1] + cb;
			rgb[0] = rgb[1] + cr;
			for (size_t c = 0; c < 3; c++)
				_info->image[row*_info->width + col][c] = _info->curve[LIM(rgb[c], 0, 255)];
		}
	}
	_info->maximum = _info->curve[0xff];
}

void CImageLoader::kodak_dc120_load_raw()
{
	static const int mul[4] = { 162, 192, 187,  92 };
	static const int add[4] = { 0, 636, 424, 212 };
	unsigned char pixel[848];

	for (int row = 0; row < _info->height; row++)
	{
		if (_reader->Read(pixel, 1, 848) < 848)
			throw CExceptionFile();

		int shift = row * mul[row & 3] + add[row & 3];
		for (int col = 0; col < _info->width; col++)
			RAW(row, col) = (unsigned short)pixel[(col + shift) % 848];
	}
	_info->maximum = 0xff;
}

#pragma region Manege method jpeg.lib
static FILE* _fileForMethod;
METHODDEF(boolean) fill_input_buffer(j_decompress_ptr cinfo)
{
	static unsigned char jpeg_buffer[4096];

	size_t nbytes = fread(jpeg_buffer, 1, 4096, _fileForMethod);
	swab((char*)jpeg_buffer, (char*)jpeg_buffer, nbytes);
	cinfo->src->next_input_byte = jpeg_buffer;
	cinfo->src->bytes_in_buffer = nbytes;
	return TRUE;
}
#pragma endregion

void CImageLoader::kodak_jpeg_load_raw()
{
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	JSAMPARRAY buf;
	JSAMPLE(*pixel)[3];
	int row, col;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, _reader->GetFILE());
#pragma region Using method jpeg.lib
	//Assign pointer to file for method jpeg.lib
	_fileForMethod = _reader->GetFILE();
	cinfo.src->fill_input_buffer = fill_input_buffer;
	_fileForMethod = nullptr;
#pragma endregion
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	if ((cinfo.output_width != _info->width) ||
		(cinfo.output_height * 2 != _info->height) ||
		(cinfo.output_components != 3))
	{
		jpeg_destroy_decompress(&cinfo);
		throw CExceptionJpegError();;
	}
	buf = (*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo, JPOOL_IMAGE, _info->width * 3, 1);

	while (cinfo.output_scanline < cinfo.output_height)
	{
		row = cinfo.output_scanline * 2;
		jpeg_read_scanlines(&cinfo, buf, 1);
		pixel = (JSAMPLE(*)[3]) buf[0];
		for (col = 0; col < _info->width; col += 2)
		{
			RAW(row + 0, col + 0) = pixel[col + 0][1] << 1;
			RAW(row + 1, col + 1) = pixel[col + 1][1] << 1;
			RAW(row + 0, col + 1) = pixel[col][0] + pixel[col + 1][0];
			RAW(row + 1, col + 0) = pixel[col][2] + pixel[col + 1][2];
		}
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	_info->maximum = 0xff << 1;
}

#define FORYX for (y=1; y < 3; y++) for (x=col+1; x >= col; x--)
#define PREDICTOR (c ? (buf[c][y-1][x] + buf[c][y][x+1]) / 2 \
: (buf[c][y-1][x+1] + 2*buf[c][y-1][x] + buf[c][y][x+1]) / 4)
void CImageLoader::kodak_radc_load_raw()
{
	static const char src[] =
	{
		1,1, 2,3, 3,4, 4,2, 5,7, 6,5, 7,6, 7,8,
		1,0, 2,1, 3,3, 4,4, 5,2, 6,7, 7,6, 8,5, 8,8,
		2,1, 2,3, 3,0, 3,2, 3,4, 4,6, 5,5, 6,7, 6,8,
		2,0, 2,1, 2,3, 3,2, 4,4, 5,6, 6,7, 7,5, 7,8,
		2,1, 2,4, 3,0, 3,2, 3,3, 4,7, 5,5, 6,6, 6,8,
		2,3, 3,1, 3,2, 3,4, 3,5, 3,6, 4,7, 5,0, 5,8,
		2,3, 2,6, 3,0, 3,1, 4,4, 4,5, 4,7, 5,2, 5,8,
		2,4, 2,7, 3,3, 3,6, 4,1, 4,2, 4,5, 5,0, 5,8,
		2,6, 3,1, 3,3, 3,5, 3,7, 3,8, 4,0, 5,2, 5,4,
		2,0, 2,1, 3,2, 3,3, 4,4, 4,5, 5,6, 5,7, 4,8,
		1,0, 2,2, 2,-2,
		1,-3, 1,3,
		2,-17, 2,-5, 2,5, 2,17,
		2,-7, 2,2, 2,9, 2,18,
		2,-18, 2,-9, 2,-2, 2,7,
		2,-28, 2,28, 3,-49, 3,-9, 3,9, 4,49, 5,-79, 5,79,
		2,-1, 2,13, 2,26, 3,39, 4,-16, 5,55, 6,-37, 6,76,
		2,-26, 2,-13, 2,1, 3,-39, 4,16, 5,-55, 6,-76, 6,37
	};
	static const unsigned short pt[] = { 0,0, 1280,1344, 2320,3616, 3328,8000, 4095,16383, 65535,16383 };
	unsigned short huff[19][256];
	short last[3] = { 16,16,16 };
	short mul[3];
	short buf[3][3][386];
	int x, y;

	for (size_t i = 2; i < 12; i += 2)
		for (size_t c = pt[i - 2]; c <= pt[i]; c++)
			_info->curve[c] = (float)(c - pt[i - 2]) / (pt[i] - pt[i - 2]) * (pt[i + 1] - pt[i - 1]) + pt[i - 1] + 0.5;
	int s = 0;
	for (size_t i = 0; i < sizeof src; i += 2)
		for (int c = 0; c < (256 >> src[i]); c++)
			((unsigned short *)huff)[s++] = src[i] << 8 | (unsigned char)src[i + 1];
	s = _info->kodak_cbpp == 243 ? 2 : 3;
	for (int c = 0; c < 256; c++)
		huff[18][c] = (8 - s) << 8 | c >> s << s | 1 << (s - 1);
	_info->getbits(-1);
	for (size_t i = 0; i < sizeof(buf) / sizeof(short); i++)
		((short *)buf)[i] = 2048;
	for (int row = 0; row < _info->height; row += 4)
	{
		for (size_t c = 0; c < 3; c++)
			mul[c] = _info->getbits(6);
		for (int c = 0; c < 3; c++)
		{
			int val = ((0x1000000 / last[c] + 0x7ff) >> 12) * mul[c];
			s = val > 65564 ? 10 : 12;
			x = ~(-1 << (s - 1));
			val <<= 12 - s;
			for (size_t i = 0; i < sizeof(buf[0]) / sizeof(short); i++)
				((short *)buf[c])[i] = (((short *)buf[c])[i] * val + x) >> s;
			last[c] = mul[c];
			for (int r = 0; r <= !c; r++)
			{
				buf[c][1][_info->width / 2] = buf[c][2][_info->width / 2] = mul[c] << 7;
				int tree = 1;
				for (int col = _info->width / 2; col > 0; )
				{
					if ((tree = radc_token(tree)))
					{
						col -= 2;
						if (tree == 8)
							FORYX buf[c][y][x] = (unsigned char)radc_token(18) * mul[c];
						else
							FORYX buf[c][y][x] = radc_token(tree + 10) * 16 + PREDICTOR;
					}
					else
					{
						int nreps;
						do
						{
							nreps = (col > 2) ? radc_token(9) + 1 : 1;
							for (int rep = 0; rep < 8 && rep < nreps && col > 0; rep++)
							{
								col -= 2;
								FORYX buf[c][y][x] = PREDICTOR;
								if (rep & 1)
								{
									int step = radc_token(10) << 4;
									FORYX buf[c][y][x] += step;
								}
							}
						} while (nreps == 9);
					}
				}
				for (y = 0; y < 2; y++)
				{
					for (x = 0; x < _info->width / 2; x++)
					{
						int val = (buf[c][y + 1][x] << 4) / mul[c];
						if (val < 0) val = 0;
						if (c)
							RAW(row + y * 2 + c - 1, x * 2 + 2 - c) = val;
						else
							RAW(row + r * 2 + y, x * 2 + y) = val;
					}
				}
				memcpy(buf[c][0] + !c, buf[c][2], sizeof buf[c][0] - 2 * !c);
			}
		}
		for (y = row; y < row + 4; y++)
		{
			for (x = 0; x < _info->width; x++)
			{
				if ((x + y) & 1)
				{
					int r = x ? x - 1 : x + 1;
					s = x + 1 < _info->width ? x + 1 : x - 1;
					int val = (RAW(y, x) - 2048) * 2 + (RAW(y, r) + RAW(y, s)) / 2;
					if (val < 0)
						val = 0;
					RAW(y, x) = val;
				}
			}
		}
	}
	for (size_t i = 0; i < _info->height*_info->width; i++)
		_info->raw_image[i] = _info->curve[_info->raw_image[i]];
	_info->maximum = 0x3fff;
}
#undef FORYX
#undef PREDICTOR

void CImageLoader::kodak_rgb_load_raw()
{
	short buf[768];
	int rgb[3];

	unsigned short *ip = _info->image[0];

	for (int row = 0; row < _info->height; row++)
	{
		for (int col = 0; col < _info->width; col += 256)
		{
			int len = MIN(256, _info->width - col);
			kodak_65000_decode(buf, len * 3);
			memset(rgb, 0, sizeof rgb);
			short* bp = buf;
			for (int i = 0; i < len; i++, ip += 4)
				for (size_t c = 0; c < 3; c++)
					if ((ip[c] = rgb[c] += *bp++) >> 12)
						throw CExceptionFile();
		}
	}
}

void CImageLoader::kodak_ycbcr_load_raw()
{
	short buf[384];
	int y[2][2];
	int rgb[3];

	if (!_info->image)
		return;
	for (int row = 0; row < _info->height; row += 2)
	{
		for (int col = 0; col < _info->width; col += 128)
		{
			int len = MIN(128, _info->width - col);
			kodak_65000_decode(buf, len * 3);
			int cb = 0;
			int cr = 0;
			y[0][1] = y[1][1] = 0;
			short* bp = buf;
			for (int i = 0; i < len; i += 2, bp += 2)
			{
				cb += bp[4];
				cr += bp[5];
				rgb[1] = -((cb + cr + 2) >> 2);
				rgb[2] = rgb[1] + cb;
				rgb[0] = rgb[1] + cr;
				for (int j = 0; j < 2; j++)
				{
					for (int k = 0; k < 2; k++)
					{
						if ((y[j][k] = y[j][k ^ 1] + *bp++) >> 10)
							throw CExceptionFile();

						unsigned short* ip = _info->image[(row + j)*_info->width + col + i + k];
						for (size_t c = 0; c < 3; c++)
							ip[c] = _info->curve[LIM(y[j][k] + rgb[c], 0, 0xfff)];
					}
				}
			}
		}
	}
}

void CImageLoader::kodak_thumb_load_raw()
{
	_info->colors = _info->thumb_misc >> 5;
	for (int row = 0; row < _info->height; row++)
		for (int col = 0; col < _info->width; col++)
			_reader->read_shorts(_info->image[row*_info->width + col], _info->colors);
	_info->maximum = (1 << (_info->thumb_misc & 31)) - 1;
}

void CImageLoader::leaf_hdr_load_raw()
{
	unsigned short *pixel = nullptr;
	CAutoFreeMemory autoFree(pixel, false);

	if (!_info->filters)
	{
		pixel = (unsigned short *)calloc(_info->raw_width, sizeof *pixel);
		autoFree.ChangePointer(pixel);
	}

	unsigned tile = 0;
	for (size_t c = 0; c < _info->tiff_samples; c++)
	{
		for (size_t r = 0; r < _info->raw_height; r++)
		{
			if (r % _info->tile_length == 0)
			{
				_reader->Seek(_info->data_offset + 4 * tile++, SEEK_SET);
				_reader->Seek(_reader->get4(), SEEK_SET);
			}
			if (_info->filters && c != _info->shot_select)
				continue;
			if (_info->filters)
				pixel = _info->raw_image + r*_info->raw_width;

			_reader->read_shorts(pixel, _info->raw_width);
			int row = r - _info->top_margin;
			if (!_info->filters && row < _info->height)
				for (size_t col = 0; col < _info->width; col++)
					_info->image[row*_info->width + col][c] = pixel[col + _info->left_margin];
		}
	}
	if (!_info->filters)
	{
		_info->maximum = 0xffff;
		_info->raw_color = 1;
	}
}

void CImageLoader::minolta_rd175_load_raw()
{
	unsigned char pixel[768];

	for (size_t irow = 0; irow < 1481; irow++)
	{
		if (_reader->Read(pixel, 1, 768) < 768)
			throw CExceptionFile();

		unsigned box = irow / 82;
		unsigned row = irow % 82 * 12 + ((box < 12) ? box | 1 : (box - 12) * 2);
		switch (irow)
		{
		case 1477:
		case 1479:
			continue;
		case 1476:
			row = 984;
			break;
		case 1480:
			row = 985;
			break;
		case 1478:
			row = 985; box = 1;
		}
		if ((box < 12) && (box & 1))
		{
			for (size_t col = 0; col < 1533; col++, row ^= 1)
				if (col != 1) RAW(row, col) = (col + 1) & 2 ?
					pixel[col / 2 - 1] + pixel[col / 2 + 1] : pixel[col / 2] << 1;
			RAW(row, 1) = pixel[1] << 1;
			RAW(row, 1533) = pixel[765] << 1;
		}
		else
		{
			for (size_t col = row & 1; col < 1534; col += 2)
				RAW(row, col) = pixel[col / 2] << 1;
		}
	}
	_info->maximum = 0xff << 1;
}

void CImageLoader::nikon_load_raw()
{
	static const unsigned char nikon_tree[][32] =
	{
		{ 0,1,5,1,1,1,1,1,1,2,0,0,0,0,0,0,	/* 12-bit lossy */
		5,4,3,6,2,7,1,0,8,9,11,10,12 },
		{ 0,1,5,1,1,1,1,1,1,2,0,0,0,0,0,0,	/* 12-bit lossy after split */
		0x39,0x5a,0x38,0x27,0x16,5,4,3,2,1,0,11,12,12 },
		{ 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,  /* 12-bit lossless */
		5,4,6,3,7,2,8,1,9,0,10,11,12 },
		{ 0,1,4,3,1,1,1,1,1,2,0,0,0,0,0,0,	/* 14-bit lossy */
		5,6,4,7,8,3,9,2,1,0,10,11,12,13,14 },
		{ 0,1,5,1,1,1,1,1,1,1,2,0,0,0,0,0,	/* 14-bit lossy after split */
		8,0x5c,0x4b,0x3a,0x29,7,6,5,4,3,2,1,0,13,14 },
		{ 0,1,4,2,2,3,1,2,0,0,0,0,0,0,0,0,	/* 14-bit lossless */
		7,6,8,5,9,4,10,3,11,12,2,0,1,13,14 }
	};
	unsigned short vpred[2][2];
	unsigned short hpred[2], csize;
	int step = 0;
	int tree = 0;
	int split = 0;

	_reader->Seek(_info->meta_offset, SEEK_SET);
	unsigned short ver0 = _reader->GetChar();
	unsigned short ver1 = _reader->GetChar();
	if (ver0 == 0x49 || ver1 == 0x58)
		_reader->Seek(2110, SEEK_CUR);
	if (ver0 == 0x46)
		tree = 2;
	if (_info->tiff_bps == 14)
		tree += 3;
	_reader->read_shorts(vpred[0], 4);
	int max = 1 << _info->tiff_bps & 0x7fff;
	if ((csize = _reader->get2()) > 1)
		step = max / (csize - 1);
	if (ver0 == 0x44 && ver1 == 0x20 && step > 0)
	{
		for (int i = 0; i < csize; i++)
			_info->curve[i*step] = _reader->get2();
		for (int i = 0; i < max; i++)
			_info->curve[i] = (_info->curve[i - i%step] * (step - i%step) + _info->curve[i - i%step + step] * (i%step)) / step;
		_reader->Seek(_info->meta_offset + 562, SEEK_SET);
		split = _reader->get2();
	}
	else if (ver0 != 0x46 && csize <= 0x4001)
	{
		_reader->read_shorts(_info->curve, max = csize);
	}

	while (_info->curve[max - 2] == _info->curve[max - 1])
		max--;

	unsigned short* huff = CDecoder::make_decoder(nikon_tree[tree]);
	CAutoFreeMemory autoFree(huff);
	_reader->Seek(_info->data_offset, SEEK_SET);
	_info->getbits(-1);
	int min = 0;
	for (int row = 0; row < _info->height; row++)
	{
		if (split && row == split)
		{
			autoFree.ChangePointer(huff = CDecoder::make_decoder(nikon_tree[tree + 1]));
			max += (min = 16) << 1;
		}
		for (int col = 0; col < _info->raw_width; col++)
		{
			int i = _info->gethuff(huff);
			int len = i & 15;
			int shl = i >> 4;
			int diff = ((_info->getbits(len - shl) << 1) + 1) << shl >> 1;
			if ((diff & (1 << (len - 1))) == 0)
				diff -= (1 << len) - !shl;
			if (col < 2)
				hpred[col] = vpred[row & 1][col] += diff;
			else
				hpred[col & 1] += diff;
			if ((unsigned short)(hpred[col & 1] + min) >= max)
				throw CExceptionFile();

			RAW(row, col) = _info->curve[LIM((short)hpred[col & 1], 0, 0x3fff)];
		}
	}
}

void CImageLoader::nikon_yuv_load_raw()
{
	int yuv[4];
	int rgb[3];
	UINT64 bitbuf = 0;

	for (int row = 0; row < _info->raw_height; row++)
	{
		for (int col = 0; col < _info->raw_width; col++)
		{
			int b = col & 1;
			if (!b)
			{
				bitbuf = 0;
				for (int c = 0; c < 6; c++)
					bitbuf |= (UINT64)_reader->GetChar() << c * 8;
				for (int c = 0; c < 4; c++)
					yuv[c] = (bitbuf >> c * 12 & 0xfff) - (c >> 1 << 11);
			}
			rgb[0] = yuv[b] + 1.370705*yuv[3];
			rgb[1] = yuv[b] - 0.337633*yuv[2] - 0.698001*yuv[3];
			rgb[2] = yuv[b] + 1.732446*yuv[2];
			for (size_t c = 0; c < 3; c++)
				_info->image[row*_info->width + col][c] = _info->curve[LIM(rgb[c], 0, 0xfff)] / _info->cam_mul[c];
		}
	}
}

void CImageLoader::nokia_load_raw()
{
	int rev = 3 * (_reader->GetOrder() == 0x4949);
	int dwide = (_info->raw_width * 5 + 1) / 4;
	unsigned char* data = (unsigned char *)malloc(dwide * 2);
	CAutoFreeMemory autoFree(data);

	for (int row = 0; row < _info->raw_height; row++)
	{
		if (_reader->Read(data + dwide, 1, dwide) < dwide)
			throw CExceptionFile();

		for (int c = 0; c < dwide; c++)
			data[c] = data[dwide + (c ^ rev)];
		unsigned char* dp = data;
		for (int col = 0; col < _info->raw_width; dp += 5, col += 4)
			for (int c = 0; c < 4; c++)
				RAW(row, col + c) = (dp[c] << 2) | (dp[4] >> (c << 1) & 3);
	}
	autoFree.Release();

	_info->maximum = 0x3ff;
	if (strcmp(_info->make, "OmniVision"))
		return;

	int row = _info->raw_height / 2;
	double sum[] = { 0,0 };
	for (int c = 0; c < (_info->width - 1); c++)
	{
		sum[c & 1] += SQR(RAW(row, c) - RAW(row + 1, c + 1));
		sum[~c & 1] += SQR(RAW(row + 1, c) - RAW(row, c + 1));
	}
	if (sum[1] > sum[0])
		_info->filters = 0x4b4b4b4b;
}

void CImageLoader::olympus_load_raw()
{
	unsigned short huff[4096];
	int acarry[2][3];

	int n = 0;
	huff[n] = 0xc0c;
	for (int i = 12; i--; )
		for (int c = 0; c < (2048 >> i); c++)
			huff[++n] = (i + 1) << 8 | i;
	_reader->Seek(7, SEEK_CUR);
	_info->getbits(-1);
	for (int row = 0; row < _info->height; row++)
	{
		memset(acarry, 0, sizeof acarry);
		for (int col = 0; col < _info->raw_width; col++)
		{
			int* carry = acarry[col & 1];
			int i = 2 * (carry[2] < 3);
			int nbits;
			for (nbits = 2 + i; (unsigned short)carry[0] >> (nbits + i); nbits++);
			int sign = _info->getbits(3);
			int low = sign & 3;
			sign = sign << 29 >> 31;
			int high = _info->getbithuff(12, huff);
			if (high == 12)
				high = _info->getbits(16 - nbits) >> 1;
			carry[0] = (high << nbits) | _info->getbits(nbits);
			int diff = (carry[0] ^ sign) + carry[1];
			carry[1] = (diff * 3 + carry[1]) >> 5;
			carry[2] = carry[0] > 16 ? 0 : carry[2] + 1;
			if (col >= _info->width)
				continue;
			int pred;
			if (row < 2 && col < 2)
				pred = 0;
			else if (row < 2)
				pred = RAW(row, col - 2);
			else if (col < 2)
				pred = RAW(row - 2, col);
			else
			{
				int w = RAW(row, col - 2);
				int n = RAW(row - 2, col);
				int nw = RAW(row - 2, col - 2);
				if ((w < nw && nw < n) || (n < nw && nw < w))
				{
					if (ABS(w - nw) > 32 || ABS(n - nw) > 32)
						pred = w + n - nw;
					else
						pred = (w + n) >> 1;
				}
				else
				{
					pred = ABS(w - nw) > ABS(n - nw) ? w : n;
				}
			}
			if ((RAW(row, col) = pred + ((diff << 2) | low)) >> 12)
				throw CExceptionFile();
		}
	}
}

void CImageLoader::panasonic_load_raw()
{
	int sh = 0;
	int pred[2];
	int nonz[2];

	pana_bits(0);
	for (int row = 0; row < _info->height; row++)
	{
		for (int col = 0; col < _info->raw_width; col++)
		{
			int i = col % 14;
			if (i == 0)
				pred[0] = pred[1] = nonz[0] = nonz[1] = 0;
			if (i % 3 == 2)
				sh = 4 >> (3 - pana_bits(2));
			if (nonz[i & 1])
			{
				int j = pana_bits(8);
				if ((j))
				{
					if ((pred[i & 1] -= 0x80 << sh) < 0 || sh == 4)
						pred[i & 1] &= ~(-1 << sh);
					pred[i & 1] += j << sh;
				}
			}
			else if ((nonz[i & 1] = pana_bits(8)) || i > 11)
			{
				pred[i & 1] = nonz[i & 1] << 4 | pana_bits(4);
			}
			if ((RAW(row, col) = pred[col & 1]) > 4098 && col < _info->width)
				throw CExceptionFile();
		}
	}
}

void CImageLoader::pentax_load_raw()
{
	unsigned short bit[2][15];
	unsigned short huff[4097];
	unsigned short vpred[2][2] = { { 0,0 },{ 0,0 } };
	unsigned short hpred[2];

	_reader->Seek(_info->meta_offset, SEEK_SET);
	int dep = (_reader->get2() + 12) & 15;
	_reader->Seek(12, SEEK_CUR);
	for (size_t c = 0; c < dep; c++)
		bit[0][c] = _reader->get2();
	for (size_t c = 0; c < dep; c++)
		bit[1][c] = _reader->GetChar();
	for (size_t c = 0; c < dep; c++)
		for (int i = bit[0][c]; i <= ((bit[0][c] + (4096 >> bit[1][c]) - 1) & 4095); )
			huff[++i] = bit[1][c] << 8 | c;
	huff[0] = 12;
	_reader->Seek(_info->data_offset, SEEK_SET);
	_info->getbits(-1);
	for (int row = 0; row < _info->raw_height; row++)
	{
		for (int col = 0; col < _info->raw_width; col++)
		{
			int diff = jhead::ljpeg_diff(huff, *_info);
			if (col < 2)
				hpred[col] = vpred[row & 1][col] += diff;
			else
				hpred[col & 1] += diff;
			RAW(row, col) = hpred[col & 1];
			if (hpred[col & 1] >> _info->tiff_bps)
				throw CExceptionFile();
		}
	}
}

void CImageLoader::phase_one_load_raw()
{
	_reader->Seek(_info->ph1.key_off, SEEK_SET);
	unsigned short akey = _reader->get2();
	unsigned short bkey = _reader->get2();
	unsigned short mask = _info->ph1.format == 1 ? 0x5555 : 0x1354;
	_reader->Seek(_info->data_offset, SEEK_SET);
	_reader->read_shorts(_info->raw_image, _info->raw_width*_info->raw_height);
	if (_info->ph1.format)
	{
		for (size_t i = 0; i < _info->raw_width*_info->raw_height; i += 2)
		{
			unsigned short a = _info->raw_image[i + 0] ^ akey;
			unsigned short b = _info->raw_image[i + 1] ^ bkey;
			_info->raw_image[i + 0] = (a & mask) | (b & ~mask);
			_info->raw_image[i + 1] = (b & mask) | (a & ~mask);
		}
	}
}

void CImageLoader::phase_one_load_raw_c()
{
	static const int length[] = { 8,7,6,9,11,10,5,12,14,13 };
	int len[2];
	int pred[2];
	short (*cblack)[2];
	short (*rblack)[2];

	unsigned short* pixel = (unsigned short *)calloc(_info->raw_width * 3 + _info->raw_height * 4, 2);
	CAutoFreeMemory autoFree(pixel);

	int* offset = (int *)(pixel + _info->raw_width);
	_reader->Seek(_info->strip_offset, SEEK_SET);
	for (size_t row = 0; row < _info->raw_height; row++)
		offset[row] = _reader->get4();
	cblack = (short(*)[2]) (offset + _info->raw_height);
	_reader->Seek(_info->ph1.black_col, SEEK_SET);
	if (_info->ph1.black_col)
		_reader->read_shorts((unsigned short *)cblack[0], _info->raw_height * 2);

	rblack = cblack + _info->raw_height;
	_reader->Seek(_info->ph1.black_row, SEEK_SET);
	if (_info->ph1.black_row)
		_reader->read_shorts((unsigned short *)rblack[0], _info->raw_width * 2);

	for (unsigned i = 0; i < 256; i++)
		_info->curve[i] = i*i / 3.969 + 0.5;
	for (int row = 0; row < _info->raw_height; row++)
	{
		_reader->Seek(_info->data_offset + offset[row], SEEK_SET);
		ph1_bits(-1);
		pred[0] = pred[1] = 0;
		for (int col = 0; col < _info->raw_width; col++)
		{
			if (col >= (_info->raw_width & -8))
			{
				len[0] = len[1] = 14;
			}
			else if ((col & 7) == 0)
			{
				for (size_t i = 0; i < 2; i++)
				{
					int j;
					for (j = 0; j < 5 && !ph1_bits(1); j++);
					if (j--)
						len[i] = length[j * 2 + ph1_bits(1)];
				}
			}
			int i = len[col & 1];
			if (i == 14)
				pixel[col] = pred[col & 1] = ph1_bits(16);
			else
				pixel[col] = pred[col & 1] += ph1_bits(i) + 1 - (1 << (i - 1));
			if (pred[col & 1] >> 16)
				throw CExceptionFile();

			if (_info->ph1.format == 5 && pixel[col] < 256)
				pixel[col] = _info->curve[pixel[col]];
		}
		for (int col = 0; col < _info->raw_width; col++)
		{
			int i = (pixel[col] << 2 * (_info->ph1.format != 8)) - _info->ph1.black
				+ cblack[row][col >= _info->ph1.split_col]
				+ rblack[col][row >= _info->ph1.split_row];
			if (i > 0)
				RAW(row, col) = i;
		}
	}
	_info->maximum = 0xfffc - _info->ph1.black;
}

void CImageLoader::quicktake_100_load_raw()
{
	static const short gstep[16] =
	{ -89,-60,-44,-32,-22,-15,-8,-2,2,8,15,22,32,44,60,89 };
	static const short rstep[6][4] =
	{
		{ -3,-1,1,3 },{ -5,-1,1,5 },{ -8,-2,2,8 },
		{ -13,-3,3,13 },{ -19,-4,4,19 },{ -28,-6,6,28 }
	};
	static const short curve[256] =
	{
		0,1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
		28,29,30,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,53,
		54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,74,75,76,77,78,
		79,80,81,82,83,84,86,88,90,92,94,97,99,101,103,105,107,110,112,114,116,
		118,120,123,125,127,129,131,134,136,138,140,142,144,147,149,151,153,155,
		158,160,162,164,166,168,171,173,175,177,179,181,184,186,188,190,192,195,
		197,199,201,203,205,208,210,212,214,216,218,221,223,226,230,235,239,244,
		248,252,257,261,265,270,274,278,283,287,291,296,300,305,309,313,318,322,
		326,331,335,339,344,348,352,357,361,365,370,374,379,383,387,392,396,400,
		405,409,413,418,422,426,431,435,440,444,448,453,457,461,466,470,474,479,
		483,487,492,496,500,508,519,531,542,553,564,575,587,598,609,620,631,643,
		654,665,676,687,698,710,721,732,743,754,766,777,788,799,810,822,833,844,
		855,866,878,889,900,911,922,933,945,956,967,978,989,1001,1012,1023
	};
	unsigned char pixel[484][644];
	int sharp = 0;
	int val = 0;

	_info->getbits(-1);
	memset(pixel, 0x80, sizeof pixel);
	for (int row = 2; row < _info->height + 2; row++)
	{
		int col;
		for (col = 2 + (row & 1); col < _info->width + 2; col += 2)
		{
			val = ((pixel[row - 1][col - 1] + 2 * pixel[row - 1][col + 1] +
				pixel[row][col - 2]) >> 2) + gstep[_info->getbits(4)];
			pixel[row][col] = val = LIM(val, 0, 255);
			if (col < 4)
				pixel[row][col - 2] = pixel[row + 1][~row & 1] = val;
			if (row == 2)
				pixel[row - 1][col + 1] = pixel[row - 1][col + 3] = val;
		}
		pixel[row][col] = val;
	}
	for (int rb = 0; rb < 2; rb++)
	{
		for (int row = 2 + rb; row < _info->height + 2; row += 2)
		{
			for (int col = 3 - (row & 1); col < _info->width + 2; col += 2)
			{
				if (row < 4 || col < 4)
				{
					sharp = 2;
				}
				else
				{
					val = ABS(pixel[row - 2][col] - pixel[row][col - 2])
						+ ABS(pixel[row - 2][col] - pixel[row - 2][col - 2])
						+ ABS(pixel[row][col - 2] - pixel[row - 2][col - 2]);
					sharp = val < 4 ? 0 : val < 8 ? 1 : val < 16 ? 2 :
						val < 32 ? 3 : val < 48 ? 4 : 5;
				}
				val = ((pixel[row - 2][col] + pixel[row][col - 2]) >> 1)
					+ rstep[sharp][_info->getbits(2)];
				pixel[row][col] = val = LIM(val, 0, 255);
				if (row < 4)
					pixel[row - 2][col + 2] = val;
				if (col < 4)
					pixel[row + 2][col - 2] = val;
			}
		}
	}
	for (int row = 2; row < _info->height + 2; row++)
	{
		for (int col = 3 - (row & 1); col < _info->width + 2; col += 2)
		{
			val = ((pixel[row][col - 1] + (pixel[row][col] << 2) + pixel[row][col + 1]) >> 1) - 0x100;
			pixel[row][col] = LIM(val, 0, 255);
		}
	}
	for (int row = 0; row < _info->height; row++)
		for (int col = 0; col < _info->width; col++)
			RAW(row, col) = curve[pixel[row + 2][col + 2]];
	_info->maximum = 0x3ff;
}

void CImageLoader::redcine_load_raw()
{
	jas_init();
	jas_stream_t* in = jas_stream_fopen(_reader->GetFileName(), "rb");
	if (!in)
		throw CExceptionFile("Jasper open file failed");

	jas_stream_seek(in, _info->data_offset + 20, SEEK_SET);
	jas_image_t* jimg = jas_image_decode(in, -1, 0);
	if (!jimg)
	{
		jas_stream_close(in);

		throw CExceptionFile("Jasper decode file failed");
	}

	jas_matrix_t* jmat = jas_matrix_create(_info->height / 2, _info->width / 2);
	if (!jmat)
	{
		jas_image_destroy(jimg);
		jas_stream_close(in);

		throw CExceptionMemory("redcine_load_raw()");
	}

	unsigned short* img = (unsigned short *)calloc((_info->height + 2), (_info->width + 2) * 2);
	CAutoFreeMemory autoFree(img, false);
	if (!img)
	{
		jas_matrix_destroy(jmat);
		jas_image_destroy(jimg);
		jas_stream_close(in);

		throw CExceptionMemory();
	}

	for (int c = 0; c < 4; c++)
	{
		jas_image_readcmpt(jimg, c, 0, 0, _info->width / 2, _info->height / 2, jmat);
		jas_seqent_t* data = jas_matrix_getref(jmat, 0, 0);
		for (int row = c >> 1; row < _info->height; row += 2)
			for (int col = c & 1; col < _info->width; col += 2)
				img[(row + 1)*(_info->width + 2) + col + 1] = data[(row / 2)*(_info->width / 2) + col / 2];
	}
	for (int col = 1; col <= _info->width; col++)
	{
		img[col] = img[2 * (_info->width + 2) + col];
		img[(_info->height + 1)*(_info->width + 2) + col] = img[(_info->height - 1)*(_info->width + 2) + col];
	}
	for (int row = 0; row < _info->height + 2; row++)
	{
		img[row*(_info->width + 2)] = img[row*(_info->width + 2) + 2];
		img[(row + 1)*(_info->width + 2) - 1] = img[(row + 1)*(_info->width + 2) - 3];
	}
	for (int row = 1; row <= _info->height; row++)
	{
		int col = 1 + (FC(row, 1) & 1);
		unsigned short* pix = img + row*(_info->width + 2) + col;
		for (; col <= _info->width; col += 2, pix += 2)
		{
			int c = (((pix[0] - 0x800) << 3) + pix[-(_info->width + 2)] + pix[_info->width + 2] + pix[-1] + pix[1]) >> 2;
			pix[0] = LIM(c, 0, 4095);
		}
	}
	for (int row = 0; row < _info->height; row++)
		for (int col = 0; col < _info->width; col++)
			RAW(row, col) = _info->curve[img[(row + 1)*(_info->width + 2) + col + 1]];
	jas_matrix_destroy(jmat);
	jas_image_destroy(jimg);
	jas_stream_close(in);
}

void CImageLoader::rollei_load_raw()
{
	unsigned char pixel[10];
	unsigned iten = 0;
	unsigned buffer = 0;
	unsigned todo[16];

	unsigned isix = _info->raw_width * _info->raw_height * 5 / 8;
	while (_reader->Read(pixel, 1, 10) == 10)
	{
		unsigned i;
		for (i = 0; i < 10; i += 2)
		{
			todo[i] = iten++;
			todo[i + 1] = pixel[i] << 8 | pixel[i + 1];
			buffer = pixel[i] >> 2 | buffer << 6;
		}
		for (; i < 16; i += 2)
		{
			todo[i] = isix++;
			todo[i + 1] = buffer >> (14 - i) * 5;
		}
		for (i = 0; i < 16; i += 2)
			_info->raw_image[todo[i]] = (todo[i + 1] & 0x3ff);
	}
	_info->maximum = 0x3ff;
}

void CImageLoader::samsung_load_raw()
{
	int op[4];
	int len[4];

	_reader->SetOrder(0x4949);
	for (int row = 0; row < _info->raw_height; row++)
	{
		_reader->Seek(_info->strip_offset + row * 4, SEEK_SET);
		_reader->Seek(_info->data_offset + _reader->get4(), SEEK_SET);
		ph1_bits(-1);
		for (size_t c = 0; c < 4; c++)
			len[c] = row < 2 ? 7 : 4;
		for (int col = 0; col < _info->raw_width; col += 16)
		{
			int dir = ph1_bits(1);
			for (size_t c = 0; c < 4; c++)
				op[c] = ph1_bits(2);
			for (size_t c = 0; c < 4; c++)
			{
				switch (op[c])
				{
				case 3:
					len[c] = ph1_bits(4);
					break;
				case 2:
					len[c]--;
					break;
				case 1:
					len[c]++;
				}
			}
			for (int c = 0; c < 16; c += 2)
			{
				int i = len[((c & 1) << 1) | (c >> 3)];
				RAW(row, col + c) = ((signed)ph1_bits(i) << (32 - i) >> (32 - i)) +
					(dir ? RAW(row + (~c | -2), col + c) : col ? RAW(row, col + (c | -2)) : 128);
				if (c == 14)
					c = -1;
			}
		}
	}
	for (int row = 0; row < _info->raw_height - 1; row += 2)
		for (int col = 0; col < _info->raw_width - 1; col += 2)
			SWAP(RAW(row, col + 1), RAW(row + 1, col));
}

void CImageLoader::samsung2_load_raw()
{
	static const unsigned short tab[14] =
	{
		0x304,0x307,0x206,0x205,0x403,0x600,0x709,
		0x80a,0x90b,0xa0c,0xa0d,0x501,0x408,0x402
	};
	unsigned short huff[1026];
	unsigned short vpred[2][2] = { { 0,0 },{ 0,0 } };
	unsigned short hpred[2];

	huff[0] = 10;
	int n = 0;
	for (size_t i = 0; i < 14; i++)
		for (int c = 0; c < (1024 >> (tab[i] >> 8)); c++)
			huff[++n] = tab[i];
	_info->getbits(-1);
	for (int row = 0; row < _info->raw_height; row++)
	{
		for (int col = 0; col < _info->raw_width; col++)
		{
			int diff = jhead::ljpeg_diff(huff, *_info);
			if (col < 2)
				hpred[col] = vpred[row & 1][col] += diff;
			else
				hpred[col & 1] += diff;
			RAW(row, col) = hpred[col & 1];
			if (hpred[col & 1] >> _info->tiff_bps)
				throw CExceptionFile();
		}
	}
}

void CImageLoader::samsung3_load_raw()
{
	unsigned short lent[3][2];
	unsigned short len[4];
	unsigned short* prow[2];

	_reader->SetOrder(0x4949);
	_reader->Seek(9, SEEK_CUR);
	int opt = _reader->GetChar();
	int init = (_reader->get2(), _reader->get2());
	for (int row = 0; row < _info->raw_height; row++)
	{
		_reader->Seek((_info->data_offset - _reader->GetPosition()) & 15, SEEK_CUR);
		ph1_bits(-1);
		int mag = 0;
		int pmode = 7;
		for (size_t c = 0; c < 6; c++)
			((unsigned short *)lent)[c] = row < 2 ? 7 : 4;
		prow[row & 1] = &RAW(row - 1, 1 - ((row & 1) << 1));	// green
		prow[~row & 1] = &RAW(row - 2, 0);			// red and blue
		for (int tab = 0; tab + 15 < _info->raw_width; tab += 16)
		{
			if (~opt & 4 && !(tab & 63))
			{
				int i = ph1_bits(2);
				mag = i < 3 ? mag - '2' + "204"[i] : ph1_bits(12);
			}
			if (opt & 2)
				pmode = 7 - 4 * ph1_bits(1);
			else if (!ph1_bits(1))
				pmode = ph1_bits(3);
			if (opt & 1 || !ph1_bits(1))
			{
				for (size_t c = 0; c < 4; c++)
					len[c] = ph1_bits(2);
				for (size_t c = 0; c < 4; c++)
				{
					int i = ((row & 1) << 1 | (c & 1)) % 3;
					len[c] = len[c] < 3 ? lent[i][0] - '1' + "120"[len[c]] : ph1_bits(4);
					lent[i][0] = lent[i][1];
					lent[i][1] = len[c];
				}
			}
			for (int c = 0; c < 16; c++)
			{
				int col = tab + (((c & 7) << 1) ^ (c >> 3) ^ (row & 1));
				int pred = (pmode == 7 || row < 2)
					? (tab ? RAW(row, tab - 2 + (col & 1)) : init)
					: (prow[col & 1][col - '4' + "0224468"[pmode]] + prow[col & 1][col - '4' + "0244668"[pmode]] + 1) >> 1;
				int i = len[c >> 2];
				int diff = ph1_bits(i);
				if (diff >> (i - 1))
					diff -= 1 << i;
				diff = diff * (mag * 2 + 1) + mag;
				RAW(row, col) = pred + diff;
			}
		}
	}
}

void CImageLoader::sinar_4shot_load_raw()
{
	if (_info->raw_image)
	{
		unsigned shot = LIM(_info->shot_select, 1, 4) - 1;
		_reader->Seek(_info->data_offset + shot * 4, SEEK_SET);
		_reader->Seek(_reader->get4(), SEEK_SET);
		unpacked_load_raw();
		return;
	}
	unsigned short* pixel = (unsigned short *)calloc(_info->raw_width, sizeof *pixel);
	CAutoFreeMemory autoFree(pixel);

	for (unsigned shot = 0; shot < 4; shot++)
	{
		_reader->Seek(_info->data_offset + shot * 4, SEEK_SET);
		_reader->Seek(_reader->get4(), SEEK_SET);
		for (unsigned row = 0; row < _info->raw_height; row++)
		{
			_reader->read_shorts(pixel, _info->raw_width);
			unsigned r = row - _info->top_margin - (shot >> 1 & 1);
			if (r >= _info->height)
				continue;
			for (unsigned col = 0; col < _info->raw_width; col++)
			{
				unsigned c = col - _info->left_margin - (shot & 1);
				if (c >= _info->width)
					continue;
				_info->image[r*_info->width + c][(row & 1) * 3 ^ (~col & 1)] = pixel[col];
			}
		}
	}
	_info->mix_green = 1;
}

void CImageLoader::smal_v6_load_raw()
{
	unsigned seg[2][2];

	_reader->Seek(16, SEEK_SET);
	seg[0][0] = 0;
	seg[0][1] = _reader->get2();
	seg[1][0] = _info->raw_width * _info->raw_height;
	seg[1][1] = INT_MAX;
	smal_decode_segment(seg, 0);
}

void CImageLoader::smal_v9_load_raw()
{
	unsigned seg[256][2];

	_reader->Seek(67, SEEK_SET);
	unsigned offset = _reader->get4();
	unsigned nseg = (unsigned char)_reader->GetChar();
	_reader->Seek(offset, SEEK_SET);
	for (size_t i = 0; i < nseg * 2; i++)
		((unsigned *)seg)[i] = _reader->get4() + _info->data_offset*(i & 1);
	_reader->Seek(78, SEEK_SET);
	unsigned holes = _reader->GetChar();
	_reader->Seek(88, SEEK_SET);
	seg[nseg][0] = _info->raw_height * _info->raw_width;
	seg[nseg][1] = _reader->get4() + _info->data_offset;
	for (size_t i = 0; i < nseg; i++)
		smal_decode_segment(seg + i, holes);
	if (holes)
		fill_holes(holes);
}

void CImageLoader::sony_load_raw()
{
	unsigned char head[40];

	_reader->Seek(200896, SEEK_SET);
	_reader->Seek((unsigned)_reader->GetChar() * 4 - 1, SEEK_CUR);
	_reader->SetOrder( 0x4d4d);
	unsigned key = _reader->get4();
	_reader->Seek(164600, SEEK_SET);
	_reader->Read(head, 1, 40);
	sony_decrypt((unsigned *)head, 10, 1, key);
	for (unsigned i = 26; i-- > 22; )
		key = key << 8 | head[i];
	_reader->Seek(_info->data_offset, SEEK_SET);
	for (unsigned row = 0; row < _info->raw_height; row++)
	{
		unsigned short* pixel = _info->raw_image + row*_info->raw_width;
		if (_reader->Read(pixel, 2, _info->raw_width) < _info->raw_width)
			throw CExceptionFile();

		sony_decrypt((unsigned *)pixel, _info->raw_width / 2, !row, key);
		for (unsigned col = 0; col < _info->raw_width; col++)
			if ((pixel[col] = ntohs(pixel[col])) >> 14)
				throw CExceptionFile();
	}
	_info->maximum = 0x3ff0;
}

void CImageLoader::sony_arw_load_raw()
{
	static const unsigned short tab[18] =
	{
		0xf11,0xf10,0xe0f,0xd0e,0xc0d,0xb0c,0xa0b,0x90a,0x809,
		0x708,0x607,0x506,0x405,0x304,0x303,0x300,0x202,0x201
	};

	unsigned short huff[32770];
	huff[0] = 15;
	int n = 0;
	int sum = 0;
	for (int i = 0; i < 18; i++)
		for (size_t c = 0; c < (32768 >> (tab[i] >> 8)); c++)
			huff[++n] = tab[i];
	_info->getbits(-1);
	for (int col = _info->raw_width; col--; )
	{
		for (int row = 0; row < _info->raw_height + 1; row += 2)
		{
			if (row == _info->raw_height) row = 1;
			if ((sum += jhead::ljpeg_diff(huff, *_info)) >> 12)
				throw CExceptionFile();
			if (row < _info->height) RAW(row, col) = sum;
		}
	}
}

void CImageLoader::sony_arw2_load_raw()
{
	unsigned short pix[16];

	unsigned char* data = (unsigned char *)malloc(_info->raw_width + 1);
	CAutoFreeMemory autoFree(data);

	for (int row = 0; row < _info->height; row++)
	{
		_reader->Read(data, 1, _info->raw_width);
		unsigned char* dp = data;
		for (int col = 0; col < _info->raw_width - 30; dp += 16)
		{
			int val = _reader->sget4(dp);
			int max = 0x7ff & val;
			int min = 0x7ff & val >> 11;
			int imax = 0x0f & val >> 22;
			int imin = 0x0f & val >> 26;
			int sh;
			for (sh = 0; sh < 4 && 0x80 << sh <= max - min; sh++);
			int bit = 30;
			for (int i = 0; i < 16; i++)
			{
				if (i == imax)
				{
					pix[i] = max;
				}
				else if (i == imin)
				{
					pix[i] = min;
				}
				else
				{
					pix[i] = ((_reader->sget2(dp + (bit >> 3)) >> (bit & 7) & 0x7f) << sh) + min;
					if (pix[i] > 0x7ff) pix[i] = 0x7ff;
					bit += 7;
				}
				for (i = 0; i < 16; i++, col += 2)
					RAW(row, col) = _info->curve[pix[i] << 1] >> 2;
				col -= col & 1 ? 1 : 31;
			}
		}
	}
}





void CImageLoader::adobe_copy_pixel(unsigned row, unsigned col, unsigned short** rp)
{
	if (_info->tiff_samples == 2 && _info->shot_select)
		(*rp)++;
	if (_info->raw_image)
	{
		if (row < _info->raw_height && col < _info->raw_width)
			RAW(row, col) = _info->curve[**rp];
		*rp += _info->tiff_samples;
	}
	else
	{
		if (row < _info->height && col < _info->width)
			for (size_t c = 0; c < _info->tiff_samples; c++)
				_info->image[row*_info->width + col][c] = _info->curve[(*rp)[c]];
		*rp += _info->tiff_samples;
	}
	if (_info->tiff_samples == 2 && _info->shot_select)
		(*rp)--;
}

void CImageLoader::crop_masked_pixels()
{
	if (_info->load_raw == LoadRawType::phase_one_load_raw ||
		_info->load_raw == LoadRawType::phase_one_load_raw_c)
	{
		phase_one_correct();
	}
	if (_info->fuji_width)
	{
		for (int row = 0; row < _info->raw_height - _info->top_margin * 2; row++)
		{
			for (int col = 0; col < _info->fuji_width << !_info->fuji_layout; col++)
			{
				unsigned r, c;
				if (_info->fuji_layout)
				{
					r = _info->fuji_width - 1 - col + (row >> 1);
					c = col + ((row + 1) >> 1);
				}
				else
				{
					r = _info->fuji_width - 1 + row - (col >> 1);
					c = row + ((col + 1) >> 1);
				}
				if (r < _info->height && c < _info->width)
					BAYER(r, c) = RAW(row + _info->top_margin, col + _info->left_margin);
			}
		}
	}
	else
	{
		for (int row = 0; row < _info->height; row++)
			for (int col = 0; col < _info->width; col++)
				BAYER2(row, col) = RAW(row + _info->top_margin, col + _info->left_margin);
	}
	if (_info->mask[0][3] <= 0)
	{
		if (_info->load_raw == LoadRawType::canon_load_raw ||
			_info->load_raw == LoadRawType::lossless_jpeg_load_raw)
		{
			_info->mask[0][1] = _info->mask[1][1] += 2;
			_info->mask[0][3] -= 2;
		}
		if (_info->load_raw == LoadRawType::canon_load_raw ||
			_info->load_raw == LoadRawType::lossless_jpeg_load_raw ||
			_info->load_raw == LoadRawType::canon_600_load_raw ||
			_info->load_raw == LoadRawType::sony_load_raw ||
			(_info->load_raw == LoadRawType::eight_bit_load_raw && strncmp(_info->model, "DC2", 3)) ||
			_info->load_raw == LoadRawType::kodak_262_load_raw ||
			(_info->load_raw == LoadRawType::packed_load_raw && (_info->load_flags & 32)))
		{
			_info->mask[0][0] = _info->mask[1][0] = _info->top_margin;
			_info->mask[0][2] = _info->mask[1][2] = _info->top_margin + _info->height;
			_info->mask[0][3] += _info->left_margin;
			_info->mask[1][1] += _info->left_margin + _info->width;
			_info->mask[1][3] += _info->raw_width;
		}
		if (_info->load_raw == LoadRawType::nokia_load_raw)
		{
			_info->mask[0][2] = _info->top_margin;
			_info->mask[0][3] = _info->width;
		}
	}

	unsigned mblack[8];
	memset(mblack, 0, sizeof mblack);
	unsigned zero = 0;
	for (size_t m = 0; m < 8; m++)
	{
		for (int row = MAX(_info->mask[m][0], 0); row < MIN(_info->mask[m][2], _info->raw_height); row++)
		{
			for (int col = MAX(_info->mask[m][1], 0); col < MIN(_info->mask[m][3], _info->raw_width); col++)
			{
				unsigned c = FC(row - _info->top_margin, col - _info->left_margin);
				unsigned val = RAW(row, col);
				mblack[c] += val;
				mblack[4 + c]++;
				zero += !val;
			}
		}
	}
	if (_info->load_raw == LoadRawType::canon_600_load_raw && _info->width < _info->raw_width)
	{
		_info->black = (mblack[0] + mblack[1] + mblack[2] + mblack[3]) / (mblack[4] + mblack[5] + mblack[6] + mblack[7]) - 4;
		canon_600_correct();
	}
	else if (zero < mblack[4] && mblack[5] && mblack[6] && mblack[7])
	{
		for (size_t c = 0; c < 4; c++)
			_info->cblack[c] = mblack[c] / mblack[4 + c];
		_info->cblack[4] = _info->cblack[5] = _info->cblack[6] = 0;
	}
}

int CImageLoader::fcol(int row, int col)
{
	static const char filter[16][16] =
	{ { 2,1,1,3,2,3,2,0,3,2,3,0,1,2,1,0 },
	  { 0,3,0,2,0,1,3,1,0,1,1,2,0,3,3,2 },
	  { 2,3,3,2,3,1,1,3,3,1,2,1,2,0,0,3 },
	  { 0,1,0,1,0,2,0,2,2,0,3,0,1,3,2,1 },
	  { 3,1,1,2,0,1,0,2,1,3,1,3,0,1,3,0 },
	  { 2,0,0,3,3,2,3,1,2,0,2,0,3,2,2,1 },
	  { 2,3,3,1,2,1,2,1,2,1,1,2,3,0,0,1 },
	  { 1,0,0,2,3,0,0,3,0,3,0,3,2,1,2,3 },
	  { 2,3,3,1,1,2,1,0,3,2,3,0,2,3,1,3 },
	  { 1,0,2,0,3,0,3,2,0,1,1,2,0,1,0,2 },
	  { 0,1,1,3,3,2,2,1,1,3,3,0,2,1,3,2 },
	  { 2,3,2,0,0,1,3,0,2,0,1,2,3,0,1,0 },
	  { 1,3,1,2,3,2,3,2,0,2,0,1,1,0,3,0 },
	  { 0,2,0,3,1,0,0,1,1,3,3,2,3,2,2,1 },
	  { 2,1,3,2,3,1,2,1,0,3,0,2,0,2,0,2 },
	  { 0,3,1,0,0,2,0,3,2,1,3,1,1,3,1,3 } };

	if (_info->filters == 1)
		return filter[(row + _info->top_margin) & 15][(col + _info->left_margin) & 15];
	if (_info->filters == 9)
		return _info->xtrans[(row + 6) % 6][(col + 6) % 6];
	return FC(row, col);
}

void CImageLoader::phase_one_correct()
{
	int val[4];
	int dev[4];
	int head[9];
	int mindiff = INT_MAX;
	int off_412 = 0;
	static const signed char dir[12][2] = { { -1,-1 },{ -1,1 },{ 1,-1 },{ 1,1 },{ -2,0 },{ 0,-2 },{ 0,2 },{ 2,0 }, { -2,-2 },{ -2,2 },{ 2,-2 },{ 2,2 } };
	float poly[8];
	float mult[2];
	float* yval[2];
	unsigned short *xval[2];
	int qmult_applied = 0;
	int qlin_applied = 0;

	if (_info->half_size || !_info->meta_length)
		return;
	_reader->Seek(_info->meta_offset, SEEK_SET);
	_reader->SetOrder(_reader->get2());
	_reader->Seek(6, SEEK_CUR);
	_reader->Seek(_info->meta_offset + _reader->get4(), SEEK_SET);
	unsigned entries = _reader->get4();
	_reader->get4();
	while (entries--)
	{
		unsigned tag = _reader->get4();
		int len = _reader->get4();
		unsigned data = _reader->get4();
		unsigned save = _reader->GetPosition();
		_reader->Seek(_info->meta_offset + data, SEEK_SET);
		if (tag == 0x419)				/* Polynomial curve */
		{
			//for (_reader->get4(), i = 0; i < 8; i++)
			_reader->get4();
			for (size_t i = 0; i < 8; i++)
				poly[i] = _reader->getreal(11);
			poly[3] += (_info->ph1.tag_210 - poly[7]) * poly[6] + 1;
			for (int i = 0; i < 0x10000; i++)
			{
				float num = (poly[5] * i + poly[3])*i + poly[1];
				_info->curve[i] = LIM(num, 0, 65535);
			}
			/* apply to right half */
			for (unsigned row = 0; row < _info->raw_height; row++)
				for (unsigned col = (tag & 1)*_info->ph1.split_col; col < _info->raw_width; col++)
					RAW(row, col) = _info->curve[RAW(row, col)];
		}
		else if (tag == 0x41a)			/* Polynomial curve */
		{
			for (size_t i = 0; i < 4; i++)
				poly[i] = _reader->getreal(11);
			for (int i = 0; i < 0x10000; i++)
			{
				float num = 0;
				for (int j = 4; j--; )
					num = num * i + poly[j];
				_info->curve[i] = LIM(num + i, 0, 65535);
			}
			/* apply to whole image */
			for (unsigned row = 0; row < _info->raw_height; row++)
				for (unsigned col = (tag & 1)*_info->ph1.split_col; col < _info->raw_width; col++)
					RAW(row, col) = _info->curve[RAW(row, col)];
		}
		else if (tag == 0x400)			/* Sensor defects */
		{
			while ((len -= 8) >= 0)
			{
				unsigned col = _reader->get2();
				unsigned row = _reader->get2();
				unsigned type = _reader->get2(); _reader->get2();
				if (col >= _info->raw_width)
					continue;
				if (type == 131 || type == 137)		/* Bad column */
				{
					for (row = 0; row < _info->raw_height; row++)
					{
						if (FC(row - _info->top_margin, col - _info->left_margin) == 1)
						{
							int sum = 0;
							for (size_t i = 0; i < 4; i++)
								sum += val[i] = raw(row + dir[i][0], col + dir[i][1]);
							int max = 0;
							for (int i = 0; i < 4; i++)
							{
								dev[i] = abs((val[i] << 2) - sum);
								if (dev[max] < dev[i])
									max = i;
							}
							RAW(row, col) = (sum - val[max]) / 3.0 + 0.5;
						}
						else
						{
							int sum = 0;
							for (size_t i = 8; i < 12; i++)
								sum += raw(row + dir[i][0], col + dir[i][1]);
							RAW(row, col) = 0.5 + sum * 0.0732233 +
								(raw(row, col - 2) + raw(row, col + 2)) * 0.3535534;
						}
					}
				}
				else if (type == 129)			/* Bad pixel */
				{
					if (row >= _info->raw_height)
						continue;
					int j = (FC(row - _info->top_margin, col - _info->left_margin) != 1) * 4;
					int sum = 0;
					for (size_t i = j; i < j + 8; i++)
						sum += raw(row + dir[i][0], col + dir[i][1]);
					RAW(row, col) = (sum + 4) >> 3;
				}
			}
		}
		else if (tag == 0x401)			/* All-color flat fields */
		{
			phase_one_flat_field(1, 2);
		}
		else if (tag == 0x416 || tag == 0x410)
		{
			phase_one_flat_field(0, 2);
		}
		else if (tag == 0x40b)			/* Red+blue flat field */
		{
			phase_one_flat_field(0, 4);
		}
		else if (tag == 0x412)
		{
			_reader->Seek(36, SEEK_CUR);
			int diff = abs(_reader->get2() - _info->ph1.tag_21a);
			if (mindiff > diff)
			{
				mindiff = diff;
				off_412 = _reader->GetPosition() - 38;
			}
		}
		else if (tag == 0x41f && !qlin_applied) /* Quadrant linearization */
		{
			unsigned short lc[2][2][16], ref[16];
			int qr, qc;
			for (qr = 0; qr < 2; qr++)
				for (qc = 0; qc < 2; qc++)
					for (size_t i = 0; i < 16; i++)
						lc[qr][qc][i] = _reader->get4();
			for (size_t i = 0; i < 16; i++)
			{
				int v = 0;
				for (qr = 0; qr < 2; qr++)
					for (qc = 0; qc < 2; qc++)
						v += lc[qr][qc][i];
				ref[i] = (v + 2) >> 2;
			}
			for (qr = 0; qr < 2; qr++)
			{
				for (qc = 0; qc < 2; qc++)
				{
					int cx[19], cf[19];
					for (size_t i = 0; i < 16; i++)
					{
						cx[1 + i] = lc[qr][qc][i];
						cf[1 + i] = ref[i];
					}
					cx[0] = cf[0] = 0;
					cx[17] = cf[17] = ((unsigned)ref[15] * 65535) / lc[qr][qc][15];
					cx[18] = cf[18] = 65535;
					cubic_spline(cx, cf, 19);
					for (unsigned row = (qr ? _info->ph1.split_row : 0); row < (qr ? _info->raw_height : _info->ph1.split_row); row++)
						for (unsigned col = (qc ? _info->ph1.split_col : 0); col < (qc ? _info->raw_width : _info->ph1.split_col); col++)
							RAW(row, col) = _info->curve[RAW(row, col)];
				}
			}
			qlin_applied = 1;
		}
		else if (tag == 0x41e && !qmult_applied) /* Quadrant multipliers */
		{
			float qmult[2][2] = { { 1, 1 },{ 1, 1 } };
			_reader->get4();
			_reader->get4();
			_reader->get4();
			_reader->get4();
			qmult[0][0] = 1.0 + _reader->getreal(11);
			_reader->get4();
			_reader->get4();
			_reader->get4();
			_reader->get4();
			_reader->get4();
			qmult[0][1] = 1.0 + _reader->getreal(11);
			_reader->get4();
			_reader->get4();
			_reader->get4();
			qmult[1][0] = 1.0 + _reader->getreal(11);
			_reader->get4();
			_reader->get4();
			_reader->get4();
			qmult[1][1] = 1.0 + _reader->getreal(11);
			for (unsigned row = 0; row < _info->raw_height; row++)
			{
				for (unsigned col = 0; col < _info->raw_width; col++)
				{
					int i = qmult[row >= _info->ph1.split_row][col >= _info->ph1.split_col] * RAW(row, col);
					RAW(row, col) = LIM(i, 0, 65535);
				}
			}
			qmult_applied = 1;
		}
		else if (tag == 0x431 && !qmult_applied) /* Quadrant combined */
		{
			unsigned short lc[2][2][7], ref[7];
			int qr, qc;
			for (size_t i = 0; i < 7; i++)
				ref[i] = _reader->get4();
			for (qr = 0; qr < 2; qr++)
				for (qc = 0; qc < 2; qc++)
					for (size_t i = 0; i < 7; i++)
						lc[qr][qc][i] = _reader->get4();
			for (qr = 0; qr < 2; qr++)
			{
				for (qc = 0; qc < 2; qc++)
				{
					int cx[9], cf[9];
					for (size_t i = 0; i < 7; i++)
					{
						cx[1 + i] = ref[i];
						cf[1 + i] = ((unsigned)ref[i] * lc[qr][qc][i]) / 10000;
					}
					cx[0] = cf[0] = 0;
					cx[8] = cf[8] = 65535;
					cubic_spline(cx, cf, 9);
					for (unsigned row = (qr ? _info->ph1.split_row : 0); row < (qr ? _info->raw_height : _info->ph1.split_row); row++)
						for (unsigned col = (qc ? _info->ph1.split_col : 0); col < (qc ? _info->raw_width : _info->ph1.split_col); col++)
							RAW(row, col) = _info->curve[RAW(row, col)];
				}
			}
			qmult_applied = 1;
			qlin_applied = 1;
		}
		_reader->Seek(save, SEEK_SET);
	}
	if (off_412)
	{
		_reader->Seek(off_412, SEEK_SET);
		for (size_t i = 0; i < 9; i++)
			head[i] = _reader->get4() & 0x7fff;
		yval[0] = (float *)calloc(head[1] * head[3] + head[2] * head[4], 6);
		CAutoFreeMemory autoFree(yval[0]);

		yval[1] = (float  *)(yval[0] + head[1] * head[3]);
		xval[0] = (unsigned short *)(yval[1] + head[2] * head[4]);
		xval[1] = (unsigned short *)(xval[0] + head[1] * head[3]);
		_reader->get2();
		for (size_t i = 0; i < 2; i++)
			for (size_t j = 0; j < head[i + 1] * head[i + 3]; j++)
				yval[i][j] = _reader->getreal(11);
		for (size_t i = 0; i < 2; i++)
			for (size_t j = 0; j < head[i + 1] * head[i + 3]; j++)
				xval[i][j] = _reader->get2();
		for (unsigned row = 0; row < _info->raw_height; row++)
		{
			for (unsigned col = 0; col < _info->raw_width; col++)
			{
				float cfrac = (float)col * head[3] / _info->raw_width;
				int cip = cfrac;
				cfrac -= cip;
				float num = RAW(row, col) * 0.5;
				for (size_t i = cip; i < cip + 2; i++)
				{
					int k = 0;
					int j;
					for (j = 0; j < head[1]; j++)
						if (num < xval[0][k = head[1] * i + j])
							break;
					float frac = (j == 0 || j == head[1]) ? 0 : (xval[0][k] - num) / (xval[0][k] - xval[0][k - 1]);
					mult[i - cip] = yval[0][k - 1] * frac + yval[0][k] * (1 - frac);
				}
				int c = ((mult[0] * (1 - cfrac) + mult[1] * cfrac) * row + num) * 2;
				RAW(row, col) = LIM(c, 0, 65535);
			}
		}
	}
}

void CImageLoader::canon_600_correct()
{
	static const short mul[4][2] = { { 1141,1145 },{ 1128,1109 },{ 1178,1149 },{ 1128,1109 } };

	for (int row = 0; row < _info->height; row++)
	{
		for (int col = 0; col < _info->width; col++)
		{
			int val = BAYER(row, col) - _info->black;
			if (val < 0)
				val = 0;
			val = val * mul[row & 3][col & 1] >> 9;
			BAYER(row, col) = val;
		}
	}
	canon_600_fixed_wb(1311);
	canon_600_auto_wb();
	canon_600_coeff();
	_info->maximum = (0x3ff - _info->black) * 1109 >> 9;
	_info->black = 0;
}

void CImageLoader::canon_600_fixed_wb(int temp)
{
	static const short mul[4][5] =
	{
		{ 667, 358,397,565,452 },
		{ 731, 390,367,499,517 },
		{ 1119, 396,348,448,537 },
		{ 1399, 485,431,508,688 }
	};
	float frac = 0;

	int lo;
	for (lo = 4; --lo; )
		if (*mul[lo] <= temp) break;
	int hi;
	for (hi = 0; hi < 3; hi++)
		if (*mul[hi] >= temp) break;
	if (lo != hi)
		frac = (float)(temp - *mul[lo]) / (*mul[hi] - *mul[lo]);
	for (size_t i = 1; i < 5; i++)
		_info->pre_mul[i - 1] = 1 / (frac * mul[hi][i] + (1 - frac) * mul[lo][i]);
}

void CImageLoader::canon_600_auto_wb()
{
	int count[] = { 0,0 };
	int test[8], total[2][8], ratio[2][2], stat[2];

	memset(&total, 0, sizeof total);
	int i = _info->canon_ev + 0.5;
	int mar;
	if (i < 10)
		mar = 150;
	else if (i > 12)
		mar = 20;
	else
		mar = 280 - 20 * i;
	if (_info->flash_used)
		mar = 80;
	for (int row = 14; row < _info->height - 14; row += 4)
	{
		for (int col = 10; col < _info->width; col += 2)
		{
			for (i = 0; i < 8; i++)
				test[(i & 4) + FC(row + (i >> 1), col + (i & 1))] = BAYER(row + (i >> 1), col + (i & 1));
			for (i = 0; i < 8; i++)
				if (test[i] < 150 || test[i] > 1500)
					goto next;
			for (i = 0; i < 4; i++)
				if (abs(test[i] - test[i + 4]) > 50)
					goto next;
			for (i = 0; i < 2; i++) {
				for (int j = 0; j < 4; j += 2)
					ratio[i][j >> 1] = ((test[i * 4 + j + 1] - test[i * 4 + j]) << 10) / test[i * 4 + j];
				stat[i] = canon_600_color(ratio[i], mar);
			}
			int st = stat[0] | stat[1];
			if (st > 1)
				goto next;
			for (i = 0; i < 2; i++)
				if (stat[i])
					for (int j = 0; j < 2; j++)
						test[i * 4 + j * 2 + 1] = test[i * 4 + j * 2] * (0x400 + ratio[i][j]) >> 10;
			for (i = 0; i < 8; i++)
				total[st][i] += test[i];
			count[st]++;
		next:;
		}
	}
	if (count[0] | count[1])
	{
		int st = count[0] * 200 < count[1];
		for (i = 0; i < 4; i++)
			_info->pre_mul[i] = 1.0 / (total[st][i] + total[st][i + 4]);
	}
}

void CImageLoader::canon_600_coeff()
{
	static const short table[6][12] =
	{
		{ -190,702,-1878,2390,   1861,-1349,905,-393, -432,944,2617,-2105 },
		{ -1203,1715,-1136,1648, 1388,-876,267,245,  -1641,2153,3921,-3409 },
		{ -615,1127,-1563,2075,  1437,-925,509,3,     -756,1268,2519,-2007 },
		{ -190,702,-1886,2398,   2153,-1641,763,-251, -452,964,3040,-2528 },
		{ -190,702,-1878,2390,   1861,-1349,905,-393, -432,944,2617,-2105 },
		{ -807,1319,-1785,2297,  1388,-876,769,-257,  -230,742,2067,-1555 }
	};

	int t = 0;
	float mc = _info->pre_mul[1] / _info->pre_mul[2];
	float yc = _info->pre_mul[3] / _info->pre_mul[2];
	if (mc > 1 && mc <= 1.28 && yc < 0.8789)
		t = 1;
	if (mc > 1.28 && mc <= 2)
	{
		if (yc < 0.8789)
			t = 3;
		else if (yc <= 2)
			t = 4;
	}
	if (_info->flash_used)
		t = 5;
	_info->raw_color = 0;
	for (size_t i = 0; i < 3; i++)
		for (size_t c = 0; c < _info->colors; c++)
			_info->rgb_cam[i][c] = table[t][i * 4 + c] / 1024.0;
}

/* Return values:  0 = white  1 = near white  2 = not white */
int CImageLoader::canon_600_color(int ratio[2], int mar)
{
	int clipped = 0;

	if (_info->flash_used)
	{
		if (ratio[1] < -104)
			ratio[1] = -104; clipped = 1;
		if (ratio[1] >   12)
			ratio[1] = 12; clipped = 1;
	}
	else
	{
		if (ratio[1] < -264 || ratio[1] > 461)
			return 2;
		if (ratio[1] < -50)
			ratio[1] = -50; clipped = 1;
		if (ratio[1] > 307)
			ratio[1] = 307; clipped = 1;
	}
	int target = _info->flash_used || ratio[1] < 197
		? -38 - (398 * ratio[1] >> 10)
		: -123 + (48 * ratio[1] >> 10);
	if (target - mar <= ratio[0] && target + 20 >= ratio[0] && !clipped)
		return 0;
	int miss = target - ratio[0];
	if (abs(miss) >= mar * 4)
		return 2;
	if (miss < -20)
		miss = -20;
	if (miss > mar)
		miss = mar;
	ratio[0] = target - miss;
	return 1;
}

inline int CImageLoader::raw(unsigned row, unsigned col)
{
	return (row < _info->raw_height && col < _info->raw_width) ? RAW(row, col) : 0;
}

void CImageLoader::phase_one_flat_field(int is_float, int nc)
{
	unsigned short head[8];
	float mult[4];

	_reader->read_shorts(head, 8);
	if (head[2] * head[3] * head[4] * head[5] == 0)
		return;
	unsigned wide = head[2] / head[4] + (head[2] % head[4] != 0);
	unsigned high = head[3] / head[5] + (head[3] % head[5] != 0);
	float* mrow = (float *)calloc(nc*wide, sizeof *mrow);
	CAutoFreeMemory autoFree(mrow);

	for (unsigned y = 0; y < high; y++)
	{
		for (unsigned x = 0; x < wide; x++)
		{
			for (size_t c = 0; c < nc; c += 2)
			{
				float num = is_float ? _reader->getreal(11) : _reader->get2() / 32768.0;
				if (y == 0) mrow[c*wide + x] = num;
				else mrow[(c + 1)*wide + x] = (num - mrow[c*wide + x]) / head[5];
			}
		}
		if (y == 0)
			continue;
		unsigned rend = head[1] + y*head[5];
		for (unsigned row = rend - head[5]; row < _info->raw_height && row < rend && row < head[1] + head[3] - head[5]; row++)
		{
			for (unsigned x = 1; x < wide; x++)
			{
				for (size_t c = 0; c < nc; c += 2)
				{
					mult[c] = mrow[c*wide + x - 1];
					mult[c + 1] = (mrow[c*wide + x] - mult[c]) / head[4];
				}
				unsigned cend = head[0] + x*head[4];
				for (unsigned col = cend - head[4]; col < _info->raw_width && col < cend && col < head[0] + head[2] - head[4]; col++)
				{
					unsigned c = nc > 2 ? FC(row - _info->top_margin, col - _info->left_margin) : 0;
					if (!(c & 1))
					{
						c = RAW(row, col) * mult[c];
						RAW(row, col) = LIM(c, 0, 65535);
					}
					for (size_t c = 0; c < nc; c += 2)
						mult[c] += mult[c + 1];
				}
			}
			for (unsigned x = 0; x < wide; x++)
				for (size_t c = 0; c < nc; c += 2)
					mrow[c*wide + x] += mrow[(c + 1)*wide + x];
		}
	}
}

void CImageLoader::cubic_spline(const int* x_, const int* y_, const int len)
{
	float** A = (float **)calloc(((2 * len + 4) * sizeof **A + sizeof *A), 2 * len);
	if (!A)
		return;
	CAutoFreeMemory autoFree(A);

	A[0] = (float *)(A + 2 * len);
	int i;
	for (i = 1; i < 2 * len; i++)
		A[i] = A[0] + 2 * len*i;
	float* b = A[0] + i*i;
	float* c = i + b;
	float* d = i + c;
	float* x = i + d;
	float* y = len + x;
	for (i = 0; i < len; i++)
	{
		x[i] = x_[i] / 65535.0;
		y[i] = y_[i] / 65535.0;
	}
	for (i = len - 1; i > 0; i--)
	{
		b[i] = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
		d[i - 1] = x[i] - x[i - 1];
	}
	for (i = 1; i < len - 1; i++)
	{
		A[i][i] = 2 * (d[i - 1] + d[i]);
		if (i > 1)
		{
			A[i][i - 1] = d[i - 1];
			A[i - 1][i] = d[i - 1];
		}
		A[i][len - 1] = 6 * (b[i + 1] - b[i]);
	}
	for (i = 1; i < len - 2; i++)
	{
		float v = A[i + 1][i] / A[i][i];
		for (int j = 1; j <= len - 1; j++)
			A[i + 1][j] -= v * A[i][j];
	}
	for (i = len - 2; i > 0; i--)
	{
		float acc = 0;
		for (int j = i; j <= len - 2; j++)
			acc += A[i][j] * c[j];
		c[i] = (A[i][len - 1] - acc) / A[i][i];
	}
	for (i = 0; i < 0x10000; i++)
	{
		float x_out = (float)(i / 65535.0);
		float y_out = 0;
		for (int j = 0; j < len - 1; j++)
		{
			if (x[j] <= x_out && x_out <= x[j + 1])
			{
				float v = x_out - x[j];
				y_out = y[j] +
					((y[j + 1] - y[j]) / d[j] - (2 * d[j] * c[j] + c[j + 1] * d[j]) / 6) * v
					+ (c[j] * 0.5) * v*v + ((c[j + 1] - c[j]) / (6 * d[j])) * v*v*v;
			}
		}
		_info->curve[i] = y_out < 0.0 ? 0 : (y_out >= 1.0 ? 65535 :
			(unsigned short)(y_out * 65535.0 + 0.5));
	}
}

void CImageLoader::crw_init_tables(unsigned table, unsigned short* huff[2])
{
	static const unsigned char first_tree[3][29] =
	{
		{ 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
		0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff },
		{ 0,2,2,3,1,1,1,1,2,0,0,0,0,0,0,0,
		0x03,0x02,0x04,0x01,0x05,0x00,0x06,0x07,0x09,0x08,0x0a,0x0b,0xff },
		{ 0,0,6,3,1,1,2,0,0,0,0,0,0,0,0,0,
		0x06,0x05,0x07,0x04,0x08,0x03,0x09,0x02,0x00,0x0a,0x01,0x0b,0xff },
	};
	static const unsigned char second_tree[3][180] =
	{
		{ 0,2,2,2,1,4,2,1,2,5,1,1,0,0,0,139,
		0x03,0x04,0x02,0x05,0x01,0x06,0x07,0x08,
		0x12,0x13,0x11,0x14,0x09,0x15,0x22,0x00,0x21,0x16,0x0a,0xf0,
		0x23,0x17,0x24,0x31,0x32,0x18,0x19,0x33,0x25,0x41,0x34,0x42,
		0x35,0x51,0x36,0x37,0x38,0x29,0x79,0x26,0x1a,0x39,0x56,0x57,
		0x28,0x27,0x52,0x55,0x58,0x43,0x76,0x59,0x77,0x54,0x61,0xf9,
		0x71,0x78,0x75,0x96,0x97,0x49,0xb7,0x53,0xd7,0x74,0xb6,0x98,
		0x47,0x48,0x95,0x69,0x99,0x91,0xfa,0xb8,0x68,0xb5,0xb9,0xd6,
		0xf7,0xd8,0x67,0x46,0x45,0x94,0x89,0xf8,0x81,0xd5,0xf6,0xb4,
		0x88,0xb1,0x2a,0x44,0x72,0xd9,0x87,0x66,0xd4,0xf5,0x3a,0xa7,
		0x73,0xa9,0xa8,0x86,0x62,0xc7,0x65,0xc8,0xc9,0xa1,0xf4,0xd1,
		0xe9,0x5a,0x92,0x85,0xa6,0xe7,0x93,0xe8,0xc1,0xc6,0x7a,0x64,
		0xe1,0x4a,0x6a,0xe6,0xb3,0xf1,0xd3,0xa5,0x8a,0xb2,0x9a,0xba,
		0x84,0xa4,0x63,0xe5,0xc5,0xf3,0xd2,0xc4,0x82,0xaa,0xda,0xe4,
		0xf2,0xca,0x83,0xa3,0xa2,0xc3,0xea,0xc2,0xe2,0xe3,0xff,0xff },
		{ 0,2,2,1,4,1,4,1,3,3,1,0,0,0,0,140,
		0x02,0x03,0x01,0x04,0x05,0x12,0x11,0x06,
		0x13,0x07,0x08,0x14,0x22,0x09,0x21,0x00,0x23,0x15,0x31,0x32,
		0x0a,0x16,0xf0,0x24,0x33,0x41,0x42,0x19,0x17,0x25,0x18,0x51,
		0x34,0x43,0x52,0x29,0x35,0x61,0x39,0x71,0x62,0x36,0x53,0x26,
		0x38,0x1a,0x37,0x81,0x27,0x91,0x79,0x55,0x45,0x28,0x72,0x59,
		0xa1,0xb1,0x44,0x69,0x54,0x58,0xd1,0xfa,0x57,0xe1,0xf1,0xb9,
		0x49,0x47,0x63,0x6a,0xf9,0x56,0x46,0xa8,0x2a,0x4a,0x78,0x99,
		0x3a,0x75,0x74,0x86,0x65,0xc1,0x76,0xb6,0x96,0xd6,0x89,0x85,
		0xc9,0xf5,0x95,0xb4,0xc7,0xf7,0x8a,0x97,0xb8,0x73,0xb7,0xd8,
		0xd9,0x87,0xa7,0x7a,0x48,0x82,0x84,0xea,0xf4,0xa6,0xc5,0x5a,
		0x94,0xa4,0xc6,0x92,0xc3,0x68,0xb5,0xc8,0xe4,0xe5,0xe6,0xe9,
		0xa2,0xa3,0xe3,0xc2,0x66,0x67,0x93,0xaa,0xd4,0xd5,0xe7,0xf8,
		0x88,0x9a,0xd7,0x77,0xc4,0x64,0xe2,0x98,0xa5,0xca,0xda,0xe8,
		0xf3,0xf6,0xa9,0xb2,0xb3,0xf2,0xd2,0x83,0xba,0xd3,0xff,0xff },
		{ 0,0,6,2,1,3,3,2,5,1,2,2,8,10,0,117,
		0x04,0x05,0x03,0x06,0x02,0x07,0x01,0x08,
		0x09,0x12,0x13,0x14,0x11,0x15,0x0a,0x16,0x17,0xf0,0x00,0x22,
		0x21,0x18,0x23,0x19,0x24,0x32,0x31,0x25,0x33,0x38,0x37,0x34,
		0x35,0x36,0x39,0x79,0x57,0x58,0x59,0x28,0x56,0x78,0x27,0x41,
		0x29,0x77,0x26,0x42,0x76,0x99,0x1a,0x55,0x98,0x97,0xf9,0x48,
		0x54,0x96,0x89,0x47,0xb7,0x49,0xfa,0x75,0x68,0xb6,0x67,0x69,
		0xb9,0xb8,0xd8,0x52,0xd7,0x88,0xb5,0x74,0x51,0x46,0xd9,0xf8,
		0x3a,0xd6,0x87,0x45,0x7a,0x95,0xd5,0xf6,0x86,0xb4,0xa9,0x94,
		0x53,0x2a,0xa8,0x43,0xf5,0xf7,0xd4,0x66,0xa7,0x5a,0x44,0x8a,
		0xc9,0xe8,0xc8,0xe7,0x9a,0x6a,0x73,0x4a,0x61,0xc7,0xf4,0xc6,
		0x65,0xe9,0x72,0xe6,0x71,0x91,0x93,0xa6,0xda,0x92,0x85,0x62,
		0xf3,0xc5,0xb2,0xa4,0x84,0xba,0x64,0xa5,0xb3,0xd2,0x81,0xe5,
		0xd3,0xaa,0xc4,0xca,0xf2,0xb1,0xe4,0xd1,0x83,0x63,0xea,0xc3,
		0xe2,0x82,0xf1,0xa3,0xc2,0xa1,0xc1,0xe3,0xa2,0xe1,0xff,0xff }
	};

	if (table > 2)
		table = 2;
	try
	{
		huff[0] = CDecoder::make_decoder(first_tree[table]);
		huff[1] = CDecoder::make_decoder(second_tree[table]);
	}
	catch (const CExceptionMemory&)
	{
		free(huff[0]);
		free(huff[1]);

		throw;
	}
}

/*
Return 0 if the image starts with compressed data,
1 if it starts with uncompressed low-order bits.

In Canon compressed data, 0xff is always followed by 0x00.
*/
int CImageLoader::canon_has_lowbits()
{
	unsigned char test[0x4000];
	int ret = 1;

	_reader->Seek(0, SEEK_SET);
	_reader->Read(test, 1, sizeof test);
	for (size_t i = 540; i < sizeof test - 1; i++)
	{
		if (test[i] == 0xff)
		{
			if (test[i + 1])
				return 1;
			ret = 0;
		}
	}
	return ret;
}

unsigned CImageLoader::ph1_bithuff(int nbits, unsigned short* huff)
{
	static UINT64 bitbuf = 0;
	static int vbits = 0;

	if (nbits == -1)
		return bitbuf = vbits = 0;
	if (nbits == 0)
		return 0;
	if (vbits < nbits)
	{
		bitbuf = bitbuf << 32 | _reader->get4();
		vbits += 32;
	}
	unsigned c = bitbuf << (64 - vbits) >> (64 - nbits);
	if (huff)
	{
		vbits -= huff[c] >> 8;
		return (unsigned char)huff[c];
	}
	vbits -= nbits;
	return c;
}

int CImageLoader::kodak_65000_decode(short *out, int bsize)
{
	unsigned char blen[768];
	unsigned short raw[6];

	int save = _reader->GetPosition();
	bsize = (bsize + 3) & -4;
	for (int i = 0; i < bsize; i += 2)
	{
		unsigned char c = _reader->GetChar();
		if ((blen[i] = c & 15) > 12 || (blen[i + 1] = c >> 4) > 12)
		{
			_reader->Seek(save, SEEK_SET);
			for (i = 0; i < bsize; i += 8)
			{
				_reader->read_shorts(raw, 6);
				out[i] = raw[0] >> 12 << 8 | raw[2] >> 12 << 4 | raw[4] >> 12;
				out[i + 1] = raw[1] >> 12 << 8 | raw[3] >> 12 << 4 | raw[5] >> 12;
				for (int j = 0; j < 6; j++)
					out[i + 2 + j] = raw[j] & 0xfff;
			}
			return 1;
		}
	}

	INT64 bitbuf = 0;
	int bits = 0;
	if ((bsize & 7) == 4)
	{
		bitbuf = _reader->GetChar() << 8;
		bitbuf += _reader->GetChar();
		bits = 16;
	}
	for (int i = 0; i < bsize; i++)
	{
		int len = blen[i];
		if (bits < len)
		{
			for (int j = 0; j < 32; j += 8)
				bitbuf += (INT64)_reader->GetChar() << (bits + (j ^ 8));
			bits += 32;
		}
		int diff = bitbuf & (0xffff >> (16 - len));
		bitbuf >>= len;
		bits -= len;
		if ((diff & (1 << (len - 1))) == 0)
			diff -= (1 << len) - 1;
		out[i] = diff;
	}
	return 0;
}

unsigned CImageLoader::pana_bits(int nbits)
{
	static unsigned char buf[0x4000];
	static int vbits;

	if (!nbits)
		return vbits = 0;
	if (!vbits)
	{
		_reader->Read(buf + _info->load_flags, 1, 0x4000 - _info->load_flags);
		_reader->Read(buf, 1, _info->load_flags);
	}
	vbits = (vbits - nbits) & 0x1ffff;
	int byte = vbits >> 3 ^ 0x3ff0;
	return (buf[byte] | buf[byte + 1] << 8) >> (vbits & 7) & ~(-1 << nbits);
}

/* Kudos to Rich Taylor for figuring out SMaL's compression algorithm. */
void CImageLoader::smal_decode_segment(unsigned seg[2][2], int holes)
{
	unsigned char hist[3][13] =
	{
		{ 7, 7, 0, 0, 63, 55, 47, 39, 31, 23, 15, 7, 0 },
		{ 7, 7, 0, 0, 63, 55, 47, 39, 31, 23, 15, 7, 0 },
		{ 3, 3, 0, 0, 63,     47,     31,     15,    0 }
	};
	int high = 0xff;
	int carry = 0;
	int nbits = 8;
	int sym[3];
	unsigned char pred[] = { 0,0 };
	unsigned short data = 0;
	unsigned short range = 0;
	int bin;

	_reader->Seek(seg[0][1] + 1, SEEK_SET);
	_info->getbits(-1);
	if (seg[1][0] > _info->raw_width*_info->raw_height)
		seg[1][0] = _info->raw_width*_info->raw_height;
	for (int pix = seg[0][0]; pix < seg[1][0]; pix++)
	{
		for (int s = 0; s < 3; s++)
		{
			data = data << nbits | _info->getbits(nbits);
			if (carry < 0)
				carry = (nbits += carry + 1) < 1 ? nbits - 1 : 0;
			while (--nbits >= 0)
				if ((data >> nbits & 0xff) == 0xff)
					break;
			if (nbits > 0)
				data = ((data & ((1 << (nbits - 1)) - 1)) << 1) |
				((data + (((data & (1 << (nbits - 1)))) << 1)) & (-1 << nbits));
			if (nbits >= 0)
			{
				data += _info->getbits(1);
				carry = nbits - 8;
			}
			int count = ((((data - range + 1) & 0xffff) << 2) - 1) / (high >> 4);
			for (bin = 0; hist[s][bin + 5] > count; bin++);
			int low = hist[s][bin + 5] * (high >> 4) >> 2;
			if (bin)
				high = hist[s][bin + 4] * (high >> 4) >> 2;
			high -= low;
			for (nbits = 0; high << nbits < 128; nbits++);
			range = (range + low) << nbits;
			high <<= nbits;
			int next = hist[s][1];
			if (++hist[s][2] > hist[s][3])
			{
				next = (next + 1) & hist[s][0];
				hist[s][3] = (hist[s][next + 4] - hist[s][next + 5]) >> 2;
				hist[s][2] = 1;
			}
			if (hist[s][hist[s][1] + 4] - hist[s][hist[s][1] + 5] > 1)
			{
				if (bin < hist[s][1])
					for (size_t i = bin; i < hist[s][1]; i++)
						hist[s][i + 5]--;
				else if (next <= bin)
					for (size_t i = hist[s][1]; i < bin; i++)
						hist[s][i + 5]++;
			}
			hist[s][1] = next;
			sym[s] = bin;
		}
		unsigned char diff = sym[2] << 5 | sym[1] << 2 | (sym[0] & 3);
		if (sym[0] & 4)
			diff = diff ? -diff : 0x80;
		if (_reader->GetPosition() + 12 >= seg[1][1])
			diff = 0;
		_info->raw_image[pix] = pred[pix & 1] += diff;
		if (!(pix & 1) && HOLE(pix / _info->raw_width))
			pix += 2;
	}
	_info->maximum = 0xff;
}

void CImageLoader::fill_holes(int holes)
{
	int val[4];

	for (int row = 2; row < _info->height - 2; row++)
	{
		if (!HOLE(row))
			continue;
		for (int col = 1; col < _info->width - 1; col += 4)
		{
			val[0] = RAW(row - 1, col - 1);
			val[1] = RAW(row - 1, col + 1);
			val[2] = RAW(row + 1, col - 1);
			val[3] = RAW(row + 1, col + 1);
			RAW(row, col) = median4(val);
		}
		for (int col = 2; col < _info->width - 2; col += 4)
		{
			if (HOLE(row - 2) || HOLE(row + 2))
			{
				RAW(row, col) = (RAW(row, col - 2) + RAW(row, col + 2)) >> 1;
			}
			else
			{
				val[0] = RAW(row, col - 2);
				val[1] = RAW(row, col + 2);
				val[2] = RAW(row - 2, col);
				val[3] = RAW(row + 2, col);
				RAW(row, col) = median4(val);
			}
		}
	}
}

int CImageLoader::median4(int *p)
{
	int sum = p[0];
	int max = sum;
	int min = max;
	for (int i = 1; i < 4; i++)
	{
		sum += p[i];
		if (min > p[i]) min = p[i];
		if (max < p[i]) max = p[i];
	}
	return (sum - min - max) >> 1;
}

void CImageLoader::sony_decrypt(unsigned *data, int len, int start, int key)
{
	static unsigned pad[128];
	static unsigned p;

	if (start)
	{
		for (p = 0; p < 4; p++)
			pad[p] = key = key * 48828125 + 1;
		pad[3] = pad[3] << 1 | (pad[0] ^ pad[2]) >> 31;
		for (p = 4; p < 127; p++)
			pad[p] = (pad[p - 4] ^ pad[p - 2]) << 1 | (pad[p - 3] ^ pad[p - 1]) >> 31;
		for (p = 0; p < 127; p++)
			pad[p] = htonl(pad[p]);
	}
	while (len-- && p++)
		*data++ ^= pad[(p - 1) & 127] = pad[p & 127] ^ pad[(p + 64) & 127];
}

void CImageLoader::remove_zeroes()
{
	for (unsigned row = 0; row < _info->height; row++)
	{
		for (unsigned col = 0; col < _info->width; col++)
		{
			if (BAYER(row, col) == 0)
			{
				unsigned tot = 0;
				unsigned n = 0;
				for (unsigned r = row - 2; r <= row + 2; r++)
				{
					for (unsigned c = col - 2; c <= col + 2; c++)
					{
						if (r < _info->height && c < _info->width &&
							FC(r, c) == FC(row, col) && BAYER(r, c))
						{
							tot += (n++, BAYER(r, c));
						}
					}
				}
				if (n)
					BAYER(row, col) = tot / n;
			}
		}
	}
}

/*
Seach from the current directory up to the root looking for
a ".badpixels" file, and fix those pixels now.
*/
void CImageLoader::bad_pixels(const char *cfname)
{
	// TODO: manage file
	FILE* fp = 0;
	int fixed = 0;

	if (!_info->filters)
		return;

	if (cfname)
	{
		fp = fopen(cfname, "r");
	}
	else
	{
		char* fname = nullptr;
		CAutoFreeMemory(fname, false);

		int len;
		for (len = 32; ; len *= 2)
		{
			fname = (char *)malloc(len);
			if (!fname)
				return;
			if (_getcwd(fname, len - 16))
				break;
			if (errno != ERANGE)
				return;
		}
		if (fname[1] == ':')
			memmove(fname, fname + 2, len - 2);
		char* cp;
		for (cp = fname; *cp; cp++)
			if (*cp == '\\')
				*cp = '/';
		cp = fname + strlen(fname);
		if (cp[-1] == '/')
			cp--;
		while (*fname == '/')
		{
			strcpy(cp, "/.badpixels");
			if ((fp = fopen(fname, "r")))
				break;
			if (cp == fname)
				break;
			while (*--cp != '/');
		}
	}
	if (!fp)
		return;

	char line[128];
	while (fgets(line, 128, fp))
	{
		char* cp = strchr(line, '#');
		if (cp)
			*cp = 0;
		int time;
		int row;
		int col;
		if (sscanf(line, "%d %d %d", &col, &row, &time) != 3)
			continue;
		if ((unsigned)col >= _info->width || (unsigned)row >= _info->height)
			continue;
		if (time > _info->timestamp)
			continue;
		int n = 0;
		int tot = 0;
		for (int rad = 1; rad < 3 && n == 0; rad++)
		{
			for (int r = row - rad; r <= row + rad; r++)
			{
				for (int c = col - rad; c <= col + rad; c++)
				{
					if ((unsigned)r < _info->height && (unsigned)c < _info->width &&
						(r != row || c != col) && fcol(r, c) == fcol(row, col))
					{
						tot += BAYER2(r, c);
						n++;
					}
				}
			}
		}
		BAYER2(row, col) = tot / n;
	}
	if (fixed)
		fputc('\n', stderr);
	fclose(fp);
}

void CImageLoader::subtract(const char *fname)
{
	// TODO: manage file
	FILE *fp;
	int dim[3] = { 0,0,0 };

	if (!(fp = fopen(fname, "rb")))
	{
		perror(fname);
		return;
	}

	int error = 0;
	if (fgetc(fp) != 'P' || fgetc(fp) != '5')
		error = 1;

	int  c;
	int  nd = 0;
	int comment = 0;
	int number = 0;
	while (!error && nd < 3 && (c = fgetc(fp)) != EOF)
	{
		if (c == '#')
			comment = 1;
		if (c == '\n')
			comment = 0;
		if (comment)
			continue;
		if (isdigit(c))
			number = 1;
		if (number)
		{
			if (isdigit(c))
			{
				dim[nd] = dim[nd] * 10 + c - '0';
			}
			else if (isspace(c))
			{
				number = 0;  nd++;
			}
			else
			{
				error = 1;
			}
		}
	}
	if (error || nd < 3)
	{
		fprintf(stderr, ("%s is not a valid PGM file!\n"), fname);
		fclose(fp);  return;
	}
	else if (dim[0] != _info->width || dim[1] != _info->height || dim[2] != 65535)
	{
		fprintf(stderr, ("%s has the wrong dimensions!\n"), fname);
		fclose(fp);  return;
	}
	unsigned short* pixel = (unsigned short *)calloc(_info->width, sizeof *pixel);
	CAutoFreeMemory autoFree(pixel);
	for (int row = 0; row < _info->height; row++)
	{
		fread(pixel, 2, _info->width, fp);
		for (int col = 0; col < _info->width; col++)
			BAYER(row, col) = MAX(BAYER(row, col) - ntohs(pixel[col]), 0);
	}
	fclose(fp);
	memset(_info->cblack, 0, sizeof _info->cblack);
	_info->black = 0;
}

void CImageLoader::colorcheck()
{
#ifdef COLORCHECK
#define NSQ 24
	// Coordinates of the GretagMacbeth ColorChecker squares
	// width, height, 1st_column, 1st_row
	int cut[NSQ][4];			// you must set these
								// ColorChecker Chart under 6500-kelvin illumination
	static const double gmb_xyY[NSQ][3] =
	{
		{ 0.400, 0.350, 10.1 },		// Dark Skin
		{ 0.377, 0.345, 35.8 },		// Light Skin
		{ 0.247, 0.251, 19.3 },		// Blue Sky
		{ 0.337, 0.422, 13.3 },		// Foliage
		{ 0.265, 0.240, 24.3 },		// Blue Flower
		{ 0.261, 0.343, 43.1 },		// Bluish Green
		{ 0.506, 0.407, 30.1 },		// Orange
		{ 0.211, 0.175, 12.0 },		// Purplish Blue
		{ 0.453, 0.306, 19.8 },		// Moderate Red
		{ 0.285, 0.202, 6.6 },		// Purple
		{ 0.380, 0.489, 44.3 },		// Yellow Green
		{ 0.473, 0.438, 43.1 },		// Orange Yellow
		{ 0.187, 0.129, 6.1 },		// Blue
		{ 0.305, 0.478, 23.4 },		// Green
		{ 0.539, 0.313, 12.0 },		// Red
		{ 0.448, 0.470, 59.1 },		// Yellow
		{ 0.364, 0.233, 19.8 },		// Magenta
		{ 0.196, 0.252, 19.8 },		// Cyan
		{ 0.310, 0.316, 90.0 },		// White
		{ 0.310, 0.316, 59.1 },		// Neutral 8
		{ 0.310, 0.316, 36.2 },		// Neutral 6.5
		{ 0.310, 0.316, 19.8 },		// Neutral 5
		{ 0.310, 0.316, 9.0 },		// Neutral 3.5
		{ 0.310, 0.316, 3.1 } };		// Black
	double gmb_cam[NSQ][4];
	double gmb_xyz[NSQ][3];
	double inverse[NSQ][3];
	double cam_xyz[4][3];
	double balance[4];
	int count[4];

	memset(gmb_cam, 0, sizeof gmb_cam);
	for (size_t sq = 0; sq < NSQ; sq++)
	{
		for (size_t c = 0; c < _info->colors; c++)
			count[c] = 0;
		for (int row = cut[sq][3]; row < cut[sq][3] + cut[sq][1]; row++)
		{
			for (int col = cut[sq][2]; col < cut[sq][2] + cut[sq][0]; col++)
			{
				int c = FC(row, col);
				if (c >= _info->colors) c -= 2;
				gmb_cam[sq][c] += BAYER2(row, col);
				BAYER2(row, col) = _info->black + (BAYER2(row, col) - _info->black) / 2;
				count[c]++;
			}
		}
		for (size_t c = 0; c < _info->colors; c++)
			gmb_cam[sq][c] = gmb_cam[sq][c] / count[c] - _info->black;
		gmb_xyz[sq][0] = gmb_xyY[sq][2] * gmb_xyY[sq][0] / gmb_xyY[sq][1];
		gmb_xyz[sq][1] = gmb_xyY[sq][2];
		gmb_xyz[sq][2] = gmb_xyY[sq][2] * (1 - gmb_xyY[sq][0] - gmb_xyY[sq][1]) / gmb_xyY[sq][1];
	}
	pseudoinverse(gmb_xyz, inverse, NSQ);
	for (size_t pass = 0; pass < 2; pass++)
	{
		_info->raw_color = 0;
		for (size_t i = 0; i < _info->colors; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				cam_xyz[i][j] = 0;
				for (size_t k = 0; k < NSQ; k++)
				{
					cam_xyz[i][j] += gmb_cam[k][i] * inverse[k][j];
				}
			}
		}
		cam_xyz_coeff(_info->rgb_cam, cam_xyz);

		for (size_t c = 0; c < _info->colors; c++)
			balance[c] = _info->pre_mul[c] * gmb_cam[20][c];
		for (size_t sq = 0; sq < NSQ; sq++)
			for (size_t c = 0; c < _info->colors; c++)
				gmb_cam[sq][c] *= balance[c];
	}
#undef NSQ
#endif
}


void CImageLoader::pseudoinverse(double(*in)[3], double(*out)[3], int size)
{
	double work[3][6];

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 6; j++)
			work[i][j] = j == i + 3;
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < size; k++)
				work[i][j] += in[k][i] * in[k][j];
	}
	for (int i = 0; i < 3; i++)
	{
		double num = work[i][i];
		for (int j = 0; j < 6; j++)
			work[i][j] /= num;
		for (int k = 0; k < 3; k++)
		{
			if (k == i)
				continue;
			num = work[k][i];
			for (int j = 0; j < 6; j++)
				work[k][j] -= work[i][j] * num;
		}
	}
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			out[i][j] = 0;
			for (int k = 0; k < 3; k++)
				out[i][j] += work[j][k + 3] * in[i][k];
		}
	}
}

void CImageLoader::cam_xyz_coeff(float rgb_cam[3][4], double cam_xyz[4][3])
{
	double cam_rgb[4][3];
	double inverse[4][3];

	for (size_t i = 0; i < _info->colors; i++)		/* Multiply out XYZ colorspace */
	{
		for (size_t j = 0; j < 3; j++)
		{
			cam_rgb[i][j] = 0;
			for (size_t k = 0; k < 3; k++)
				cam_rgb[i][j] += cam_xyz[i][k] * _info->xyz_rgb[k][j];
		}
	}

	for (size_t i = 0; i < _info->colors; i++)		/* Normalize cam_rgb so that */
	{
		double num = 0;
		for (size_t j = 0; j < 3; j++)		/* cam_rgb * (1,1,1) is (1,1,1,1) */
			num += cam_rgb[i][j];
		for (size_t j = 0; j < 3; j++)
			cam_rgb[i][j] /= num;
		_info->pre_mul[i] = 1 / num;
	}
	pseudoinverse(cam_rgb, inverse, _info->colors);
	for (size_t i = 0; i < 3; i++)
		for (size_t j = 0; j < _info->colors; j++)
			rgb_cam[i][j] = inverse[j][i];
}

void CImageLoader::scale_colors()
{
	unsigned sum[8];
	int val;
	double dsum[8];
	float scale_mul[4];

	if (_options->user_mul[0])
		memcpy(_info->pre_mul, _options->user_mul, sizeof _info->pre_mul);
	if (_options->use_auto_wb || (_options->use_camera_wb && _info->cam_mul[0] == -1))
	{
		memset(dsum, 0, sizeof dsum);
		unsigned bottom = MIN(_options->greybox[1] + _options->greybox[3], _info->height);
		unsigned right = MIN(_options->greybox[0] + _options->greybox[2], _info->width);
		for (unsigned row = _options->greybox[1]; row < bottom; row += 8)
		{
			for (unsigned col = _options->greybox[0]; col < right; col += 8)
			{
				memset(sum, 0, sizeof sum);
				for (unsigned y = row; y < row + 8 && y < bottom; y++)
				{
					for (unsigned x = col; x < col + 8 && x < right; x++)
					{
						for (size_t c = 0; c < 4; c++)
						{
							if (_info->filters)
							{
								c = fcol(y, x);
								val = BAYER2(y, x);
							}
							else
							{
								val = _info->image[y*_info->width + x][c];
							}
							if (val > _info->maximum - 25)
								goto skip_block;
							if ((val -= _info->cblack[c]) < 0)
								val = 0;
							sum[c] += val;
							sum[c + 4]++;
							if (_info->filters) break;
						}
					}
				}
				for (size_t c = 0; c < 8; c++)
					dsum[c] += sum[c];
			skip_block:;
			}
		}
		for (size_t c = 0; c < 4; c++)
			if (dsum[c])
				_info->pre_mul[c] = dsum[c + 4] / dsum[c];
	}
	if (_options->use_camera_wb && _info->cam_mul[0] != -1)
	{
		memset(sum, 0, sizeof sum);
		for (unsigned row = 0; row < 8; row++)
		{
			for (unsigned col = 0; col < 8; col++)
			{
				unsigned c = FC(row, col);
				if ((val = _info->white[row][col] - _info->cblack[c]) > 0)
					sum[c] += val;
				sum[c + 4]++;
			}
		}
		if (sum[0] && sum[1] && sum[2] && sum[3])
			for (size_t c = 0; c < 4; c++)
				_info->pre_mul[c] = (float)sum[c + 4] / sum[c];
		else if (_info->cam_mul[0] && _info->cam_mul[2])
			memcpy(_info->pre_mul, _info->cam_mul, sizeof _info->pre_mul);
		else
			fprintf(stderr, ("%s: Cannot use camera white balance.\n"), _reader->GetFileName());
	}
	if (_info->pre_mul[1] == 0)
		_info->pre_mul[1] = 1;
	if (_info->pre_mul[3] == 0)
		_info->pre_mul[3] = _info->colors < 4 ? _info->pre_mul[1] : 1;
	int dark = _info->black;
	int sat = _info->maximum;
	if (_options->threshold)
		wavelet_denoise();
	_info->maximum -= _info->black;

	double dmin = DBL_MAX;
	double dmax = 0;
	for (size_t c = 0; c < 4; c++)
	{
		if (dmin > _info->pre_mul[c])
			dmin = _info->pre_mul[c];
		if (dmax < _info->pre_mul[c])
			dmax = _info->pre_mul[c];
	}
	if (!_options->highlight)
		dmax = dmin;
	for (size_t c = 0; c < 4; c++)
		scale_mul[c] = (_info->pre_mul[c] /= dmax) * 65535.0 / _info->maximum;
	if (_info->filters > 1000 && (_info->cblack[4] + 1) / 2 == 1 && (_info->cblack[5] + 1) / 2 == 1)
	{
		for (size_t c = 0; c < 4; c++)
			_info->cblack[FC(c / 2, c % 2)] += _info->cblack[6 + c / 2 % _info->cblack[4] * _info->cblack[5] + c % 2 % _info->cblack[5]];
		_info->cblack[4] = _info->cblack[5] = 0;
	}
	unsigned size = _info->iheight*_info->iwidth;
	for (unsigned i = 0; i < size * 4; i++)
	{
		if (!(val = ((unsigned short *)_info->image)[i]))
			continue;
		if (_info->cblack[4] && _info->cblack[5])
			val -= _info->cblack[6 + i / 4 / _info->iwidth % _info->cblack[4] * _info->cblack[5] + i / 4 % _info->iwidth % _info->cblack[5]];
		val -= _info->cblack[i & 3];
		val *= scale_mul[i & 3];
		((unsigned short *)_info->image)[i] = CLIP(val);
	}
	if ((_options->aber[0] != 1 || _options->aber[2] != 1) && _info->colors == 3)
	{
		for (size_t c = 0; c < 4; c += 2)
		{
			if (_options->aber[c] == 1)
				continue;
			unsigned short* img = (unsigned short *)malloc(size * sizeof *img);
			CAutoFreeMemory autoFree(img);
			for (size_t i = 0; i < size; i++)
				img[i] = _info->image[i][c];
			for (unsigned row = 0; row < _info->iheight; row++)
			{
				float fr = (row - _info->iheight*0.5) * _options->aber[c] + _info->iheight*0.5;
				unsigned ur = fr;
				if (ur > _info->iheight - 2)
					continue;
				fr -= ur;
				for (unsigned col = 0; col < _info->iwidth; col++)
				{
					float fc = (col - _info->iwidth*0.5) * _options->aber[c] + _info->iwidth*0.5;
					unsigned uc = fc;
					if (uc > _info->iwidth - 2)
						continue;
					fc -= uc;
					unsigned short* pix = img + ur*_info->iwidth + uc;
					_info->image[row*_info->iwidth + col][c] =
						(pix[0] * (1 - fc) + pix[1] * fc) * (1 - fr) +
						(pix[_info->iwidth] * (1 - fc) + pix[_info->iwidth + 1] * fc) * fr;
				}
			}
		}
	}
}

void CImageLoader::wavelet_denoise()
{
	static const float noise[] = { 0.8002,0.2735,0.1202,0.0585,0.0291,0.0152,0.0080,0.0044 };
	float mul[2];
	int blk[2];
	unsigned short* window[4];

	int scale = 1;
	while (_info->maximum << scale < 0x10000)
		scale++;
	_info->maximum <<= --scale;
	_info->black <<= scale;
	for (size_t c = 0; c < 4; c++)
		_info->cblack[c] <<= scale;

	float *fimg = 0;
	int size = _info->iheight*_info->iwidth;
	if (size < 0x15550000)
		fimg = (float *)malloc((size * 3 + _info->iheight + _info->iwidth) * sizeof *fimg);
	CAutoFreeMemory autoFree(fimg);
	float* temp = fimg + size * 3;
	int nc = _info->colors;
	if (nc == 3 && _info->filters)
		nc++;
	for (size_t c = 0; c < nc; c++)
	{			/* denoise R,G1,B,G3 individually */
		for (int i = 0; i < size; i++)
			fimg[i] = 256 * sqrt(_info->image[i][c] << scale);
		int hpass = 0;
		int lpass;
		for (int lev = 0; lev < 5; lev++)
		{
			lpass = size*((lev & 1) + 1);
			for (int row = 0; row < _info->iheight; row++)
			{
				hat_transform(temp, fimg + hpass + row*_info->iwidth, 1, _info->iwidth, 1 << lev);
				for (int col = 0; col < _info->iwidth; col++)
					fimg[lpass + row*_info->iwidth + col] = temp[col] * 0.25;
			}
			for (int col = 0; col < _info->iwidth; col++) {
				hat_transform(temp, fimg + lpass + col, _info->iwidth, _info->iheight, 1 << lev);
				for (int row = 0; row < _info->iheight; row++)
					fimg[lpass + row*_info->iwidth + col] = temp[row] * 0.25;
			}
			float thold = _options->threshold * noise[lev];
			for (int i = 0; i < size; i++)
			{
				fimg[hpass + i] -= fimg[lpass + i];
				if (fimg[hpass + i] < -thold) fimg[hpass + i] += thold;
				else if (fimg[hpass + i] >  thold) fimg[hpass + i] -= thold;
				else	 fimg[hpass + i] = 0;
				if (hpass) fimg[i] += fimg[hpass + i];
			}
			hpass = lpass;
		}
		for (int i = 0; i < size; i++)
			_info->image[i][c] = CLIP(SQR(fimg[i] + fimg[lpass + i]) / 0x10000);
	}
	if (_info->filters && _info->colors == 3)  /* pull G1 and G3 closer together */
	{
		for (int row = 0; row < 2; row++)
		{
			mul[row] = 0.125 * _info->pre_mul[FC(row + 1, 0) | 1] / _info->pre_mul[FC(row, 0) | 1];
			blk[row] = _info->cblack[FC(row, 0) | 1];
		}
		for (int i = 0; i < 4; i++)
			window[i] = (unsigned short *)fimg + _info->width*i;
		int wlast = -1;
		for (int row = 1; row < _info->height - 1; row++)
		{
			while (wlast < row + 1)
			{
				wlast++;
				for (int i = 0; i < 4; i++)
					window[(i + 3) & 3] = window[i];
				for (int col = FC(wlast, 1) & 1; col < _info->width; col += 2)
					window[2][col] = BAYER(wlast, col);
			}
			float thold = _options->threshold / 512;
			for (int col = (FC(row, 0) & 1) + 1; col < _info->width - 1; col += 2)
			{
				float avg = (window[0][col - 1] + window[0][col + 1] +
					window[2][col - 1] + window[2][col + 1] - blk[~row & 1] * 4)
					* mul[row & 1] + (window[1][col] + blk[row & 1]) * 0.5;
				avg = avg < 0 ? 0 : sqrt(avg);
				float diff = sqrt(BAYER(row, col)) - avg;
				if (diff < -thold)
					diff += thold;
				else if (diff >  thold)
					diff -= thold;
				else
					diff = 0;
				BAYER(row, col) = CLIP(SQR(avg + diff) + 0.5);
			}
		}
	}
}

void CImageLoader::hat_transform(float *temp, float *base, int st, int size, int sc)
{
	int i;
	for (i = 0; i < sc; i++)
		temp[i] = 2 * base[st*i] + base[st*(sc - i)] + base[st*(i + sc)];
	for (; i + sc < size; i++)
		temp[i] = 2 * base[st*i] + base[st*(i - sc)] + base[st*(i + sc)];
	for (; i < size; i++)
		temp[i] = 2 * base[st*i] + base[st*(i - sc)] + base[st*(2 * size - 2 - (i + sc))];
}

void CImageLoader::pre_interpolate()
{
	if (_info->shrink)
	{
		if (_options->half_size)
		{
			_info->height = _info->iheight;
			_info->width = _info->iwidth;
			if (_info->filters == 9)
			{
				int row;
				int col;
				for (row = 0; row < 3; row++)
					for (col = 1; col < 4; col++)
						if (!(_info->image[row*_info->width + col][0] | _info->image[row*_info->width + col][2]))
							goto break2;
			break2:
				for (; row < _info->height; row += 3)
				{
					for (col = (col - 1) % 3 + 1; col < _info->width - 1; col += 3)
					{
						unsigned short(*img)[4] = _info->image + row*_info->width + col;
						for (size_t c = 0; c < 3; c += 2)
							img[0][c] = (img[-1][c] + img[1][c]) >> 1;
					}
				}
			}
		}
		else
		{
			unsigned short(*img)[4] = (unsigned short(*)[4]) calloc(_info->height, _info->width * sizeof *img);
			CAutoFreeMemory autoFree(img);
			for (int row = 0; row < _info->height; row++)
			{
				for (int col = 0; col < _info->width; col++)
				{
					int c = fcol(row, col);
					img[row*_info->width + col][c] = _info->image[(row >> 1)*_info->iwidth + (col >> 1)][c];
				}
			}
			free(_info->image);
			_info->image = img;
			_info->shrink = 0;
		}
	}
	if (_info->filters > 1000 && _info->colors == 3)
	{
		_info->mix_green = _options->four_color_rgb ^ _info->half_size;
		if (_options->four_color_rgb | _info->half_size)
		{
			_info->colors++;
		}
		else
		{
			for (int row = FC(1, 0) >> 1; row < _info->height; row += 2)
				for (int col = FC(row, 1) & 1; col < _info->width; col += 2)
					_info->image[row*_info->width + col][1] = _info->image[row*_info->width + col][3];
			_info->filters &= ~((_info->filters & 0x55555555) << 1);
		}
	}
	if (_info->half_size)
		_info->filters = 0;
}

void CImageLoader::lin_interpolate()
{
	int code[16][16][32];
	int sum[4];

	int size = 16;
	if (_info->filters == 9)
		size = 6;
	border_interpolate(1);
	for (int row = 0; row < size; row++)
	{
		for (int col = 0; col < size; col++)
		{
			int* ip = code[row][col] + 1;
			int f = fcol(row, col);
			int color;
			memset(sum, 0, sizeof sum);
			for (int y = -1; y <= 1; y++)
			{
				for (int x = -1; x <= 1; x++)
				{
					int shift = (y == 0) + (x == 0);
					color = fcol(row + y, col + x);
					if (color == f)
						continue;
					*ip++ = (_info->width*y + x) * 4 + color;
					*ip++ = shift;
					*ip++ = color;
					sum[color] += 1 << shift;
				}
			}
			code[row][col][0] = (ip - code[row][col]) / 3;
			for (int c = 0; c < _info->colors; c++)
			{
				if (c != f)
				{
					*ip++ = c;
					*ip++ = 256 / sum[c];
				}
			}
		}
	}
	for (int row = 1; row < _info->height - 1; row++)
	{
		for (int col = 1; col < _info->width - 1; col++)
		{
			unsigned short* pix = _info->image[row*_info->width + col];
			int* ip = code[row % size][col % size];
			memset(sum, 0, sizeof sum);
			for (int i = *ip++; i--; ip += 3)
				sum[ip[2]] += pix[ip[0]] << ip[1];
			for (int i = _info->colors; --i; ip += 2)
				pix[ip[0]] = sum[ip[0]] * ip[1] >> 8;
		}
	}
}

/*
This algorithm is officially called:

"Interpolation using a Threshold-based variable number of gradients"

described in http://scien.stanford.edu/pages/labsite/1999/psych221/projects/99/tingchen/algodep/vargra.html

I've extended the basic idea to work with non-Bayer filter arrays.
Gradients are numbered clockwise from NW=0 to W=7.
*/
void CImageLoader::vng_interpolate()
{
	static const signed char* cp;
	static const signed char terms[] =
	{
		-2,-2,+0,-1,0,0x01, -2,-2,+0,+0,1,0x01, -2,-1,-1,+0,0,0x01,
		-2,-1,+0,-1,0,0x02, -2,-1,+0,+0,0,0x03, -2,-1,+0,+1,1,0x01,
		-2,+0,+0,-1,0,0x06, -2,+0,+0,+0,1,0x02, -2,+0,+0,+1,0,0x03,
		-2,+1,-1,+0,0,0x04, -2,+1,+0,-1,1,0x04, -2,+1,+0,+0,0,0x06,
		-2,+1,+0,+1,0,0x02, -2,+2,+0,+0,1,0x04, -2,+2,+0,+1,0,0x04,
		-1,-2,-1,+0,0,0x80, -1,-2,+0,-1,0,0x01, -1,-2,+1,-1,0,0x01,
		-1,-2,+1,+0,1,0x01, -1,-1,-1,+1,0,0x88, -1,-1,+1,-2,0,0x40,
		-1,-1,+1,-1,0,0x22, -1,-1,+1,+0,0,0x33, -1,-1,+1,+1,1,0x11,
		-1,+0,-1,+2,0,0x08, -1,+0,+0,-1,0,0x44, -1,+0,+0,+1,0,0x11,
		-1,+0,+1,-2,1,0x40, -1,+0,+1,-1,0,0x66, -1,+0,+1,+0,1,0x22,
		-1,+0,+1,+1,0,0x33, -1,+0,+1,+2,1,0x10, -1,+1,+1,-1,1,0x44,
		-1,+1,+1,+0,0,0x66, -1,+1,+1,+1,0,0x22, -1,+1,+1,+2,0,0x10,
		-1,+2,+0,+1,0,0x04, -1,+2,+1,+0,1,0x04, -1,+2,+1,+1,0,0x04,
		+0,-2,+0,+0,1,0x80, +0,-1,+0,+1,1,0x88, +0,-1,+1,-2,0,0x40,
		+0,-1,+1,+0,0,0x11, +0,-1,+2,-2,0,0x40, +0,-1,+2,-1,0,0x20,
		+0,-1,+2,+0,0,0x30, +0,-1,+2,+1,1,0x10, +0,+0,+0,+2,1,0x08,
		+0,+0,+2,-2,1,0x40, +0,+0,+2,-1,0,0x60, +0,+0,+2,+0,1,0x20,
		+0,+0,+2,+1,0,0x30, +0,+0,+2,+2,1,0x10, +0,+1,+1,+0,0,0x44,
		+0,+1,+1,+2,0,0x10, +0,+1,+2,-1,1,0x40, +0,+1,+2,+0,0,0x60,
		+0,+1,+2,+1,0,0x20, +0,+1,+2,+2,0,0x10, +1,-2,+1,+0,0,0x80,
		+1,-1,+1,+1,0,0x88, +1,+0,+1,+2,0,0x08, +1,+0,+2,-1,0,0x40,
		+1,+0,+2,+1,0,0x10
	};
	static const signed char chood[] = { -1,-1, -1,0, -1,+1, 0,+1, +1,+1, +1,0, +1,-1, 0,-1 };

	unsigned short(*brow[5])[4], *pix;
	int* code[16][16];
	int gval[8];
	int sum[4];
	int row;
	int col;
	int g;

	lin_interpolate();

	int prow = 8;
	int pcol = 2;
	if (_info->filters == 1)
	{
		prow = 16;
		pcol = 16;
	}
	if (_info->filters == 9)
	{
		prow = 6;
		pcol = 6;
	}
	int* pip = (int *)calloc(prow*pcol, 1280);
	CAutoFreeMemory autoFreePip(pip);
	int* ip = pip;
	for (row = 0; row < prow; row++)		/* Precalculate for VNG */
	{
		for (col = 0; col < pcol; col++)
		{
			code[row][col] = ip;
			cp = terms;
			for (int t = 0; t < 64; t++)
			{
				int y1 = *cp++;
				int x1 = *cp++;
				int y2 = *cp++;
				int x2 = *cp++;
				int weight = *cp++;
				int grads = *cp++;
				int color = fcol(row + y1, col + x1);
				if (fcol(row + y2, col + x2) != color)
					continue;
				int diag = (fcol(row, col + 1) == color && fcol(row + 1, col) == color) ? 2 : 1;
				if (abs(y1 - y2) == diag && abs(x1 - x2) == diag)
					continue;
				*ip++ = (y1*_info->width + x1) * 4 + color;
				*ip++ = (y2*_info->width + x2) * 4 + color;
				*ip++ = weight;
				for (g = 0; g < 8; g++)
					if (grads & 1 << g) *ip++ = g;
				*ip++ = -1;
			}
			*ip++ = INT_MAX;
			cp = chood;
			for (g = 0; g < 8; g++)
			{
				int y = *cp++;
				int x = *cp++;
				*ip++ = (y*_info->width + x) * 4;
				int color = fcol(row, col);
				if (fcol(row + y, col + x) != color && fcol(row + y * 2, col + x * 2) == color)
					*ip++ = (y*_info->width + x) * 8 + color;
				else
					*ip++ = 0;
			}
		}
	}
	brow[4] = (unsigned short(*)[4]) calloc(_info->width * 3, sizeof **brow);
	CAutoFreeMemory autoFreeBrow(brow[4]);
	for (row = 0; row < 3; row++)
		brow[row] = brow[4] + row*_info->width;
	for (row = 2; row < _info->height - 2; row++)			/* Do VNG interpolation */
	{
		for (col = 2; col < _info->width - 2; col++)
		{
			pix = _info->image[row*_info->width + col];
			ip = code[row % prow][col % pcol];
			memset(gval, 0, sizeof gval);
			while ((g = ip[0]) != INT_MAX)			/* Calculate gradients */
			{
				int diff = ABS(pix[g] - pix[ip[1]]) << ip[2];
				gval[ip[3]] += diff;
				ip += 5;
				if ((g = ip[-1]) == -1)
					continue;
				gval[g] += diff;
				while ((g = *ip++) != -1)
					gval[g] += diff;
			}
			ip++;
			int gmax = gval[0];				/* Choose a threshold */
			int gmin = gmax;
			for (g = 1; g < 8; g++)
			{
				if (gmin > gval[g])
					gmin = gval[g];
				if (gmax < gval[g])
					gmax = gval[g];
			}
			if (gmax == 0) {
				memcpy(brow[2][col], pix, sizeof *_info->image);
				continue;
			}
			int thold = gmin + (gmax >> 1);
			memset(sum, 0, sizeof sum);
			int color = fcol(row, col);
			int num = 0;
			for (g = 0; g < 8; g++, ip += 2)
			{		/* Average the neighbors */
				if (gval[g] <= thold)
				{
					for (size_t c = 0; c < _info->colors; c++)
					{
						if (c == color && ip[1])
							sum[c] += (pix[c] + pix[ip[1]]) >> 1;
						else
							sum[c] += pix[ip[0] + c];
					}
					num++;
				}
			}
			for (size_t c = 0; c < _info->colors; c++)					/* Save to buffer */
			{
				int t = pix[color];
				if (c != color)
					t += (sum[c] - sum[color]) / num;
				brow[2][col][c] = CLIP(t);
			}
		}
		if (row > 3)				/* Write buffer to image */
			memcpy(_info->image[(row - 2)*_info->width + 2], brow[0] + 2, (_info->width - 4) * sizeof *_info->image);
		for (g = 0; g < 4; g++)
			brow[(g - 1) & 3] = brow[g];
	}
	memcpy(_info->image[(row - 2)*_info->width + 2], brow[0] + 2, (_info->width - 4) * sizeof *_info->image);
	memcpy(_info->image[(row - 1)*_info->width + 2], brow[1] + 2, (_info->width - 4) * sizeof *_info->image);
}

/*
Patterned Pixel Grouping Interpolation by Alain Desbiolles
*/
void CImageLoader::ppg_interpolate()
{
	int dir[5] = { 1, _info->width, -1, -_info->width, 1 };
	int diff[2];
	int guess[2];
	int c;
	int d;
	int i;
	unsigned short(*pix)[4];

	border_interpolate(3);

	/*  Fill in the green layer with gradients and pattern recognition: */
	for (int row = 3; row < _info->height - 3; row++)
	{
		for (int col = 3 + (FC(row, 3) & 1), c = FC(row, col); col < _info->width - 3; col += 2)
		{
			pix = _info->image + row*_info->width + col;
			for (i = 0; (d = dir[i]) > 0; i++)
			{
				guess[i] = (pix[-d][1] + pix[0][c] + pix[d][1]) * 2
					- pix[-2 * d][c] - pix[2 * d][c];
				diff[i] = (ABS(pix[-2 * d][c] - pix[0][c]) +
					ABS(pix[2 * d][c] - pix[0][c]) +
					ABS(pix[-d][1] - pix[d][1])) * 3 +
					(ABS(pix[3 * d][1] - pix[d][1]) +
						ABS(pix[-3 * d][1] - pix[-d][1])) * 2;
			}
			d = dir[i = diff[0] > diff[1]];
			pix[0][1] = ULIM(guess[i] >> 2, pix[d][1], pix[-d][1]);
		}
	}
	/*  Calculate red and blue for each green pixel:		*/
	for (int row = 1; row < _info->height - 1; row++)
		for (int col = 1 + (FC(row, 2) & 1), c = FC(row, col + 1); col < _info->width - 1; col += 2)
		{
			pix = _info->image + row*_info->width + col;
			for (i = 0; (d = dir[i]) > 0; c = 2 - c, i++)
				pix[0][c] = CLIP((pix[-d][c] + pix[d][c] + 2 * pix[0][1]
					- pix[-d][1] - pix[d][1]) >> 1);
		}
	/*  Calculate blue for red pixels and vice versa:		*/
	for (int row = 1; row < _info->height - 1; row++)
	{
		for (int col = 1 + (FC(row, 1) & 1), c = 2 - FC(row, col); col < _info->width - 1; col += 2)
		{
			pix = _info->image + row*_info->width + col;
			for (i = 0; (d = dir[i] + dir[i + 1]) > 0; i++)
			{
				diff[i] = ABS(pix[-d][c] - pix[d][c]) +
					ABS(pix[-d][1] - pix[0][1]) +
					ABS(pix[d][1] - pix[0][1]);
				guess[i] = pix[-d][c] + pix[d][c] + 2 * pix[0][1]
					- pix[-d][1] - pix[d][1];
			}
			if (diff[0] != diff[1])
				pix[0][c] = CLIP(guess[diff[0] > diff[1]] >> 1);
			else
				pix[0][c] = CLIP((guess[0] + guess[1]) >> 2);
		}
	}
}

void CImageLoader::border_interpolate(int border)
{
	unsigned sum[8];

	for (unsigned row = 0; row < _info->height; row++)
	{
		for (unsigned col = 0; col < _info->width; col++)
		{
			if (col == border && row >= border && row < _info->height - border)
				col = _info->width - border;
			memset(sum, 0, sizeof sum);
			for (unsigned y = row - 1; y != row + 2; y++)
			{
				for (unsigned x = col - 1; x != col + 2; x++)
				{
					if (y < _info->height && x < _info->width)
					{
						unsigned f = fcol(y, x);
						sum[f] += _info->image[y*_info->width + x][f];
						sum[f + 4]++;
					}
				}
			}
			unsigned f = fcol(row, col);
			for (size_t c = 0; c < _info->colors; c++)
				if (c != f && sum[c + 4])
					_info->image[row*_info->width + col][c] = sum[c] / sum[c + 4];
		}
	}
}

#define TS 512		/* Tile Size */
#define fcol(row,col) _info->xtrans[(row+6) % 6][(col+6) % 6]
/*
Frank Markesteijn's algorithm for Fuji X-Trans sensors
*/
void CImageLoader::xtrans_interpolate(int passes)
{
	static const short orth[12] = { 1,0,0,1,-1,0,0,-1,1,0,0,1 };
	static const short patt[2][16] =
	{
		{ 0,1,0,-1,2,0,-1,0,1,1,1,-1,0,0,0,0 },
		{ 0,1,0,-2,1,0,-2,0,1,1,-2,-2,1,-1,-1,1 }
	};
	static const short dir[4] = { 1,TS,TS + 1,TS - 1 };

	int hm[8];
	int avg[4];
	int color[3][8];

	short allhex[3][3][2][8];
	unsigned short sgrow;
	unsigned short sgcol;
	unsigned short(*rgb)[TS][TS][3];
	unsigned short(*rix)[3];
	unsigned short(*pix)[4];
	short(*lab)[TS][3];
	short(*lix)[3];
	float(*drv)[TS][TS];
	float diff[6];
	char(*homo)[TS][TS];

	cielab(0, 0);
	int ndir = 4 << (passes > 1);
	char* buffer = (char *)malloc(TS*TS*(ndir * 11 + 6));
	CAutoFreeMemory autoFree(buffer);
	rgb = (unsigned short(*)[TS][TS][3]) buffer;
	lab = (short(*)[TS][3])(buffer + TS*TS*(ndir * 6));
	drv = (float(*)[TS][TS])   (buffer + TS*TS*(ndir * 6 + 6));
	homo = (char(*)[TS][TS])   (buffer + TS*TS*(ndir * 10 + 6));

	/* Map a green hexagon around each non-green pixel and vice versa:	*/
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			int ng = 0;
			for (int d = 0; d < 10; d += 2)
			{
				int g = fcol(row, col) == 1;
				if (fcol(row + orth[d], col + orth[d + 2]) == 1)
					ng = 0;
				else
					ng++;
				if (ng == 4)
				{
					sgrow = row;
					sgcol = col;
				}
				if (ng == g + 1)
				{
					for (int c = 0; c < 8; c++)
					{
						int v = orth[d] * patt[g][c * 2] + orth[d + 1] * patt[g][c * 2 + 1];
						int h = orth[d + 2] * patt[g][c * 2] + orth[d + 3] * patt[g][c * 2 + 1];
						allhex[row][col][0][c ^ (g * 2 & d)] = h + v*_info->width;
						allhex[row][col][1][c ^ (g * 2 & d)] = h + v*TS;
					}
				}
			}
		}
	}

	/* Set green1 and green3 to the minimum and maximum allowed values:	*/
	for (int row = 2; row < _info->height - 2; row++)
	{
		unsigned short max = 0;
		unsigned short min = ~max;
		for (int col = 2; col < _info->width - 2; col++)
		{
			if (fcol(row, col) == 1 && (min = ~(max = 0)))
				continue;
			pix = _info->image + row*_info->width + col;
			short* hex = allhex[row % 3][col % 3][0];
			if (!max)
			{
				for (int c = 0; c < 6; c++)
				{
					int val = pix[hex[c]][1];
					if (min > val)
						min = val;
					if (max < val)
						max = val;
				}
			}
			pix[0][1] = min;
			pix[0][3] = max;
			switch ((row - sgrow) % 3)
			{
			case 1:
				if (row < _info->height - 3)
				{
					row++;
					col--;
				}
				break;
			case 2:
				if ((min = ~(max = 0)) && (col += 2) < _info->width - 3 && row > 2)
					row--;
			}
		}
	}

	for (int top = 3; top < _info->height - 19; top += TS - 16)
	{
		for (int left = 3; left < _info->width - 19; left += TS - 16)
		{
			int mrow = MIN(top + TS, _info->height - 3);
			int mcol = MIN(left + TS, _info->width - 3);
			for (int row = top; row < mrow; row++)
				for (int col = left; col < mcol; col++)
					memcpy(rgb[0][row - top][col - left], _info->image[row*_info->width + col], 6);
			for (int c = 0; c < 3; c++)
				memcpy(rgb[c + 1], rgb[0], sizeof *rgb);

			/* Interpolate green horizontally, vertically, and along both diagonals: */
			for (int row = top; row < mrow; row++)
			{
				for (int col = left; col < mcol; col++)
				{
					int f = fcol(row, col);
					if (f == 1)
						continue;
					pix = _info->image + row*_info->width + col;
					short* hex = allhex[row % 3][col % 3][0];
					color[1][0] = 174 * (pix[hex[1]][1] + pix[hex[0]][1]) -
						46 * (pix[2 * hex[1]][1] + pix[2 * hex[0]][1]);
					color[1][1] = 223 * pix[hex[3]][1] + pix[hex[2]][1] * 33 +
						92 * (pix[0][f] - pix[-hex[2]][f]);
					for (int c = 0; c < 2; c++)
						color[1][2 + c] =
						164 * pix[hex[4 + c]][1] + 92 * pix[-2 * hex[4 + c]][1] + 33 *
						(2 * pix[0][f] - pix[3 * hex[4 + c]][f] - pix[-3 * hex[4 + c]][f]);
					for (int c = 0; c < 4; c++)
						rgb[c ^ !((row - sgrow) % 3)][row - top][col - left][1] =
						LIM(color[1][c] >> 8, pix[0][1], pix[0][3]);
				}
			}

			for (int pass = 0; pass < passes; pass++)
			{
				if (pass == 1)
					memcpy(rgb += 4, buffer, 4 * sizeof *rgb);

				/* Recalculate green from interpolated values of closer pixels:	*/
				if (pass)
				{
					for (int row = top + 2; row < mrow - 2; row++)
					{
						for (int col = left + 2; col < mcol - 2; col++)
						{
							int f = fcol(row, col);
							if (f == 1)
								continue;
							pix = _info->image + row*_info->width + col;
							short* hex = allhex[row % 3][col % 3][1];
							for (int d = 3; d < 6; d++)
							{
								rix = &rgb[(d - 2) ^ !((row - sgrow) % 3)][row - top][col - left];
								int val = rix[-2 * hex[d]][1] + 2 * rix[hex[d]][1]
									- rix[-2 * hex[d]][f] - 2 * rix[hex[d]][f] + 3 * rix[0][f];
								rix[0][1] = LIM(val / 3, pix[0][1], pix[0][3]);
							}
						}
					}
				}

				/* Interpolate red and blue values for solitary green pixels:	*/
				for (int row = (top - sgrow + 4) / 3 * 3 + sgrow; row < mrow - 2; row += 3)
				{
					for (int col = (left - sgcol + 4) / 3 * 3 + sgcol; col < mcol - 2; col += 3)
					{
						rix = &rgb[0][row - top][col - left];
						int h = fcol(row, col + 1);
						memset(diff, 0, sizeof diff);
						int i = 1;
						for (int d = 0; d < 6; d++, i ^= TS ^ 1, h ^= 2)
						{
							for (int c = 0; c < 2; c++, h ^= 2)
							{
								int g = 2 * rix[0][1] - rix[i << c][1] - rix[-i << c][1];
								color[h][d] = g + rix[i << c][h] + rix[-i << c][h];
								if (d > 1)
									diff[d] += SQR(rix[i << c][1] - rix[-i << c][1]
										- rix[i << c][h] + rix[-i << c][h]) + SQR(g);
							}
							if (d > 1 && (d & 1))
								if (diff[d - 1] < diff[d])
									for (int c = 0; c < 2; c++)
										color[c * 2][d] = color[c * 2][d - 1];
							if (d < 2 || (d & 1))
							{
								for (int c = 0; c < 2; c++)
									rix[0][c * 2] = CLIP(color[c * 2][d] / 2);
								rix += TS*TS;
							}
						}
					}
				}

				/* Interpolate red for blue pixels and vice versa:		*/
				for (int row = top + 3; row < mrow - 3; row++)
				{
					for (int col = left + 3; col < mcol - 3; col++)
					{
						int f = 2 - fcol(row, col);
						if ((f) == 1)
							continue;
						rix = &rgb[0][row - top][col - left];
						int c = (row - sgrow) % 3 ? TS : 1;
						int h = 3 * (c ^ TS ^ 1);
						for (int d = 0; d < 4; d++, rix += TS*TS)
						{
							int i = d > 1 || ((d ^ c) & 1) ||
								((ABS(rix[0][1] - rix[c][1]) + ABS(rix[0][1] - rix[-c][1])) <
									2 * (ABS(rix[0][1] - rix[h][1]) + ABS(rix[0][1] - rix[-h][1]))) ? c : h;
							rix[0][f] = CLIP((rix[i][f] + rix[-i][f] +
								2 * rix[0][1] - rix[i][1] - rix[-i][1]) / 2);
						}
					}
				}

				/* Fill in red and blue for 2x2 blocks of green:		*/
				for (int row = top + 2; row < mrow - 2; row++)
				{
					if ((row - sgrow) % 3)
					{
						for (int col = left + 2; col < mcol - 2; col++)
						{
							if ((col - sgcol) % 3)
							{
								rix = &rgb[0][row - top][col - left];
								short* hex = allhex[row % 3][col % 3][1];
								for (int d = 0; d < ndir; d += 2, rix += TS*TS)
								{
									if (hex[d] + hex[d + 1])
									{
										int g = 3 * rix[0][1] - 2 * rix[hex[d]][1] - rix[hex[d + 1]][1];
										for (int c = 0; c < 4; c += 2)
											rix[0][c] = CLIP((g + 2 * rix[hex[d]][c] + rix[hex[d + 1]][c]) / 3);
									}
									else
									{
										int g = 2 * rix[0][1] - rix[hex[d]][1] - rix[hex[d + 1]][1];
										for (int c = 0; c < 4; c += 2)
											rix[0][c] = CLIP((g + rix[hex[d]][c] + rix[hex[d + 1]][c]) / 2);
									}
								}
							}
						}
					}
				}
			}
			rgb = (unsigned short(*)[TS][TS][3]) buffer;
			mrow -= top;
			mcol -= left;

			/* Convert to CIELab and differentiate in all directions:	*/
			for (int d = 0; d < ndir; d++)
			{
				for (int row = 2; row < mrow - 2; row++)
					for (int col = 2; col < mcol - 2; col++)
						cielab(rgb[d][row][col], lab[row][col]);
				int f = dir[d & 3];
				for (int row = 3; row < mrow - 3; row++)
				{
					for (int col = 3; col < mcol - 3; col++)
					{
						lix = &lab[row][col];
						int g = 2 * lix[0][0] - lix[f][0] - lix[-f][0];
						drv[d][row][col] = SQR(g)
							+ SQR((2 * lix[0][1] - lix[f][1] - lix[-f][1] + g * 500 / 232))
							+ SQR((2 * lix[0][2] - lix[f][2] - lix[-f][2] - g * 500 / 580));
					}
				}
			}

			/* Build homogeneity maps from the derivatives:			*/
			memset(homo, 0, ndir*TS*TS);
			for (int row = 4; row < mrow - 4; row++)
			{
				for (int col = 4; col < mcol - 4; col++)
				{
					float tr = FLT_MAX;
					for (int d = 0; d < ndir; d++)
						if (tr > drv[d][row][col])
							tr = drv[d][row][col];
					tr *= 8;
					for (int d = 0; d < ndir; d++)
						for (int v = -1; v <= 1; v++)
							for (int h = -1; h <= 1; h++)
								if (drv[d][row + v][col + h] <= tr)
									homo[d][row][col]++;
				}
			}

			/* Average the most homogenous pixels for the final result:	*/
			if (_info->height - top < TS + 4)
				mrow = _info->height - top + 2;
			if (_info->width - left < TS + 4)
				mcol = _info->width - left + 2;
			for (int row = MIN(top, 8); row < mrow - 8; row++)
			{
				for (int col = MIN(left, 8); col < mcol - 8; col++)
				{
					for (int d = 0; d < ndir; d++)
					{
						hm[d] = 0;
						for (int v = -2; v <= 2; v++)
						{
							for (int h = -2; h <= 2; h++)
							{
								hm[d] += homo[d][row + v][col + h];
							}
						}
					}
					for (int d = 0; d < ndir - 4; d++)
						if (hm[d] < hm[d + 4]) hm[d] = 0; else
							if (hm[d] > hm[d + 4]) hm[d + 4] = 0;
					unsigned short max = hm[0];
					for (int d = 1; d < ndir; d++)
						if (max < hm[d]) max = hm[d];
					max -= max >> 3;
					memset(avg, 0, sizeof avg);
					for (int d = 0; d < ndir; d++)
						if (hm[d] >= max)
						{
							for (int c = 0; c < 3; c++)
								avg[c] += rgb[d][row][col][c];
							avg[3]++;
						}
					for (int c = 0; c < 3; c++)
						_info->image[(row + top)*_info->width + col + left][c] = avg[c] / avg[3];
				}
			}
		}
	}
	border_interpolate(8);
}
#undef fcol

/*
Adaptive Homogeneity-Directed interpolation is based on
the work of Keigo Hirakawa, Thomas Parks, and Paul Lee.
*/
void CImageLoader::ahd_interpolate()
{
	static const int dir[4] = { -1, 1, -TS, TS };
	int hm[2];
	unsigned ldiff[2][4];
	unsigned abdiff[2][4];

	cielab(0, 0);
	border_interpolate(5);
	char* buffer = (char *)malloc(26 * TS*TS);
	CAutoFreeMemory autoFree(buffer);
	unsigned short(*rgb)[TS][TS][3] = (unsigned short(*)[TS][TS][3]) buffer;
	short(*lab)[TS][TS][3] = (short(*)[TS][TS][3]) (buffer + 12 * TS*TS);
	char(*homo)[TS][TS] = (char(*)[TS][TS]) (buffer + 24 * TS*TS);

	for (int top = 2; top < _info->height - 5; top += TS - 6)
	{
		for (int left = 2; left < _info->width - 5; left += TS - 6)
		{
			/*  Interpolate green horizontally and vertically:		*/
			for (int row = top; row < top + TS && row < _info->height - 2; row++)
			{
				int col = left + (FC(row, left) & 1);
				for (int c = FC(row, col); col < left + TS && col < _info->width - 2; col += 2)
				{
					unsigned short(*pix)[4] = _info->image + row*_info->width + col;
					int val = ((pix[-1][1] + pix[0][c] + pix[1][1]) * 2
						- pix[-2][c] - pix[2][c]) >> 2;
					rgb[0][row - top][col - left][1] = ULIM(val, pix[-1][1], pix[1][1]);
					val = ((pix[-_info->width][1] + pix[0][c] + pix[_info->width][1]) * 2
						- pix[-2 * _info->width][c] - pix[2 * _info->width][c]) >> 2;
					rgb[1][row - top][col - left][1] = ULIM(val, pix[-_info->width][1], pix[_info->width][1]);
				}
			}
			/*  Interpolate red and blue, and convert to CIELab:		*/
			for (int d = 0; d < 2; d++)
			{
				for (int row = top + 1; row < top + TS - 1 && row < _info->height - 3; row++)
				{
					for (int col = left + 1; col < left + TS - 1 && col < _info->width - 3; col++)
					{
						unsigned short(*pix)[4] = _info->image + row*_info->width + col;
						unsigned short(*rix)[3] = &rgb[d][row - top][col - left];
						short(*lix)[3] = &lab[d][row - top][col - left];
						int c = 2 - FC(row, col);
						int val;
						if (c == 1)
						{
							c = FC(row + 1, col);
							val = pix[0][1] + ((pix[-1][2 - c] + pix[1][2 - c]
								- rix[-1][1] - rix[1][1]) >> 1);
							rix[0][2 - c] = CLIP(val);
							val = pix[0][1] + ((pix[-_info->width][c] + pix[_info->width][c]
								- rix[-TS][1] - rix[TS][1]) >> 1);
						}
						else
						{
							val = rix[0][1] + ((pix[-_info->width - 1][c] + pix[-_info->width + 1][c]
								+ pix[+_info->width - 1][c] + pix[+_info->width + 1][c]
								- rix[-TS - 1][1] - rix[-TS + 1][1]
								- rix[+TS - 1][1] - rix[+TS + 1][1] + 1) >> 2);
						}
						rix[0][c] = CLIP(val);
						c = FC(row, col);
						rix[0][c] = pix[0][c];
						cielab(rix[0], lix[0]);
					}
				}
			}
			/*  Build homogeneity maps from the CIELab images:		*/
			memset(homo, 0, 2 * TS*TS);
			for (int row = top + 2; row < top + TS - 2 && row < _info->height - 4; row++)
			{
				int tr = row - top;
				for (int col = left + 2; col < left + TS - 2 && col < _info->width - 4; col++)
				{
					int tc = col - left;
					for (int d = 0; d < 2; d++)
					{
						short(*lix)[3] = &lab[d][tr][tc];
						for (int i = 0; i < 4; i++)
						{
							ldiff[d][i] = ABS(lix[0][0] - lix[dir[i]][0]);
							abdiff[d][i] = SQR(lix[0][1] - lix[dir[i]][1])
								+ SQR(lix[0][2] - lix[dir[i]][2]);
						}
					}
					unsigned leps = MIN(MAX(ldiff[0][0], ldiff[0][1]),
						MAX(ldiff[1][2], ldiff[1][3]));
					unsigned abeps = MIN(MAX(abdiff[0][0], abdiff[0][1]),
						MAX(abdiff[1][2], abdiff[1][3]));
					for (int d = 0; d < 2; d++)
						for (int i = 0; i < 4; i++)
							if (ldiff[d][i] <= leps && abdiff[d][i] <= abeps)
								homo[d][tr][tc]++;
				}
			}
			/*  Combine the most homogenous pixels for the final result:	*/
			for (int row = top + 3; row < top + TS - 3 && row < _info->height - 5; row++)
			{
				int tr = row - top;
				for (int col = left + 3; col < left + TS - 3 && col < _info->width - 5; col++)
				{
					int tc = col - left;
					for (int d = 0; d < 2; d++)
					{
						hm[d] = 0;
						for (int i = tr - 1; i <= tr + 1; i++)
							for (int j = tc - 1; j <= tc + 1; j++)
								hm[d] += homo[d][i][j];
					}
					if (hm[0] != hm[1])
					{
						for (int c = 0; c < 3; c++)
							_info->image[row*_info->width + col][c] = rgb[hm[1] > hm[0]][tr][tc][c];
					}
					else
					{
						for (int c = 0; c < 3; c++)
							_info->image[row*_info->width + col][c] = (rgb[0][tr][tc][c] + rgb[1][tr][tc][c]) >> 1;
					}
				}
			}
		}
	}
}
#undef TS

void CImageLoader::cielab(unsigned short rgb[3], short lab[3])
{
	float r, xyz[3];
	static float cbrt[0x10000], xyz_cam[3][4];

	if (!rgb)
	{
		for (int i = 0; i < 0x10000; i++)
		{
			r = i / 65535.0;
			cbrt[i] = r > 0.008856 ? pow(r, 1 / 3.0) : 7.787*r + 16 / 116.0;
		}
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < _info->colors; j++)
			{
				xyz_cam[i][j] = 0;
				for (int k = 0; k < 3; k++)
				{
					xyz_cam[i][j] += _info->xyz_rgb[i][k] * _info->rgb_cam[k][j] / _info->d65_white[i];
				}
			}
		}
		return;
	}
	xyz[0] = xyz[1] = xyz[2] = 0.5;
	for (size_t c = 0; c < _info->colors; c++)
	{
		xyz[0] += xyz_cam[0][c] * rgb[c];
		xyz[1] += xyz_cam[1][c] * rgb[c];
		xyz[2] += xyz_cam[2][c] * rgb[c];
	}
	xyz[0] = cbrt[CLIP((int)xyz[0])];
	xyz[1] = cbrt[CLIP((int)xyz[1])];
	xyz[2] = cbrt[CLIP((int)xyz[2])];
	lab[0] = 64 * (116 * xyz[1] - 16);
	lab[1] = 64 * 500 * (xyz[0] - xyz[1]);
	lab[2] = 64 * 200 * (xyz[1] - xyz[2]);
}



/* RESTRICTED code starts here */

void CImageLoader::foveon_decoder(unsigned size, unsigned code)
{
	static unsigned huff[1024];

	if (!code)
	{
		for (int i = 0; i < size; i++)
			huff[i] = _reader->get4();
		memset(first_decode, 0, sizeof first_decode);
		free_decode = first_decode;
	}
	decode* cur = free_decode++;
	if (free_decode > first_decode + 2048)
		throw CExceptionDecoderTableOverflow();

	if (code)
	{
		for (int i = 0; i < size; i++)
		{
			if (huff[i] == code)
			{
				cur->leaf = i;
				return;
			}
		}
	}
	int len = code >> 27;
	if ((len) > 26)
		return;
	code = (len + 1) << 27 | (code & 0x3ffffff) << 1;

	cur->branch[0] = free_decode;
	foveon_decoder(size, code);
	cur->branch[1] = free_decode;
	foveon_decoder(size, code + 1);
}

void CImageLoader::foveon_sd_load_raw()
{
	decode* dindex;
	short diff[1024];
	unsigned bitbuf = 0;
	int pred[3];
	int bit = -1;

	_reader->read_shorts((unsigned short *)diff, 1024);
	if (!_info->load_flags)
		foveon_decoder(1024, 0);

	for (int row = 0; row < _info->height; row++)
	{
		memset(pred, 0, sizeof pred);
		if (!bit && !_info->load_flags && atoi(_info->model + 2) < 14)
			_reader->get4();
		bit = 0;
		for (int col = 0; col < _info->width; col++)
		{
			if (_info->load_flags)
			{
				bitbuf = _reader->get4();
				for (size_t c = 0; c < 3; c++)
					pred[2 - c] += diff[bitbuf >> c * 10 & 0x3ff];
			}
			else
			{
				for (size_t c = 0; c < 3; c++)
				{
					for (dindex = first_decode; dindex->branch[0]; )
					{
						bit = (bit - 1) & 31;
						if (bit == 31)
							for (size_t i = 0; i < 4; i++)
								bitbuf = (bitbuf << 8) + _reader->GetChar();
						dindex = dindex->branch[bitbuf >> bit & 1];
					}
					pred[c] += diff[dindex->leaf];
					if (pred[c] >> 16 && ~pred[c] >> 16)
						throw CExceptionFile();
				}
			}
			for (size_t c = 0; c < 3; c++)
				_info->image[row*_info->width + col][c] = pred[c];
		}
	}
}

void CImageLoader::foveon_huff(unsigned short *huff)
{
	huff[0] = 8;
	for (int i = 0; i < 13; i++)
	{
		int clen = _reader->GetChar();
		int code = _reader->GetChar();
		for (int j = 0; j < 256 >> clen; )
			huff[code + ++j] = clen << 8 | i;
	}
	_reader->get2();
}

void CImageLoader::foveon_dp_load_raw()
{
	unsigned roff[4];
	unsigned short huff[512];
	unsigned short vpred[2][2];
	unsigned short hpred[2];

	_reader->Seek(8, SEEK_CUR);
	foveon_huff(huff);
	roff[0] = 48;
	for (size_t c = 0; c < 3; c++)
		roff[c + 1] = -(-(roff[c] + _reader->get4()) & -16);
	for (size_t c = 0; c < 3; c++)
	{
		_reader->Seek(_info->data_offset + roff[c], SEEK_SET);
		_info->getbits(-1);
		vpred[0][0] = vpred[0][1] = vpred[1][0] = vpred[1][1] = 512;
		for (unsigned row = 0; row < _info->height; row++)
		{
			for (unsigned col = 0; col < _info->width; col++)
			{
				unsigned diff = jhead::ljpeg_diff(huff, *_info);
				if (col < 2) hpred[col] = vpred[row & 1][col] += diff;
				else hpred[col & 1] += diff;
				_info->image[row*_info->width + col][c] = hpred[col & 1];
			}
		}
	}
}

void CImageLoader::foveon_load_camf()
{
	unsigned short huff[258];
	unsigned short vpred[2][2] = { { 512,512 },{ 512,512 } };
	unsigned short hpred[2];

	_reader->Seek(_info->meta_offset, SEEK_SET);
	unsigned type = _reader->get4();
	_reader->get4();
	_reader->get4();
	unsigned wide = _reader->get4();
	unsigned high = _reader->get4();
	if (type == 2)
	{
		_reader->Read(_info->meta_data, 1, _info->meta_length);
		for (size_t i = 0; i < _info->meta_length; i++)
		{
			high = (high * 1597 + 51749) % 244944;
			wide = high * (INT64)301593171 >> 24;
			_info->meta_data[i] ^= ((((high << 8) - wide) >> 1) + wide) >> 17;
		}
	}
	else if (type == 4)
	{
		free(_info->meta_data);
		_info->meta_data = (char *)malloc(_info->meta_length = wide*high * 3 / 2);
		if (!_info->meta_data)
			throw CExceptionMemory();

		foveon_huff(huff);
		_reader->get4();
		_info->getbits(-1);
		unsigned j = 0;
		for (unsigned row = 0; row < high; row++)
		{
			for (unsigned col = 0; col < wide; col++)
			{
				unsigned diff = jhead::ljpeg_diff(huff, *_info);
				if (col < 2)
					hpred[col] = vpred[row & 1][col] += diff;
				else
					hpred[col & 1] += diff;
				if (col & 1)
				{
					_info->meta_data[j++] = hpred[0] >> 4;
					_info->meta_data[j++] = hpred[0] << 4 | hpred[1] >> 8;
					_info->meta_data[j++] = hpred[1];
				}
			}
		}
	}
#ifdef _DEBUG
	else
		fprintf(stderr, ("%s has unknown CAMF type %d.\n"), _reader->GetFileName(), type);
#endif // DEBUG
}

#define sget4(s) sget4((unsigned char *)s)
const char* CImageLoader::foveon_camf_param(const char *block, const char *param)
{
	char* pos;
	for (unsigned idx = 0; idx < _info->meta_length; idx += _reader->sget4(pos + 8))
	{
		pos = _info->meta_data + idx;
		if (strncmp(pos, "CMb", 3))
			break;
		if (pos[3] != 'P')
			continue;
		if (strcmp(block, pos + _reader->sget4(pos + 12)))
			continue;
		char* cp = pos + _reader->sget4(pos + 16);
		unsigned num = _reader->sget4(cp);
		char* dp = pos + _reader->sget4(cp + 4);
		while (num--)
		{
			cp += 8;
			if (!strcmp(param, dp + _reader->sget4(cp)))
				return dp + _reader->sget4(cp + 4);
		}
	}
	return nullptr;
}

void* CImageLoader::foveon_camf_matrix(unsigned dim[3], const char *name)
{
	char* pos;
	for (unsigned idx = 0; idx < _info->meta_length; idx += _reader->sget4(pos + 8))
	{
		pos = _info->meta_data + idx;
		if (strncmp(pos, "CMb", 3))
			break;
		if (pos[3] != 'M')
			continue;
		if (strcmp(name, pos + _reader->sget4(pos + 12)))
			continue;
		dim[0] = dim[1] = dim[2] = 1;
		char* cp = pos + _reader->sget4(pos + 16);
		unsigned type = _reader->sget4(cp);
		unsigned ndim = _reader->sget4(cp + 4);
		if (ndim > 3)
			break;
		char* dp = pos + _reader->sget4(cp + 8);
		for (unsigned i = ndim; i--; )
		{
			cp += 12;
			dim[i] = _reader->sget4(cp);
		}
		double dsize = (double)dim[0] * dim[1] * dim[2];
		if (dsize > _info->meta_length / 4)
			break;
		unsigned size = dsize;
		unsigned* mat = (unsigned *)malloc((size) * 4);
		if (!mat)
			throw CExceptionMemory();

		for (unsigned i = 0; i < size; i++)
		{
			if (type && type != 6)
				mat[i] = _reader->sget4(dp + i * 4);
			else
				mat[i] = _reader->sget4(dp + i * 2) & 0xffff;
		}
		return mat;
	}
#ifdef _DEBUG
	fprintf(stderr, ("%s: \"%s\" matrix not found!\n"), _reader->GetFileName(), name);
#endif // DEBUG
	return nullptr;
}
#undef sget4

int CImageLoader::foveon_fixed(void *ptr, int size, const char *name)
{
	if (!name)
		return 0;

	unsigned dim[3];
	void *dp = foveon_camf_matrix(dim, name);
	if (!dp)
		return 0;
	CAutoFreeMemory autoFree(dp);
	memcpy(ptr, dp, size * 4);
	return 1;
}

float CImageLoader::foveon_avg(short *pix, int range[2], float cfilt)
{
	float min = FLT_MAX;
	float max = -FLT_MAX;
	float sum = 0;

	for (int i = range[0]; i <= range[1]; i++)
	{
		float val = pix[i * 4] + (pix[i * 4] - pix[(i - 1) * 4]) * cfilt;
		sum += val;
		if (min > val) min = val;
		if (max < val) max = val;
	}
	if (range[1] - range[0] == 1)
		return sum / 2;
	return (sum - min - max) / (range[1] - range[0] - 1);
}

short* CImageLoader::foveon_make_curve(double max, double mul, double filt)
{
	if (!filt)
		filt = 0.8;
	unsigned size = 4 * M_PI*max / filt;
	if (size == UINT_MAX)
		size--;
	short* curve = (short *)calloc(size + 1, sizeof *curve);
	if (!curve)
		throw CExceptionMemory();

	curve[0] = size;
	for (unsigned i = 0; i < size; i++)
	{
		double x = i*filt / max / 4;
		curve[i + 1] = (cos(x) + 1) / 2 * tanh(i*filt / mul) * mul + 0.5;
	}
	return curve;
}

void CImageLoader::foveon_make_curves(short **curvep, float dq[3], float div[3], float filt)
{
	double mul[3];
	double max = 0;

	for (size_t c = 0; c < 3; c++)
		mul[c] = dq[c] / div[c];
	for (size_t c = 0; c < 3; c++)
		if (max < mul[c])
			max = mul[c];
	for (size_t c = 0; c < 3; c++)
		curvep[c] = foveon_make_curve(max, mul[c], filt);
}

int CImageLoader::foveon_apply_curve(short *curve, int i)
{
	if (abs(i) >= curve[0])
		return 0;
	return i < 0 ? -curve[1 - i] : curve[1 + i];
}

#define image ((short (*)[4]) _info->image)
void CImageLoader::foveon_interpolate()
{
	static const short hood[] = { -1,-1, -1,0, -1,1, 0,-1, 0,1, 1,-1, 1,0, 1,1 };

	short prev[3];
	float cfilt = 0;
	float ddft[3][3][2];
	float ppm[3][3][3];
	float cam_xyz[3][3];
	float correct[3][3];
	float last[3][3];
	float trans[3][3];
	float chroma_dq[3];
	float color_dq[3];
	float diag[3][3];
	float div[3];
	float fsum[3];
	int row;
	int col;
	int dscr[2][2];
	int dstb[4];
	int(*smrow[7])[3];
	int total[4];
	int ipix[3];
	int work[3][3];
	int smred_p = 0;
	int dev[3];
	int satlev[3];
	int keep[4];
	int active[4];
	unsigned dim[3];
	double dsum = 0;
	double trsum[3];
	char str[128];

	foveon_load_camf();
	foveon_fixed(dscr, 4, "DarkShieldColRange");
	foveon_fixed(ppm[0][0], 27, "PostPolyMatrix");
	foveon_fixed(satlev, 3, "SaturationLevel");
	foveon_fixed(keep, 4, "KeepImageArea");
	foveon_fixed(active, 4, "ActiveImageArea");
	foveon_fixed(chroma_dq, 3, "ChromaDQ");
	foveon_fixed(color_dq, 3,
		foveon_camf_param("IncludeBlocks", "ColorDQ") ?
		"ColorDQ" : "ColorDQCamRGB");
	if (foveon_camf_param("IncludeBlocks", "ColumnFilter"))
		foveon_fixed(&cfilt, 1, "ColumnFilter");

	memset(ddft, 0, sizeof ddft);
	if (!foveon_camf_param("IncludeBlocks", "DarkDrift")
		|| !foveon_fixed(ddft[1][0], 12, "DarkDrift"))
	{
		for (int i = 0; i < 2; i++)
		{
			foveon_fixed(dstb, 4, i ? "DarkShieldBottom" : "DarkShieldTop");
			for (row = dstb[1]; row <= dstb[3]; row++)
				for (col = dstb[0]; col <= dstb[2]; col++)
					for (size_t c = 0; c < 3; c++)
						ddft[i + 1][c][1] += (short)image[row*_info->width + col][c];
			for (size_t c = 0; c < 3; c++)
				ddft[i + 1][c][1] /= (dstb[3] - dstb[1] + 1) * (dstb[2] - dstb[0] + 1);
		}
	}

	const char* cp = foveon_camf_param("WhiteBalanceIlluminants", _info->model2);
	if (!cp)
	{
#ifdef _DEBUG
		fprintf(stderr, ("%s: Invalid white balance \"%s\"\n"), _reader->GetFileName(), _info->model2);
#endif // DEBUG
		return;
	}
	foveon_fixed(cam_xyz, 9, cp);
	foveon_fixed(correct, 9, foveon_camf_param("WhiteBalanceCorrections", _info->model2));
	memset(last, 0, sizeof last);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			for (size_t c = 0; c < 3; c++)
				last[i][j] += correct[i][c] * cam_xyz[c][j];

#define LAST(x,y) last[(i+x)%3][(c+y)%3]
	for (int i = 0; i < 3; i++)
		for (size_t c = 0; c < 3; c++)
			diag[c][i] = LAST(1, 1)*LAST(2, 2) - LAST(1, 2)*LAST(2, 1);
#undef LAST
	for (size_t c = 0; c < 3; c++)
		div[c] = diag[c][0] * 0.3127 + diag[c][1] * 0.329 + diag[c][2] * 0.3583;
	sprintf(str, "%sRGBNeutral", _info->model2);
	if (foveon_camf_param("IncludeBlocks", str))
		foveon_fixed(div, 3, str);
	float num = 0;
	for (size_t c = 0; c < 3; c++)
		if (num < div[c])
			num = div[c];
	for (size_t c = 0; c < 3; c++)
		div[c] /= num;

	memset(trans, 0, sizeof trans);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			for (size_t c = 0; c < 3; c++)
				trans[i][j] += _info->rgb_cam[i][c] * last[c][j] * div[j];
	for (size_t c = 0; c < 3; c++)
		trsum[c] = trans[c][0] + trans[c][1] + trans[c][2];
	dsum = (6 * trsum[0] + 11 * trsum[1] + 3 * trsum[2]) / 20;
	for (int i = 0; i < 3; i++)
		for (size_t c = 0; c < 3; c++)
			last[i][c] = trans[i][c] * dsum / trsum[i];
	memset(trans, 0, sizeof trans);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			for (size_t c = 0; c < 3; c++)
				trans[i][j] += (i == c ? 32 : -1) * last[c][j] / 30;

	short *curve[8];
	for (size_t c = 0; c < 8; c++)
		curve[c] = nullptr;
	CAutoFreeMemory autoFreeCurve0(curve[0]);
	CAutoFreeMemory autoFreeCurve1(curve[1]);
	CAutoFreeMemory autoFreeCurve2(curve[2]);
	CAutoFreeMemory autoFreeCurve3(curve[3]);
	CAutoFreeMemory autoFreeCurve4(curve[4]);
	CAutoFreeMemory autoFreeCurve5(curve[5]);
	CAutoFreeMemory autoFreeCurve6(curve[6]);
	CAutoFreeMemory autoFreeCurve7(curve[7]);
	foveon_make_curves(curve, color_dq, div, cfilt);
	for (size_t c = 0; c < 3; c++)
		chroma_dq[c] /= 3;
	foveon_make_curves(curve + 3, chroma_dq, div, cfilt);
	for (size_t c = 0; c < 3; c++)
		dsum += chroma_dq[c] / div[c];
	curve[6] = foveon_make_curve(dsum, dsum, cfilt);
	curve[7] = foveon_make_curve(dsum * 2, dsum * 2, cfilt);

	float(*sgain)[3] = (float(*)[3]) foveon_camf_matrix(dim, "SpatialGain");
	if (!sgain)
		return;
	CAutoFreeMemory autoFreeSgain(sgain);
	float(*sgrow)[3] = (float(*)[3]) calloc(dim[1], sizeof *sgrow);
	CAutoFreeMemory autoFreeSgrow(sgrow);
	int sgx = (_info->width + dim[1] - 2) / (dim[1] - 1);

	float(*black)[3] = (float(*)[3]) calloc(_info->height, sizeof *black);
	CAutoFreeMemory autoFreeBlack(black);
	for (row = 0; row < _info->height; row++)
	{
		for (int i = 0; i < 6; i++)
			((float *)ddft[0])[i] = ((float *)ddft[1])[i] + row / (_info->height - 1.0) * (((float *)ddft[2])[i] - ((float *)ddft[1])[i]);
		for (size_t c = 0; c < 3; c++)
			black[row][c] = (foveon_avg(image[row*_info->width] + c, dscr[0], cfilt) +
				foveon_avg(image[row*_info->width] + c, dscr[1], cfilt) * 3
				- ddft[0][c][0]) / 4 - ddft[0][c][1];
	}
	memcpy(black, black + 8, sizeof *black * 8);
	memcpy(black + _info->height - 11, black + _info->height - 22, 11 * sizeof *black);
	memcpy(last, black, sizeof last);

	for (row = 1; row < _info->height - 1; row++)
	{
		for (size_t c = 0; c < 3; c++)
		{
			if (last[1][c] > last[0][c])
			{
				if (last[1][c] > last[2][c])
					black[row][c] = (last[0][c] > last[2][c]) ? last[0][c] : last[2][c];
			}
			else
			{
				if (last[1][c] < last[2][c])
					black[row][c] = (last[0][c] < last[2][c]) ? last[0][c] : last[2][c];
			}
		}
		memmove(last, last + 1, 2 * sizeof last[0]);
		memcpy(last[2], black[row + 1], sizeof last[2]);
	}
	for (size_t c = 0; c < 3; c++)
		black[row][c] = (last[0][c] + last[1][c]) / 2;
	for (size_t c = 0; c < 3; c++)
		black[0][c] = (black[1][c] + black[3][c]) / 2;

	float val = 1 - exp(-1 / 24.0);
	memcpy(fsum, black, sizeof fsum);
	for (row = 1; row < _info->height; row++)
		for (size_t c = 0; c < 3; c++)
			fsum[c] += black[row][c] = (black[row][c] - black[row - 1][c])*val + black[row - 1][c];
	memcpy(last[0], black[_info->height - 1], sizeof last[0]);
	for (size_t c = 0; c < 3; c++)
		fsum[c] /= _info->height;
	for (row = _info->height; row--; )
		for (size_t c = 0; c < 3; c++)
			last[0][c] = black[row][c] = (black[row][c] - fsum[c] - last[0][c])*val + last[0][c];

	memset(total, 0, sizeof total);
	for (row = 2; row < _info->height; row += 4)
	{
		for (col = 2; col < _info->width; col += 4)
		{
			for (size_t c = 0; c < 3; c++)
				total[c] += (short)image[row*_info->width + col][c];
			total[3]++;
		}
	}
	for (row = 0; row < _info->height; row++)
		for (size_t c = 0; c < 3; c++)
			black[row][c] += fsum[c] / 2 + total[c] / (total[3] * 100.0);

	for (row = 0; row < _info->height; row++)
	{
		for (int i = 0; i < 6; i++)
			((float *)ddft[0])[i] = ((float *)ddft[1])[i] +
			row / (_info->height - 1.0) * (((float *)ddft[2])[i] - ((float *)ddft[1])[i]);
		short* pix = image[row*_info->width];
		memcpy(prev, pix, sizeof prev);
		float frow = row / (_info->height - 1.0) * (dim[2] - 1);
		int irow = frow;
		if (irow == dim[2] - 1)
			irow--;
		frow -= irow;
		for (int i = 0; i < dim[1]; i++)
			for (size_t c = 0; c < 3; c++)
				sgrow[i][c] = sgain[irow   *dim[1] + i][c] * (1 - frow) + sgain[(irow + 1)*dim[1] + i][c] * frow;
		for (col = 0; col < _info->width; col++)
		{
			for (size_t c = 0; c < 3; c++)
			{
				int diff = pix[c] - prev[c];
				prev[c] = pix[c];
				ipix[c] = pix[c] + floor((diff + (diff*diff >> 14)) * cfilt
					- ddft[0][c][1] - ddft[0][c][0] * ((float)col / _info->width - 0.5)
					- black[row][c]);
			}
			for (size_t c = 0; c < 3; c++)
			{
				work[0][c] = ipix[c] * ipix[c] >> 14;
				work[2][c] = ipix[c] * work[0][c] >> 14;
				work[1][2 - c] = ipix[(c + 1) % 3] * ipix[(c + 2) % 3] >> 14;
			}
			for (size_t c = 0; c < 3; c++)
			{
				val = 0;
				for (int i = 0; i < 3; i++)
					for (int j = 0; j < 3; j++)
						val += ppm[c][i][j] * work[i][j];
				ipix[c] = floor((ipix[c] + floor(val)) *
					(sgrow[col / sgx][c] * (sgx - col%sgx) +
						sgrow[col / sgx + 1][c] * (col%sgx)) / sgx / div[c]);
				if (ipix[c] > 32000) ipix[c] = 32000;
				pix[c] = ipix[c];
			}
			pix += 4;
		}
	}
	autoFreeSgrow.Release();
	autoFreeSgain.Release();

	unsigned* badpix = (unsigned *)foveon_camf_matrix(dim, "BadPixels");
	if (badpix)
	{
		CAutoFreeMemory autoFreeBadPix(badpix);
		for (int i = 0; i < dim[0]; i++)
		{
			col = (badpix[i] >> 8 & 0xfff) - keep[0];
			row = (badpix[i] >> 20) - keep[1];
			if ((unsigned)(row - 1) > _info->height - 3 || (unsigned)(col - 1) > _info->width - 3)
				continue;
			memset(fsum, 0, sizeof fsum);
			int sum = 0;
			for (int j = 0; j < 8; j++)
			{
				if (badpix[i] & (1 << j))
				{
					for (size_t c = 0; c < 3; c++)
						fsum[c] += (short)image[(row + hood[j * 2])*_info->width + col + hood[j * 2 + 1]][c];
					sum++;
				}
			}
			if (sum)
				for (size_t c = 0; c < 3; c++)
					image[row*_info->width + col][c] = fsum[c] / sum;
		}
	}

	/* Array for 5x5 Gaussian averaging of red values */
	smrow[6] = (int(*)[3]) calloc(_info->width * 5, sizeof **smrow);
	CAutoFreeMemory autoFreeSmrow(smrow[6]);
	for (int i = 0; i < 5; i++)
		smrow[i] = smrow[6] + i*_info->width;

	/* Sharpen the reds against these Gaussian averages */
	int smlast = -1;
	for (row = 2; row < _info->height - 2; row++)
	{
		while (smlast < row + 2)
		{
			for (int i = 0; i < 6; i++)
				smrow[(i + 5) % 6] = smrow[i];
			short* pix = image[++smlast*_info->width + 2];
			for (col = 2; col < _info->width - 2; col++)
			{
				smrow[4][col][0] =
					(pix[0] * 6 + (pix[-4] + pix[4]) * 4 + pix[-8] + pix[8] + 8) >> 4;
				pix += 4;
			}
		}
		short* pix = image[row*_info->width + 2];
		for (col = 2; col < _info->width - 2; col++)
		{
			int smred = (6 * smrow[2][col][0]
				+ 4 * (smrow[1][col][0] + smrow[3][col][0])
				+ smrow[0][col][0] + smrow[4][col][0] + 8) >> 4;
			if (col == 2)
				smred_p = smred;
			int i = pix[0] + ((pix[0] - ((smred * 7 + smred_p) >> 3)) >> 3);
			if (i > 32000)
				i = 32000;
			pix[0] = i;
			smred_p = smred;
			pix += 4;
		}
	}

	/* Adjust the brighter pixels for better linearity */
	int min = 0xffff;
	for (size_t c = 0; c < 3; c++)
	{
		int i = satlev[c] / div[c];
		if (min > i)
			min = i;
	}
	int limit = min * 9 >> 4;
	for (short* pix = image[0]; pix < image[_info->height*_info->width]; pix += 4)
	{
		if (pix[0] <= limit || pix[1] <= limit || pix[2] <= limit)
			continue;
		int max = pix[0];
		min = pix[0];
		for (size_t c = 1; c < 3; c++)
		{
			if (min > pix[c])
				min = pix[c];
			if (max < pix[c])
				max = pix[c];
		}
		if (min >= limit * 2)
		{
			pix[0] = pix[1] = pix[2] = max;
		}
		else
		{
			int i = 0x4000 - ((min - limit) << 14) / limit;
			i = 0x4000 - (i*i >> 14);
			i = i*i >> 14;
			for (size_t c = 0; c < 3; c++)
				pix[c] += (max - pix[c]) * i >> 14;
		}
	}
	/*
	Because photons that miss one detector often hit another,
	the sum R+G+B is much less noisy than the individual colors.
	So smooth the hues without smoothing the total.
	*/
	smlast = -1;
	for (row = 2; row < _info->height - 2; row++)
	{
		while (smlast < row + 2)
		{
			for (int i = 0; i < 6; i++)
				smrow[(i + 5) % 6] = smrow[i];
			short* pix = image[++smlast*_info->width + 2];
			for (col = 2; col < _info->width - 2; col++)
			{
				for (size_t c = 0; c < 3; c++)
					smrow[4][col][c] = (pix[c - 4] + 2 * pix[c] + pix[c + 4] + 2) >> 2;
				pix += 4;
			}
		}
		short* pix = image[row*_info->width + 2];
		for (col = 2; col < _info->width - 2; col++)
		{
			for (size_t c = 0; c < 3; c++)
				dev[c] = -foveon_apply_curve(curve[7], pix[c] - ((smrow[1][col][c] + 2 * smrow[2][col][c] + smrow[3][col][c]) >> 2));
			int sum = (dev[0] + dev[1] + dev[2]) >> 3;
			for (size_t c = 0; c < 3; c++)
				pix[c] += dev[c] - sum;
			pix += 4;
		}
	}

	smlast = -1;
	for (row = 2; row < _info->height - 2; row++)
	{
		while (smlast < row + 2)
		{
			for (int i = 0; i < 6; i++)
				smrow[(i + 5) % 6] = smrow[i];
			short* pix = image[++smlast*_info->width + 2];
			for (col = 2; col < _info->width - 2; col++)
			{
				for (size_t c = 0; c < 3; c++)
					smrow[4][col][c] = (pix[c - 8] + pix[c - 4] + pix[c] + pix[c + 4] + pix[c + 8] + 2) >> 2;
				pix += 4;
			}
		}
		short* pix = image[row*_info->width + 2];
		for (col = 2; col < _info->width - 2; col++)
		{
			total[3] = 375;
			int sum = 60;
			for (size_t c = 0; c < 3; c++)
			{
				total[c] = 0;
				for (int i = 0; i < 5; i++)
					total[c] += smrow[i][col][c];
				total[3] += total[c];
				sum += pix[c];
			}
			if (sum < 0)
				sum = 0;
			int j = total[3] > 375 ? (sum << 16) / total[3] : sum * 174;
			for (size_t c = 0; c < 3; c++)
				pix[c] += foveon_apply_curve(curve[6], ((j*total[c] + 0x8000) >> 16) - pix[c]);
			pix += 4;
		}
	}

	/* Transform the image to a different colorspace */
	for (short* pix = image[0]; pix < image[_info->height*_info->width]; pix += 4)
	{
		for (size_t c = 0; c < 3; c++)
			pix[c] -= foveon_apply_curve(curve[c], pix[c]);
		int sum = (pix[0] + pix[1] + pix[1] + pix[2]) >> 2;
		for (size_t c = 0; c < 3; c++)
			pix[c] -= foveon_apply_curve(curve[c], pix[c] - sum);
		for (size_t c = 0; c < 3; c++) 
		{
			dsum = 0;
			for (int i = 0; i < 3; i++)
				dsum += trans[c][i] * pix[i];
			if (dsum < 0)
				dsum = 0;
			if (dsum > 24000)
				dsum = 24000;
			ipix[c] = dsum + 0.5;
		}
		for (size_t c = 0; c < 3; c++)
			pix[c] = ipix[c];
	}

	/* Smooth the image bottom-to-top and save at 1/4 scale */
	short(*shrink)[3] = (short(*)[3]) calloc((_info->height / 4), (_info->width / 4) * sizeof *shrink);
	CAutoFreeMemory autoFreeShrink(shrink);
	for (row = _info->height / 4; row--; )
		for (col = 0; col < _info->width / 4; col++)
		{
			ipix[0] = ipix[1] = ipix[2] = 0;
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					for (size_t c = 0; c < 3; c++)
						ipix[c] += image[(row * 4 + i)*_info->width + col * 4 + j][c];
			for (size_t c = 0; c < 3; c++)
			{
				if (row + 2 > _info->height / 4)
					shrink[row*(_info->width / 4) + col][c] = ipix[c] >> 4;
				else
					shrink[row*(_info->width / 4) + col][c] = (shrink[(row + 1)*(_info->width / 4) + col][c] * 1840 + ipix[c] * 141 + 2048) >> 12;
			}
		}
	/* From the 1/4-scale image, smooth right-to-left */
	for (row = 0; row < (_info->height & ~3); row++)
	{
		ipix[0] = ipix[1] = ipix[2] = 0;
		if ((row & 3) == 0)
			for (col = _info->width & ~3; col--; )
				for (size_t c = 0; c < 3; c++)
					smrow[0][col][c] = ipix[c] = (shrink[(row / 4)*(_info->width / 4) + col / 4][c] * 1485 + ipix[c] * 6707 + 4096) >> 13;

		/* Then smooth left-to-right */
		ipix[0] = ipix[1] = ipix[2] = 0;
		for (col = 0; col < (_info->width & ~3); col++)
			for (size_t c = 0; c < 3; c++)
				smrow[1][col][c] = ipix[c] = (smrow[0][col][c] * 1485 + ipix[c] * 6707 + 4096) >> 13;

		/* Smooth top-to-bottom */
		if (row == 0)
			memcpy(smrow[2], smrow[1], sizeof **smrow * _info->width);
		else
			for (col = 0; col < (_info->width & ~3); col++)
				for (size_t c = 0; c < 3; c++)
					smrow[2][col][c] = (smrow[2][col][c] * 6707 + smrow[1][col][c] * 1485 + 4096) >> 13;

		/* Adjust the chroma toward the smooth values */
		for (col = 0; col < (_info->width & ~3); col++)
		{
			int j = 30;
			int i = 30;
			for (size_t c = 0; c < 3; c++)
			{
				i += smrow[2][col][c];
				j += image[row*_info->width + col][c];
			}
			j = (j << 16) / i;
			int sum = 0;
			for (size_t c = 0; c < 3; c++) {
				ipix[c] = foveon_apply_curve(curve[c + 3],
					((smrow[2][col][c] * j + 0x8000) >> 16) - image[row*_info->width + col][c]);
				sum += ipix[c];
			}
			sum >>= 3;
			for (size_t c = 0; c < 3; c++)
			{
				i = image[row*_info->width + col][c] + ipix[c] - sum;
				if (i < 0)
					i = 0;
				image[row*_info->width + col][c] = i;
			}
		}
	}

	/* Trim off the black border */
	active[1] -= keep[1];
	active[3] -= 2;
	int i = active[2] - active[0];
	for (row = 0; row < active[3] - active[1]; row++)
		memcpy(image[row*i], image[(row + active[1])*_info->width + active[0]],
			i * sizeof *image);
	_info->width = i;
	_info->height = row;
}
#undef image

void CImageLoader::foveon_thumb()
{
	unsigned bwide = _reader->get4();
	_writer->Print("P6\n%d %d\n255\n", _info->thumb_width, _info->thumb_height);
	if (bwide > 0)
	{
		if (bwide < _info->thumb_width * 3)
			return;
		char* buf = (char *)malloc(bwide);
		CAutoFreeMemory autoFree(buf);
		for (unsigned row = 0; row < _info->thumb_height; row++)
		{
			_reader->Read(buf, 1, bwide);
			_writer->Write(buf, 3, _info->thumb_width);
		}
		return;
	}
	foveon_decoder(256, 0);

	unsigned bitbuf = 0;
	unsigned bit = 1;
	decode* dindex;
	short pred[3];
	for (unsigned row = 0; row < _info->thumb_height; row++)
	{
		memset(pred, 0, sizeof pred);
		if (!bit)
			_reader->get4();
		bit = 0;
		for (unsigned col = 0; col < _info->thumb_width; col++)
		{
			for (size_t c = 0; c < 3; c++)
			{
				for (dindex = first_decode; dindex->branch[0]; )
				{
					if ((bit = (bit - 1) & 31) == 31)
						for (size_t i = 0; i < 4; i++)
							bitbuf = (bitbuf << 8) + _reader->GetChar();
					dindex = dindex->branch[bitbuf >> bit & 1];
				}
				pred[c] += dindex->leaf;
				_writer->PutChar(pred[c]);
			}
		}
	}
}

/* RESTRICTED code ends here */





















void CImageLoader::ppm_thumb()
{
	_info->thumb_length = _info->thumb_width*_info->thumb_height * 3;
	char* thumb = (char *)malloc(_info->thumb_length);
	CAutoFreeMemory autoFree(thumb);
	_writer->Print("P6\n%d %d\n255\n", _info->thumb_width, _info->thumb_height);
	_reader->Read(thumb, 1, _info->thumb_length);
	_writer->Write(thumb, 1, _info->thumb_length);
}

void CImageLoader::ppm16_thumb()
{
	_info->thumb_length = _info->thumb_width*_info->thumb_height * 3;
	char* thumb = (char *)calloc(_info->thumb_length, 2);
	CAutoFreeMemory autoFree(thumb);
	_reader->read_shorts((unsigned short *)thumb, _info->thumb_length);
	for (int i = 0; i < _info->thumb_length; i++)
		thumb[i] = ((unsigned short *)thumb)[i] >> 8;
	_writer->Print("P6\n%d %d\n255\n", _info->thumb_width, _info->thumb_height);
	_writer->Write(thumb, 1, _info->thumb_length);
}

void CImageLoader::layer_thumb()
{
	char map[][4] = { "012","102" };

	_info->colors = _info->thumb_misc >> 5 & 7;
	_info->thumb_length = _info->thumb_width*_info->thumb_height;
	char* thumb = (char *)calloc(_info->colors, _info->thumb_length);
	CAutoFreeMemory autoFree(thumb);
	_writer->Print("P%d\n%d %d\n255\n", 5 + (_info->colors >> 1), _info->thumb_width, _info->thumb_height);
	_reader->Read(thumb, _info->thumb_length, _info->colors);
	for (int i = 0; i < _info->thumb_length; i++)
		for (size_t c = 0; c < _info->colors; c++)
			_writer->PutChar(thumb[i + _info->thumb_length*(map[_info->thumb_misc >> 8][c] - '0')]);
}

void CImageLoader::rollei_thumb()
{
	_info->thumb_length = _info->thumb_width * _info->thumb_height;
	unsigned short* thumb = (unsigned short *)calloc(_info->thumb_length, 2);
	CAutoFreeMemory autoFree(thumb);
	_writer->Print("P6\n%d %d\n255\n", _info->thumb_width, _info->thumb_height);
	_reader->read_shorts(thumb, _info->thumb_length);
	for (size_t i = 0; i < _info->thumb_length; i++)
	{
		_writer->PutChar(thumb[i] << 3);
		_writer->PutChar(thumb[i] >> 5 << 2);
		_writer->PutChar(thumb[i] >> 11 << 3);
	}
}



































void CImageLoader::median_filter()
{
	unsigned short(*pix)[4];
	int pass, c, i, j, k, med[9];
	static const unsigned char opt[] =	/* Optimal 9-element median search */
	{ 1,2, 4,5, 7,8, 0,1, 3,4, 6,7, 1,2, 4,5, 7,8,
		0,3, 5,8, 4,7, 3,6, 1,4, 2,5, 4,7, 4,2, 6,4, 4,2 };

	for (pass = 1; pass <= _options->med_passes; pass++)
	{
		for (c = 0; c < 3; c += 2)
		{
			for (pix = _info->image; pix < _info->image + _info->width*_info->height; pix++)
				pix[0][3] = pix[0][c];
			for (pix = _info->image + _info->width; pix < _info->image + _info->width*(_info->height - 1); pix++)
			{
				if ((pix - _info->image + 1) % _info->width < 2)
					continue;
				for (k = 0, i = -_info->width; i <= _info->width; i += _info->width)
					for (j = i - 1; j <= i + 1; j++)
						med[k++] = pix[j][3] - pix[j][1];
				for (i = 0; i < sizeof opt; i += 2)
					if (med[opt[i]] > med[opt[i + 1]])
						SWAP(med[opt[i]], med[opt[i + 1]]);
				pix[0][c] = CLIP(med[4] + pix[0][1]);
			}
		}
	}
}

void CImageLoader::blend_highlights()
{
	int clip = INT_MAX, row, col, i, j;
	static const float trans[2][4][4] =
	{ { { 1,1,1 },{ 1.7320508,-1.7320508,0 },{ -1,-1,2 } },
	{ { 1,1,1,1 },{ 1,-1,1,-1 },{ 1,1,-1,-1 },{ 1,-1,-1,1 } } };
	static const float itrans[2][4][4] =
	{ { { 1,0.8660254,-0.5 },{ 1,-0.8660254,-0.5 },{ 1,0,1 } },
	{ { 1,1,1,1 },{ 1,-1,1,-1 },{ 1,1,-1,-1 },{ 1,-1,-1,1 } } };
	float cam[2][4], lab[2][4], sum[2], chratio;

	if ((unsigned)(_info->colors - 3) > 1)
		return;
	for (size_t c = 0; c < _info->colors; c++)
		if (clip > (i = 65535 * _info->pre_mul[c]))
			clip = i;
	for (row = 0; row < _info->height; row++)
	{
		for (col = 0; col < _info->width; col++)
		{
			int c;
			for (c = 0; c < _info->colors; c++)
				if (_info->image[row*_info->width + col][c] > clip)
					break;
			if (c == _info->colors)
				continue;
			for (size_t c = 0; c < _info->colors; c++)
			{
				cam[0][c] = _info->image[row*_info->width + col][c];
				cam[1][c] = MIN(cam[0][c], clip);
			}
			for (i = 0; i < 2; i++)
			{
				for (size_t c = 0; c < _info->colors; c++)
					for (lab[i][c] = j = 0; j < _info->colors; j++)
						lab[i][c] += trans[_info->colors - 3][c][j] * cam[i][j];
				sum[i] = 0;
				for (size_t c = 1; c < _info->colors; c++)
					sum[i] += SQR(lab[i][c]);
			}
			chratio = sqrt(sum[1] / sum[0]);
			for (c = 1; c < _info->colors; c++)
				lab[0][c] *= chratio;
			for (size_t c = 0; c < _info->colors; c++)
				for (cam[0][c] = j = 0; j < _info->colors; j++)
					cam[0][c] += itrans[_info->colors - 3][c][j] * lab[0][j];
			for (size_t c = 0; c < _info->colors; c++)
				_info->image[row*_info->width + col][c] = cam[0][c] / _info->colors;
		}
	}
}

#define SCALE (4 >> _info->shrink)
void CImageLoader::recover_highlights()
{
	float sum, wgt, grow;
	int hsat[4], count, spread, change, val, i;
	unsigned high, wide, mrow, mcol, row, col, kc, c, d, y, x;
	unsigned short *pixel;
	static const signed char dir[8][2] =
	{ { -1,-1 },{ -1,0 },{ -1,1 },{ 0,1 },{ 1,1 },{ 1,0 },{ 1,-1 },{ 0,-1 } };

	grow = pow(2, 4 - _options->highlight);
	for (size_t c = 0; c < _info->colors; c++)
		hsat[c] = 32000 * _info->pre_mul[c];
	kc = 0;
	for (size_t c = 1; c < _info->colors; c++)
		if (_info->pre_mul[kc] < _info->pre_mul[c])
			kc = c;
	high = _info->height / SCALE;
	wide = _info->width / SCALE;
	float* map = (float *)calloc(high, wide * sizeof *map);
	CAutoFreeMemory autoFree(map);
	for (size_t c = 0; c < _info->colors; c++)
	{
		if (c != kc)
		{
			memset(map, 0, high*wide * sizeof *map);
			for (mrow = 0; mrow < high; mrow++)
			{
				for (mcol = 0; mcol < wide; mcol++)
				{
					count = 0;
					wgt = 0;
					sum = 0;
					for (row = mrow*SCALE; row < (mrow + 1)*SCALE; row++)
					{
						for (col = mcol*SCALE; col < (mcol + 1)*SCALE; col++)
						{
							pixel = _info->image[row*_info->width + col];
							if (pixel[c] / hsat[c] == 1 && pixel[kc] > 24000)
							{
								sum += pixel[c];
								wgt += pixel[kc];
								count++;
							}
						}
					}
					if (count == SCALE*SCALE)
						map[mrow*wide + mcol] = sum / wgt;
				}
			}
			for (spread = 32 / grow; spread--; )
			{
				for (mrow = 0; mrow < high; mrow++)
				{
					for (mcol = 0; mcol < wide; mcol++)
					{
						if (map[mrow*wide + mcol])
							continue;
						count = 0;
						sum = 0;
						for (d = 0; d < 8; d++)
						{
							y = mrow + dir[d][0];
							x = mcol + dir[d][1];
							if (y < high && x < wide && map[y*wide + x] > 0)
							{
								sum += (1 + (d & 1)) * map[y*wide + x];
								count += 1 + (d & 1);
							}
						}
						if (count > 3)
							map[mrow*wide + mcol] = -(sum + grow) / (count + grow);
					}
				}
				for (change = i = 0; i < high*wide; i++)
				{
					if (map[i] < 0)
					{
						map[i] = -map[i];
						change = 1;
					}
				}
				if (!change)
					break;
			}
			for (i = 0; i < high*wide; i++)
				if (map[i] == 0) map[i] = 1;
			for (mrow = 0; mrow < high; mrow++)
			{
				for (mcol = 0; mcol < wide; mcol++)
				{
					for (row = mrow*SCALE; row < (mrow + 1)*SCALE; row++)
					{
						for (col = mcol*SCALE; col < (mcol + 1)*SCALE; col++)
						{
							pixel = _info->image[row*_info->width + col];
							if (pixel[c] / hsat[c] > 1)
							{
								val = pixel[kc] * map[mrow*wide + mcol];
								if (pixel[c] < val) pixel[c] = CLIP(val);
							}
						}
					}
				}
			}
		}
	}
}
#undef SCALE

#ifndef NO_LCMS
void CImageLoader::apply_profile(const char *input, const char *output)
{
	char *prof;
	cmsHPROFILE hInProfile = 0;
	cmsHPROFILE hOutProfile = 0;
	cmsHTRANSFORM hTransform;
	FILE *fp;
	unsigned size;

	if (strcmp(input, "embed"))
	{
		hInProfile = cmsOpenProfileFromFile(input, "r");
	}
	else if (_info->profile_length)
	{
		prof = (char *)malloc(_info->profile_length);
		CAutoFreeMemory autoFree(prof);
		_reader->Seek(_info->profile_offset, SEEK_SET);
		_reader->Read(prof, 1, _info->profile_length);
		hInProfile = cmsOpenProfileFromMem(prof, _info->profile_length);
	}
	else
	{
		fprintf(stderr, ("%s has no embedded profile.\n"), _reader->GetFileName());
	}
	if (!hInProfile)
		return;
	if (!output)
		hOutProfile = cmsCreate_sRGBProfile();
	else if ((fp = fopen(output, "rb")))
	{
		fread(&size, 4, 1, fp);
		fseek(fp, 0, SEEK_SET);
		_info->oprof = (unsigned *)malloc(size = ntohl(size));
		if (!_info->oprof)
			throw CExceptionMemory();
		fread(_info->oprof, 1, size, fp);
		fclose(fp);
		if (!(hOutProfile = cmsOpenProfileFromMem(_info->oprof, size)))
		{
			free(_info->oprof);
			_info->oprof = nullptr;
		}
	}
	else
	{
		fprintf(stderr, ("Cannot open file %s!\n"), output);
	}
	if (!hOutProfile)
		goto quit;
	hTransform = cmsCreateTransform(hInProfile, TYPE_RGBA_16, hOutProfile, TYPE_RGBA_16, INTENT_PERCEPTUAL, 0);
	cmsDoTransform(hTransform, _info->image, _info->image, _info->width*_info->height);
	_info->raw_color = 1;		/* Don't use rgb_cam with a profile */
	cmsDeleteTransform(hTransform);
	cmsCloseProfile(hOutProfile);
quit:
	cmsCloseProfile(hInProfile);
}
#endif

void CImageLoader::convert_to_rgb()
{
	static const double xyzd50_srgb[3][3] =
	{
		{ 0.436083, 0.385083, 0.143055 },
		{ 0.222507, 0.716888, 0.060608 },
		{ 0.013930, 0.097097, 0.714022 }
	};
	static const double rgb_rgb[3][3] =
	{
		{ 1,0,0 },{ 0,1,0 },{ 0,0,1 }
	};
	static const double adobe_rgb[3][3] =
	{
		{ 0.715146, 0.284856, 0.000000 },
		{ 0.000000, 1.000000, 0.000000 },
		{ 0.000000, 0.041166, 0.958839 }
	};
	static const double wide_rgb[3][3] =
	{
		{ 0.593087, 0.404710, 0.002206 },
		{ 0.095413, 0.843149, 0.061439 },
		{ 0.011621, 0.069091, 0.919288 }
	};
	static const double prophoto_rgb[3][3] =
	{
		{ 0.529317, 0.330092, 0.140588 },
		{ 0.098368, 0.873465, 0.028169 },
		{ 0.016879, 0.117663, 0.865457 }
	};
	static const double aces_rgb[3][3] =
	{
		{ 0.432996, 0.375380, 0.189317 },
		{ 0.089427, 0.816523, 0.102989 },
		{ 0.019165, 0.118150, 0.941914 }
	};
	static const double(*out_rgb[])[3] =
	{ rgb_rgb, adobe_rgb, wide_rgb, prophoto_rgb, _info->xyz_rgb, aces_rgb };
	static const char *name[] =
	{ "sRGB", "Adobe RGB (1998)", "WideGamut D65", "ProPhoto D65", "XYZ", "ACES" };
	static const unsigned phead[] =
	{
		1024, 0, 0x2100000, 0x6d6e7472, 0x52474220, 0x58595a20, 0, 0, 0,
		0x61637370, 0, 0, 0x6e6f6e65, 0, 0, 0, 0, 0xf6d6, 0x10000, 0xd32d
	};
	static const unsigned pwhite[] = { 0xf351, 0x10000, 0x116cc };

	unsigned pbody[] =
	{ 10, 0x63707274, 0, 36,	/* cprt */
		0x64657363, 0, 40,	/* desc */
		0x77747074, 0, 20,	/* wtpt */
		0x626b7074, 0, 20,	/* bkpt */
		0x72545243, 0, 14,	/* rTRC */
		0x67545243, 0, 14,	/* gTRC */
		0x62545243, 0, 14,	/* bTRC */
		0x7258595a, 0, 20,	/* rXYZ */
		0x6758595a, 0, 20,	/* gXYZ */
		0x6258595a, 0, 20	/* bXYZ */
	};
	unsigned pcurve[] = { 0x63757276, 0, 1, 0x1000000 };
	int row, col, i, j, k;
	unsigned short *img;
	float out[3], out_cam[3][4];
	double num, inverse[3][3];

	_info->gamma_curve(_info->gamm[0], _info->gamm[1], 0, 0);
	memcpy(out_cam, _info->rgb_cam, sizeof out_cam);
	_info->raw_color |= _info->colors == 1 || _options->document_mode ||
		_options->output_color < 1 || _options->output_color > 6;
	if (!_info->raw_color)
	{
		_info->oprof = (unsigned *)calloc(phead[0], 1);
		if (!_info->oprof)
			throw CExceptionMemory();

		memcpy(_info->oprof, phead, sizeof phead);
		if (_options->output_color == 5)
			_info->oprof[4] = _info->oprof[5];
		_info->oprof[0] = 132 + 12 * pbody[0];
		for (i = 0; i < pbody[0]; i++)
		{
			_info->oprof[_info->oprof[0] / 4] = i ? (i > 1 ? 0x58595a20 : 0x64657363) : 0x74657874;
			pbody[i * 3 + 2] = _info->oprof[0];
			_info->oprof[0] += (pbody[i * 3 + 3] + 3) & -4;
		}
		memcpy(_info->oprof + 32, pbody, sizeof pbody);
		_info->oprof[pbody[5] / 4 + 2] = strlen(name[_options->output_color - 1]) + 1;
		memcpy((char *)_info->oprof + pbody[8] + 8, pwhite, sizeof pwhite);
		pcurve[3] = (short)(256 / _info->gamm[5] + 0.5) << 16;
		for (i = 4; i < 7; i++)
			memcpy((char *)_info->oprof + pbody[i * 3 + 2], pcurve, sizeof pcurve);
		pseudoinverse((double(*)[3]) out_rgb[_options->output_color - 1], inverse, 3);
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				for (num = k = 0; k < 3; k++)
					num += xyzd50_srgb[i][k] * inverse[j][k];
				_info->oprof[pbody[j * 3 + 23] / 4 + i + 2] = num * 0x10000 + 0.5;
			}
		}
		for (i = 0; i < phead[0] / 4; i++)
			_info->oprof[i] = htonl(_info->oprof[i]);
		strcpy((char *)_info->oprof + pbody[2] + 8, "auto-generated by dcraw");
		strcpy((char *)_info->oprof + pbody[5] + 12, name[_options->output_color - 1]);
		for (i = 0; i < 3; i++)
			for (j = 0; j < _info->colors; j++)
				for (out_cam[i][j] = k = 0; k < 3; k++)
					out_cam[i][j] += out_rgb[_options->output_color - 1][i][k] * _info->rgb_cam[k][j];
	}

	memset(_info->histogram, 0, sizeof _info->histogram);
	for (img = _info->image[0], row = 0; row < _info->height; row++)
	{
		for (col = 0; col < _info->width; col++, img += 4)
		{
			if (!_info->raw_color)
			{
				out[0] = out[1] = out[2] = 0;
				for (size_t c = 0; c < _info->colors; c++)
				{
					out[0] += out_cam[0][c] * img[c];
					out[1] += out_cam[1][c] * img[c];
					out[2] += out_cam[2][c] * img[c];
				}
				for (size_t c = 0; c < 3; c++)
					img[c] = CLIP((int)out[c]);
			}
			else if (_options->document_mode)
			{
				img[0] = img[fcol(row, col)];
			}
			for (size_t c = 0; c < _info->colors; c++)
				_info->histogram[c][img[c] >> 3]++;
		}
	}
	if (_info->colors == 4 && _options->output_color)
		_info->colors = 3;
	if (_options->document_mode && _info->filters)
		_info->colors = 1;
}

void CImageLoader::fuji_rotate()
{
	int i, row, col;
	double step;
	float r, c, fr, fc;
	unsigned ur, uc;
	unsigned short wide, high, (*img)[4], (*pix)[4];

	if (!_info->fuji_width)
		return;
	_info->fuji_width = (_info->fuji_width - 1 + _info->shrink) >> _info->shrink;
	step = sqrt(0.5);
	wide = _info->fuji_width / step;
	high = (_info->height - _info->fuji_width) / step;
	img = (unsigned short(*)[4]) calloc(high, wide * sizeof *img);
	if (!img)
		throw CExceptionMemory();

	for (row = 0; row < high; row++)
	{
		for (col = 0; col < wide; col++)
		{
			ur = r = _info->fuji_width + (row - col)*step;
			uc = c = (row + col)*step;
			if (ur > _info->height - 2 || uc > _info->width - 2)
				continue;
			fr = r - ur;
			fc = c - uc;
			pix = _info->image + ur*_info->width + uc;
			for (i = 0; i < _info->colors; i++)
				img[row*wide + col][i] = (pix[0][i] * (1 - fc) + pix[1][i] * fc) * (1 - fr) + (pix[_info->width][i] * (1 - fc) + pix[_info->width + 1][i] * fc) * fr;
		}
	}
	free(_info->image);
	_info->width = wide;
	_info->height = high;
	_info->image = img;
	_info->fuji_width = 0;
}

void CImageLoader::stretch()
{
	unsigned short newdim, (*img)[4], *pix0, *pix1;
	int row, col;
	double rc, frac;

	if (_info->pixel_aspect == 1)
		return;
	if (_info->pixel_aspect < 1)
	{
		newdim = _info->height / _info->pixel_aspect + 0.5;
		img = (unsigned short(*)[4]) calloc(_info->width, newdim * sizeof *img);
		if (!img)
			throw CExceptionMemory();
		rc = 0;
		for (row = 0; row < newdim; row++, rc += _info->pixel_aspect)
		{
			int c = rc;
			frac = rc - c;
			pix0 = pix1 = _info->image[c*_info->width];
			if (c + 1 < _info->height)
				pix1 += _info->width * 4;
			for (col = 0; col < _info->width; col++, pix0 += 4, pix1 += 4)
				for (size_t c = 0; c < _info->colors; c++)
					img[row*_info->width + col][c] = pix0[c] * (1 - frac) + pix1[c] * frac + 0.5;
		}
		_info->height = newdim;
	}
	else
	{
		newdim = _info->width * _info->pixel_aspect + 0.5;
		img = (unsigned short(*)[4]) calloc(_info->height, newdim * sizeof *img);
		if (!img)
			throw CExceptionMemory();
		for (rc = col = 0; col < newdim; col++, rc += 1 / _info->pixel_aspect)
		{
			int c = rc;
			frac = rc - c;
			pix0 = pix1 = _info->image[c];
			if (c + 1 < _info->width) pix1 += 4;
			for (row = 0; row < _info->height; row++, pix0 += _info->width * 4, pix1 += _info->width * 4)
				for (size_t c = 0; c < _info->colors; c++)
					img[row*newdim + col][c] = pix0[c] * (1 - frac) + pix1[c] * frac + 0.5;
		}
		_info->width = newdim;
	}
	free(_info->image);
	_info->image = img;
}

int CImageLoader::flip_index(int row, int col)
{
	if (_info->flip & 4)
		SWAP(row, col);
	if (_info->flip & 2)
		row = _info->iheight - 1 - row;
	if (_info->flip & 1)
		col = _info->iwidth - 1 - col;
	return row * _info->iwidth + col;
}

void CImageLoader::tiff_set(tiff_hdr *th, unsigned short *ntag, unsigned short tag, unsigned short type, int count, int val)
{
	tiff_tag* tt = (tiff_tag *)(ntag + 1) + (*ntag)++;
	tt->val.i = val;
	if (type == 1 && count <= 4)
		for (int c = 0; c < 4; c++)
			tt->val.c[c] = val >> (c << 3);
	else if (type == 2)
	{
		count = strnlen((char *)th + val, count - 1) + 1;
		if (count <= 4)
			for (int c = 0; c < 4; c++)
				tt->val.c[c] = ((char *)th)[val + c];
	}
	else if (type == 3 && count <= 2)
	{
		for (int c = 0; c < 2; c++)
			tt->val.s[c] = val >> (c << 4);
	}
	tt->count = count;
	tt->type = type;
	tt->tag = tag;
}

#define TOFF(ptr) ((char *)(&(ptr)) - (char *)th)
void CImageLoader::tiff_head(tiff_hdr *th, int full)
{
	int psize = 0;

	memset(th, 0, sizeof *th);
	th->order = htonl(0x4d4d4949) >> 16;
	th->magic = 42;
	th->ifd = 10;
	th->rat[0] = th->rat[2] = 300;
	th->rat[1] = th->rat[3] = 1;
	for (size_t c = 0; c < 6; c++)
		th->rat[4 + c] = 1000000;
	th->rat[4] *= _info->shutter;
	th->rat[6] *= _info->aperture;
	th->rat[8] *= _info->focal_len;
	strncpy(th->desc, _info->desc, 512);
	strncpy(th->make, _info->make, 64);
	strncpy(th->model, _info->model, 64);
	//strcpy(th->soft, "CRL v""1.0.0.0");
	strcpy(th->soft, "dcraw v9.27");
	tm* t = localtime(&_info->timestamp);
	sprintf(th->date, "%04d:%02d:%02d %02d:%02d:%02d",
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	strncpy(th->artist, _info->artist, 64);
	if (full) {
		tiff_set(th, &th->ntag, 254, 4, 1, 0);
		tiff_set(th, &th->ntag, 256, 4, 1, _info->width);
		tiff_set(th, &th->ntag, 257, 4, 1, _info->height);
		tiff_set(th, &th->ntag, 258, 3, _info->colors, _options->output_bps);
		if (_info->colors > 2)
			th->tag[th->ntag - 1].val.i = TOFF(th->bps);
		for (size_t c = 0; c < 4; c++)
			th->bps[c] = _options->output_bps;
		tiff_set(th, &th->ntag, 259, 3, 1, 1);
		tiff_set(th, &th->ntag, 262, 3, 1, 1 + (_info->colors > 1));
	}
	tiff_set(th, &th->ntag, 270, 2, 512, TOFF(th->desc));
	tiff_set(th, &th->ntag, 271, 2, 64, TOFF(th->make));
	tiff_set(th, &th->ntag, 272, 2, 64, TOFF(th->model));
	if (full)
	{
		if (_info->oprof)
			psize = ntohl(_info->oprof[0]);
		tiff_set(th, &th->ntag, 273, 4, 1, sizeof *th + psize);
		tiff_set(th, &th->ntag, 277, 3, 1, _info->colors);
		tiff_set(th, &th->ntag, 278, 4, 1, _info->height);
		tiff_set(th, &th->ntag, 279, 4, 1, _info->height*_info->width*_info->colors*_options->output_bps / 8);
	}
	else
	{
		tiff_set(th, &th->ntag, 274, 3, 1, "12435867"[_info->flip] - '0');
	}
	tiff_set(th, &th->ntag, 282, 5, 1, TOFF(th->rat[0]));
	tiff_set(th, &th->ntag, 283, 5, 1, TOFF(th->rat[2]));
	tiff_set(th, &th->ntag, 284, 3, 1, 1);
	tiff_set(th, &th->ntag, 296, 3, 1, 2);
	tiff_set(th, &th->ntag, 305, 2, 32, TOFF(th->soft));
	tiff_set(th, &th->ntag, 306, 2, 20, TOFF(th->date));
	tiff_set(th, &th->ntag, 315, 2, 64, TOFF(th->artist));
	tiff_set(th, &th->ntag, 34665, 4, 1, TOFF(th->nexif));
	if (psize)
		tiff_set(th, &th->ntag, 34675, 7, psize, sizeof *th);
	tiff_set(th, &th->nexif, 33434, 5, 1, TOFF(th->rat[4]));
	tiff_set(th, &th->nexif, 33437, 5, 1, TOFF(th->rat[6]));
	tiff_set(th, &th->nexif, 34855, 3, 1, _info->iso_speed);
	tiff_set(th, &th->nexif, 37386, 5, 1, TOFF(th->rat[8]));
	if (_info->gpsdata[1])
	{
		tiff_set(th, &th->ntag, 34853, 4, 1, TOFF(th->ngps));
		tiff_set(th, &th->ngps, 0, 1, 4, 0x202);
		tiff_set(th, &th->ngps, 1, 2, 2, _info->gpsdata[29]);
		tiff_set(th, &th->ngps, 2, 5, 3, TOFF(th->gps[0]));
		tiff_set(th, &th->ngps, 3, 2, 2, _info->gpsdata[30]);
		tiff_set(th, &th->ngps, 4, 5, 3, TOFF(th->gps[6]));
		tiff_set(th, &th->ngps, 5, 1, 1, _info->gpsdata[31]);
		tiff_set(th, &th->ngps, 6, 5, 1, TOFF(th->gps[18]));
		tiff_set(th, &th->ngps, 7, 5, 3, TOFF(th->gps[12]));
		tiff_set(th, &th->ngps, 18, 2, 12, TOFF(th->gps[20]));
		tiff_set(th, &th->ngps, 29, 2, 12, TOFF(th->gps[23]));
		memcpy(th->gps, _info->gpsdata, sizeof th->gps);
	}
}
#undef TOFF
void CImageLoader::jpeg_thumb()
{
	unsigned short exif[5];
	tiff_hdr th;

	char* thumb = (char *)malloc(_info->thumb_length);
	CAutoFreeMemory autoFree(thumb);

	_reader->Read(thumb, 1, _info->thumb_length);
	_writer->PutChar(0xff);
	_writer->PutChar(0xd8);
	if (strcmp(thumb + 6, "Exif"))
	{
		memcpy(exif, "\xff\xe1  Exif\0\0", 10);
		exif[1] = htons(8 + sizeof th);
		_writer->Write(exif, 1, sizeof exif);
		tiff_head(&th, 0);
		_writer->Write(&th, 1, sizeof th);
	}
	_writer->Write(thumb + 2, 1, _info->thumb_length - 2);
}

void CImageLoader::write_ppm_tiff()
{
	tiff_hdr th;
	int row, col, soff, rstep, cstep;
	int perc, val, total, white = 0x2000;

	perc = _info->width * _info->height * 0.01;		/* 99th percentile white level */
	if (_info->fuji_width)
		perc /= 2;
	if (!((_options->highlight & ~2) || _options->no_auto_bright))
	{
		white = 0;
		for (size_t c = 0; c < _info->colors; c++)
		{
			for (val = 0x2000, total = 0; --val > 32; )
				if ((total += _info->histogram[c][val]) > perc)
					break;
			if (white < val)
				white = val;
		}
	}
	_info->gamma_curve(_info->gamm[0], _info->gamm[1], 2, (white << 3) / _options->bright);
	_info->iheight = _info->height;
	_info->iwidth = _info->width;
	if (_info->flip & 4) SWAP(_info->height, _info->width);
	unsigned char* ppm = (unsigned char *)calloc(_info->width, _info->colors*_options->output_bps / 8);
	CAutoFreeMemory autoFree(ppm);
	unsigned short* ppm2 = (unsigned short *)ppm;
	if (_options->output_tiff)
	{
		tiff_head(&th, 1);
		_writer->Write(&th, sizeof th, 1);
		if (_info->oprof)
			_writer->Write(_info->oprof, ntohl(_info->oprof[0]), 1);
	}
	else if (_info->colors > 3)
	{
		_writer->Print("P7\nWIDTH %d\nHEIGHT %d\nDEPTH %d\nMAXVAL %d\nTUPLTYPE %s\nENDHDR\n",
			_info->width, _info->height, _info->colors, (1 << _options->output_bps) - 1, _info->cdesc);
	}
	else
	{
		_writer->Print("P%d\n%d %d\n%d\n",
			_info->colors / 2 + 5, _info->width, _info->height, (1 << _options->output_bps) - 1);
	}
	soff = flip_index(0, 0);
	cstep = flip_index(0, 1) - soff;
	rstep = flip_index(1, 0) - flip_index(0, _info->width);
	for (row = 0; row < _info->height; row++, soff += rstep)
	{
		for (col = 0; col < _info->width; col++, soff += cstep)
			if (_options->output_bps == 8)
				for (size_t c = 0; c < _info->colors; c++)
					ppm[col*_info->colors + c] = _info->curve[_info->image[soff][c]] >> 8;
			else
				for (size_t c = 0; c < _info->colors; c++)
					ppm2[col*_info->colors + c] = _info->curve[_info->image[soff][c]];
			if (_options->output_bps == 16 && !_options->output_tiff && htons(0x55aa) != 0x55aa)
				swab((char*)ppm2, (char*)ppm2, _info->width*_info->colors * 2);
			_writer->Write(ppm, _info->colors*_options->output_bps / 8, _info->width);
	}
}
