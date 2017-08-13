#pragma once

#include "CReader.h"
#include "CWriter.h"
#include "tiff_ifd.h"
#include "ph1.h"

namespace Unmanaged
{
	class CSimpleInfo
	{
	private:
		CSimpleInfo();
		~CSimpleInfo();

		void GetInfo();
	public:
		static CSimpleInfo* GetInfo(const char* fileName);
		static void Release(CSimpleInfo* info);

	private:
		char* memmem(char* haystack, size_t haystacklen, char* needle, size_t needlelen);
		char* strcasestr(char* haystack, const char* needle);
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
		void gamma_curve(double pwr, double ts, int mode, int imax);
		short guess_byte_order(int words);
		int canon_s2is();
		int nikon_e995();
		int nikon_e2100();
		void nikon_3700();
		int minolta_z2();
		void parse_phase_one(int base);
		void romm_coeff(float romm_cam[3][3]);


	public:
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
			foveon_thumb
		};

	private:
		CReader* m_reader;

		static const size_t LenMake = 64;
		static const size_t LenModel = 64;
		static const size_t LenModel2 = 64;
		static const size_t LenArtist = 64;
		static const size_t LenCDesc = 5;
		static const size_t LenDesc = 512;

		unsigned short raw_height, raw_width, height, width, top_margin, left_margin;
		unsigned flip, tiff_flip, filters, colors;
		float cam_mul[4], pre_mul[4], cmatrix[3][4], rgb_cam[3][4];
		float flash_used, canon_ev, iso_speed, shutter, aperture, focal_len;
		unsigned short *raw_image, (*image)[4], cblack[4102];
		off_t thumb_offset, meta_offset, profile_offset;
		unsigned shot_order, kodak_cbpp, exif_cfa, unique_id;
		unsigned thumb_length, meta_length, profile_length;
		unsigned thumb_misc, *oprof, fuji_layout, shot_select = 0, multi_out = 0;
		unsigned tiff_nifds, tiff_samples, tiff_bps, tiff_compress;
		unsigned black, maximum, mix_green, raw_color, zero_is_bad;
		unsigned zero_after_ff, is_raw, dng_version, is_foveon, data_error;
		unsigned tile_width, tile_length, gpsdata[32], load_flags;
		char cdesc[LenCDesc], desc[LenDesc], make[LenMake], model[LenModel], model2[LenModel2], artist[LenArtist];
		off_t strip_offset, data_offset;
		unsigned short white[8][8], curve[0x10000], cr2_slice[3], sraw_mul[4];
		double pixel_aspect, aber[4] = { 1,1,1,1 }, gamm[6] = { 0.45,4.5,0,0,0,0 };
		unsigned short shrink, iheight, iwidth, fuji_width, thumb_width, thumb_height;
		char *meta_data, xtrans[6][6], xtrans_abs[6][6];
		int mask[8][4];
		int half_size = 0, four_color_rgb = 0, document_mode = 0, highlight = 0;
		int verbose = 0, use_auto_wb = 0, use_camera_wb = 0, use_camera_matrix = 1;
		time_t timestamp;

		const float d65_white[3] = { 0.950456f, 1.0f, 1.088754f };

		const double xyz_rgb[3][3] = {			/* XYZ from RGB */
			{ 0.412453, 0.357580, 0.180423 },
			{ 0.212671, 0.715160, 0.072169 },
			{ 0.019334, 0.119193, 0.950227 } };


		tiff_ifd tiff_ifd[10];
		ph1 ph1;

		LoadRawType load_raw;
		LoadRawType thumb_load_raw;
		WriteThumbType write_thumb;
	};
}
