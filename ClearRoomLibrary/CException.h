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
}
