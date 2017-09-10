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

#include "stdafx.h"
#include "CDecoder.h"
#include "CException.h"

using namespace Unmanaged;

CDecoder::CDecoder()
{
}

CDecoder::~CDecoder()
{
}

/*
Construct a decode tree according the specification in *source.
The first 16 bytes specify how many codes should be 1-bit, 2-bit
3-bit, etc.  Bytes after that are the leaf values.

For example, if the source is

{ 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

then the code is

00		0x04
010		0x03
011		0x05
100		0x06
101		0x02
1100		0x07
1101		0x01
11100		0x08
11101		0x09
11110		0x00
111110		0x0a
1111110		0x0b
1111111		0xff
*/
unsigned short* CDecoder::MakeDecoderRef(const unsigned char** source)
{
	const unsigned char* count = (*source += 16) - 17;
	int max;
	for (max = 16; max && !count[max]; max--);
	unsigned short* huff = (unsigned short *)calloc(1 + (1 << max), sizeof *huff);
	if (!huff)
		throw CExceptionMemory("MakeDecoder()");

	huff[0] = max;
	int h = 1;
	for (int len = 1; len <= max; len++)
		for (int i = 0; i < count[len]; i++, ++*source)
			for (int j = 0; j < 1 << (max - len); j++)
				if (h <= 1 << max)
					huff[h++] = len << 8 | **source;
	return huff;
}

unsigned short* CDecoder::MakeDecoder(const unsigned char* source)
{
	return MakeDecoderRef(&source);
}
