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
	class COptions
	{
	public:
		COptions();
		~COptions();

		// TODO: implementation
		/*
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

		// Average a grey box for _white balance
		// <x y w h>
		void SetWhiteBalanceAverageBox(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

		// Average the whole image for _white balance
		void SetWhiteBalanceAverageWholeImage(bool whole);

		// Use camera _white balance, if possible
		void SetWhiteBalanceCamera(bool camera);

		// Use/don't use an embedded color matrix
		void UseCameraMatrix(bool useEmbedded);
	private:
	*/
		float _threshold = 0;

		float _bright = 1;

		float _userMul[4] = { 0,0,0,0 };
		// puts(_("-r <r g b g> Set custom _white balance"));

		double _aber[4] = { 1,1,1,1 };
		//	puts(_("-C <r b>  Correct chromatic aberration"));
		//double _aber[4]; // _aber[0] = 1 / atof(argv[arg++]);     _aber[2] = 1 / atof(argv[arg++]);

		double _gamma[6] = { 0.45,4.5,0,0,0,0 };	// TODO: defined in CSimpleInfo.h

		int _userBlack = -1;
		// puts(_("-k <num>  Set the darkness level"));

		int _userSaturation = -1;
		//puts(_("-S <num>  Set the saturation level"));

		int _userFlip = -1;
		// puts(_("-t [0-7]  Flip image (0=none, 3=180, 5=90CCW, 6=90CW)"));

		int _userQuality = -1;

		int _medianPasses = 0;
		// puts(_("-m <num>  Apply a 3x3 median filter to R-G and B-G"));

		int _highlight = 0;
		// puts(_("-H [0-9]  Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)"));

		unsigned _shotSelect = 0;	// TODO: defined in CSimpleInfo.h
		bool _multiOut = false;

		int _outputColor = 1;
		const char* _outProfile;
		// puts(_("-o [0-6]  Output colorspace (raw,sRGB,Adobe,Wide,ProPhoto,XYZ,ACES)"));
		// puts(_("-o <file> Apply output ICC profile from file"));
		//int _outputColor;
		//if (isdigit(argv[arg][0]) && !argv[arg][1])
		//	_outputColor = atoi(argv[arg++]);
		//else     _outProfile = argv[arg++];

		const char* _cameraProfile = nullptr;
		// puts(_("-p <file> Apply camera ICC profile from file or \"embed\""));

		const char* _bpFile = nullptr;
		// puts(_("-P <file> Fix the dead pixels listed in this file"));

		const char* _darkFrame = nullptr;
		// puts(_("-K <file> Subtract dark frame (16-bit raw PGM)"));

		bool _thumbnailOnly = false;
		// puts(_("-e        Extract embedded thumbnail image"));
		//_thumbnailOnly = true;
		//bool ExtractEmbeddedThumbnail;

		bool _halfSize = false;	// TODO: defined in CSimpleInfo.h

		bool _fourColorRGB = false;
		// puts(_("-f        Interpolate RGGB as four colors"));
		//_fourColorRGB = true;

		unsigned _greybox[4] = { 0, 0, UINT_MAX, UINT_MAX };

		bool _useAutoWB = false;

		bool _useCameraWB = false;	// TODO: defined in CSimpleInfo.h

		bool _useCameraMatrix = true;	// TODO: defined in CSimpleInfo.h

		int _documentMode = 0;
		// puts(_("-E        Document mode (???)"));
		//int _documentMode++;

		// puts(_("-D        Document mode without scaling (totally raw)"));
		//int _documentMode++;

		// puts(_("-d        Document mode (no color, no interpolation)"));
		//int _documentMode++;

		bool _useFujiRotate = true;
		// puts(_("-j        Don't stretch or rotate raw pixels"));
		//_useFujiRotate = false;

		bool _noAutoBright = false;
		// puts(_("-W        Don't automatically brighten the image"));
		//_noAutoBright = true;

		//bool _outputTiff = false;
		bool _outputTiff = true;
		// puts(_("-T        Write TIFF instead of PPM"));
		//_outputTiff = true;

		// puts(_("-4        Linear 16-bit, same as \"-6 -W -g 1 1\""));
		//_gamma[0] = _gamma[1] = 1;
		//_noAutoBright = true;
		//_outputBps = 16;

		int _outputBps = 8;
		// puts(_("-6        Write 16-bit instead of 8-bit"));
		//_outputBps = 16;
	};
}
