// File DLL principale.

#include "stdafx.h"
#include "SimpleInfo.h"

using namespace ClearRoomLibrary;

SimpleInfo::SimpleInfo(CSimpleInfo* info)
{
	_info = info;
}

SimpleInfo::~SimpleInfo()
{
	_info = nullptr;
}
