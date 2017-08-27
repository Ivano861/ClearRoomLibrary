#include "stdafx.h"
#include "CRaw.h"
#include "CException.h"

using namespace Unmanaged;

CRaw::CRaw(const char* fileName) : _info(nullptr)
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

CSimpleInfo& CRaw::GetInfo()
{
	_info = new CSimpleInfo(_reader);
	_info->GetInfo();
	return *(_info);
}

CImageLoader& CRaw::GetLoadRaw()
{
	_load = new CImageLoader(_options, _info, _reader);
	_load->LoadImageRaw();
	return *(_load);
}
