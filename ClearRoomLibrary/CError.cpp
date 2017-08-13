#include "stdafx.h"
#include "CError.h"

using namespace Unmanaged;

CError::CError()
{
}

CError::~CError()
{
}

void CError::merror(void* ptr, const char* file, const char* where)
{
	if (ptr)
		return;
	fprintf(stderr, ("%s: Out of memory in %s\n"), file, where);
}

void CError::derror(CReader& reader)
{
	fprintf(stderr, "%s: ", reader.GetFileName());
	if (reader.Eof())
		fprintf(stderr, ("Unexpected end of file\n"));
	else
		fprintf(stderr, ("Corrupt data near 0x%llx\n"), (INT64)reader.GetPosition());
}

