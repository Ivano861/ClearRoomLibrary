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

namespace Unmanaged
{
	enum LoadRawType
	{
		UnknownLoadRaw,
		PackedLoadRaw,
		UnpackedLoadRaw,
		LosslessDngLoadRaw,
		LossyDngLoadRaw,
		PackedDngLoadRaw,
		LosslessJpegLoadRaw,
		CanonLoadRaw,
		Canon600LoadRaw,
		CanonRmfLoadRaw,
		CanonSrawLoadRaw,
		EightBitLoadRaw,
		FoveonDpLoadRaw,
		FoveonSdLoadRaw,
		HasselbladLoadRaw,
		ImaconFullLoadRaw,
		Kodak262LoadRaw,
		Kodak65000LoadRaw,
		KodakC330LoadRaw,
		KodakC603LoadRaw,
		KodakDC120LoadRaw,
		KodakJpegLoadRaw,
		KodakRadcLoadRaw,
		KodakRgbLoadRaw,
		KodakYcbcrLoadRaw,
		LeafHdrLoadRaw,
		MinoltaRD175LoadRaw,
		NikonLoadRaw,
		NikonYuvLoadRaw,
		NokiaLoadRaw,
		OlympusLoadRaw,
		PanasonicLoadRaw,
		PentaxLoadRaw,
		PhaseOneLoadRaw,
		PhaseOneLoadRawC,
		Quicktake100LoadRaw,
		RedcineLoadRaw,
		RolleiLoadRaw,
		SamsungLoadRaw,
		Samsung2LoadRaw,
		Samsung3LoadRaw,
		Sinar4ShotLoadRaw,
		SmalV6LoadRaw,
		SmalV9LoadRaw,
		SonyLoadRaw,
		SonyArwLoadRaw,
		SonyArw2LoadRaw,

		LodakThumbLoadRaw,
	};

	enum WriteThumbType
	{
		UnknownWrite,
		LayerThumb,
		PpmThumb,
		Ppm16Thumb,
		RolleiThumb,
		JpegThumb,
		FoveonThumb,

		WritePpmTiff
	};
}
