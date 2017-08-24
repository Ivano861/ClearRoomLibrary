#pragma once

#include "CReader.h"
#include "CWriter.h"
#include "tiff_ifd.h"
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

		unsigned tiff_flip, filters;
		unsigned short raw_height, raw_width, top_margin, left_margin;

		float canon_ev;


		float cam_mul[4], pre_mul[4], cmatrix[3][4], rgb_cam[3][4];
		unsigned short *raw_image, (*image)[4], cblack[4102];
		off_t thumb_offset, meta_offset, profile_offset;
		unsigned shot_order, kodak_cbpp, exif_cfa, unique_id;
		unsigned thumb_length, meta_length, profile_length;
		unsigned thumb_misc, *oprof, fuji_layout, shot_select = 0;
		unsigned tiff_nifds, tiff_samples, tiff_bps, tiff_compress;
		unsigned black, maximum, mix_green, raw_color, zero_is_bad;
		unsigned zero_after_ff, is_raw, dng_version, is_foveon, data_error;
		unsigned tile_width, tile_length, gpsdata[32], load_flags;
		off_t strip_offset, data_offset;
		unsigned short white[8][8], curve[0x10000], cr2_slice[3], sraw_mul[4];
		double pixel_aspect, gamm[6] = { 0.45,4.5,0,0,0,0 };
		unsigned short shrink, iheight, iwidth, fuji_width, thumb_width, thumb_height;
		char *meta_data, xtrans[6][6], xtrans_abs[6][6];
		int mask[8][4];
		int half_size = 0;
		int use_camera_wb = 0, use_camera_matrix = 1;
		int histogram[4][0x2000];
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
