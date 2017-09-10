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
#include "decode.h"
#include "tiffHdr.h"

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
		CWriter* _writer;

		void LoadRaw();
		void PackedLoadRaw();
		void UnpackedLoadRaw();
		void LosslessDngLoadRaw();
		void LossyDngLoadRaw();
		void PackedDngLoadRaw();
		void LosslessJpegLoadRaw();
		void CanonLoadRaw();
		void Canon600LoadRaw();
		void CanonRmfLoadRaw();
		void CanonSrawLoadRaw();
		void EightBitLoadRaw();
		void HasselbladLoadRaw();
		void ImaconFullLoadRaw();
		void Kodak262LoadRaw();
		void Kodak65000LoadRaw();
		void KodakC330LoadRaw();
		void KodakC603LoadRaw();
		void KodakDC120LoadRaw();
		void KodakJpegLoadRaw();
		void KodakRadcLoadRaw();
		void KodakRgbLoadRaw();
		void LodakThumbLoadRaw();
		void KodakYcbcrLoadRaw();
		void LeafHdrLoadRaw();
		void MinoltaRD175LoadRaw();
		void NikonLoadRaw();
		void NikonYuvLoadRaw();
		void NokiaLoadRaw();
		void OlympusLoadRaw();
		void PanasonicLoadRaw();
		void PentaxLoadRaw();
		void PhaseOneLoadRaw();
		void PhaseOneLoadRawC();
		void Quicktake100LoadRaw();
		void RedcineLoadRaw();
		void RolleiLoadRaw();
		void SamsungLoadRaw();
		void Samsung2LoadRaw();
		void Samsung3LoadRaw();
		void Sinar4ShotLoadRaw();
		void SmalV6LoadRaw();
		void SmalV9LoadRaw();
		void SonyLoadRaw();
		void SonyArwLoadRaw();
		void SonyArw2LoadRaw();

		void AdobeCopyPixel(unsigned row, unsigned col, unsigned short** rp);
		void CropMaskedPixels();
		int FCol(int row, int col);
		void PhaseOneCorrect();
		void Canon600Correct();
		void Canon600FixedWB(int temp);
		void Canon600AutoWB();
		void Canon600Coeff();
		int Canon600Color(int ratio[2], int mar);
		int Raw(unsigned row, unsigned col);
		void PhaseOneFlatField(int is_float, int nc);
		void CubicSpline(const int* x_, const int* y_, const int len);
		void CrwInitTables(unsigned table, unsigned short* huff[2]);
		int CanonHasLowbits();
		unsigned Ph1Bithuff(int nbits, unsigned short* huff);
#define Ph1Bits(n) Ph1Bithuff(n,0)
#define Ph1Huff(h) Ph1Bithuff(*h,h+1)
		int Kodak65000Decode(short *out, int bsize);
		unsigned PanaBits(int nbits);
		void SmalDecodeSegment(unsigned seg[2][2], int holes);
#define HOLE(row) ((holes >> (((row) - _info->_rawHeight) & 7)) & 1)
		void FillHoles(int holes);
		int Median4(int *p);
#define RadcToken(tree) ((signed char) _info->GetBitHuff(8,huff[tree]))

		void RemoveZeroes();
		void BadPixels(const char *cfname);
		void Subtract(const char *fname);
//#ifdef COLORCHECK
		void Colorcheck();
//#endif
		void ScaleColors();
		void WaveletDenoise();
		void HatTransform(float *temp, float *base, int st, int size, int sc);
		void PreInterpolate();
		void LinInterpolate();
		void VngInterpolate();
		void PpgInterpolate();
		void BorderInterpolate(int border);
		void XtransInterpolate(int passes);
		void AhdInterpolate();
		void Cielab(unsigned short rgb[3], short lab[3]);

		/* RESTRICTED code starts here */
		// TODO: variables
		decode _firstDecode[2048];
		decode* _secondDecode;
		decode* _freeDecode;

		void FoveonDecoder(unsigned size, unsigned code);
		void FoveonThumb();
		void FoveonSdLoadRaw();
		void FoveonHuff(unsigned short *huff);
		void FoveonDpLoadRaw();
		void FoveonLoadCamf();
		const char* FoveonCamfParam(const char *block, const char *param);
		void* FoveonCamfMatrix(unsigned dim[3], const char *name);
		int FoveonFixed(void *ptr, int size, const char *name);
		float FoveonAvg(short *pix, int range[2], float cfilt);
		short* FoveonMakeCurve(double max, double mul, double filt);
		void FoveonMakeCurves(short **curvep, float dq[3], float div[3], float filt);
		int FoveonApplyCurve(short *curve, int i);
		void FoveonInterpolate();
		/* RESTRICTED code ends here */






		void PpmThumb();
		void Ppm16Thumb();
		void LayerThumb();
		void RolleiThumb();
		void WritePpmTiff();



		void MedianFilter();
		void BlendHighlights();
		void RecoverHighlights();
		void ApplyProfile(const char *input, const char *output);
		void ConvertToRGB();
		void FujiRotate();
		void Stretch();
		int FlipIndex(int row, int col);
		void TiffSet(TiffHDR *th, unsigned short *ntag, unsigned short tag, unsigned short type, int count, int val);
		void TiffHead(TiffHDR *th, int full);
		void JpegThumb();
	};
}
