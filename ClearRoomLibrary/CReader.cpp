#include "stdafx.h"
#include "CReader.h"
#include "CError.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace Unmanaged;

CReader::CReader(const char *fileName) : m_ifp(nullptr), m_order(0), m_dataError(0)
{
	int len = strlen(fileName) + 1;
	m_fileName = new char[len];
	strcpy_s(m_fileName, len, fileName);

	errno_t err = fopen_s(&m_ifp, m_fileName, "rb");
	if (err)
		throw err;
}

CReader::~CReader()
{
	if (m_ifp != nullptr)
		fclose(m_ifp);

	delete m_fileName;
}

const char* CReader::GetFileName()
{
	return m_fileName;
}

void CReader::SetOrder(short order)
{
	m_order = order;
}

short  CReader::GetOrder()
{
	return m_order;
}

int CReader::Seek(long offset, int origin)
{
	return fseek(m_ifp, offset, origin);
}

size_t CReader::Read(void* buffer, size_t elementSize, size_t elementCount)
{
	return fread(buffer, elementSize, elementCount, m_ifp);
}

long CReader::GetPosition() const
{
	return ftell(m_ifp);
}
int CReader::Eof()
{
	return feof(m_ifp);
}
int CReader::GetChar()
{
	return fgetc(m_ifp);
}

char* CReader::GetString(char* str, int n)
{
	return fgets(str, n, m_ifp);
}

int CReader::scanf(const char *format, void* arg)
{
	return fscanf_s(m_ifp, format, arg);
}

unsigned short CReader::sget2(unsigned char* s)
{
	if (m_order == 0x4949)		/* "II" means little-endian */
		return s[0] | s[1] << 8;
	else						/* "MM" means big-endian */
		return s[0] << 8 | s[1];
}

unsigned short CReader::get2()
{
	unsigned char str[2] = { 0xff,0xff };
	fread(str, 1, 2, m_ifp);
	return sget2(str);
}

unsigned CReader::sget4(unsigned char* s)
{
	if (m_order == 0x4949)
		return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
	else
		return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}

unsigned CReader::get4()
{
	unsigned char str[4] = { 0xff,0xff,0xff,0xff };
	fread(str, 1, 4, m_ifp);
	return sget4(str);
}

unsigned CReader::getint(int type)
{
	return type == 3 ? get2() : get4();
}

float CReader::int_to_float(int i)
{
	union
	{
		int i; float f;
	} u;
	u.i = i;
	return u.f;
}

double CReader::getreal(int type)
{
	union
	{
		char c[8]; double d;
	} u;
	int i, rev;

	switch (type)
	{
	case 3: return (unsigned short)get2();
	case 4: return (unsigned int)get4();
	case 5:  u.d = (unsigned int)get4();
		return u.d / (unsigned int)get4();
	case 8: return (signed short)get2();
	case 9: return (signed int)get4();
	case 10: u.d = (signed int)get4();
		return u.d / (signed int)get4();
	case 11: return int_to_float(get4());
	case 12:
		rev = 7 * ((m_order == 0x4949) == (ntohs(0x1234) == 0x1234));
		for (i = 0; i < 8; i++)
			u.c[i ^ rev] = fgetc(m_ifp);
		return u.d;
	default: return fgetc(m_ifp);
	}
}

void CReader::read_shorts(unsigned short* pixel, unsigned count)
{
	if (fread(pixel, 2, count, m_ifp) < count)
		CError::derror(*this);
	if ((m_order == 0x4949) == (ntohs(0x1234) == 0x1234))
		_swab((char*)pixel, (char*)pixel, count * 2);
}
