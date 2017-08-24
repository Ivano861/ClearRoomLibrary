#pragma once

#include "CImageLoader.h"

using namespace System;
using namespace Unmanaged;

namespace ClearRoomLibrary
{
	public ref class ImageLoader
	{
	internal:
		// Constructor
		ImageLoader(CImageLoader* load);

		// Destructor
		~ImageLoader();

	public:

	private:
		// Private members
		CImageLoader* _load;
	};
}
