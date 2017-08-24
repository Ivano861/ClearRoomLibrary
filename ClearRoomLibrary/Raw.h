#pragma once

#include "CRaw.h"
#include "SimpleInfo.h"
#include "ImageLoader.h"

using namespace System;
using namespace Unmanaged;

namespace ClearRoomLibrary
{
	public ref class Raw
	{
	public:
		Raw(String^ fileName);
		~Raw();

		SimpleInfo^ GetInfo();
		ImageLoader^ GetImageRaw();

	private:
		//Private members
		CRaw* _raw;

	};
}
