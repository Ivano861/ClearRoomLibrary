#pragma once

#include "CReader.h"

namespace Unmanaged
{
	class CError
	{
	private:
		CError();
		~CError();

	public:
		static void merror(void* ptr, const char* file, const char* where);
		static void derror(CReader& reader);
	};
}