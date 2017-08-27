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
