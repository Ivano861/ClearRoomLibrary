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
#include "jhead.h"
#include "CDecoder.h"
#include "CException.h"
#include "Macro.h"

#define _USE_MATH_DEFINES // for C++  
#include <math.h> 

using namespace Unmanaged;

JHead::JHead(CReader& reader, CSimpleInfo& info, bool info_only) : _reader(reader), _info(info)
{
	unsigned char data[0x10000];

	memset(&_jdata, 0, sizeof _jdata);
	_jdata.restart = INT_MAX;
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
			_jdata.sraw = ((data[7] >> 4) * (data[7] & 15) - 1) & 3;
		case 0xffc1:
		case 0xffc0:
			_jdata.algo = tag & 0xff;
			_jdata.bits = data[0];
			_jdata.high = data[1] << 8 | data[2];
			_jdata.wide = data[3] << 8 | data[4];
			_jdata.clrs = data[5] + _jdata.sraw;
			if (len == 9 && !info._dngVersion)
				_reader.GetChar();
			break;
		case 0xffc4:
			if (info_only)
				break;
			{
				unsigned short c;
				for (const unsigned char* dp = data; dp < data + len && !((c = *dp++) & -20); )
					_jdata.free[c] = _jdata.huff[c] = CDecoder::MakeDecoderRef(&dp);
			}
			break;
		case 0xffda:
			_jdata.psv = data[1 + data[0] * 2];
			_jdata.bits -= data[3 + data[0] * 2] & 15;
			break;
		case 0xffdb:
			for (size_t c = 0; c < 64; c++)
				_jdata.quant[c] = data[c * 2 + 1] << 8 | data[c * 2 + 2];
			break;
		case 0xffdd:
			_jdata.restart = data[0] << 8 | data[1];
		}
	} while (tag != 0xffda);

	if (_jdata.bits > 16 || _jdata.clrs > 6 || !_jdata.bits || !_jdata.high || !_jdata.wide || !_jdata.clrs)
	{
		_success = false;
		return;
	}
	if (info_only)
	{
		_success = true;
		return;
	}
	if (!_jdata.huff[0])
	{
		_success = false;
		return;
	}
	for (size_t c = 0; c < 19; c++)
		if (!_jdata.huff[c + 1])
			_jdata.huff[c + 1] = _jdata.huff[c];
	if (_jdata.sraw)
	{
		for (size_t c = 0; c < 4; c++)
			_jdata.huff[2 + c] = _jdata.huff[1];
		for (int c = 0; c < _jdata.sraw; c++)
			_jdata.huff[1 + c] = _jdata.huff[0];
	}
	_jdata.row = (unsigned short *)calloc(_jdata.wide*_jdata.clrs, 4);
	if (!_jdata.row)
		throw CExceptionMemory("ljpeg::ctor");

	info._zeroAfterFF = 1;
	_success = true;
}
JHead::JHead::~JHead()
{
	for (size_t c = 0; c < 4; c++)
		if (_jdata.free[c])
			::free(_jdata.free[c]);

	::free(_jdata.row);
}

unsigned short* JHead::LJpegRow(int jrow)
{
	int col, c, diff, pred, spred = 0;
	unsigned short mark = 0, *row[3];

	if (jrow * _jdata.wide % _jdata.restart == 0)
	{
		for (size_t c = 0; c < 6; c++)
			_jdata.vpred[c] = 1 << (_jdata.bits - 1);
		if (jrow)
		{
			_reader.Seek(-2, SEEK_CUR);
			do mark = (mark << 8) + (c = _reader.GetChar());
			while (c != EOF && mark >> 4 != 0xffd);
		}
		_info.getbits(-1);
	}
	for (size_t c = 0; c < 3; c++)
		row[c] = _jdata.row + _jdata.wide*_jdata.clrs*((jrow + c) & 1);
	for (col = 0; col < _jdata.wide; col++)
	{
		for (size_t c = 0; c < _jdata.clrs; c++)
		{
			diff = JHead::LJpegDiff(_jdata.huff[c], _info);
			if (_jdata.sraw && c <= _jdata.sraw && (col | c))
				pred = spred;
			else if (col)
				pred = row[0][-_jdata.clrs];
			else
				pred = (_jdata.vpred[c] += diff) - diff;
			if (jrow && col)
			{
				switch (_jdata.psv)
				{
				case 1:
					break;
				case 2: pred = row[1][0];
					break;
				case 3: pred = row[1][-_jdata.clrs];
					break;
				case 4: pred = pred + row[1][0] - row[1][-_jdata.clrs];
					break;
				case 5: pred = pred + ((row[1][0] - row[1][-_jdata.clrs]) >> 1);
					break;
				case 6: pred = row[1][0] + ((pred - row[1][-_jdata.clrs]) >> 1);
					break;
				case 7: pred = (pred + row[1][0]) >> 1;
					break;
				default: pred = 0;
				}
			}
			if ((**row = pred + diff) >> _jdata.bits)
				throw CExceptionFile();

			if (c <= _jdata.sraw)
				spred = **row;
			row[0]++; row[1]++;
		}
	}
	return row[2];
}

void JHead::LJpegIdct()
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
	work[0][0][0] = _jdata.vpred[0] += JHead::LJpegDiff(_jdata.huff[0], _info) * _jdata.quant[0];
	for (int i = 1; i < 64; i++)
	{
		int len = _info.gethuff(_jdata.huff[16]);
		int skip = len >> 4;
		i += skip;
		if (!(len &= 15) && skip < 15) break;
		int coef = _info.getbits(len);
		if ((coef & (1 << (len - 1))) == 0)
			coef -= (1 << len) - 1;
		((float *)work)[zigzag[i]] = coef * _jdata.quant[i];
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
		_jdata.idct[c] = CLIP(((float *)work[2])[c] + 0.5);
}

int JHead::LJpegDiff(unsigned short *huff, CSimpleInfo& info)
{
	int len, diff;

	len = info.gethuff(huff);
	if (len == 16 && (!info._dngVersion || info._dngVersion >= 0x1010000))
		return -32768;
	diff = info.getbits(len);
	if ((diff & (1 << (len - 1))) == 0)
		diff -= (1 << len) - 1;
	return diff;
}
