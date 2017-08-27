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