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

#pragma once

namespace Unmanaged
{
	class CAutoFreeMemory
	{
	public:
		CAutoFreeMemory(void* ptr);
		CAutoFreeMemory(void* ptr, bool verify);

		~CAutoFreeMemory();

		void ChangePointer(void* ptr, bool verify = true, bool releaseOld = true);
		void Release();

	private:
		void* _ptr;
	};
}
