#pragma once

#include "COptions.h"
#include "CSimpleInfo.h"

namespace Unmanaged
{
	class CRaw
	{
	public:
		CRaw(const char* fileName);
		~CRaw();

		COptions& Options();
		CSimpleInfo& Info();
		CSimpleInfo& GetInfo();

	private:
		COptions* _options;
		CSimpleInfo* _info;

		CReader* _reader;
	};
}
