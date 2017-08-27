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
#include "CException.h"

using namespace Unmanaged;

CException::CException() noexcept : _what(nullptr)
{
}

CException::CException(const CException &right) noexcept : _what(nullptr)
{
	size_t len = strlen(right._what) + 1;
	_what = (char*)malloc(len);
	if (!_what)
		return;			// no error return: _reason is nullptr

	strcpy_s(_what, len, right._what);
}

CException& CException::operator=(const CException &right) noexcept
{
	if (this == &right)
		return *this;

	if (_what)
		free(_what);

	if (!right._what)
	{
		_what = nullptr;
		return *this;
	}

	size_t len = strlen(right._what) + 1;
	_what = (char*)malloc(len);
	if (!_what)
		return *this;	// no error return: _reason is nullptr

	strcpy_s(_what, len, right._what);

	return *this;
}

CException::~CException() noexcept
{
	if (_what)
	{
		free(_what);
		_what = nullptr;
	}
}

const char* CException::what() const noexcept
{
	return _what ? _what : "Unknown exception";
}


const char* CExceptionFile::Message = "Invalid data file";

CExceptionFile::CExceptionFile()
{
}

// Create an exception based on the value errno indicated
CExceptionFile::CExceptionFile(errno_t err)
{
	char* buffer = (char*)malloc(256);
	if (!buffer)
		return;			// No errors if the buffer is not created: returns _what equal to nullptr

	strerror_s(buffer, 256, err);

	size_t len = strlen(buffer) + 1;
	_what = (char*)malloc(len);
	if (_what)			// No errors if the _what is not created: returns _what equal to nullptr
		strcpy_s(_what, len, buffer);

	free(buffer);
}

CExceptionFile::CExceptionFile(const char* message)
{
	size_t len = strlen(message) + 1;
	_what = (char*)malloc(len);
	if (!_what)
		return;			// No errors if the _what is not created: returns _what equal to nullptr

	strcpy_s(_what, len, message);
}


const char* CExceptionMemory::Message = "Out of memory";

CExceptionMemory::CExceptionMemory()
{
}

CExceptionMemory::CExceptionMemory(const char* source)
{
	size_t lenMessage = strlen(Message);
	size_t len = lenMessage + 4 + strlen(source) + 1;
	_what = (char*)malloc(len);
	if (!_what)
		return;			// No errors if the _what is not created: returns _what equal to nullptr

	strcpy_s(_what, len, Message);
	strcpy_s(&_what[lenMessage], len - lenMessage, " in ");
	strcpy_s(&_what[lenMessage + 4], len - (lenMessage + 4), source);
}

const char* CExceptionMemory::what() const noexcept
{
	return _what ? _what : Message;
}


const char* CExceptionNoRaw::Message = "It is not a raw file";

CExceptionNoRaw::CExceptionNoRaw()
{
}

const char* CExceptionNoRaw::what() const noexcept
{
	return _what ? _what : Message;
}


const char* CExceptionNoThumbnail::Message = "Thumbnail image does not exist";

CExceptionNoThumbnail::CExceptionNoThumbnail()
{
}

const char* CExceptionNoThumbnail::what() const noexcept
{
	return _what ? _what : Message;
}


const char* CExceptionInvalidImageRequest::Message = "Requested image does not exist";

CExceptionInvalidImageRequest::CExceptionInvalidImageRequest()
{
}

const char* CExceptionInvalidImageRequest::what() const noexcept
{
	return _what ? _what : Message;
}


const char* CExceptionJpegError::Message = "Incorrect JPEG dimensions";

CExceptionJpegError::CExceptionJpegError()
{
}

const char* CExceptionJpegError::what() const noexcept
{
	return _what ? _what : Message;
}
