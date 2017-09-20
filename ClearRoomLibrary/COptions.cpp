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
#include "COptions.h"

using namespace Unmanaged;

COptions::COptions() : _threshold(0), _bright(1.0f), _userBlack(-1), _aber{ 1, 1, 1, 1 }, _gamma{ 0.45, 4.5, 0, 0, 0, 0 },
_userSaturation(-1), _userFlip(-1), _userQuality(-1), _medianPasses(0), _highlight(0), _shotSelect(0), _multiOut(false), _outputColor(1),
_outProfile(nullptr), _cameraProfile(nullptr), _bpFile(nullptr), _darkFrame(nullptr), _halfSize(false), _fourColorRGB(false),
_userMul{ 0, 0, 0, 0 }, _greybox{ 0, 0, UINT_MAX, UINT_MAX }, _useAutoWB(false), _useCameraWB(false),
_useCameraMatrix(true), _documentMode(0), _useFujiRotate(true),
_noAutoBright(false), _outputTiff(true), _outputBps(8), _thumbnailOnly(false)
{
}

COptions::~COptions()
{
	if (_outProfile != nullptr)
	{
		delete[] _outProfile;
		_outProfile = nullptr;
	}
	if (_cameraProfile != nullptr)
	{
		delete[] _cameraProfile;
		_cameraProfile = nullptr;
	}
	if (_bpFile != nullptr)
	{
		delete[] _bpFile;
		_bpFile = nullptr;
	}
	if (_darkFrame != nullptr)
	{
		delete[] _darkFrame;
		_darkFrame = nullptr;
	}
}

float& COptions::Threshold()
{
	return _threshold;
}

float& COptions::Bright()
{
	return _bright;
}

float& COptions::CustomWhiteBalance()
{
	return _userMul[0];
}

unsigned& COptions::AverageAreaWhiteBalance()
{
	return _greybox[0];
}

bool& COptions::AutoWhiteBalance()
{
	return _useAutoWB;
}

bool& COptions::CameraWhiteBalance()
{
	return _useCameraWB;
}

bool& COptions::UseCameraMatrix()
{
	return _useCameraMatrix;
}

double& COptions::GammaCurveCustom()
{
	return _gamma[0];
}

double COptions::GetCorrectAberration0()
{
	return 1.0 / _aber[0];
}
double COptions::GetCorrectAberration2()
{
	return 1.0 / _aber[2];
}
void COptions::SetCorrectAberration0(double value)
{
	if (value != 0.0)
		_aber[0] = 1.0 / value;
}
void COptions::SetCorrectAberration2(double value)
{
	if (value != 0.0)
		_aber[2] = 1.0 / value;
}

int& COptions::Darkness()
{
	return _userBlack;
}

int& COptions::Saturation()
{
	return _userSaturation;
}

int& COptions::MedianPasses()
{
	return _medianPasses;
}

int& COptions::Flip()
{
	return _userFlip;
}

int& COptions::Quality()
{
	return _userQuality;
}

int& COptions::Highlight()
{
	return _highlight;
}

int COptions::GetSelectRawImage()
{
	if (_multiOut)
		return -1;

	return _shotSelect;
}
void COptions::SetSelectRawImage(int value)
{
	if (value == -1)
	{
		_multiOut = true;
		_shotSelect = 0;
	}
	else
	{
		_multiOut = false;
		_shotSelect = value;
	}
}

int& COptions::ColorSpace()
{
	return _outputColor;
}

const char* COptions::GetProfileICC()
{
	return _outProfile;
}
void COptions::SetProfileICC(const char* value)
{
	if (_outProfile != nullptr)
	{
		delete[] _outProfile;
		_outProfile = nullptr;
	}
	int len = strlen(value) + 1;
	_outProfile = new char[len];
	strcpy_s(_outProfile, len, value);
}

const char* COptions::GetCameraProfileICC()
{
	return _cameraProfile;
}
void COptions::SetCameraProfileICC(const char* value)
{
	if (_cameraProfile != nullptr)
	{
		delete[] _cameraProfile;
		_cameraProfile = nullptr;
	}
	int len = strlen(value) + 1;
	_cameraProfile = new char[len];
	strcpy_s(_cameraProfile, len, value);
}

const char* COptions::GetDeadPixel()
{
	return _bpFile;
}
void COptions::SetDeadPixel(const char* value)
{
	if (_bpFile != nullptr)
	{
		delete[] _bpFile;
		_bpFile = nullptr;
	}
	int len = strlen(value) + 1;
	_bpFile = new char[len];
	strcpy_s(_bpFile, len, value);
}

const char* COptions::GetDarkFrame()
{
	return _darkFrame;
}
void COptions::SetDarkFrame(const char* value)
{
	if (_darkFrame != nullptr)
	{
		delete[] _darkFrame;
		_darkFrame = nullptr;
	}
	int len = strlen(value) + 1;
	_darkFrame = new char[len];
	strcpy_s(_darkFrame, len, value);
}

bool& COptions::ThumbnailOnly()
{
	return _thumbnailOnly;
}

bool& COptions::HalfSize()
{
	return _halfSize;
}

bool& COptions::FourColorRGB()
{
	return _fourColorRGB;
}

bool& COptions::UseFujiRotate()
{
	return _useFujiRotate;
}

bool& COptions::NoAutoBright()
{
	return _noAutoBright;
}

bool& COptions::OutputTiff()
{
	return _outputTiff;
}

int& COptions::OutputBps()
{
	return _outputBps;
}

int& COptions::DocumentMode()
{
	return _documentMode;
}

