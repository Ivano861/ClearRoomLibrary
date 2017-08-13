// SimpleInfo.h

#pragma once

#include "CSimpleInfo.h"

using namespace System;
using namespace Unmanaged;

namespace ClearRoomLibrary
{

	public ref class SimpleInfo sealed
	{
	private:
		// Constructor
		SimpleInfo();

		// Destructor
		~SimpleInfo();

	private:
		// Private members
		CSimpleInfo* m_info;

	public:
		static SimpleInfo^ GetInfo(String^ fileName);
	};
}
