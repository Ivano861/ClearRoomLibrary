#include "stdafx.h"
#include "CDecoder.h"
#include "CError.h"

using namespace Unmanaged;

CDecoder::CDecoder()
{
}

CDecoder::~CDecoder()
{
}

unsigned short* CDecoder::make_decoder_ref(const unsigned char** source)
{
	int max, len, h, i, j;
	const unsigned char *count;
	unsigned short *huff;

	count = (*source += 16) - 17;
	for (max = 16; max && !count[max]; max--);
	huff = (unsigned short *)calloc(1 + (1 << max), sizeof *huff);
	CError::merror(huff, "ClearRoomLibrary", "make_decoder()");
	huff[0] = max;
	for (h = len = 1; len <= max; len++)
		for (i = 0; i < count[len]; i++, ++*source)
			for (j = 0; j < 1 << (max - len); j++)
				if (h <= 1 << max)
					huff[h++] = len << 8 | **source;
	return huff;
}

unsigned short* CDecoder::make_decoder(const unsigned char* source)
{
	return make_decoder_ref(&source);
}
