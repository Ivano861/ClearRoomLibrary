#include "stdafx.h"
#include "ImageLoader.h"

using namespace ClearRoomLibrary;

ImageLoader::ImageLoader(CImageLoader* load)
{
	_load = load;
}


ImageLoader::~ImageLoader()
{
	_load = nullptr;
}
