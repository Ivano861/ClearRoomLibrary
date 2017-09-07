/*
This file is part of ClearRoomLibrary.

ClearRoomLibrary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

solidity is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ClearRoomLibrary.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "SimpleInfo.h"

using namespace ClearRoomLibrary;
using namespace msclr::interop;

SimpleInfo::SimpleInfo(CSimpleInfo* info)
{
	_info = info;
}

SimpleInfo::~SimpleInfo()
{
	_info = nullptr;
}

#pragma region Property
String^ SimpleInfo::Make::get()
{
	return marshal_as<String^>(_info->make);
}

String^ SimpleInfo::Model::get()
{
	return marshal_as<String^>(_info->model);
}

String^ SimpleInfo::Model2::get()
{
	return marshal_as<String^>(_info->model2);
}

String^ SimpleInfo::Artist::get()
{
	return marshal_as<String^>(_info->artist);
}

String^ SimpleInfo::Description::get()
{
	return marshal_as<String^>(_info->desc);
}
#pragma endregion
