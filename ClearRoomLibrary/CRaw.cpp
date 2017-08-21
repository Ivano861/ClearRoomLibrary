#include "stdafx.h"
#include "CRaw.h"

using namespace Unmanaged;

CRaw::CRaw(const char* fileName) : _info(nullptr)
{
	_options = new COptions();
	try
	{
		_reader = new CReader(fileName);
	}
	catch (const errno_t)
	{
		// TODO: manage exception.
		throw;
	}
}

CRaw::~CRaw()
{
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
