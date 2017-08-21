#pragma once

#include "COptions.h"

using namespace Unmanaged;

namespace ClearRoomLibrary
{
	public ref class Options sealed
	{
	public:
		// Constructor
		Options();

		~Options();

	private:
		//Private members
		COptions* _options;
	};
}
