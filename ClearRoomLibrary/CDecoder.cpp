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

unsigned short* CDecoder::make_decoder_ref(const unsigned char** source)
{
	const unsigned char* count = (*source += 16) - 17;
	int max;
	for (max = 16; max && !count[max]; max--);
	unsigned short* huff = (unsigned short *)calloc(1 + (1 << max), sizeof *huff);
	if (!huff)
		throw CExceptionMemory("make_decoder()");

	huff[0] = max;
	int h = 1;
	for (int len = 1; len <= max; len++)
		for (int i = 0; i < count[len]; i++, ++*source)
			for (int j = 0; j < 1 << (max - len); j++)
				if (h <= 1 << max)
					huff[h++] = len << 8 | **source;
	return huff;
}

unsigned short* CDecoder::make_decoder(const unsigned char* source)
{
	return make_decoder_ref(&source);
}
