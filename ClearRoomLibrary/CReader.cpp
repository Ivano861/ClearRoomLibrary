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
#include "CReader.h"
#include "CException.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace Unmanaged;

CReader::CReader(const char *fileName) : _ifp(nullptr), _order(0), _dataError(0)
{
	int len = strlen(fileName) + 1;
	_fileName = new char[len];
	strcpy_s(_fileName, len, fileName);

	errno_t err = fopen_s(&_ifp, _fileName, "rb");
	if (err)
		throw CExceptionFile(err);
}

CReader::~CReader()
{
	if (_ifp != nullptr)
		fclose(_ifp);

	delete[] _fileName;
}

const char* CReader::GetFileName()
{
	return _fileName;
}

FILE* CReader::GetFILE()
{
	return _ifp;
}


void CReader::SetOrder(short order)
{
	_order = order;
}

short  CReader::GetOrder()
{
	return _order;
}

int CReader::Seek(long offset, int origin)
{
	return fseek(_ifp, offset, origin);
}

size_t CReader::Read(void* buffer, size_t elementSize, size_t elementCount)
{
	return fread(buffer, elementSize, elementCount, _ifp);
}

long CReader::GetPosition() const
{
	return ftell(_ifp);
}
int CReader::Eof()
{
	return feof(_ifp);
}
int CReader::GetChar()
{
	return fgetc(_ifp);
}

char* CReader::GetString(char* str, int n)
{
	return fgets(str, n, _ifp);
}

int CReader::GetScanf(const char *format, void* arg)
{
	return fscanf_s(_ifp, format, arg);
}

unsigned short CReader::GetUShort(unsigned char* s)
{
	if (_order == 0x4949)		/* "II" means little-endian */
		return s[0] | s[1] << 8;
	else						/* "MM" means big-endian */
		return s[0] << 8 | s[1];
}

unsigned short CReader::GetUShort()
{
	unsigned char str[2] = { 0xff,0xff };
	fread(str, 1, 2, _ifp);
	return GetUShort(str);
}

unsigned CReader::GetUInt(unsigned char* s)
{
	if (_order == 0x4949)
		return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
	else
		return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}

unsigned CReader::GetUInt()
{
	unsigned char str[4] = { 0xff,0xff,0xff,0xff };
	fread(str, 1, 4, _ifp);
	return GetUInt(str);
}

unsigned CReader::GetUInt(int type)
{
	return type == 3 ? GetUShort() : GetUInt();
}

float CReader::IntToFloat(int i)
{
	union
	{
		int i; float f;
	} u;
	u.i = i;
	return u.f;
}

double CReader::GetReal(int type)
{
	union
	{
		char c[8]; double d;
	} u;
	int i, rev;

	switch (type)
	{
	case 3: return (unsigned short)GetUShort();
	case 4: return (unsigned int)GetUInt();
	case 5:  u.d = (unsigned int)GetUInt();
		return u.d / (unsigned int)GetUInt();
	case 8: return (signed short)GetUShort();
	case 9: return (signed int)GetUInt();
	case 10: u.d = (signed int)GetUInt();
		return u.d / (signed int)GetUInt();
	case 11: return IntToFloat(GetUInt());
	case 12:
		rev = 7 * ((_order == 0x4949) == (ntohs(0x1234) == 0x1234));
		for (i = 0; i < 8; i++)
			u.c[i ^ rev] = fgetc(_ifp);
		return u.d;
	default: return fgetc(_ifp);
	}
}

void CReader::ReadShorts(unsigned short* pixel, unsigned count)
{
	if (fread(pixel, 2, count, _ifp) < count)
		throw CExceptionFile();

	if ((_order == 0x4949) == (ntohs(0x1234) == 0x1234))
		_swab((char*)pixel, (char*)pixel, count * 2);
}
