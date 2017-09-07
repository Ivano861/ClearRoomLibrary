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

#include "CReader.h"

namespace Unmanaged
{
	class CException
	{
	public:
		CException() noexcept;
		CException(const CException& right) noexcept;
		CException& operator=(const CException& right) noexcept;
		virtual ~CException() noexcept;
		virtual const char* what() const noexcept;

	protected:
		char* _what;
	};

	class CExceptionFile : CException
	{
	public:
		// Create an ddefault exception
		CExceptionFile();

		// Create an exception based on the value errno indicated
		CExceptionFile(errno_t err);

		// Create an exception based on the contents of message
		CExceptionFile(const char* message);
	private:
		static const char* Message;
	};

	class CExceptionMemory : CException
	{
	public:
		// Create an default exception
		CExceptionMemory();

		// Create an exception by adding x to the default message.
		CExceptionMemory(const char* source);

		virtual const char* what() const noexcept;
	private:
		static const char* Message;
	};

	class CExceptionNoRaw : CException
	{
	public:
		// Create an default exception
		CExceptionNoRaw();

		virtual const char* what() const noexcept;
	private:
		static const char* Message;
	};

	class CExceptionNoThumbnail : CException
	{
	public:
		// Create an default exception
		CExceptionNoThumbnail();

		virtual const char* what() const noexcept;
	private:
		static const char* Message;
	};

	class CExceptionInvalidImageRequest : CException
	{
	public:
		// Create an default exception
		CExceptionInvalidImageRequest();

		virtual const char* what() const noexcept;
	private:
		static const char* Message;
	};

	class CExceptionJpegError : CException
	{
	public:
		// Create an default exception
		CExceptionJpegError();

		virtual const char* what() const noexcept;
	private:
		static const char* Message;
	};

	class CExceptionDecoderTableOverflow : CException
	{
	public:
		// Create an default exception
		CExceptionDecoderTableOverflow();

		virtual const char* what() const noexcept;
	private:
		static const char* Message;
	};
}
