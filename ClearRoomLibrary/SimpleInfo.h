// SimpleInfo.h

#pragma once

#include "CSimpleInfo.h"
#include "Options.h"

using namespace System;
using namespace Unmanaged;

namespace ClearRoomLibrary
{
	public ref class SimpleInfo sealed
	{
	internal:
		// Constructor
		SimpleInfo(CSimpleInfo* info);

		// Destructor
		~SimpleInfo();

	public:


	private:
		// Private members
		CSimpleInfo* _info;
	};
}
