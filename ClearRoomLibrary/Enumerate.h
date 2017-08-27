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

namespace Unmanaged
{
	enum LoadRawType
	{
		unknown_load_raw,
		packed_load_raw,
		unpacked_load_raw,
		lossless_dng_load_raw,
		lossy_dng_load_raw,
		packed_dng_load_raw,
		lossless_jpeg_load_raw,
		canon_load_raw,
		canon_600_load_raw,
		canon_rmf_load_raw,
		canon_sraw_load_raw,
		eight_bit_load_raw,
		foveon_dp_load_raw,
		foveon_sd_load_raw,
		hasselblad_load_raw,
		imacon_full_load_raw,
		kodak_262_load_raw,
		kodak_65000_load_raw,
		kodak_c330_load_raw,
		kodak_c603_load_raw,
		kodak_dc120_load_raw,
		kodak_jpeg_load_raw,
		kodak_radc_load_raw,
		kodak_rgb_load_raw,
		kodak_ycbcr_load_raw,
		leaf_hdr_load_raw,
		minolta_rd175_load_raw,
		nikon_load_raw,
		nikon_yuv_load_raw,
		nokia_load_raw,
		olympus_load_raw,
		panasonic_load_raw,
		pentax_load_raw,
		phase_one_load_raw,
		phase_one_load_raw_c,
		quicktake_100_load_raw,
		redcine_load_raw,
		rollei_load_raw,
		samsung_load_raw,
		samsung2_load_raw,
		samsung3_load_raw,
		sinar_4shot_load_raw,
		smal_v6_load_raw,
		smal_v9_load_raw,
		sony_load_raw,
		sony_arw_load_raw,
		sony_arw2_load_raw,

		kodak_thumb_load_raw,
	};

	enum WriteThumbType
	{
		layer_thumb,
		ppm_thumb,
		ppm16_thumb,
		rollei_thumb,
		jpeg_thumb,
		foveon_thumb,

		write_ppm_tiff
	};
}
