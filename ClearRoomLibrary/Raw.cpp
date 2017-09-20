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
#include "CException.h"

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
	try
	{
		CSimpleInfo& info = _raw->GetInfo();
		SimpleInfo^ simple = gcnew SimpleInfo(&info);
		return simple;
	}
	catch (CException& e)
	{
		throw gcnew Exception(marshal_as<String^>(e.what()));
	}
	catch (...)
	{
		throw;
	}
}

ImageLoader^ Raw::GetImageRaw()
{
	try
	{
		CImageLoader& loadRaw = _raw->GetLoadRaw();
		ImageLoader^ simple = gcnew ImageLoader(&loadRaw);
		return simple;
	}
	catch (CException& e)
	{
		throw gcnew Exception(marshal_as<String^>(e.what()));
	}
	catch (...)
	{
		throw;
	}
}

array<System::Byte>^ Raw::GetImage()
{
	CImageLoader& l = _raw->Load();

	int len = l._iheight * l._iwidth * sizeof l._image * 2; //strlen(buf);
	array<Byte>^ byteArray = gcnew array<Byte>(len);
	System::Runtime::InteropServices::Marshal::Copy((IntPtr)l._image, byteArray, 0, len);
	return byteArray;
}

Options^ Raw::Options::get()
{
	COptions& options = _raw->Options();

	ClearRoomLibrary::Options^ result = gcnew ClearRoomLibrary::Options(&options);

	return result;
}
