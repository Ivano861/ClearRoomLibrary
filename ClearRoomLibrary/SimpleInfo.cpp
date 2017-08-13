// File DLL principale.

#include "stdafx.h"
#include "SimpleInfo.h"

using namespace ClearRoomLibrary;
using namespace msclr::interop;

SimpleInfo::SimpleInfo()
{
	m_info = nullptr;
}

ClearRoomLibrary::SimpleInfo::~SimpleInfo()
{
	if (m_info != nullptr)
		CSimpleInfo::Release(m_info);

	m_info = nullptr;
}

SimpleInfo^ SimpleInfo::GetInfo(String^ fileName)
{
	SimpleInfo^ info = gcnew SimpleInfo();

	marshal_context^ context = gcnew marshal_context();
	info->m_info = CSimpleInfo::GetInfo(context->marshal_as<const char*>(fileName));
	delete context;

	return info;
}
