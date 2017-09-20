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

#pragma once

#include "COptions.h"

using namespace Unmanaged;
using namespace System;

namespace ClearRoomLibrary
{
	public ref class Options sealed
	{
	public:
		// Constructor
		Options();
		Options(COptions* options);

		~Options();

		property Single Threshold { Single get(); void set(Single value); }
		property Single Bright { Single get(); void set(Single value); }
		property array<Single>^ CustomWhiteBalance {array<Single>^ get(); void set(array<Single>^ value); }
		property array<UInt32>^ AverageAreaWhiteBalance {array<UInt32>^ get(); void set(array<UInt32>^ value); }
		property Boolean AutoWhiteBalance { Boolean get(); void set(Boolean value); }
		property Boolean CameraWhiteBalance { Boolean get(); void set(Boolean value); }
		property Boolean UseCameraMatrix { Boolean get(); void set(Boolean value); }
		property array<Double>^ GammaCurveCustom {array<Double>^ get(); void set(array<Double>^ value); }
		property array<Double>^ CorrectAberration {array<Double>^ get(); void set(array<Double>^ value); }
		property Int32 Darkness {Int32 get(); void set(Int32 value); }
		property Int32 Saturation {Int32 get(); void set(Int32 value); }
		property Int32 MedianPasses {Int32 get(); void set(Int32 value); }
		property Int32 Flip {Int32 get(); void set(Int32 value); }
		property Int32 Quality {Int32 get(); void set(Int32 value); }
		property Int32 Highlight {Int32 get(); void set(Int32 value); }
		property Int32 SelectRawImage {Int32 get(); void set(Int32 value); }
		property Int32 ColorSpace {Int32 get(); void set(Int32 value); }
		property String^ ProfileICC {String^ get(); void set(String^ value); }
		property String^ CameraProfileICC {String^ get(); void set(String^ value); }
		property String^ DeadPixel {String^ get(); void set(String^ value); }
		property String^ DarkFrame {String^ get(); void set(String^ value); }
		property Boolean ThumbnailOnly { Boolean get(); void set(Boolean value); }
		property Boolean HalfSize { Boolean get(); void set(Boolean value); }
		property Boolean FourColorRGB { Boolean get(); void set(Boolean value); }
		property Boolean UseFujiRotate { Boolean get(); void set(Boolean value); }
		property Boolean NoAutoBright { Boolean get(); void set(Boolean value); }
		property Boolean OutputTiff { Boolean get(); void set(Boolean value); }
		property Int32 OutputBps {Int32 get(); void set(Int32 value); }
		property Int32 DocumentMode {Int32 get(); void set(Int32 value); }

	private:
		//Private members
		COptions* _options;
	};
}
