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
#include "CRaw.h"
#include "CException.h"

using namespace Unmanaged;

CRaw::CRaw(const char* fileName) : _info(nullptr), _load(nullptr)
{
	_options = new COptions();
	_reader = new CReader(fileName);
}

CRaw::~CRaw()
{
	if (_load != nullptr)
	{
		delete _load;
		_load = nullptr;
	}
	if (_info != nullptr)
	{
		delete _info;
		_info = nullptr;
	}
	if (_options != nullptr)
	{
		delete _options;
		_options = nullptr;
	}
	if (_reader != nullptr)
	{
		delete _reader;
		_reader = nullptr;
	}
}

COptions& CRaw::Options()
{
	return *_options;
}

CSimpleInfo& CRaw::Info()
{
	return *_info;
}

CImageLoader& CRaw::Load()
{
	return *_load;
}

CSimpleInfo& CRaw::GetInfo()
{
	_info = new CSimpleInfo(_reader, _options);
	memcpy(_info->_gamma, _options->_gamma, sizeof _info->_gamma);
	_info->GetInfo();
	return *(_info);
}

CImageLoader& CRaw::GetLoadRaw()
{
	_load = new CImageLoader(_options, _info, _reader);
	_load->LoadImageRaw();
	return *(_load);
}
