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
	class CWriter
	{
	public:
		CWriter(const char *fileName);
		~CWriter();
	protected:
		CWriter();

	public:
		static CWriter* CreateTempFile();

	private:
		FILE* _ifp;
		short _order;
		unsigned _dataError;
		char* _fileName;

	public:
		const char* GetFileName();

		void SetOrder(short order);
		short GetOrder();

		int Seek(long offset, int origin);
		size_t Write(void* buffer, size_t elementSize, size_t elementCount);
		long GetPosition(); //const;
		int Eof();
	};
}
