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

#include "tiffTag.h"

struct tiff_hdr
{
	unsigned short order;
	unsigned short magic;
	int ifd;
	unsigned short pad;
	unsigned short ntag;
	tiff_tag tag[23];
	int nextifd;
	unsigned short pad2;
	unsigned short nexif;
	tiff_tag exif[4];
	unsigned short pad3;
	unsigned short ngps;
	tiff_tag gpst[10];
	short bps[4];
	int rat[10];
	unsigned gps[26];
	char desc[512];
	char make[64];
	char model[64];
	char soft[32];
	char date[20];
	char artist[64];
};
