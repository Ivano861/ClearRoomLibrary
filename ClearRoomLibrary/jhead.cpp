#include "stdafx.h"
#include "jhead.h"
#include "CDecoder.h"
#include "CError.h"

using namespace Unmanaged;

int jhead::ljpeg_start(jhead& jh, CReader& reader, unsigned int dngVersion, bool info_only)
{
	unsigned short c, tag, len;
	unsigned char data[0x10000];
	const unsigned char *dp;

	memset(&jh, 0, sizeof jh);
	jh.restart = INT_MAX;
	if ((reader.GetChar(), reader.GetChar()) != 0xd8)
		return 0;

	do
	{
		if (!reader.Read(data, 2, 2)) return 0;
		tag = data[0] << 8 | data[1];
		len = (data[2] << 8 | data[3]) - 2;
		if (tag <= 0xff00) return 0;
		reader.Read(data, 1, len);
		switch (tag)
		{
		case 0xffc3:
			jh.sraw = ((data[7] >> 4) * (data[7] & 15) - 1) & 3;
		case 0xffc1:
		case 0xffc0:
			jh.algo = tag & 0xff;
			jh.bits = data[0];
			jh.high = data[1] << 8 | data[2];
			jh.wide = data[3] << 8 | data[4];
			jh.clrs = data[5] + jh.sraw;
			if (len == 9 && !dngVersion)
				reader.GetChar();
			break;
		case 0xffc4:
			if (info_only)
				break;
			for (dp = data; dp < data + len && !((c = *dp++) & -20); )
				jh.free[c] = jh.huff[c] = CDecoder::make_decoder_ref(&dp);
			break;
		case 0xffda:
			jh.psv = data[1 + data[0] * 2];
			jh.bits -= data[3 + data[0] * 2] & 15;
			break;
		case 0xffdb:
			for (size_t c = 0; c < 64; c++)
				jh.quant[c] = data[c * 2 + 1] << 8 | data[c * 2 + 2];
			break;
		case 0xffdd:
			jh.restart = data[0] << 8 | data[1];
		}
	} while (tag != 0xffda);

	if (jh.bits > 16 || jh.clrs > 6 ||
		!jh.bits || !jh.high || !jh.wide || !jh.clrs) return 0;
	if (info_only)
		return 1;
	if (!jh.huff[0])
		return 0;
	for (size_t c = 0; c < 19; c++)
		if (!jh.huff[c + 1])
			jh.huff[c + 1] = jh.huff[c];
	if (jh.sraw)
	{
		for (size_t c = 0; c < 4; c++)
			jh.huff[2 + c] = jh.huff[1];
		for (int c = 0; c < jh.sraw; c++)
			jh.huff[1 + c] = jh.huff[0];
	}
	jh.row = (unsigned short *)calloc(jh.wide*jh.clrs, 4);
	CError::merror(jh.row, "ClearRoomLibrary", "ljpeg_start()");
	// TODO: setting zero_after_ff 
	//return zero_after_ff = 1;
	return 1;
}
