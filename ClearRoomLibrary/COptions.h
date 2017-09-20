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

		// Set threshold for wavelet denoising
		// <num>
		float& Threshold();

		// Adjust brightness (default = 1.0)
		// <num>
		float& Bright();

		// Set custom white balance
		// <r g b g>
		float& CustomWhiteBalance();

		// Average a grey box for white balance
		// <x y w h>
		unsigned& AverageAreaWhiteBalance();

		// Average the whole image for white balance
		bool& AutoWhiteBalance();

		// Use camera white balance, if possible
		bool& CameraWhiteBalance();

		// Use/don't use an embedded color matrix
		bool& UseCameraMatrix();

		// Set custom gamma curve (default = 2.222 4.5)
		// <p ts>
		double& GammaCurveCustom();

		// Correct chromatic aberration"));
		// <r b>
		double GetCorrectAberration0();
		double GetCorrectAberration2();
		void SetCorrectAberration0(double value);
		void SetCorrectAberration2(double value);

		// Set the darkness level
		// <num>
		int& Darkness();

		// Set the saturation level
		// <num>
		int& Saturation();

		// Apply a 3x3 median filter to R-G and B-G
		// <num>
		int& MedianPasses();

		// Flip image (0=none, 3=180, 5=90CCW, 6=90CW)
		// [0-7]
		int& Flip();

		// Set the interpolation quality
		// [0-3]
		int& Quality();

		//  Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)
		// [0-9]
		int& Highlight();

		// Select one raw image (-1 Select all raw image)
		// [0..N - 1 | -1]
		int GetSelectRawImage();
		void SetSelectRawImage(int value);

		// Output colorspace (raw,sRGB,Adobe,Wide,ProPhoto,XYZ,ACES)
		// [0-6]
		int& ColorSpace();

		// Apply output ICC profile from file
		// <file>
		const char* COptions::GetProfileICC();
		void SetProfileICC(const char* value);

		// Apply camera ICC profile from file or "embed"
		// <file>
		const char* COptions::GetCameraProfileICC();
		void SetCameraProfileICC(const char* value);

		// Fix the dead pixels listed in this file
		// <file>
		const char* COptions::GetDeadPixel();
		void SetDeadPixel(const char* value);

		// Subtract dark frame (16-bit raw PGM)
		// <file>
		const char* COptions::GetDarkFrame();
		void SetDarkFrame(const char* value);

		// Extract embedded thumbnail image
		bool& ThumbnailOnly();

		// Half-size color image (twice as fast as SetInterpolationQuality(0))
		bool& HalfSize();

		// Interpolate RGGB as four colors
		bool& FourColorRGB();

		// Don't stretch or rotate raw pixels
		bool& UseFujiRotate();

		// Don't automatically brighten the image
		bool& NoAutoBright();

		// Write TIFF instead of PPM
		bool& OutputTiff();

		// Select output 8 or 16 bit
		int& OutputBps();

		// Document mode (0-None,1-Document mode,2-Document mode without scaling,3-Document mode with no color and no interpolation)
		// [0-3]
		int& DocumentMode();

	//private:
		float _threshold;
		float _bright;
		float _userMul[4];
		unsigned _greybox[4];
		bool _useAutoWB;
		bool _useCameraWB;
		bool _useCameraMatrix;
		double _gamma[6];
		double _aber[4];
		int _userBlack;
		int _userSaturation;
		int _medianPasses;
		int _userFlip;
		int _userQuality;
		int _highlight;
		bool _multiOut;
		unsigned _shotSelect;
		int _outputColor;
		char* _outProfile;
		char* _cameraProfile;
		char* _bpFile;
		char* _darkFrame;
		bool _thumbnailOnly;
		bool _halfSize;
		bool _fourColorRGB;
		bool _useFujiRotate;
		bool _noAutoBright;
		bool _outputTiff;
		int _outputBps;
		int _documentMode;

		// puts(_("-4        Linear 16-bit, same as \"-6 -W -g 1 1\""));
		//_gamma[0] = _gamma[1] = 1;
		//_noAutoBright = true;
		//_outputBps = 16;
	};
}
