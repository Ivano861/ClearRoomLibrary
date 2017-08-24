#pragma once

#include "CReader.h"
#include "CSimpleInfo.h"

namespace Unmanaged
{
	class jhead
	{
	public:
		jhead(CReader& reader, CSimpleInfo& info, bool info_only);
		~jhead();

		CReader& _reader;
		CSimpleInfo& _info;

		struct
		{
			int algo;
			int bits;
			int high;
			int wide;
			int clrs;
			int sraw;
			int psv;
			int restart;
			int vpred[6];
			unsigned short quant[64];
			unsigned short idct[64];
			unsigned short* huff[20];
			unsigned short* free[20];
			unsigned short* row;
		} jh;

		bool _success;

		unsigned short* jhead::ljpeg_row(int jrow);
		void ljpeg_idct();

		static int ljpeg_diff(unsigned short *huff, CSimpleInfo& info);
	};
}