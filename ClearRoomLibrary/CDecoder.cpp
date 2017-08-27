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
