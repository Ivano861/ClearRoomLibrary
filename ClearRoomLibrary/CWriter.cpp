#include "stdafx.h"
#include "CWriter.h"

using namespace Unmanaged;

CWriter::CWriter() : m_ifp(nullptr), m_order(0), m_dataError(0)
{
}

CWriter::CWriter(const char *fileName) : CWriter()
{
	int len = strlen(fileName) + 1;
	m_fileName = new char[len];
	strcpy_s(m_fileName, len, fileName);

	errno_t err = fopen_s(&m_ifp, m_fileName, "wb");
	if (err)
		throw err;
}

CWriter::~CWriter()
{
	if (m_ifp != nullptr)
		fclose(m_ifp);

	delete m_fileName;
}

CWriter* CWriter::CreateTempFile()
{
	CWriter* result = new CWriter();

	tmpfile_s(&result->m_ifp);

	return result;
}

const char* CWriter::GetFileName()
{
	return m_fileName;
}

void CWriter::SetOrder(short order)
{
	m_order = order;
}

short  CWriter::GetOrder()
{
	return m_order;
}

int CWriter::Seek(long offset, int origin)
{
	return fseek(m_ifp, offset, origin);
}

size_t CWriter::Write(void* buffer, size_t elementSize, size_t elementCount)
{
	return fwrite(buffer, elementSize, elementCount, m_ifp);
}

long CWriter::GetPosition()
{
	return ftell(m_ifp);
}
int CWriter::Eof()
{
	return feof(m_ifp);
}
