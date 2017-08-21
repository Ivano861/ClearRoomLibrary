#include "stdafx.h"
#include "CWriter.h"

using namespace Unmanaged;

CWriter::CWriter() : _ifp(nullptr), _order(0), _dataError(0)
{
}

CWriter::CWriter(const char *fileName) : CWriter()
{
	int len = strlen(fileName) + 1;
	_fileName = new char[len];
	strcpy_s(_fileName, len, fileName);

	errno_t err = fopen_s(&_ifp, _fileName, "wb");
	if (err)
		throw err;
}

CWriter::~CWriter()
{
	if (_ifp != nullptr)
		fclose(_ifp);

	delete _fileName;
}

CWriter* CWriter::CreateTempFile()
{
	CWriter* result = new CWriter();

	tmpfile_s(&result->_ifp);

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
	return fseek(_ifp, offset, origin);
}

size_t CWriter::Write(void* buffer, size_t elementSize, size_t elementCount)
{
	return fwrite(buffer, elementSize, elementCount, _ifp);
}

long CWriter::GetPosition()
{
	return ftell(_ifp);
}
int CWriter::Eof()
{
	return feof(_ifp);
}
