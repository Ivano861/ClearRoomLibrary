#pragma once

#include "CRaw.h"
#include "SimpleInfo.h"

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

	private:
		//Private members
		CRaw* _raw;

	};
}
