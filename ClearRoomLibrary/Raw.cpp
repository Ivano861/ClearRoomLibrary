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
	if (_raw != nullptr)
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
