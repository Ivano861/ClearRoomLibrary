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
#include "Options.h"

using namespace ClearRoomLibrary;
using namespace msclr::interop;

Options::Options()
{
	_options = new COptions();
}
Options::Options(COptions* options)
{
	_options = options;
}

Options::~Options()
{
	if (_options)
	{
		delete _options;
		_options = nullptr;
	}
}

Single Options::Threshold::get()
{
	return _options->Threshold();
}
void Options::Threshold::set(Single value)
{
	_options->Threshold() = value;
}

Single Options::Bright::get()
{
	return _options->Bright();
}
void Options::Bright::set(Single value)
{
	_options->Bright() = value;
}

array<Single>^ Options::CustomWhiteBalance::get()
{
	array<Single>^ result = gcnew array<Single>(4);

	for (size_t i = 0; i < 4; i++)
		result[i] = _options->_userMul[i];

	return result;
}
void Options::CustomWhiteBalance::set(array<Single>^ value)
{
	for (size_t i = 0; i < value->Length, i < 4; i++)
		_options->_userMul[i] = value[i];
}

array<UInt32>^ Options::AverageAreaWhiteBalance::get()
{
	array<UInt32>^ result = gcnew array<UInt32>(4);

	for (size_t i = 0; i < 4; i++)
		result[i] = _options->_greybox[i];

	return result;
}
void Options::AverageAreaWhiteBalance::set(array<UInt32>^ value)
{
	for (size_t i = 0; i < value->Length, i < 4; i++)
		_options->_greybox[i] = value[i];
}

Boolean Options::AutoWhiteBalance::get()
{
	return _options->AutoWhiteBalance();
}
void Options::AutoWhiteBalance::set(Boolean value)
{
	_options->AutoWhiteBalance() = value;
}

Boolean Options::CameraWhiteBalance::get()
{
	return _options->CameraWhiteBalance();
}
void Options::CameraWhiteBalance::set(Boolean value)
{
	_options->CameraWhiteBalance() = value;
}

Boolean Options::UseCameraMatrix::get()
{
	return _options->UseCameraMatrix();
}
void Options::UseCameraMatrix::set(Boolean value)
{
	_options->UseCameraMatrix() = value;
}

array<Double>^ Options::GammaCurveCustom::get()
{
	array<Double>^ result = gcnew array<Double>(2);

	for (size_t i = 0; i < 2; i++)
		result[i] = _options->_gamma[i];

	return result;
}
void Options::GammaCurveCustom::set(array<Double>^ value)
{
	for (size_t i = 0; i < value->Length, i < 2; i++)
		_options->_gamma[i] = value[i];
}

array<Double>^ Options::CorrectAberration::get()
{
	array<Double>^ result = gcnew array<Double>(2);

	result[0] = _options->GetCorrectAberration0();
	result[1] = _options->GetCorrectAberration2();

	return result;
}
void Options::CorrectAberration::set(array<Double>^ value)
{
	if (value->Length >= 1)
		_options->SetCorrectAberration0(value[0]);
	if (value->Length >= 2)
		_options->SetCorrectAberration2(value[1]);
}

Int32 Options::Darkness::get()
{
	return _options->Darkness();
}
void Options::Darkness::set(Int32 value)
{
	_options->Darkness() = value;
}

Int32 Options::Saturation::get()
{
	return _options->Saturation();
}
void Options::Saturation::set(Int32 value)
{
	_options->Saturation() = value;
}

Int32 Options::MedianPasses::get()
{
	return _options->MedianPasses();
}
void Options::MedianPasses::set(Int32 value)
{
	_options->MedianPasses() = value;
}

Int32 Options::Flip::get()
{
	return _options->Flip();
}
void Options::Flip::set(Int32 value)
{
	if (value < 0)
		_options->Flip() = 0;
	else if(value > 7)
		_options->Flip() = 7;
	else
		_options->Flip() = value;
}

Int32 Options::Quality::get()
{
	return _options->Quality();
}
void Options::Quality::set(Int32 value)
{
	if (value < 0)
		_options->Quality() = 0;
	else if (value > 3)
		_options->Quality() = 3;
	else
		_options->Quality() = value;
}

Int32 Options::Highlight::get()
{
	return _options->Highlight();
}
void Options::Highlight::set(Int32 value)
{
	if (value < 0)
		_options->Highlight() = 0;
	else if (value > 9)
		_options->Highlight() = 9;
	else
		_options->Highlight() = value;
}

Int32 Options::SelectRawImage::get()
{
	return _options->GetSelectRawImage();
}
void Options::SelectRawImage::set(Int32 value)
{
	_options->SetSelectRawImage(value);
}

Int32 Options::ColorSpace::get()
{
	return _options->ColorSpace();
}
void Options::ColorSpace::set(Int32 value)
{
	_options->ColorSpace() = value;
}

String^ Options::ProfileICC::get()
{
	return marshal_as<String^>(_options->GetProfileICC());
}
void Options::ProfileICC::set(String^ value)
{
	marshal_context^ context = gcnew marshal_context();
	_options->SetProfileICC(context->marshal_as<const char*>(value));
	delete context;
}

String^ Options::CameraProfileICC::get()
{
	return marshal_as<String^>(_options->GetCameraProfileICC());
}
void Options::CameraProfileICC::set(String^ value)
{
	marshal_context^ context = gcnew marshal_context();
	_options->SetCameraProfileICC(context->marshal_as<const char*>(value));
	delete context;
}

String^ Options::DeadPixel::get()
{
	return marshal_as<String^>(_options->GetDeadPixel());
}
void Options::DeadPixel::set(String^ value)
{
	marshal_context^ context = gcnew marshal_context();
	_options->SetDeadPixel(context->marshal_as<const char*>(value));
	delete context;
}

String^ Options::DarkFrame::get()
{
	return marshal_as<String^>(_options->GetDarkFrame());
}
void Options::DarkFrame::set(String^ value)
{
	marshal_context^ context = gcnew marshal_context();
	_options->SetDarkFrame(context->marshal_as<const char*>(value));
	delete context;
}

Boolean Options::ThumbnailOnly::get()
{
	return _options->ThumbnailOnly();
}
void Options::ThumbnailOnly::set(Boolean value)
{
	_options->ThumbnailOnly() = value;
}

Boolean Options::HalfSize::get()
{
	return _options->HalfSize();
}
void Options::HalfSize::set(Boolean value)
{
	_options->HalfSize() = value;
}

Boolean Options::FourColorRGB::get()
{
	return _options->FourColorRGB();
}
void Options::FourColorRGB::set(Boolean value)
{
	_options->FourColorRGB() = value;
}

Boolean Options::UseFujiRotate::get()
{
	return _options->UseFujiRotate();
}
void Options::UseFujiRotate::set(Boolean value)
{
	_options->UseFujiRotate() = value;
}

Boolean Options::NoAutoBright::get()
{
	return _options->NoAutoBright();
}
void Options::NoAutoBright::set(Boolean value)
{
	_options->NoAutoBright() = value;
}

Boolean Options::OutputTiff::get()
{
	return _options->OutputTiff();
}
void Options::OutputTiff::set(Boolean value)
{
	_options->OutputTiff() = value;
}

Int32 Options::OutputBps::get()
{
	return _options->OutputBps();
}
void Options::OutputBps::set(Int32 value)
{
	if (value <= 8)
		_options->OutputBps() = 8;
	else
		_options->OutputBps() = 16;
}

Int32 Options::DocumentMode::get()
{
	return _options->DocumentMode();
}
void Options::DocumentMode::set(Int32 value)
{
	if (value < 0)
		_options->DocumentMode() = 0;
	else if (value > 3)
		_options->DocumentMode() = 3;
	else
		_options->DocumentMode() = value;
}
