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
#include "CWriter.h"
#include "CException.h"

using namespace Unmanaged;

CWriter::CWriter() : _ofp(nullptr), _order(0), _dataError(0)
{
}

CWriter::CWriter(const char *fileName) : CWriter()
{
	int len = strlen(fileName) + 1;
	_fileName = new char[len];
	strcpy_s(_fileName, len, fileName);

	errno_t err = fopen_s(&_ofp, _fileName, "wb");
	if (err)
		throw CExceptionFile(err);
}

CWriter::~CWriter()
{
	if (_ofp != nullptr)
		fclose(_ofp);

	delete[] _fileName;
}

CWriter* CWriter::CreateTempFile()
{
	CWriter* result = new CWriter();

	errno_t err = tmpfile_s(&result->_ofp);
	if (err)
		throw CExceptionFile(err);

	return result;
}

const char* CWriter::GetFileName()
{
	return _fileName;
}

void CWriter::SetOrder(short order)
{
	_order = order;
}

short  CWriter::GetOrder()
{
	return _order;
}

int CWriter::Seek(long offset, int origin)
{
	return fseek(_ofp, offset, origin);
}

size_t CWriter::Write(void* buffer, size_t elementSize, size_t elementCount)
{
	return fwrite(buffer, elementSize, elementCount, _ofp);
}

int CWriter::PutChar(int c)
{
	return fputc(c, _ofp);
}

int CWriter::Print(const char* format, unsigned short value1, unsigned short value2)
{
	return fprintf(_ofp, format, value1, value2);
}
int CWriter::Print(const char* format, int value1, unsigned short value2, unsigned short value3)
{
	return fprintf(_ofp, format, value1, value2, value3);
}
int CWriter::Print(const char* format, unsigned short value1, unsigned short value2, unsigned int value3, int value4, const char* value5)
{
	return fprintf(_ofp, format, value1, value2, value3, value4, value5);
}
int CWriter::Print(const char* format, int value1, unsigned short value2, unsigned short value3, int value4)
{
	return fprintf(_ofp, format, value1, value2, value3, value4);
}

long CWriter::GetPosition()
{
	return ftell(_ofp);
}
int CWriter::Eof()
{
	return feof(_ofp);
}
