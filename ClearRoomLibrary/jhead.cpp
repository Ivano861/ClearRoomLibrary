#include "stdafx.h"
#include "jhead.h"
#include "CDecoder.h"
#include "CException.h"
#include "Macro.h"

#define _USE_MATH_DEFINES // for C++  
#include <math.h> 

using namespace Unmanaged;

jhead::jhead(CReader& reader, CSimpleInfo& info, bool info_only) : _reader(reader), _info(info)
{
	unsigned char data[0x10000];

	memset(&jh, 0, sizeof jh);
	jh.restart = INT_MAX;
	if ((_reader.GetChar(), _reader.GetChar()) != 0xd8)
	{
		_success = false;
		return;
	}

	unsigned short tag;
	do
	{
		if (!_reader.Read(data, 2, 2))
		{
			_success = false;
			return;
		}
		tag = data[0] << 8 | data[1];
		unsigned short len = (data[2] << 8 | data[3]) - 2;
		if (tag <= 0xff00)
		{
			_success = false;
			return;
		}
		_reader.Read(data, 1, len);
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
			if (len == 9 && !info.dng_version)
				_reader.GetChar();
			break;
		case 0xffc4:
			if (info_only)
				break;
			{
				unsigned short c;
				for (const unsigned char* dp = data; dp < data + len && !((c = *dp++) & -20); )
					jh.free[c] = jh.huff[c] = CDecoder::make_decoder_ref(&dp);
			}
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

	if (jh.bits > 16 || jh.clrs > 6 || !jh.bits || !jh.high || !jh.wide || !jh.clrs)
	{
		_success = false;
		return;
	}
	if (info_only)
	{
		_success = true;
		return;
	}
	if (!jh.huff[0])
	{
		_success = false;
		return;
	}
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
	if (!jh.row)
		throw CExceptionMemory("ljpeg::ctor");

	info.zero_after_ff = 1;
	_success = true;
}
jhead::jhead::~jhead()
{
	for (size_t c = 0; c < 4; c++)
		if (jh.free[c])
			::free(jh.free[c]);

	::free(jh.row);
}

unsigned short* jhead::ljpeg_row(int jrow)
{
	int col, c, diff, pred, spred = 0;
	unsigned short mark = 0, *row[3];

	if (jrow * jh.wide % jh.restart == 0)
	{
		for (size_t c = 0; c < 6; c++)
			jh.vpred[c] = 1 << (jh.bits - 1);
		if (jrow)
		{
			_reader.Seek(-2, SEEK_CUR);
			do mark = (mark << 8) + (c = _reader.GetChar());
			while (c != EOF && mark >> 4 != 0xffd);
		}
		_info.getbits(-1);
	}
	for (size_t c = 0; c < 3; c++)
		row[c] = jh.row + jh.wide*jh.clrs*((jrow + c) & 1);
	for (col = 0; col < jh.wide; col++)
	{
		for (size_t c = 0; c < jh.clrs; c++)
		{
			diff = jhead::ljpeg_diff(jh.huff[c], _info);
			if (jh.sraw && c <= jh.sraw && (col | c))
				pred = spred;
			else if (col)
				pred = row[0][-jh.clrs];
			else
				pred = (jh.vpred[c] += diff) - diff;
			if (jrow && col)
			{
				switch (jh.psv)
				{
				case 1:
					break;
				case 2: pred = row[1][0];
					break;
				case 3: pred = row[1][-jh.clrs];
					break;
				case 4: pred = pred + row[1][0] - row[1][-jh.clrs];
					break;
				case 5: pred = pred + ((row[1][0] - row[1][-jh.clrs]) >> 1);
					break;
				case 6: pred = row[1][0] + ((pred - row[1][-jh.clrs]) >> 1);
					break;
				case 7: pred = (pred + row[1][0]) >> 1;
					break;
				default: pred = 0;
				}
			}
			if ((**row = pred + diff) >> jh.bits)
				throw CExceptionFile();

			if (c <= jh.sraw)
				spred = **row;
			row[0]++; row[1]++;
		}
	}
	return row[2];
}

void jhead::ljpeg_idct()
{
	float work[3][8][8];
	static float cs[106] = { 0 };
	static const unsigned char zigzag[80] =
	{
		0, 1, 8,16, 9, 2, 3,10,17,24,32,25,18,11, 4, 5,12,19,26,33,
		40,48,41,34,27,20,13, 6, 7,14,21,28,35,42,49,56,57,50,43,36,
		29,22,15,23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,
		47,55,62,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63
	};

	if (!cs[0])
	{
		for (size_t c = 0; c < 106; c++)
			cs[c] = cos((c & 31)*M_PI / 16) / 2;
	}
	memset(work, 0, sizeof work);
	work[0][0][0] = jh.vpred[0] += jhead::ljpeg_diff(jh.huff[0], _info) * jh.quant[0];
	for (int i = 1; i < 64; i++)
	{
		int len = _info.gethuff(jh.huff[16]);
		int skip = len >> 4;
		i += skip;
		if (!(len &= 15) && skip < 15) break;
		int coef = _info.getbits(len);
		if ((coef & (1 << (len - 1))) == 0)
			coef -= (1 << len) - 1;
		((float *)work)[zigzag[i]] = coef * jh.quant[i];
	}
	for (size_t c = 0; c < 8; c++)
		work[0][0][c] *= M_SQRT1_2;
	for (size_t c = 0; c < 8; c++)
		work[0][c][0] *= M_SQRT1_2;
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			for (size_t c = 0; c < 8; c++)
				work[1][i][j] += work[0][i][c] * cs[(j * 2 + 1)*c];
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			for (size_t c = 0; c < 8; c++)
				work[2][i][j] += work[1][c][j] * cs[(i * 2 + 1)*c];

	for (size_t c = 0; c < 64; c++)
		jh.idct[c] = CLIP(((float *)work[2])[c] + 0.5);
}

int jhead::ljpeg_diff(unsigned short *huff, CSimpleInfo& info)
{
	int len, diff;

	len = info.gethuff(huff);
	if (len == 16 && (!info.dng_version || info.dng_version >= 0x1010000))
		return -32768;
	diff = info.getbits(len);
	if ((diff & (1 << (len - 1))) == 0)
		diff -= (1 << len) - 1;
	return diff;
}
