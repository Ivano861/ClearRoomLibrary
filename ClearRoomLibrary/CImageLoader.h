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

#include "COptions.h"
#include "CSimpleInfo.h"
#include "CReader.h"

namespace Unmanaged
{
	class CImageLoader
	{
	public:
		CImageLoader(COptions* options, CSimpleInfo* info, CReader* reader);

		~CImageLoader();

		void LoadImageRaw();
		void LoadImageThumbnail();

	private:
		COptions* _options;
		CSimpleInfo* _info;
		CReader* _reader;

		void LoadRaw();
		void packed_load_raw();
		void unpacked_load_raw();
		void lossless_dng_load_raw();
		void lossy_dng_load_raw();
		void packed_dng_load_raw();
		void lossless_jpeg_load_raw();
		void canon_load_raw();
		void canon_600_load_raw();
		void canon_rmf_load_raw();
		void canon_sraw_load_raw();
		void eight_bit_load_raw();
		void hasselblad_load_raw();
		void imacon_full_load_raw();
		void kodak_262_load_raw();
		void kodak_65000_load_raw();
		void kodak_c330_load_raw();
		void kodak_c603_load_raw();
		void kodak_dc120_load_raw();
		void kodak_jpeg_load_raw();
		void kodak_radc_load_raw();
		void kodak_rgb_load_raw();
		void kodak_ycbcr_load_raw();
		void leaf_hdr_load_raw();
		void minolta_rd175_load_raw();
		void nikon_load_raw();
		void nikon_yuv_load_raw();
		void nokia_load_raw();
		void olympus_load_raw();
		void panasonic_load_raw();
		void pentax_load_raw();
		void phase_one_load_raw();
		void phase_one_load_raw_c();
		void quicktake_100_load_raw();
		void redcine_load_raw();
		void rollei_load_raw();
		void samsung_load_raw();
		void samsung2_load_raw();
		void samsung3_load_raw();
		void sinar_4shot_load_raw();
		void smal_v6_load_raw();
		void smal_v9_load_raw();
		void sony_load_raw();
		void sony_arw_load_raw();
		void sony_arw2_load_raw();

		void adobe_copy_pixel(unsigned row, unsigned col, unsigned short** rp);
		void crop_masked_pixels();
		int fcol(int row, int col);
		void phase_one_correct();
		void canon_600_correct();
		void canon_600_fixed_wb(int temp);
		void canon_600_auto_wb();
		void canon_600_coeff();
		int canon_600_color(int ratio[2], int mar);
		int raw(unsigned row, unsigned col);
		void phase_one_flat_field(int is_float, int nc);
		void cubic_spline(const int* x_, const int* y_, const int len);
		void crw_init_tables(unsigned table, unsigned short* huff[2]);
		int canon_has_lowbits();
		unsigned ph1_bithuff(int nbits, unsigned short* huff);
#define ph1_bits(n) ph1_bithuff(n,0)
#define ph1_huff(h) ph1_bithuff(*h,h+1)
		int kodak_65000_decode(short *out, int bsize);
		unsigned pana_bits(int nbits);
		void smal_decode_segment(unsigned seg[2][2], int holes);
#define HOLE(row) ((holes >> (((row) - _info->raw_height) & 7)) & 1)
		void fill_holes(int holes);
		int median4(int *p);
		void sony_decrypt(unsigned *data, int len, int start, int key);

#define radc_token(tree) ((signed char) _info->getbithuff(8,huff[tree]))
	};
}
