#include "stdafx.h"
#include "Options.h"

using namespace ClearRoomLibrary;

Options::Options()
{
	_options = new COptions();
}

Options::~Options()
{
	if (_options != nullptr)
	{
		delete _options;
		_options = nullptr;
	}
}
