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
#include "Raw.h"

using namespace ClearRoomLibrary;
using namespace msclr::interop;

Raw::Raw(String^ fileName)
{
	marshal_context^ context = gcnew marshal_context();
	_raw = new CRaw(context->marshal_as<const char*>(fileName));
	delete context;
}

Raw::~Raw()
{
	if (_raw)
	{
		delete _raw;
		_raw = nullptr;
	}
}

SimpleInfo^ Raw::GetInfo()
{
	CSimpleInfo& info = _raw->GetInfo();

	SimpleInfo^ simple = gcnew SimpleInfo(&info);

	return simple;
}

ImageLoader^ Raw::GetImageRaw()
{
	CImageLoader& loadRaw = _raw->GetLoadRaw();

	ImageLoader^ simple = gcnew ImageLoader(&loadRaw);

	return simple;
}