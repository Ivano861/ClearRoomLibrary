#pragma once

namespace Unmanaged
{
	class COptions
	{
	public:
		COptions();
		~COptions();

		// Set threshold for wavelet denoising
		// <num>
		float& Threshold();
		// Adjust brightness (default = 1.0)
		// <num>
		float& Bright();

		// Set custom gamma curve (default = 2.222 4.5)
		// <p ts>
		void SetGammaCurveCustom(double g0, double g1);

		// Select one raw image
		// [0..N - 1]
		void SelectRawImage(unsigned select);
		// Select all raw image
		void SelectAllRawImage();

		// Half-size color image (twice as fast as SetInterpolationQuality(0))
		void SetHalfSize(bool halfSize);

		// Set the interpolation quality
		// [0-3]
		void SetInterpolationQuality(int quality);

		// Average a grey box for white balance
		// <x y w h>
		void SetWhiteBalanceAverageBox(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

		// Average the whole image for white balance
		void SetWhiteBalanceAverageWholeImage(bool whole);

		// Use camera white balance, if possible
		void SetWhiteBalanceCamera(bool camera);

		// Use/don't use an embedded color matrix
		void UseCameraMatrix(bool useEmbedded);

	private:
		float threshold = 0;

		float bright = 1;

		float user_mul[4] = { 0,0,0,0 };
		// puts(_("-r <r g b g> Set custom white balance"));

		double aber[4] = { 1,1,1,1 };
		//	puts(_("-C <r b>  Correct chromatic aberration"));
		//double aber[4]; // aber[0] = 1 / atof(argv[arg++]);     aber[2] = 1 / atof(argv[arg++]);

		double gamm[6] = { 0.45,4.5,0,0,0,0 };	// TODO: defined in CSimpleInfo.h

		int user_black = -1;
		// puts(_("-k <num>  Set the darkness level"));

		int user_sat = -1;
		//puts(_("-S <num>  Set the saturation level"));

		int user_flip = -1;
		// puts(_("-t [0-7]  Flip image (0=none, 3=180, 5=90CCW, 6=90CW)"));

		int user_qual = -1;

		int med_passes = 0;
		// puts(_("-m <num>  Apply a 3x3 median filter to R-G and B-G"));

		int highlight = 0;
		// puts(_("-H [0-9]  Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)"));

		unsigned shot_select = 0;	// TODO: defined in CSimpleInfo.h
		bool multi_out = false;

		int output_color = 1;
		const char* out_profile;
		// puts(_("-o [0-6]  Output colorspace (raw,sRGB,Adobe,Wide,ProPhoto,XYZ,ACES)"));
		// puts(_("-o <file> Apply output ICC profile from file"));
		//int output_color;
		//if (isdigit(argv[arg][0]) && !argv[arg][1])
		//	output_color = atoi(argv[arg++]);
		//else     out_profile = argv[arg++];

		const char* cam_profile = nullptr;
		// puts(_("-p <file> Apply camera ICC profile from file or \"embed\""));

		const char* bpfile = nullptr;
		// puts(_("-P <file> Fix the dead pixels listed in this file"));

		const char* dark_frame = nullptr;
		// puts(_("-K <file> Subtract dark frame (16-bit raw PGM)"));

		bool thumbnail_only = false;
		// puts(_("-e        Extract embedded thumbnail image"));
		//thumbnail_only = true;
		//bool ExtractEmbeddedThumbnail;

		bool half_size = false;	// TODO: defined in CSimpleInfo.h

		bool four_color_rgb = false;
		// puts(_("-f        Interpolate RGGB as four colors"));
		//four_color_rgb = true;

		unsigned greybox[4] = { 0, 0, UINT_MAX, UINT_MAX };

		bool use_auto_wb = false;

		bool use_camera_wb = false;	// TODO: defined in CSimpleInfo.h

		bool use_camera_matrix = true;	// TODO: defined in CSimpleInfo.h

		int document_mode = 0;
		// puts(_("-E        Document mode (???)"));
		//int document_mode++;

		// puts(_("-D        Document mode without scaling (totally raw)"));
		//int document_mode++;

		// puts(_("-d        Document mode (no color, no interpolation)"));
		//int document_mode++;

		bool use_fuji_rotate = true;
		// puts(_("-j        Don't stretch or rotate raw pixels"));
		//use_fuji_rotate = false;

		bool no_auto_bright = false;
		// puts(_("-W        Don't automatically brighten the image"));
		//no_auto_bright = true;

		bool output_tiff = false;
		// puts(_("-T        Write TIFF instead of PPM"));
		//output_tiff = true;

		// puts(_("-4        Linear 16-bit, same as \"-6 -W -g 1 1\""));
		//gamm[0] = gamm[1] = 1;
		//no_auto_bright = true;
		//output_bps = 16;

		int output_bps = 8;
		// puts(_("-6        Write 16-bit instead of 8-bit"));
		//output_bps = 16;
	};
}
