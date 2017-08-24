#pragma once

#include "COptions.h"
#include "CSimpleInfo.h"
#include "CImageLoader.h"

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
		CImageLoader& GetLoadRaw();

	private:
		COptions* _options;
		CSimpleInfo* _info;
		CImageLoader* _load;

		CReader* _reader;
	};
}
