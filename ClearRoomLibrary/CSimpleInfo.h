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

#pragma once

#include "CReader.h"
#include "CWriter.h"
#include "tiffIfd.h"
#include "ph1.h"
#include "Enumerate.h"

namespace Unmanaged
{
	class CSimpleInfo
	{
	public:
		CSimpleInfo(CReader* reader);
		~CSimpleInfo();

		void GetInfo();

		void gamma_curve(double pwr, double ts, int mode, int imax);
		unsigned getbithuff(int nbits, unsigned short *huff);
#define getbits(n) getbithuff(n, nullptr)
#define gethuff(h) getbithuff(*h, h+1)

	private:
		int parse_tiff(int base);
		int parse_tiff_ifd(int base);
		void parse_exif(int base);
		void parse_gps(int base);
		void parse_makernote(int base, int uptag);
		void parse_thumb_note(int base, unsigned toff, unsigned tlen);
		void get_timestamp(int reversed);
		void tiff_get(unsigned base, unsigned* tag, unsigned* type, unsigned* len, unsigned* save);
		void apply_tiff();
		void parse_external_jpeg();
		void adobe_coeff(const char *make, const char *model);
		void cam_xyz_coeff(float rgb_cam[3][4], double cam_xyz[4][3]);
		void pseudoinverse(double(*in)[3], double(*out)[3], int size);
		float find_green(int bps, int bite, int off0, int off1);
		void simple_coeff(int index);
		short guess_byte_order(int words);
		int canon_s2is();
		int nikon_e995();
		int nikon_e2100();
		void nikon_3700();
		int minolta_z2();
		void parse_phase_one(int base);
		void parse_ciff(int offset, int length, int depth);
		void ciff_block_1030();
		void parse_fuji(int offset);
		int parse_jpeg(int offset);
		void parse_riff();
		void parse_qt(int end);
		void parse_smal(int offset, int fsize);
		void parse_cine();
		void parse_redcine();
		void parse_rollei();
		void parse_sinar_ia();
		void parse_minolta(int base);
		void parse_foveon();
		char* foveon_gets(int offset, char* str, int len);
		void parse_kodak_ifd(int base);
		void parse_mos(int offset);

		void sony_decrypt(unsigned* data, int len, int start, int key);
		void linear_table(unsigned len);
		void romm_coeff(float romm_cam[3][3]);

		char* memmem(char* haystack, size_t haystacklen, char* needle, size_t needlelen);
		char* strcasestr(char* haystack, const char* needle);

	public:

		// TODO: is private
	//private:

		CReader* _reader;

		static const size_t LenMake = 64;
		static const size_t LenModel = 64;
		static const size_t LenModel2 = 64;
		static const size_t LenArtist = 64;
		static const size_t LenCDesc = 5;
		static const size_t LenDesc = 512;

		char make[LenMake];
		char model[LenModel];
		char model2[LenModel2];
		char artist[LenArtist];
		char desc[LenDesc];
		char cdesc[LenCDesc];
		float flash_used;
		float iso_speed;
		float shutter;
		float aperture;
		float focal_len;
		time_t timestamp;
		unsigned short width;
		unsigned short height;
		unsigned colors;
		unsigned flip;

		unsigned tiff_flip;
		unsigned filters;
		unsigned short raw_height;
		unsigned short raw_width;
		unsigned short top_margin;
		unsigned short left_margin;

		float canon_ev;

		// Managed
		char* meta_data;
		unsigned short* raw_image;
		unsigned short(*image)[4];
		unsigned* oprof;

		unsigned shot_select = 0;
		double gamm[6] = { 0.45,4.5,0,0,0,0 };
		int half_size = 0;
		int use_camera_wb = 0;
		int use_camera_matrix = 1;

		double pixel_aspect;
		float cam_mul[4];
		float pre_mul[4];
		float cmatrix[3][4];
		float rgb_cam[3][4];
		unsigned short cblack[4102];
		off_t thumb_offset;
		off_t meta_offset;
		off_t profile_offset;
		unsigned shot_order;
		unsigned kodak_cbpp;
		unsigned exif_cfa;
		unsigned unique_id;
		unsigned thumb_length;
		unsigned meta_length;
		unsigned profile_length;
		unsigned thumb_misc;
		unsigned fuji_layout;
		unsigned tiff_nifds;
		unsigned tiff_samples;
		unsigned tiff_bps;
		unsigned tiff_compress;
		unsigned black;
		unsigned maximum;
		unsigned mix_green;
		unsigned raw_color;
		unsigned zero_is_bad;
		unsigned zero_after_ff;
		unsigned is_raw;
		unsigned dng_version;
		unsigned is_foveon;
		unsigned data_error;
		unsigned tile_width;
		unsigned tile_length;
		unsigned gpsdata[32];
		unsigned load_flags;
		off_t strip_offset;
		off_t data_offset;
		unsigned short white[8][8];
		unsigned short curve[0x10000];
		unsigned short cr2_slice[3];
		unsigned short sraw_mul[4];
		unsigned short shrink;
		unsigned short iheight;
		unsigned short iwidth;
		unsigned short fuji_width;
		unsigned short thumb_width;
		unsigned short thumb_height;
		char xtrans[6][6];
		char xtrans_abs[6][6];
		int mask[8][4];
		int histogram[4][0x2000];
		const float d65_white[3] = { 0.950456f, 1.0f, 1.088754f };

		const double xyz_rgb[3][3] =			/* XYZ from RGB */
		{
			{ 0.412453, 0.357580, 0.180423 },
			{ 0.212671, 0.715160, 0.072169 },
			{ 0.019334, 0.119193, 0.950227 }
		};

		tiff_ifd tiff_ifd[10];
		ph1 ph1;

		LoadRawType load_raw;
		LoadRawType thumb_load_raw;
		WriteThumbType write_thumb;
		WriteThumbType write_fun;
	};
}
