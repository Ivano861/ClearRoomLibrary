#include "stdafx.h"
#include "CAutoFreeMemory.h"
#include "CException.h"

using namespace Unmanaged;

CAutoFreeMemory::CAutoFreeMemory(void* ptr) : CAutoFreeMemory(ptr, true)
{
}

CAutoFreeMemory::CAutoFreeMemory(void* ptr, bool verify) : _ptr(ptr)
{
	if (verify && !_ptr)
		throw CExceptionMemory();
}

CAutoFreeMemory::~CAutoFreeMemory()
{
	Release();
}

void CAutoFreeMemory::ChangePointer(void* ptr, bool verify, bool releaseOld)
{
	if (releaseOld)
		Release();

	_ptr = ptr;

	if (verify && !_ptr)
		throw CExceptionMemory();
}

void CAutoFreeMemory::Release()
{
	if (_ptr)
	{
		free(_ptr);
		_ptr = nullptr;
	}
}
