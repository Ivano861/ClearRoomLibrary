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

		void GammaCurve(double pwr, double ts, int mode, int imax);
		unsigned GetBitHuff(int nbits, unsigned short *huff);
#define getbits(n) GetBitHuff(n, nullptr)
#define gethuff(h) GetBitHuff(*h, h+1)
		void CamXyzCoeff(float _rgbCam[3][4], double cam_xyz[4][3]);
		void PseudoInverse(double(*in)[3], double(*out)[3], int size);
		void SonyDecrypt(unsigned* data, int len, int start, int key);

	private:
		int ParseTiff(int base);
		int ParseTiffIFD(int base);
		void ParseExif(int base);
		void ParseGps(int base);
		void ParseMakernote(int base, int uptag);
		void ParseThumbNote(int base, unsigned toff, unsigned tlen);
		void GetTimestamp(int reversed);
		void TiffGet(unsigned base, unsigned* tag, unsigned* type, unsigned* len, unsigned* save);
		void ApplyTiff();
		void ParseExternalJpeg();
		void AdobeCoeff(const char *make, const char *model);
		float FindGreen(int bps, int bite, int off0, int off1);
		void SimpleCoeff(int index);
		short GuessByteOrder(int words);
		int CanonS2is();
		int NikonE995();
		int NikonE2100();
		void Nikon3700();
		int MinoltaZ2();
		void ParsePhaseOne(int base);
		void ParseCiff(int offset, int length, int depth);
		void CiffBlock1030();
		void ParseFuji(int offset);
		int ParseJpeg(int offset);
		void ParseRiff();
		void ParseQt(int end);
		void ParseSmal(int offset, int fsize);
		void ParseCine();
		void ParseRedcine();
		void ParseRollei();
		void ParseSinarIA();
		void ParseMinolta(int base);
		void ParseFoveon();
		char* FoveonGets(int offset, char* str, int len);
		void ParseKodakIFD(int base);
		void ParseMos(int offset);

		void LinearTable(unsigned len);
		void RommCoeff(float romm_cam[3][3]);

		char* memmem(char* haystack, size_t haystacklen, char* needle, size_t needlelen);
		char* strcasestr(char* haystack, const char* needle);

	public:

		// TODO: is private
	//private:

		const float d65White[3] = { 0.950456f, 1.0f, 1.088754f };

		const double xyzRGB[3][3] =			/* XYZ from RGB */
		{
			{ 0.412453, 0.357580, 0.180423 },
			{ 0.212671, 0.715160, 0.072169 },
			{ 0.019334, 0.119193, 0.950227 }
		};

		CReader* _reader;

		static const size_t LenMake = 64;
		static const size_t LenModel = 64;
		static const size_t LenModel2 = 64;
		static const size_t LenArtist = 64;
		static const size_t LenCDesc = 5;
		static const size_t LenDesc = 512;

		char* _metaData;
		unsigned short* _rawImage;
		unsigned short(*_image)[4];
		unsigned* _profile;

		char _make[LenMake];
		char _model[LenModel];
		char _model2[LenModel2];
		char _artist[LenArtist];
		char _desc[LenDesc];
		char _cdesc[LenCDesc];
		float _flashUsed;
		float _isoSpeed;
		float _shutter;
		float _aperture;
		float _focalLen;
		time_t _timestamp;
		unsigned short _width;
		unsigned short _height;
		unsigned _colors;
		unsigned _flip;

		unsigned _tiffFlip;
		unsigned _filters;
		unsigned short _rawHeight;
		unsigned short _rawWidth;
		unsigned short _topMargin;
		unsigned short _leftMargin;

		float _canonEV;

		unsigned _shotSelect = 0;
		double _gamma[6] = { 0.45,4.5,0,0,0,0 };
		bool _halfSize = false;
		int _useCameraWB = 0;
		int _useCameraMatrix = 1;

		double _pixelAspect;
		float _camMul[4];
		float _preMul[4];
		float _cMatrix[3][4];
		float _rgbCam[3][4];
		unsigned short _cBlack[4102];
		off_t _thumbOffset;
		off_t _metaOffset;
		off_t _profileOffset;
		unsigned _shotOrder;
		unsigned _kodakCbpp;
		unsigned _exifCfa;
		unsigned _uniqueId;
		unsigned _thumbLength;
		unsigned _metaLength;
		unsigned _profileLength;
		unsigned _thumbMisc;
		unsigned _fujiLayout;
		unsigned _tiffNifds;
		unsigned _tiffSamples;
		unsigned _tiffBps;
		unsigned _tiffCompress;
		unsigned _black;
		unsigned _maximum;
		unsigned _mixGreen;
		unsigned _rawColor;
		unsigned _zeroIsBad;
		unsigned _zeroAfterFF;
		unsigned _isRaw;
		unsigned _dngVersion;
		unsigned _isFoveon;
		unsigned _dataError;
		unsigned _tileWidth;
		unsigned _tileLength;
		unsigned _gpsData[32];
		unsigned _loadFlags;
		off_t _stripOffset;
		off_t _dataOffset;
		unsigned short _white[8][8];
		unsigned short _curve[0x10000];
		unsigned short _cr2Slice[3];
		unsigned short _srawMul[4];
		unsigned short _shrink;
		unsigned short _iheight;
		unsigned short _iwidth;
		unsigned short _fujiWidth;
		unsigned short _thumbWidth;
		unsigned short _thumbHeight;
		char _xtrans[6][6];
		char _xtransAbs[6][6];
		int _mask[8][4];
		int _histogram[4][0x2000];

		TiffIFD _tiffIfd[10];
		Ph1 _ph1;

		LoadRawType _loadRaw;
		LoadRawType _thumbLoadRaw;
		WriteThumbType _writeThumb;
		WriteThumbType _writeFun;
	};
}
