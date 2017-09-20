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

#include "Stdafx.h"
#include "CSimpleInfo.h"
#include "jhead.h"
#include "CException.h"
#include "CAutoFreeMemory.h"
#include <time.h>
#include "Macro.h"

using namespace Unmanaged;

#pragma region Costructors
CSimpleInfo::CSimpleInfo(CReader* reader, COptions* options) : _reader(reader), _options(options)
{
}
#pragma endregion

#pragma region Destructor
CSimpleInfo::~CSimpleInfo()
{
	_reader = nullptr;
}
#pragma endregion

#pragma region Public methods
void CSimpleInfo::GetInfo()
{
	static const short pana[][6] = {
		{ 3130, 1743,  4,  0, -6,  0 },
		{ 3130, 2055,  4,  0, -6,  0 },
		{ 3130, 2319,  4,  0, -6,  0 },
		{ 3170, 2103, 18,  0,-42, 20 },
		{ 3170, 2367, 18, 13,-42,-21 },
		{ 3177, 2367,  0,  0, -1,  0 },
		{ 3304, 2458,  0,  0, -1,  0 },
		{ 3330, 2463,  9,  0, -5,  0 },
		{ 3330, 2479,  9,  0,-17,  4 },
		{ 3370, 1899, 15,  0,-44, 20 },
		{ 3370, 2235, 15,  0,-44, 20 },
		{ 3370, 2511, 15, 10,-44,-21 },
		{ 3690, 2751,  3,  0, -8, -3 },
		{ 3710, 2751,  0,  0, -3,  0 },
		{ 3724, 2450,  0,  0,  0, -2 },
		{ 3770, 2487, 17,  0,-44, 19 },
		{ 3770, 2799, 17, 15,-44,-19 },
		{ 3880, 2170,  6,  0, -6,  0 },
		{ 4060, 3018,  0,  0,  0, -2 },
		{ 4290, 2391,  3,  0, -8, -1 },
		{ 4330, 2439, 17, 15,-44,-19 },
		{ 4508, 2962,  0,  0, -3, -4 },
		{ 4508, 3330,  0,  0, -3, -6 },
	};
	static const unsigned short canon[][11] = {
		{ 1944, 1416,   0,  0, 48,  0 },
		{ 2144, 1560,   4,  8, 52,  2, 0, 0, 0, 25 },
		{ 2224, 1456,  48,  6,  0,  2 },
		{ 2376, 1728,  12,  6, 52,  2 },
		{ 2672, 1968,  12,  6, 44,  2 },
		{ 3152, 2068,  64, 12,  0,  0, 16 },
		{ 3160, 2344,  44, 12,  4,  4 },
		{ 3344, 2484,   4,  6, 52,  6 },
		{ 3516, 2328,  42, 14,  0,  0 },
		{ 3596, 2360,  74, 12,  0,  0 },
		{ 3744, 2784,  52, 12,  8, 12 },
		{ 3944, 2622,  30, 18,  6,  2 },
		{ 3948, 2622,  42, 18,  0,  2 },
		{ 3984, 2622,  76, 20,  0,  2, 14 },
		{ 4104, 3048,  48, 12, 24, 12 },
		{ 4116, 2178,   4,  2,  0,  0 },
		{ 4152, 2772, 192, 12,  0,  0 },
		{ 4160, 3124, 104, 11,  8, 65 },
		{ 4176, 3062,  96, 17,  8,  0, 0, 16, 0, 7, 0x49 },
		{ 4192, 3062,  96, 17, 24,  0, 0, 16, 0, 0, 0x49 },
		{ 4312, 2876,  22, 18,  0,  2 },
		{ 4352, 2874,  62, 18,  0,  0 },
		{ 4476, 2954,  90, 34,  0,  0 },
		{ 4480, 3348,  12, 10, 36, 12, 0, 0, 0, 18, 0x49 },
		{ 4480, 3366,  80, 50,  0,  0 },
		{ 4496, 3366,  80, 50, 12,  0 },
		{ 4768, 3516,  96, 16,  0,  0, 0, 16 },
		{ 4832, 3204,  62, 26,  0,  0 },
		{ 4832, 3228,  62, 51,  0,  0 },
		{ 5108, 3349,  98, 13,  0,  0 },
		{ 5120, 3318, 142, 45, 62,  0 },
		{ 5280, 3528,  72, 52,  0,  0 },
		{ 5344, 3516, 142, 51,  0,  0 },
		{ 5344, 3584, 126,100,  0,  2 },
		{ 5360, 3516, 158, 51,  0,  0 },
		{ 5568, 3708,  72, 38,  0,  0 },
		{ 5632, 3710,  96, 17,  0,  0, 0, 16, 0, 0, 0x49 },
		{ 5712, 3774,  62, 20, 10,  2 },
		{ 5792, 3804, 158, 51,  0,  0 },
		{ 5920, 3950, 122, 80,  2,  0 },
		{ 6096, 4056,  72, 34,  0,  0 },
		{ 6288, 4056, 264, 34,  0,  0 },
		{ 8896, 5920, 160, 64,  0,  0 },
	};
	static const struct
	{
		unsigned short id;
		char model[20];
	} unique[] = {
		{ 0x168, "EOS 10D" },{ 0x001, "EOS-1D" },
		{ 0x175, "EOS 20D" },{ 0x174, "EOS-1D Mark II" },
		{ 0x234, "EOS 30D" },{ 0x232, "EOS-1D Mark II N" },
		{ 0x190, "EOS 40D" },{ 0x169, "EOS-1D Mark III" },
		{ 0x261, "EOS 50D" },{ 0x281, "EOS-1D Mark IV" },
		{ 0x287, "EOS 60D" },{ 0x167, "EOS-1DS" },
		{ 0x325, "EOS 70D" },
		{ 0x350, "EOS 80D" },{ 0x328, "EOS-1D X Mark II" },
		{ 0x170, "EOS 300D" },{ 0x188, "EOS-1Ds Mark II" },
		{ 0x176, "EOS 450D" },{ 0x215, "EOS-1Ds Mark III" },
		{ 0x189, "EOS 350D" },{ 0x324, "EOS-1D C" },
		{ 0x236, "EOS 400D" },{ 0x269, "EOS-1D X" },
		{ 0x252, "EOS 500D" },{ 0x213, "EOS 5D" },
		{ 0x270, "EOS 550D" },{ 0x218, "EOS 5D Mark II" },
		{ 0x286, "EOS 600D" },{ 0x285, "EOS 5D Mark III" },
		{ 0x301, "EOS 650D" },{ 0x302, "EOS 6D" },
		{ 0x326, "EOS 700D" },{ 0x250, "EOS 7D" },
		{ 0x393, "EOS 750D" },{ 0x289, "EOS 7D Mark II" },
		{ 0x347, "EOS 760D" },
		{ 0x254, "EOS 1000D" },
		{ 0x288, "EOS 1100D" },
		{ 0x327, "EOS 1200D" },{ 0x382, "Canon EOS 5DS" },
		{ 0x404, "EOS 1300D" },{ 0x401, "Canon EOS 5DS R" },
		{ 0x346, "EOS 100D" },
	}, sonique[] = {
		{ 0x002, "DSC-R1" },{ 0x100, "DSLR-A100" },
		{ 0x101, "DSLR-A900" },{ 0x102, "DSLR-A700" },
		{ 0x103, "DSLR-A200" },{ 0x104, "DSLR-A350" },
		{ 0x105, "DSLR-A300" },{ 0x108, "DSLR-A330" },
		{ 0x109, "DSLR-A230" },{ 0x10a, "DSLR-A290" },
		{ 0x10d, "DSLR-A850" },{ 0x111, "DSLR-A550" },
		{ 0x112, "DSLR-A500" },{ 0x113, "DSLR-A450" },
		{ 0x116, "NEX-5" },{ 0x117, "NEX-3" },
		{ 0x118, "SLT-A33" },{ 0x119, "SLT-A55V" },
		{ 0x11a, "DSLR-A560" },{ 0x11b, "DSLR-A580" },
		{ 0x11c, "NEX-C3" },{ 0x11d, "SLT-A35" },
		{ 0x11e, "SLT-A65V" },{ 0x11f, "SLT-A77V" },
		{ 0x120, "NEX-5N" },{ 0x121, "NEX-7" },
		{ 0x123, "SLT-A37" },{ 0x124, "SLT-A57" },
		{ 0x125, "NEX-F3" },{ 0x126, "SLT-A99V" },
		{ 0x127, "NEX-6" },{ 0x128, "NEX-5R" },
		{ 0x129, "DSC-RX100" },{ 0x12a, "DSC-RX1" },
		{ 0x12e, "ILCE-3000" },{ 0x12f, "SLT-A58" },
		{ 0x131, "NEX-3N" },{ 0x132, "ILCE-7" },
		{ 0x133, "NEX-5T" },{ 0x134, "DSC-RX100M2" },
		{ 0x135, "DSC-RX10" },{ 0x136, "DSC-RX1R" },
		{ 0x137, "ILCE-7R" },{ 0x138, "ILCE-6000" },
		{ 0x139, "ILCE-5000" },{ 0x13d, "DSC-RX100M3" },
		{ 0x13e, "ILCE-7S" },{ 0x13f, "ILCA-77M2" },
		{ 0x153, "ILCE-5100" },{ 0x154, "ILCE-7M2" },
		{ 0x155, "DSC-RX100M4" },{ 0x156, "DSC-RX10M2" },
		{ 0x158, "DSC-RX1RM2" },{ 0x15a, "ILCE-QX1" },
		{ 0x15b, "ILCE-7RM2" },{ 0x15e, "ILCE-7SM2" },
		{ 0x161, "ILCA-68" },{ 0x165, "ILCE-6300" },
	};
	static const struct
	{
		unsigned fsize;
		unsigned short rw, rh;
		unsigned char lm, tm, rm, bm, lf, cf, max, flags;
		char make[10], model[20];
		unsigned short offset;
	} table[] = {
		{ 786432,1024, 768, 0, 0, 0, 0, 0,0x94,0,0,"AVT","F-080C" },
		{ 1447680,1392,1040, 0, 0, 0, 0, 0,0x94,0,0,"AVT","F-145C" },
		{ 1920000,1600,1200, 0, 0, 0, 0, 0,0x94,0,0,"AVT","F-201C" },
		{ 5067304,2588,1958, 0, 0, 0, 0, 0,0x94,0,0,"AVT","F-510C" },
		{ 5067316,2588,1958, 0, 0, 0, 0, 0,0x94,0,0,"AVT","F-510C",12 },
		{ 10134608,2588,1958, 0, 0, 0, 0, 9,0x94,0,0,"AVT","F-510C" },
		{ 10134620,2588,1958, 0, 0, 0, 0, 9,0x94,0,0,"AVT","F-510C",12 },
		{ 16157136,3272,2469, 0, 0, 0, 0, 9,0x94,0,0,"AVT","F-810C" },
		{ 15980544,3264,2448, 0, 0, 0, 0, 8,0x61,0,1,"AgfaPhoto","DC-833m" },
		{ 9631728,2532,1902, 0, 0, 0, 0,96,0x61,0,0,"Alcatel","5035D" },
		{ 2868726,1384,1036, 0, 0, 0, 0,64,0x49,0,8,"Baumer","TXG14",1078 },
		{ 5298000,2400,1766,12,12,44, 2,40,0x94,0,2,"Canon","PowerShot SD300" },
		{ 6553440,2664,1968, 4, 4,44, 4,40,0x94,0,2,"Canon","PowerShot A460" },
		{ 6573120,2672,1968,12, 8,44, 0,40,0x94,0,2,"Canon","PowerShot A610" },
		{ 6653280,2672,1992,10, 6,42, 2,40,0x94,0,2,"Canon","PowerShot A530" },
		{ 7710960,2888,2136,44, 8, 4, 0,40,0x94,0,2,"Canon","PowerShot S3 IS" },
		{ 9219600,3152,2340,36,12, 4, 0,40,0x94,0,2,"Canon","PowerShot A620" },
		{ 9243240,3152,2346,12, 7,44,13,40,0x49,0,2,"Canon","PowerShot A470" },
		{ 10341600,3336,2480, 6, 5,32, 3,40,0x94,0,2,"Canon","PowerShot A720 IS" },
		{ 10383120,3344,2484,12, 6,44, 6,40,0x94,0,2,"Canon","PowerShot A630" },
		{ 12945240,3736,2772,12, 6,52, 6,40,0x94,0,2,"Canon","PowerShot A640" },
		{ 15636240,4104,3048,48,12,24,12,40,0x94,0,2,"Canon","PowerShot A650" },
		{ 15467760,3720,2772, 6,12,30, 0,40,0x94,0,2,"Canon","PowerShot SX110 IS" },
		{ 15534576,3728,2778,12, 9,44, 9,40,0x94,0,2,"Canon","PowerShot SX120 IS" },
		{ 18653760,4080,3048,24,12,24,12,40,0x94,0,2,"Canon","PowerShot SX20 IS" },
		{ 19131120,4168,3060,92,16, 4, 1,40,0x94,0,2,"Canon","PowerShot SX220 HS" },
		{ 21936096,4464,3276,25,10,73,12,40,0x16,0,2,"Canon","PowerShot SX30 IS" },
		{ 24724224,4704,3504, 8,16,56, 8,40,0x94,0,2,"Canon","PowerShot A3300 IS" },
		{ 30858240,5248,3920, 8,16,56,16,40,0x94,0,2,"Canon","IXUS 160" },
		{ 1976352,1632,1211, 0, 2, 0, 1, 0,0x94,0,1,"Casio","QV-2000UX" },
		{ 3217760,2080,1547, 0, 0,10, 1, 0,0x94,0,1,"Casio","QV-3*00EX" },
		{ 6218368,2585,1924, 0, 0, 9, 0, 0,0x94,0,1,"Casio","QV-5700" },
		{ 7816704,2867,2181, 0, 0,34,36, 0,0x16,0,1,"Casio","EX-Z60" },
		{ 2937856,1621,1208, 0, 0, 1, 0, 0,0x94,7,13,"Casio","EX-S20" },
		{ 4948608,2090,1578, 0, 0,32,34, 0,0x94,7,1,"Casio","EX-S100" },
		{ 6054400,2346,1720, 2, 0,32, 0, 0,0x94,7,1,"Casio","QV-R41" },
		{ 7426656,2568,1928, 0, 0, 0, 0, 0,0x94,0,1,"Casio","EX-P505" },
		{ 7530816,2602,1929, 0, 0,22, 0, 0,0x94,7,1,"Casio","QV-R51" },
		{ 7542528,2602,1932, 0, 0,32, 0, 0,0x94,7,1,"Casio","EX-Z50" },
		{ 7562048,2602,1937, 0, 0,25, 0, 0,0x16,7,1,"Casio","EX-Z500" },
		{ 7753344,2602,1986, 0, 0,32,26, 0,0x94,7,1,"Casio","EX-Z55" },
		{ 9313536,2858,2172, 0, 0,14,30, 0,0x94,7,1,"Casio","EX-P600" },
		{ 10834368,3114,2319, 0, 0,27, 0, 0,0x94,0,1,"Casio","EX-Z750" },
		{ 10843712,3114,2321, 0, 0,25, 0, 0,0x94,0,1,"Casio","EX-Z75" },
		{ 10979200,3114,2350, 0, 0,32,32, 0,0x94,7,1,"Casio","EX-P700" },
		{ 12310144,3285,2498, 0, 0, 6,30, 0,0x94,0,1,"Casio","EX-Z850" },
		{ 12489984,3328,2502, 0, 0,47,35, 0,0x94,0,1,"Casio","EX-Z8" },
		{ 15499264,3754,2752, 0, 0,82, 0, 0,0x94,0,1,"Casio","EX-Z1050" },
		{ 18702336,4096,3044, 0, 0,24, 0,80,0x94,7,1,"Casio","EX-ZR100" },
		{ 7684000,2260,1700, 0, 0, 0, 0,13,0x94,0,1,"Casio","QV-4000" },
		{ 787456,1024, 769, 0, 1, 0, 0, 0,0x49,0,0,"Creative","PC-CAM 600" },
		{ 28829184,4384,3288, 0, 0, 0, 0,36,0x61,0,0,"DJI" },
		{ 15151104,4608,3288, 0, 0, 0, 0, 0,0x94,0,0,"Matrix" },
		{ 3840000,1600,1200, 0, 0, 0, 0,65,0x49,0,0,"Foculus","531C" },
		{ 307200, 640, 480, 0, 0, 0, 0, 0,0x94,0,0,"Generic" },
		{ 62464, 256, 244, 1, 1, 6, 1, 0,0x8d,0,0,"Kodak","DC20" },
		{ 124928, 512, 244, 1, 1,10, 1, 0,0x8d,0,0,"Kodak","DC20" },
		{ 1652736,1536,1076, 0,52, 0, 0, 0,0x61,0,0,"Kodak","DCS200" },
		{ 4159302,2338,1779, 1,33, 1, 2, 0,0x94,0,0,"Kodak","C330" },
		{ 4162462,2338,1779, 1,33, 1, 2, 0,0x94,0,0,"Kodak","C330",3160 },
		{ 2247168,1232, 912, 0, 0,16, 0, 0,0x00,0,0,"Kodak","C330" },
		{ 3370752,1232, 912, 0, 0,16, 0, 0,0x00,0,0,"Kodak","C330" },
		{ 6163328,2864,2152, 0, 0, 0, 0, 0,0x94,0,0,"Kodak","C603" },
		{ 6166488,2864,2152, 0, 0, 0, 0, 0,0x94,0,0,"Kodak","C603",3160 },
		{ 460800, 640, 480, 0, 0, 0, 0, 0,0x00,0,0,"Kodak","C603" },
		{ 9116448,2848,2134, 0, 0, 0, 0, 0,0x00,0,0,"Kodak","C603" },
		{ 12241200,4040,3030, 2, 0, 0,13, 0,0x49,0,0,"Kodak","12MP" },
		{ 12272756,4040,3030, 2, 0, 0,13, 0,0x49,0,0,"Kodak","12MP",31556 },
		{ 18000000,4000,3000, 0, 0, 0, 0, 0,0x00,0,0,"Kodak","12MP" },
		{ 614400, 640, 480, 0, 3, 0, 0,64,0x94,0,0,"Kodak","KAI-0340" },
		{ 15360000,3200,2400, 0, 0, 0, 0,96,0x16,0,0,"Lenovo","A820" },
		{ 3884928,1608,1207, 0, 0, 0, 0,96,0x16,0,0,"Micron","2010",3212 },
		{ 1138688,1534, 986, 0, 0, 0, 0, 0,0x61,0,0,"Minolta","RD175",513 },
		{ 1581060,1305, 969, 0, 0,18, 6, 6,0x1e,4,1,"Nikon","E900" },
		{ 2465792,1638,1204, 0, 0,22, 1, 6,0x4b,5,1,"Nikon","E950" },
		{ 2940928,1616,1213, 0, 0, 0, 7,30,0x94,0,1,"Nikon","E2100" },
		{ 4771840,2064,1541, 0, 0, 0, 1, 6,0xe1,0,1,"Nikon","E990" },
		{ 4775936,2064,1542, 0, 0, 0, 0,30,0x94,0,1,"Nikon","E3700" },
		{ 5865472,2288,1709, 0, 0, 0, 1, 6,0xb4,0,1,"Nikon","E4500" },
		{ 5869568,2288,1710, 0, 0, 0, 0, 6,0x16,0,1,"Nikon","E4300" },
		{ 7438336,2576,1925, 0, 0, 0, 1, 6,0xb4,0,1,"Nikon","E5000" },
		{ 8998912,2832,2118, 0, 0, 0, 0,30,0x94,7,1,"Nikon","COOLPIX S6" },
		{ 5939200,2304,1718, 0, 0, 0, 0,30,0x16,0,0,"Olympus","C770UZ" },
		{ 3178560,2064,1540, 0, 0, 0, 0, 0,0x94,0,1,"Pentax","Optio S" },
		{ 4841984,2090,1544, 0, 0,22, 0, 0,0x94,7,1,"Pentax","Optio S" },
		{ 6114240,2346,1737, 0, 0,22, 0, 0,0x94,7,1,"Pentax","Optio S4" },
		{ 10702848,3072,2322, 0, 0, 0,21,30,0x94,0,1,"Pentax","Optio 750Z" },
		{ 4147200,1920,1080, 0, 0, 0, 0, 0,0x49,0,0,"Photron","BC2-HD" },
		{ 4151666,1920,1080, 0, 0, 0, 0, 0,0x49,0,0,"Photron","BC2-HD",8 },
		{ 13248000,2208,3000, 0, 0, 0, 0,13,0x61,0,0,"Pixelink","A782" },
		{ 6291456,2048,1536, 0, 0, 0, 0,96,0x61,0,0,"RoverShot","3320AF" },
		{ 311696, 644, 484, 0, 0, 0, 0, 0,0x16,0,8,"ST Micro","STV680 VGA" },
		{ 16098048,3288,2448, 0, 0,24, 0, 9,0x94,0,1,"Samsung","S85" },
		{ 16215552,3312,2448, 0, 0,48, 0, 9,0x94,0,1,"Samsung","S85" },
		{ 20487168,3648,2808, 0, 0, 0, 0,13,0x94,5,1,"Samsung","WB550" },
		{ 24000000,4000,3000, 0, 0, 0, 0,13,0x94,5,1,"Samsung","WB550" },
		{ 12582980,3072,2048, 0, 0, 0, 0,33,0x61,0,0,"Sinar","",68 },
		{ 33292868,4080,4080, 0, 0, 0, 0,33,0x61,0,0,"Sinar","",68 },
		{ 44390468,4080,5440, 0, 0, 0, 0,33,0x61,0,0,"Sinar","",68 },
		{ 1409024,1376,1024, 0, 0, 1, 0, 0,0x49,0,0,"Sony","XCD-SX910CR" },
		{ 2818048,1376,1024, 0, 0, 1, 0,97,0x49,0,0,"Sony","XCD-SX910CR" },
	};
	static const char *corp[] =
	{ "AgfaPhoto", "Canon", "Casio", "Epson", "Fujifilm",
		"Mamiya", "Minolta", "Motorola", "Kodak", "Konica", "Leica",
		"Nikon", "Nokia", "Olympus", "Ricoh", "Pentax", "Phase One",
		"Samsung", "Sigma", "Sinar", "Sony" };

	_tiffFlip = _flip = _filters = UINT_MAX;	/* unknown */
	_rawHeight = _rawWidth = _fujiWidth = _fujiLayout = _cr2Slice[0] = 0;
	_maximum = _height = _width = _topMargin = _leftMargin = 0;
	_cdesc[0] = _desc[0] = _artist[0] = _make[0] = _model[0] = _model2[0] = 0;
	_isoSpeed = _shutter = _aperture = _focalLen = 0;
	_uniqueId = 0;
	_tiffNifds = 0;
	memset(_tiffIfd, 0, sizeof _tiffIfd);
	memset(_gpsData, 0, sizeof _gpsData);
	memset(_cBlack, 0, sizeof _cBlack);
	memset(_white, 0, sizeof _white);
	memset(_mask, 0, sizeof _mask);
	_thumbOffset = _thumbLength = _thumbWidth = _thumbHeight = 0;
	_loadRaw = LoadRawType::UnknownLoadRaw;
	_thumbLoadRaw = LoadRawType::UnknownLoadRaw;
	_writeThumb = WriteThumbType::JpegThumb;
	_writeFun = WriteThumbType::UnknownWrite;
	_dataOffset = _metaOffset = _metaLength = _tiffBps = _tiffCompress = 0;
	_kodakCbpp = _zeroAfterFF = _dngVersion = _loadFlags = 0;
	_timestamp = _shotOrder = _tiffSamples = _black = _isFoveon = 0;
	_profileLength = _dataError = _zeroIsBad = 0;
	_pixelAspect = _isRaw = _rawColor = 1;
	_tileWidth = _tileLength = 0;
	for (size_t i = 0; i < 4; i++)
	{
		_camMul[i] = i == 1;
		_preMul[i] = i < 3;
		for (size_t c = 0; c < 3; c++)
			_cMatrix[c][i] = 0;
		for (size_t c = 0; c < 3; c++)
			_rgbCam[c][i] = c == i;
	}
	_colors = 3;
	for (size_t i = 0; i < 0x10000; i++)
		_curve[i] = (unsigned short)i;

	_tiffNifds = 0;

	_reader->SetOrder(_reader->GetUShort());
	int hlen = _reader->GetUInt();

	_reader->Seek(0, SEEK_SET);
	char head[32];
	_reader->Read(head, 1, 32);
	_reader->Seek(0, SEEK_END);
	int flen = _reader->GetPosition();
	int fsize = flen;

	char* cp;
	if ((cp = (char *)memmem(head, 32, "MMMM", 4)) ||
		(cp = (char *)memmem(head, 32, "IIII", 4)))
	{
		ParsePhaseOne(cp - head);
		if (cp - head && ParseTiff(0))
			ApplyTiff();
	}
	else if (_reader->GetOrder() == 0x4949 || _reader->GetOrder() == 0x4d4d)
	{
		if (!memcmp(head + 6, "HEAPCCDR", 8))
		{
			_dataOffset = hlen;
			ParseCiff(hlen, flen - hlen, 0);
			_loadRaw = LoadRawType::CanonLoadRaw;
		}
		else if (ParseTiff(0))
			ApplyTiff();
	}
	else if (!memcmp(head, "\xff\xd8\xff\xe1", 4) &&
		!memcmp(head + 6, "Exif", 4))
	{
		_reader->Seek(4, SEEK_SET);
		_dataOffset = 4 + _reader->GetUShort();
		_reader->Seek(_dataOffset, SEEK_SET);
		if (_reader->GetChar() != 0xff)
			ParseTiff(12);
		_thumbOffset = 0;
	}
	else if (!memcmp(head + 25, "ARECOYK", 7))
	{
		strcpy_s(_make, LenMake, "Contax");
		strcpy_s(_model, LenModel, "N Digital");
		_reader->Seek(33, SEEK_SET);
		GetTimestamp(1);
		_reader->Seek(60, SEEK_SET);
		for (size_t c = 0; c < 4; c++)
			_camMul[c ^ (c >> 1)] = _reader->GetUInt();
	}
	else if (!strcmp(head, "PXN"))
	{
		strcpy_s(_make, LenMake, "Logitech");
		strcpy_s(_model, LenModel, "Fotoman Pixtura");
	}
	else if (!strcmp(head, "qktk"))
	{
		strcpy_s(_make, LenMake, "Apple");
		strcpy_s(_model, LenModel, "QuickTake 100");
		_loadRaw = LoadRawType::Quicktake100LoadRaw;
	}
	else if (!strcmp(head, "qktn"))
	{
		strcpy_s(_make, LenMake, "Apple");
		strcpy_s(_model, LenModel, "QuickTake 150");
		_loadRaw = LoadRawType::KodakRadcLoadRaw;
	}
	else if (!memcmp(head, "FUJIFILM", 8))
	{
		_reader->Seek(84, SEEK_SET);
		_thumbOffset = _reader->GetUInt();
		_thumbLength = _reader->GetUInt();
		_reader->Seek(92, SEEK_SET);
		ParseFuji(_reader->GetUInt());
		if (_thumbOffset > 120)
		{
			int i;
			_reader->Seek(120, SEEK_SET);
			_isRaw += (i = _reader->GetUInt()) && 1;
			if (_isRaw == 2 && _options->_shotSelect)
				ParseFuji(i);
		}
		_loadRaw = LoadRawType::UnpackedLoadRaw;
		_reader->Seek(100 + 28 * (_options->_shotSelect > 0), SEEK_SET);
		ParseTiff(_dataOffset = _reader->GetUInt());
		ParseTiff(_thumbOffset + 12);
		ApplyTiff();
	}
	else if (!memcmp(head, "RIFF", 4))
	{
		_reader->Seek(0, SEEK_SET);
		ParseRiff();
	}
	else if (!memcmp(head + 4, "ftypqt   ", 9))
	{
		_reader->Seek(0, SEEK_SET);
		ParseQt(fsize);
		_isRaw = 0;
	}
	else if (!memcmp(head, "\0\001\0\001\0@", 6))
	{
		_reader->Seek(6, SEEK_SET);
		_reader->Read(_make, 1, 8);
		_reader->Read(_model, 1, 8);
		_reader->Read(_model2, 1, 16);
		_dataOffset = _reader->GetUShort();
		_reader->GetUShort();
		_rawWidth = _reader->GetUShort();
		_rawHeight = _reader->GetUShort();
		_loadRaw = LoadRawType::NokiaLoadRaw;
		_filters = 0x61616161;
	}
	else if (!memcmp(head, "NOKIARAW", 8))
	{
		strcpy_s(_make, LenMake, "NOKIA");
		_reader->SetOrder(0x4949);
		_reader->Seek(300, SEEK_SET);
		_dataOffset = _reader->GetUInt();
		unsigned int i = _reader->GetUInt();
		_width = _reader->GetUShort();
		_height = _reader->GetUShort();
		_tiffBps = i * 8 / (_width * _height);
		switch (_tiffBps)
		{
		case  8:
			_loadRaw = LoadRawType::EightBitLoadRaw;
			break;
		case 10:
			_loadRaw = LoadRawType::NokiaLoadRaw;
		}
		_topMargin = i / (_width * _tiffBps / 8) - _height;
		_rawHeight = _height + _topMargin;
		_mask[0][3] = 1;
		_filters = 0x61616161;
	}
	else if (!memcmp(head, "ARRI", 4))
	{
		_reader->SetOrder(0x4949);
		_reader->Seek(20, SEEK_SET);
		_width = _reader->GetUInt();
		_height = _reader->GetUInt();
		strcpy_s(_make, LenMake, "ARRI");
		_reader->Seek(668, SEEK_SET);
		_reader->Read(_model, 1, LenModel);
		_dataOffset = 4096;
		_loadRaw = LoadRawType::PackedLoadRaw;
		_loadFlags = 88;
		_filters = 0x61616161;
	}
	else if (!memcmp(head, "XPDS", 4))
	{
		_reader->SetOrder( 0x4949);
		_reader->Seek(0x800, SEEK_SET);
		_reader->Read(_make, 1, 41);
		_rawHeight = _reader->GetUShort();
		_rawWidth = _reader->GetUShort();
		_reader->Seek(56, SEEK_CUR);
		_reader->Read(_model, 1, 30);
		_dataOffset = 0x10000;
		_loadRaw = LoadRawType::CanonRmfLoadRaw;
		GammaCurve(0, 12.25, 1, 1023);
	}
	else if (!memcmp(head + 4, "RED1", 4))
	{
		strcpy_s(_make, LenMake, "Red");
		strcpy_s(_model, LenModel, "One");
		ParseRedcine();
		_loadRaw = LoadRawType::RedcineLoadRaw;
		GammaCurve(1 / 2.4, 12.92, 1, 4095);
		_filters = 0x49494949;
	}
	else if (!memcmp(head, "DSC-Image", 9))
		ParseRollei();
	else if (!memcmp(head, "PWAD", 4))
		ParseSinarIA();
	else if (!memcmp(head, "\0MRM", 4))
		ParseMinolta(0);
	else if (!memcmp(head, "FOVb", 4))
		ParseFoveon();
	else if (!memcmp(head, "CI", 2))
		ParseCine();

	int zero_fsize = 1;
	if (_make[0] == 0)
	{
		zero_fsize = 0;
		for (size_t i = 0; i < sizeof table / sizeof *table; i++)
		{
			if (fsize == table[i].fsize)
			{
				strcpy_s(_make, LenMake, table[i].make);
				strcpy_s(_model, LenModel, table[i].model);
				_flip = table[i].flags >> 2;
				_zeroIsBad = table[i].flags & 2;
				if (table[i].flags & 1)
					ParseExternalJpeg();
				_dataOffset = table[i].offset;
				_rawWidth = table[i].rw;
				_rawHeight = table[i].rh;
				_leftMargin = table[i].lm;
				_topMargin = table[i].tm;
				_width = _rawWidth - _leftMargin - table[i].rm;
				_height = _rawHeight - _topMargin - table[i].bm;
				_filters = 0x1010101 * table[i].cf;
				_colors = 4 - !((_filters & _filters >> 1) & 0x5555);
				_loadFlags = table[i].lf;
				_tiffBps = (fsize - _dataOffset) * 8 / (_rawWidth*_rawHeight);
				switch (_tiffBps)
				{
				case 6:
					_loadRaw = LoadRawType::MinoltaRD175LoadRaw;
					break;
				case 8:
					_loadRaw = LoadRawType::EightBitLoadRaw;
					break;
				case 10: case 12:
					_loadFlags |= 128;
					_loadRaw = LoadRawType::PackedLoadRaw;
					break;
				case 16:
					_reader->SetOrder(0x4949 | 0x404 * (_loadFlags & 1));
					_tiffBps -= _loadFlags >> 4;
					_tiffBps -= _loadFlags = _loadFlags >> 1 & 7;
					_loadRaw = LoadRawType::UnpackedLoadRaw;
				}
				_maximum = (1 << _tiffBps) - (1 << table[i].max);
			}
		}
	}
	if (zero_fsize)
		fsize = 0;
	if (_make[0] == 0)
	{
		ParseSmal(0, flen);
	}
	if (_make[0] == 0)
	{
		ParseJpeg(0);
		if (!(strncmp(_model, "ov", 2) && strncmp(_model, "RP_OV", 5)) &&
			!_reader->Seek(-6404096, SEEK_END) &&
			_reader->Read(head, 1, 32) && !strcmp(head, "BRCMn"))
		{
			strcpy_s(_make, LenMake, "OmniVision");
			_dataOffset = _reader->GetPosition() + 0x8000 - 32;
			_width = _rawWidth;
			_rawWidth = 2611;
			_loadRaw = LoadRawType::NokiaLoadRaw;
			_filters = 0x16161616;
		}
		else
		{
			_isRaw = 0;
		}
	}

	for (size_t i = 0; i < sizeof corp / sizeof *corp; i++)
		if (strcasestr(_make, corp[i]))	/* Simplify company names */
			strcpy_s(_make, LenMake, corp[i]);
	if ((!strcmp(_make, "Kodak") || !strcmp(_make, "Leica")) &&
		((cp = strcasestr(_model, " DIGITAL CAMERA")) ||
		(cp = strstr(_model, "FILE VERSION"))))
	{
		*cp = 0;
	}
	if (!_strnicmp(_model, "PENTAX", 6))
		strcpy_s(_make, LenMake, "Pentax");
	cp = _make + strlen(_make);		/* Remove trailing spaces */
	while (*--cp == ' ')
		*cp = 0;
	cp = _model + strlen(_model);
	while (*--cp == ' ')
		*cp = 0;
	size_t ii = strlen(_make);			/* Remove make from model */
	if (!_strnicmp(_model, _make, ii) && _model[ii++] == ' ')
		memmove(_model, _model + ii, LenModel - ii);
	if (!strncmp(_model, "FinePix ", 8))
		strcpy_s(_model, LenModel, _model + 8);
	if (!strncmp(_model, "Digital Camera ", 15))
		strcpy_s(_model, LenModel, _model + 15);
	_desc[511] = _artist[63] = _make[63] = _model[63] = _model2[63] = 0;
	if (!_isRaw)
		goto notraw;

	if (!_height)
		_height = _rawHeight;
	if (!_width)
		_width = _rawWidth;
	if (_height == 2624 && _width == 3936)	/* Pentax K10D and Samsung GX10 */
	{
		_height = 2616;   _width = 3896;
	}
	if (_height == 3136 && _width == 4864)  /* Pentax K20D and Samsung GX20 */
	{
		_height = 3124;   _width = 4688; _filters = 0x16161616;
	}
	if (_width == 4352 && (!strcmp(_model, "K-r") || !strcmp(_model, "K-x")))
	{
		_width = 4309; _filters = 0x16161616;
	}
	if (_width >= 4960 && !strncmp(_model, "K-5", 3))
	{
		_leftMargin = 10; _width = 4950; _filters = 0x16161616;
	}
	if (_width == 4736 && !strcmp(_model, "K-7"))
	{
		_height = 3122;   _width = 4684; _filters = 0x16161616; _topMargin = 2;
	}
	if (_width == 6080 && !strcmp(_model, "K-3"))
	{
		_leftMargin = 4;  _width = 6040;
	}
	if (_width == 7424 && !strcmp(_model, "645D"))
	{
		_height = 5502;   _width = 7328; _filters = 0x61616161; _topMargin = 29;
		_leftMargin = 48;
	}
	if (_height == 3014 && _width == 4096)	/* Ricoh GX200 */
		_width = 4014;
	if (_dngVersion)
	{
		if (_filters == UINT_MAX)
			_filters = 0;
		if (_filters)
			_isRaw *= _tiffSamples;
		else
			_colors = _tiffSamples;
		switch (_tiffCompress)
		{
		case 0:
		case 1:
			_loadRaw = LoadRawType::PackedDngLoadRaw;
			break;
		case 7:
			_loadRaw = LoadRawType::LosslessDngLoadRaw;
			break;
		case 34892:
			_loadRaw = LoadRawType::LossyDngLoadRaw;
			break;
		default:
			_loadRaw = LoadRawType::UnknownLoadRaw;
		}
		goto dng_skip;
	}
	if (!strcmp(_make, "Canon") && !fsize && _tiffBps != 15)
	{
		if (!_loadRaw)
			_loadRaw = LoadRawType::LosslessJpegLoadRaw;
		for (size_t i = 0; i < sizeof canon / sizeof *canon; i++)
		{
			if (_rawWidth == canon[i][0] && _rawHeight == canon[i][1])
			{
				_width = _rawWidth - (_leftMargin = canon[i][2]);
				_height = _rawHeight - (_topMargin = canon[i][3]);
				_width -= canon[i][4];
				_height -= canon[i][5];
				_mask[0][1] = canon[i][6];
				_mask[0][3] = -canon[i][7];
				_mask[1][1] = canon[i][8];
				_mask[1][3] = -canon[i][9];
				if (canon[i][10])
					_filters = canon[i][10] * 0x01010101;
			}
		}
		if ((_uniqueId | 0x20000) == 0x2720000)
		{
			_leftMargin = 8;
			_topMargin = 16;
		}
	}
	for (size_t i = 0; i < sizeof unique / sizeof *unique; i++)
	{
		if (_uniqueId == 0x80000000 + unique[i].id)
		{
			AdobeCoeff("Canon", unique[i].model);
			if (_model[4] == 'K' && strlen(_model) == 8)
				strcpy_s(_model, LenModel, unique[i].model);
		}
	}
	for (size_t i = 0; i < sizeof sonique / sizeof *sonique; i++)
	{
		if (_uniqueId == sonique[i].id)
			strcpy_s(_model, LenModel, sonique[i].model);
	}
	if (!strcmp(_make, "Nikon"))
	{
		if (!_loadRaw)
			_loadRaw = LoadRawType::PackedLoadRaw;
		if (_model[0] == 'E')
			_loadFlags |= !_dataOffset << 2 | 2;
	}

	/* Set parameters based on camera name (for non-DNG files). */

	if (!strcmp(_model, "KAI-0340")
		&& FindGreen(16, 16, 3840, 5120) < 25)
	{
		_height = 480;
		_topMargin = _filters = 0;
		strcpy_s(_model, LenModel, "C603");
	}
	if (!strcmp(_make, "Sony") && _rawWidth > 3888)
		_black = 128 << (_tiffBps - 12);
	if (_isFoveon)
	{
		if (_height * 2 < _width) _pixelAspect = 0.5;
		if (_height   > _width) _pixelAspect = 2;
		_filters = 0;
		SimpleCoeff(0);
	}
	else if (!strcmp(_make, "Canon") && _tiffBps == 15)
	{
		switch (_width)
		{
		case 3344: _width -= 66;
		case 3872: _width -= 6;
		}
		if (_height > _width)
		{
			SWAP(_height, _width);
			SWAP(_rawHeight, _rawWidth);
		}
		if (_width == 7200 && _height == 3888)
		{
			_rawWidth = _width = 6480;
			_rawHeight = _height = 4320;
		}
		_filters = 0;
		_tiffSamples = _colors = 3;
		_loadRaw = LoadRawType::CanonSrawLoadRaw;
	}
	else if (!strcmp(_model, "PowerShot 600"))
	{
		_height = 613;
		_width = 854;
		_rawWidth = 896;
		_colors = 4;
		_filters = 0xe1e4e1e4;
		_loadRaw = LoadRawType::Canon600LoadRaw;
	}
	else if (!strcmp(_model, "PowerShot A5") ||
		!strcmp(_model, "PowerShot A5 Zoom"))
	{
		_height = 773;
		_width = 960;
		_rawWidth = 992;
		_pixelAspect = 256 / 235.0;
		_filters = 0x1e4e1e4e;
		goto canon_a5;
	}
	else if (!strcmp(_model, "PowerShot A50"))
	{
		_height = 968;
		_width = 1290;
		_rawWidth = 1320;
		_filters = 0x1b4e4b1e;
		goto canon_a5;
	}
	else if (!strcmp(_model, "PowerShot Pro70"))
	{
		_height = 1024;
		_width = 1552;
		_filters = 0x1e4b4e1b;
	canon_a5:
		_colors = 4;
		_tiffBps = 10;
		_loadRaw = LoadRawType::PackedLoadRaw;
		_loadFlags = 40;
	}
	else if (!strcmp(_model, "PowerShot Pro90 IS") ||
		!strcmp(_model, "PowerShot G1"))
	{
		_colors = 4;
		_filters = 0xb4b4b4b4;
	}
	else if (!strcmp(_model, "PowerShot A610"))
	{
		if (CanonS2is())
			strcpy_s(_model + 10, LenModel - 10, "S2 IS");
	}
	else if (!strcmp(_model, "PowerShot SX220 HS"))
	{
		_mask[1][3] = -4;
	}
	else if (!strcmp(_model, "EOS D2000C"))
	{
		_filters = 0x61616161;
		_black = _curve[200];
	}
	else if (!strcmp(_model, "D1"))
	{
		_camMul[0] *= 256 / 527.0;
		_camMul[2] *= 256 / 317.0;
	}
	else if (!strcmp(_model, "D1X"))
	{
		_width -= 4;
		_pixelAspect = 0.5;
	}
	else if (!strcmp(_model, "D40X") ||
		!strcmp(_model, "D60") ||
		!strcmp(_model, "D80") ||
		!strcmp(_model, "D3000"))
	{
		_height -= 3;
		_width -= 4;
	}
	else if (!strcmp(_model, "D3") ||
		!strcmp(_model, "D3S") ||
		!strcmp(_model, "D700"))
	{
		_width -= 4;
		_leftMargin = 2;
	}
	else if (!strcmp(_model, "D3100"))
	{
		_width -= 28;
		_leftMargin = 6;
	}
	else if (!strcmp(_model, "D5000") ||
		!strcmp(_model, "D90"))
	{
		_width -= 42;
	}
	else if (!strcmp(_model, "D5100") ||
		!strcmp(_model, "D7000") ||
		!strcmp(_model, "COOLPIX A"))
	{
		_width -= 44;
	}
	else if (!strcmp(_model, "D3200") ||
		!strncmp(_model, "D6", 2) ||
		!strncmp(_model, "D800", 4))
	{
		_width -= 46;
	}
	else if (!strcmp(_model, "D4") ||
		!strcmp(_model, "Df"))
	{
		_width -= 52;
		_leftMargin = 2;
	}
	else if (!strncmp(_model, "D40", 3) ||
		!strncmp(_model, "D50", 3) ||
		!strncmp(_model, "D70", 3))
	{
		_width--;
	}
	else if (!strcmp(_model, "D100"))
	{
		if (_loadFlags)
			_rawWidth = (_width += 3) + 3;
	}
	else if (!strcmp(_model, "D200"))
	{
		_leftMargin = 1;
		_width -= 4;
		_filters = 0x94949494;
	}
	else if (!strncmp(_model, "D2H", 3))
	{
		_leftMargin = 6;
		_width -= 14;
	}
	else if (!strncmp(_model, "D2X", 3))
	{
		if (_width == 3264) _width -= 32;
		else _width -= 8;
	}
	else if (!strncmp(_model, "D300", 4))
	{
		_width -= 32;
	}
	else if (!strncmp(_model, "COOLPIX P", 9) && _rawWidth != 4032)
	{
		_loadFlags = 24;
		_filters = 0x94949494;
		if (_model[9] == '7' && _isoSpeed >= 400)
			_black = 255;
	}
	else if (!strncmp(_model, "1 ", 2))
	{
		_height -= 2;
	}
	else if (fsize == 1581060)
	{
		SimpleCoeff(3);
		_preMul[0] = 1.2085;
		_preMul[1] = 1.0943;
		_preMul[3] = 1.1103;
	}
	else if (fsize == 3178560)
	{
		_camMul[0] *= 4;
		_camMul[2] *= 4;
	}
	else if (fsize == 4771840)
	{
		if (!_timestamp && NikonE995())
			strcpy_s(_model, LenModel, "E995");
		if (strcmp(_model, "E995"))
		{
			_filters = 0xb4b4b4b4;
			SimpleCoeff(3);
			_preMul[0] = 1.196;
			_preMul[1] = 1.246;
			_preMul[2] = 1.018;
		}
	}
	else if (fsize == 2940928)
	{
		if (!_timestamp && !NikonE2100())
			strcpy_s(_model, LenModel, "E2500");
		if (!strcmp(_model, "E2500"))
		{
			_height -= 2;
			_loadFlags = 6;
			_colors = 4;
			_filters = 0x4b4b4b4b;
		}
	}
	else if (fsize == 4775936)
	{
		if (!_timestamp)
			Nikon3700();
		if (_model[0] == 'E' && atoi(_model + 1) < 3700)
			_filters = 0x49494949;
		if (!strcmp(_model, "Optio 33WR"))
		{
			_flip = 1;
			_filters = 0x16161616;
		}
		if (_make[0] == 'O')
		{
			int i = FindGreen(12, 32, 1188864, 3576832);
			int c = FindGreen(12, 32, 2383920, 2387016);
			if (abs(i) < abs(c))
			{
				SWAP(i, c);
				_loadFlags = 24;
			}
			if (i < 0) _filters = 0x61616161;
		}
	}
	else if (fsize == 5869568)
	{
		if (!_timestamp && MinoltaZ2())
		{
			strcpy_s(_make, LenMake, "Minolta");
			strcpy_s(_model, LenModel, "DiMAGE Z2");
		}
		_loadFlags = 6 + 24 * (_make[0] == 'M');
	}
	else if (fsize == 6291456)
	{
		_reader->Seek(0x300000, SEEK_SET);
		_reader->SetOrder(GuessByteOrder(0x10000));
		if (_reader->GetOrder() == 0x4d4d)
		{
			_height -= (_topMargin = 16);
			_width -= (_leftMargin = 28);
			_maximum = 0xf5c0;
			strcpy_s(_make, LenMake, "ISG");
			_model[0] = 0;
		}
	}
	else if (!strcmp(_make, "Fujifilm"))
	{
		if (!strcmp(_model + 7, "S2Pro"))
		{
			strcpy_s(_model, LenModel, "S2Pro");
			_height = 2144;
			_width = 2880;
			_flip = 6;
		}
		else if (_loadRaw != LoadRawType::PackedLoadRaw)
			_maximum = (_isRaw == 2 && _options->_shotSelect) ? 0x2f00 : 0x3e00;
		_topMargin = (_rawHeight - _height) >> 2 << 1;
		_leftMargin = (_rawWidth - _width) >> 2 << 1;
		if (_width == 2848 || _width == 3664) _filters = 0x16161616;
		if (_width == 4032 || _width == 4952 || _width == 6032) _leftMargin = 0;
		if (_width == 3328 && (_width -= 66)) _leftMargin = 34;
		if (_width == 4936) _leftMargin = 4;
		if (!strcmp(_model, "HS50EXR") ||
			!strcmp(_model, "F900EXR"))
		{
			_width += 2;
			_leftMargin = 0;
			_filters = 0x16161616;
		}
		if (_fujiLayout) _rawWidth *= _isRaw;
		if (_filters == 9)
			for (size_t c = 0; c < 36; c++)
				((char *)_xtrans)[c] = _xtransAbs[(c / 6 + _topMargin) % 6][(c + _leftMargin) % 6];
	}
	else if (!strcmp(_model, "KD-400Z"))
	{
		_height = 1712;
		_width = 2312;
		_rawWidth = 2336;
		goto konica_400z;
	}
	else if (!strcmp(_model, "KD-510Z"))
	{
		goto konica_510z;
	}
	else if (!_stricmp(_make, "Minolta"))
	{
		if (!_loadRaw && (_maximum = 0xfff))
			_loadRaw = LoadRawType::UnpackedLoadRaw;
		if (!strncmp(_model, "DiMAGE A", 8))
		{
			if (!strcmp(_model, "DiMAGE A200"))
				_filters = 0x49494949;
			_tiffBps = 12;
			_loadRaw = LoadRawType::PackedLoadRaw;
		}
		else if (!strncmp(_model, "ALPHA", 5) ||
			!strncmp(_model, "DYNAX", 5) ||
			!strncmp(_model, "MAXXUM", 6))
		{
			sprintf_s(_model + 20, LenModel - 20, "DYNAX %-10s", _model + 6 + (_model[0] == 'M'));
			AdobeCoeff(_make, _model + 20);
			_loadRaw = LoadRawType::PackedLoadRaw;
		}
		else if (!strncmp(_model, "DiMAGE G", 8))
		{
			if (_model[8] == '4')
			{
				_height = 1716;
				_width = 2304;
			}
			else if (_model[8] == '5')
			{
			konica_510z:
				_height = 1956;
				_width = 2607;
				_rawWidth = 2624;
			}
			else if (_model[8] == '6')
			{
				_height = 2136;
				_width = 2848;
			}
			_dataOffset += 14;
			_filters = 0x61616161;
		konica_400z:
			_loadRaw = LoadRawType::UnpackedLoadRaw;
			_maximum = 0x3df;
			_reader->SetOrder(0x4d4d);
		}
	}
	else if (!strcmp(_model, "*ist D"))
	{
		_loadRaw = LoadRawType::UnpackedLoadRaw;
		_dataError = -1;	// TODO: ignore first file error
	}
	else if (!strcmp(_model, "*ist DS"))
	{
		_height -= 2;
	}
	else if (!strcmp(_make, "Samsung") && _rawWidth == 4704)
	{
		_height -= _topMargin = 8;
		_width -= 2 * (_leftMargin = 8);
		_loadFlags = 32;
	}
	else if (!strcmp(_make, "Samsung") && _rawHeight == 3714)
	{
		_height -= _topMargin = 18;
		_leftMargin = _rawWidth - (_width = 5536);
		if (_rawWidth != 5600)
			_leftMargin = _topMargin = 0;
		_filters = 0x61616161;
		_colors = 3;
	}
	else if (!strcmp(_make, "Samsung") && _rawWidth == 5632)
	{
		_reader->SetOrder(0x4949);
		_height = 3694;
		_topMargin = 2;
		_width = 5574 - (_leftMargin = 32 + _tiffBps);
		if (_tiffBps == 12) _loadFlags = 80;
	}
	else if (!strcmp(_make, "Samsung") && _rawWidth == 5664)
	{
		_height -= _topMargin = 17;
		_leftMargin = 96;
		_width = 5544;
		_filters = 0x49494949;
	}
	else if (!strcmp(_make, "Samsung") && _rawWidth == 6496)
	{
		_filters = 0x61616161;
		_black = 1 << (_tiffBps - 7);
	}
	else if (!strcmp(_model, "EX1"))
	{
		_reader->SetOrder(0x4949);
		_height -= 20;
		_topMargin = 2;
		if ((_width -= 6) > 3682)
		{
			_height -= 10;
			_width -= 46;
			_topMargin = 8;
		}
	}
	else if (!strcmp(_model, "WB2000"))
	{
		_reader->SetOrder(0x4949);
		_height -= 3;
		_topMargin = 2;
		if ((_width -= 10) > 3718)
		{
			_height -= 28;
			_width -= 56;
			_topMargin = 8;
		}
	}
	else if (strstr(_model, "WB550"))
	{
		strcpy_s(_model, LenModel, "WB550");
	}
	else if (!strcmp(_model, "EX2F"))
	{
		_height = 3045;
		_width = 4070;
		_topMargin = 3;
		_reader->SetOrder(0x4949);
		_filters = 0x49494949;
		_loadRaw = LoadRawType::UnpackedLoadRaw;
	}
	else if (!strcmp(_model, "STV680 VGA"))
	{
		_black = 16;
	}
	else if (!strcmp(_model, "N95"))
	{
		_height = _rawHeight - (_topMargin = 2);
	}
	else if (!strcmp(_model, "640x480"))
	{
		GammaCurve(0.45, 4.5, 1, 255);
	}
	else if (!strcmp(_make, "Hasselblad"))
	{
		if (_loadRaw == LoadRawType::LosslessJpegLoadRaw)
			_loadRaw = LoadRawType::HasselbladLoadRaw;
		if (_rawWidth == 7262)
		{
			_height = 5444;
			_width = 7248;
			_topMargin = 4;
			_leftMargin = 7;
			_filters = 0x61616161;
		}
		else if (_rawWidth == 7410 || _rawWidth == 8282)
		{
			_height -= 84;
			_width -= 82;
			_topMargin = 4;
			_leftMargin = 41;
			_filters = 0x61616161;
		}
		else if (_rawWidth == 9044)
		{
			_height = 6716;
			_width = 8964;
			_topMargin = 8;
			_leftMargin = 40;
			_black += _loadFlags = 256;
			_maximum = 0x8101;
		}
		else if (_rawWidth == 4090)
		{
			strcpy_s(_model, LenModel, "V96C");
			_height -= (_topMargin = 6);
			_width -= (_leftMargin = 3) + 7;
			_filters = 0x61616161;
		}
		if (_tiffSamples > 1)
		{
			_isRaw = _tiffSamples + 1;
			if (!_options->_shotSelect && !_options->_halfSize)
				_filters = 0;
		}
	}
	else if (!strcmp(_make, "Sinar"))
	{
		if (!_loadRaw) _loadRaw = LoadRawType::UnpackedLoadRaw;
		if (_isRaw > 1 && !_options->_shotSelect && !_options->_halfSize)
			_filters = 0;
		_maximum = 0x3fff;
	}
	else if (!strcmp(_make, "Leaf"))
	{
		_maximum = 0x3fff;
		_reader->Seek(_dataOffset, SEEK_SET);
		JHead jh(*_reader, *this, true);
		if (jh._success && jh._jdata.bits == 15)
			_maximum = 0x1fff;
		if (_tiffSamples > 1) _filters = 0;
		if (_tiffSamples > 1 || _tileLength < _rawHeight)
		{
			_loadRaw = LoadRawType::LeafHdrLoadRaw;
			_rawWidth = _tileWidth;
		}
		if ((_width | _height) == 2048)
		{
			if (_tiffSamples == 1)
			{
				_filters = 1;
				strcpy_s(_cdesc, LenCDesc, "RBTG");
				strcpy_s(_model, LenModel, "CatchLight");
				_topMargin = 8; _leftMargin = 18; _height = 2032; _width = 2016;
			}
			else
			{
				strcpy_s(_model, LenModel, "DCB2");
				_topMargin = 10; _leftMargin = 16; _height = 2028; _width = 2022;
			}
		}
		else if (_width + _height == 3144 + 2060)
		{
			if (!_model[0])
				strcpy_s(_model, LenModel, "Cantare");
			if (_width > _height)
			{
				_topMargin = 6; _leftMargin = 32; _height = 2048;  _width = 3072;
				_filters = 0x61616161;
			}
			else
			{
				_leftMargin = 6;  _topMargin = 32;  _width = 2048; _height = 3072;
				_filters = 0x16161616;
			}
			if (!_camMul[0] || _model[0] == 'V') _filters = 0;
			else _isRaw = _tiffSamples;
		}
		else if (_width == 2116)
		{
			strcpy_s(_model, LenModel, "Valeo 6");
			_height -= 2 * (_topMargin = 30);
			_width -= 2 * (_leftMargin = 55);
			_filters = 0x49494949;
		}
		else if (_width == 3171)
		{
			strcpy_s(_model, LenModel, "Valeo 6");
			_height -= 2 * (_topMargin = 24);
			_width -= 2 * (_leftMargin = 24);
			_filters = 0x16161616;
		}
	}
	else if (!strcmp(_make, "Leica") || !strcmp(_make, "Panasonic"))
	{
		if ((flen - _dataOffset) / (_rawWidth * 8 / 7) == _rawHeight)
			_loadRaw = LoadRawType::PanasonicLoadRaw;
		if (!_loadRaw)
		{
			_loadRaw = LoadRawType::UnpackedLoadRaw;
			_loadFlags = 4;
		}
		_zeroIsBad = 1;
		if ((_height += 12) > _rawHeight) _height = _rawHeight;
		for (size_t i = 0; i < sizeof pana / sizeof *pana; i++)
			if (_rawWidth == pana[i][0] && _rawHeight == pana[i][1])
			{
				_leftMargin = pana[i][2];
				_topMargin = pana[i][3];
				_width += pana[i][4];
				_height += pana[i][5];
			}
		_filters = 0x01010101 * (unsigned char) "\x94\x61\x49\x16"
			[((_filters - 1) ^ (_leftMargin & 1) ^ (_topMargin << 1)) & 3];
	}
	else if (!strcmp(_model, "C770UZ"))
	{
		_height = 1718;
		_width = 2304;
		_filters = 0x16161616;
		_loadRaw = LoadRawType::PackedLoadRaw;
		_loadFlags = 30;
	}
	else if (!strcmp(_make, "Olympus"))
	{
		_height += _height & 1;
		if (_exifCfa) _filters = _exifCfa;
		if (_width == 4100) _width -= 4;
		if (_width == 4080) _width -= 24;
		if (_width == 9280)
		{
			_width -= 6; _height -= 6;
		}
		if (_loadRaw == LoadRawType::UnpackedLoadRaw)
			_loadFlags = 4;
		_tiffBps = 12;
		if (!strcmp(_model, "E-300") ||
			!strcmp(_model, "E-500"))
		{
			_width -= 20;
			if (_loadRaw == LoadRawType::UnpackedLoadRaw)
			{
				_maximum = 0xfc3;
				memset(_cBlack, 0, sizeof _cBlack);
			}
		}
		else if (!strcmp(_model, "E-330"))
		{
			_width -= 30;
			if (_loadRaw == LoadRawType::UnpackedLoadRaw)
				_maximum = 0xf79;
		}
		else if (!strcmp(_model, "SP550UZ"))
		{
			_thumbLength = flen - (_thumbOffset = 0xa39800);
			_thumbHeight = 480;
			_thumbWidth = 640;
		}
		else if (!strcmp(_model, "TG-4"))
		{
			_width -= 16;
		}
	}
	else if (!strcmp(_model, "N Digital"))
	{
		_height = 2047;
		_width = 3072;
		_filters = 0x61616161;
		_dataOffset = 0x1a00;
		_loadRaw = LoadRawType::PackedLoadRaw;
	}
	else if (!strcmp(_model, "DSC-F828"))
	{
		_width = 3288;
		_leftMargin = 5;
		_mask[1][3] = -17;
		_dataOffset = 862144;
		_loadRaw = LoadRawType::SonyLoadRaw;
		_filters = 0x9c9c9c9c;
		_colors = 4;
		strcpy_s(_cdesc, LenCDesc, "RGBE");
	}
	else if (!strcmp(_model, "DSC-V3"))
	{
		_width = 3109;
		_leftMargin = 59;
		_mask[0][1] = 9;
		_dataOffset = 787392;
		_loadRaw = LoadRawType::SonyLoadRaw;
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 3984)
	{
		_width = 3925;
		_reader->SetOrder(0x4d4d);
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 4288)
	{
		_width -= 32;
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 4600)
	{
		if (!strcmp(_model, "DSLR-A350"))
			_height -= 4;
		_black = 0;
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 4928)
	{
		if (_height < 3280) _width -= 8;
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 5504)
	{
		_width -= _height > 3664 ? 8 : 32;
		if (!strncmp(_model, "DSC", 3))
			_black = 200 << (_tiffBps - 12);
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 6048)
	{
		_width -= 24;
		if (strstr(_model, "RX1") || strstr(_model, "A99"))
			_width -= 6;
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 7392)
	{
		_width -= 30;
	}
	else if (!strcmp(_make, "Sony") && _rawWidth == 8000)
	{
		_width -= 32;
		if (!strncmp(_model, "DSC", 3))
		{
			_tiffBps = 14;
			_loadRaw = LoadRawType::UnpackedLoadRaw;
			_black = 512;
		}
	}
	else if (!strcmp(_model, "DSLR-A100"))
	{
		if (_width == 3880)
		{
			_height--;
			_width = ++_rawWidth;
		}
		else
		{
			_height -= 4;
			_width -= 4;
			_reader->SetOrder(0x4d4d);
			_loadFlags = 2;
		}
		_filters = 0x61616161;
	}
	else if (!strcmp(_model, "PIXL"))
	{
		_height -= _topMargin = 4;
		_width -= _leftMargin = 32;
		GammaCurve(0, 7, 1, 255);
	}
	else if (!strcmp(_model, "C603") || !strcmp(_model, "C330")
		|| !strcmp(_model, "12MP"))
	{
		_reader->SetOrder(0x4949);
		if (_filters && _dataOffset)
		{
			_reader->Seek(_dataOffset < 4096 ? 168 : 5252, SEEK_SET);
			_reader->ReadShorts(_curve, 256);
		}
		else GammaCurve(0, 3.875, 1, 255);
		_loadRaw = _filters ? LoadRawType::EightBitLoadRaw :
			strcmp(_model, "C330") ? LoadRawType::KodakC603LoadRaw :
			LoadRawType::KodakC330LoadRaw;
		_loadFlags = _tiffBps > 16;
		_tiffBps = 8;
	}
	else if (!_strnicmp(_model, "EasyShare", 9))
	{
		_dataOffset = _dataOffset < 0x15000 ? 0x15000 : 0x17000;
		_loadRaw = LoadRawType::PackedLoadRaw;
	}
	else if (!_stricmp(_make, "Kodak"))
	{
		if (_filters == UINT_MAX) _filters = 0x61616161;
		if (!strncmp(_model, "NC2000", 6) ||
			!strncmp(_model, "EOSDCS", 6) ||
			!strncmp(_model, "DCS4", 4))
		{
			_width -= 4;
			_leftMargin = 2;
			if (_model[6] == ' ') _model[6] = 0;
			if (!strcmp(_model, "DCS460A")) goto bw;
		}
		else if (!strcmp(_model, "DCS660M"))
		{
			_black = 214;
			goto bw;
		}
		else if (!strcmp(_model, "DCS760M"))
		{
		bw:   _colors = 1;
			_filters = 0;
		}
		if (!strcmp(_model + 4, "20X"))
			strcpy_s(_cdesc, LenCDesc, "MYCY");
		if (strstr(_model, "DC25"))
		{
			strcpy_s(_model, LenModel, "DC25");
			_dataOffset = 15424;
		}
		if (!strncmp(_model, "DC2", 3))
		{
			_rawHeight = 2 + (_height = 242);
			if (flen < 100000)
			{
				_rawWidth = 256; _width = 249;
				_pixelAspect = (4.0*_height) / (3.0*_width);
			}
			else
			{
				_rawWidth = 512; _width = 501;
				_pixelAspect = (493.0*_height) / (373.0*_width);
			}
			_topMargin = _leftMargin = 1;
			_colors = 4;
			_filters = 0x8d8d8d8d;
			SimpleCoeff(1);
			_preMul[1] = 1.179;
			_preMul[2] = 1.209;
			_preMul[3] = 1.036;
			_loadRaw = LoadRawType::EightBitLoadRaw;
		}
		else if (!strcmp(_model, "40"))
		{
			strcpy_s(_model, LenModel, "DC40");
			_height = 512;
			_width = 768;
			_dataOffset = 1152;
			_loadRaw = LoadRawType::KodakRadcLoadRaw;
			_tiffBps = 12;
		}
		else if (strstr(_model, "DC50"))
		{
			strcpy_s(_model, LenModel, "DC50");
			_height = 512;
			_width = 768;
			_dataOffset = 19712;
			_loadRaw = LoadRawType::KodakRadcLoadRaw;
		}
		else if (strstr(_model, "DC120"))
		{
			strcpy_s(_model, LenModel, "DC120");
			_height = 976;
			_width = 848;
			_pixelAspect = _height / 0.75 / _width;
			_loadRaw = _tiffCompress == 7 ? LoadRawType::KodakJpegLoadRaw : LoadRawType::KodakDC120LoadRaw;
		}
		else if (!strcmp(_model, "DCS200"))
		{
			_thumbHeight = 128;
			_thumbWidth = 192;
			_thumbOffset = 6144;
			_thumbMisc = 360;
			_writeThumb = WriteThumbType::LayerThumb;
			_black = 17;
		}
	}
	else if (!strcmp(_model, "Fotoman Pixtura"))
	{
		_height = 512;
		_width = 768;
		_dataOffset = 3632;
		_loadRaw = LoadRawType::KodakRadcLoadRaw;
		_filters = 0x61616161;
		SimpleCoeff(2);
	}
	else if (!strncmp(_model, "QuickTake", 9))
	{
		if (head[5])
			strcpy_s(_model + 10, LenModel - 10, "200");
		_reader->Seek(544, SEEK_SET);
		_height = _reader->GetUShort();
		_width = _reader->GetUShort();
		_dataOffset = (_reader->GetUInt(), _reader->GetUShort()) == 30 ? 738 : 736;
		if (_height > _width)
		{
			SWAP(_height, _width);
			_reader->Seek(_dataOffset - 6, SEEK_SET);
			_flip = ~_reader->GetUShort() & 3 ? 5 : 6;
		}
		_filters = 0x61616161;
	}
	else if (!strcmp(_make, "Rollei") && !_loadRaw)
	{
		switch (_rawWidth)
		{
		case 1316:
			_height = 1030;
			_width = 1300;
			_topMargin = 1;
			_leftMargin = 6;
			break;
		case 2568:
			_height = 1960;
			_width = 2560;
			_topMargin = 2;
			_leftMargin = 8;
		}
		_filters = 0x16161616;
		_loadRaw = LoadRawType::RolleiLoadRaw;
	}
	if (!_model[0])
		sprintf_s(_model, LenModel, "%dx%d", _width, _height);
	if (_filters == UINT_MAX) _filters = 0x94949494;
	if (_thumbOffset && !_thumbHeight)
	{
		_reader->Seek(_thumbOffset, SEEK_SET);
		JHead jh(*_reader, *this, true);
		if (jh._success)
		{
			_thumbWidth = jh._jdata.wide;
			_thumbHeight = jh._jdata.high;
		}
	}
dng_skip:
	if ((_options->_useCameraMatrix & (_options->_useCameraWB || _dngVersion)) && _cMatrix[0][0] > 0.125)
	{
		memcpy(_rgbCam, _cMatrix, sizeof _cMatrix);
		_rawColor = 0;
	}
	if (_rawColor)
		AdobeCoeff(_make, _model);
	if (_loadRaw == LoadRawType::KodakRadcLoadRaw)
		if (_rawColor) AdobeCoeff("Apple", "Quicktake");
	if (_fujiWidth)
	{
		_fujiWidth = _width >> !_fujiLayout;
		_filters = _fujiWidth & 1 ? 0x94949494 : 0x49494949;
		_width = (_height >> _fujiLayout) + _fujiWidth;
		_height = _width - 1;
		_pixelAspect = 1;
	}
	else
	{
		if (_rawHeight < _height)
			_rawHeight = _height;
		if (_rawWidth  < _width)
			_rawWidth = _width;
	}
	if (!_tiffBps)
		_tiffBps = 12;
	if (!_maximum)
		_maximum = (1 << _tiffBps) - 1;
	if (!_loadRaw || _height < 22 || _width < 22 || _tiffBps > 16 || _tiffSamples > 6 || _colors > 4)
		_isRaw = 0;

#ifdef NO_JASPER
	if (_loadRaw == LoadRawType::RedcineLoadRaw)
	{
		throw CExceptionFile("libjasper not included");
	}
#endif
#ifdef NO_JPEG
	if (_loadRaw == LoadRawType::KodakJpegLoadRaw ||
		_loadRaw == LoadRawType::LossyDngLoadRaw)
	{
		throw CExceptionFile("libjpeg not included");
	}
#endif
#ifndef NO_RESTRICTED_DCRAW
	if (_writeFun == WriteThumbType::FoveonThumb ||
		_loadRaw == LoadRawType::FoveonDpLoadRaw ||
		_loadRaw == LoadRawType::FoveonSdLoadRaw)
	{
		throw CExceptionFile("Dave Coffin's license required (https://cybercom.net/~dcoffin/dcraw/).");
	}
#endif // NO_RESTRICTED_DCRAW

	if (!_cdesc[0])
		strcpy_s(_cdesc, LenCDesc, _colors == 3 ? "RGBG" : "GMCY");
	if (!_rawHeight)
		_rawHeight = _height;
	if (!_rawWidth)
		_rawWidth = _width;
	if (_filters > 999 && _colors == 3)
		_filters |= ((_filters >> 2 & 0x22222222) | (_filters << 2 & 0x88888888)) & _filters << 1;

notraw:
	if (_flip == UINT_MAX)
		_flip = _tiffFlip;
	if (_flip == UINT_MAX)
		_flip = 0;
}
#pragma endregion

#pragma region Private methods
int CSimpleInfo::ParseTiff(int base)
{
	_reader->Seek(base, SEEK_SET);
	_reader->SetOrder(_reader->GetUShort());
	if (_reader->GetOrder() != 0x4949 && _reader->GetOrder() != 0x4d4d)
		return 0;
	_reader->GetUShort();
	int doff;
	while ((doff = _reader->GetUInt()))
	{
		_reader->Seek(doff + base, SEEK_SET);
		if (ParseTiffIFD(base)) break;
	}
	return 1;
}


int CSimpleInfo::ParseTiffIFD(int base)
{
	unsigned tag, type, len, plen = 16, save;
	int cfa, i, j, c;
	int ima_len = 0;
	char software[64], *cbuf, *cp;
	unsigned char cfa_pat[16], cfa_pc[] = { 0,1,2,3 }, tab[256];
	double cc[4][4], cm[4][3], cam_xyz[4][3], num;
	double ab[] = { 1,1,1,1 }, asn[] = { 0,0,0,0 }, xyz[] = { 1,1,1 };
	unsigned sony_curve[] = { 0,0,0,0,0,4095 };
	unsigned sony_offset = 0, sony_length = 0, sony_key = 0;

	if (_tiffNifds >= sizeof _tiffIfd / sizeof _tiffIfd[0])
		return 1;
	int ifd = _tiffNifds++;
	for (j = 0; j < 4; j++)
		for (i = 0; i < 4; i++)
			cc[j][i] = i == j;
	unsigned entries = _reader->GetUShort();
	if (entries > 512)
		return 1;

	bool use_cm = false;

	while (entries--)
	{
		TiffGet(base, &tag, &type, &len, &save);
		switch (tag)
		{
		case 5:
			_width = _reader->GetUShort();
			break;
		case 6:
			_height = _reader->GetUShort();
			break;
		case 7:
			_width += _reader->GetUShort();
			break;
		case 9:
			if ((i = _reader->GetUShort()))
				_filters = i;
			break;
		case 17:
		case 18:
			if (type == 3 && len == 1)
				_camMul[(tag - 17) * 2] = _reader->GetUShort() / 256.0;
			break;
		case 23:
			if (type == 3)
				_isoSpeed = _reader->GetUShort();
			break;
		case 28: case 29: case 30:
			_cBlack[tag - 28] = _reader->GetUShort();
			_cBlack[3] = _cBlack[1];
			break;
		case 36: case 37: case 38:
			_camMul[tag - 36] = _reader->GetUShort();
			break;
		case 39:
			if (len < 50 || _camMul[0])
				break;
			_reader->Seek(12, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = _reader->GetUShort();
			break;
		case 46:
			if (type != 7 || _reader->GetChar() != 0xff || _reader->GetChar() != 0xd8) break;
			_thumbOffset = _reader->GetPosition() - 2;
			_thumbLength = len;
			break;
		case 61440:			/* Fuji HS10 table */
			_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
			ParseTiffIFD(base);
			break;
		case 2: case 256: case 61441:	/* ImageWidth */
			_tiffIfd[ifd].width = _reader->GetUInt(type);
			break;
		case 3: case 257: case 61442:	/* ImageHeight */
			_tiffIfd[ifd].height = _reader->GetUInt(type);
			break;
		case 258:				/* BitsPerSample */
		case 61443:
			_tiffIfd[ifd].samples = len & 7;
			_tiffIfd[ifd].bps = _reader->GetUInt(type);
			if (_tiffBps < _tiffIfd[ifd].bps)
				_tiffBps = _tiffIfd[ifd].bps;
			break;
		case 61446:
			_rawHeight = 0;
			if (_tiffIfd[ifd].bps > 12)
				break;
			_loadRaw = LoadRawType::PackedLoadRaw;
			_loadFlags = _reader->GetUInt() ? 24 : 80;
			break;
		case 259:				/* Compression */
			_tiffIfd[ifd].comp = _reader->GetUInt(type);
			break;
		case 262:				/* PhotometricInterpretation */
			_tiffIfd[ifd].phint = _reader->GetUShort();
			break;
		case 270:				/* ImageDescription */
			_reader->Read(_desc, 512, 1);
			break;
		case 271:				/* Make */
			_reader->GetString(_make, LenMake);
			break;
		case 272:				/* Model */
			_reader->GetString(_model, LenModel);
			break;
		case 280:				/* Panasonic RW2 offset */
			if (type != 4) break;
			_loadRaw = LoadRawType::PanasonicLoadRaw;
			_loadFlags = 0x2008;
		case 273:				/* StripOffset */
		case 513:				/* JpegIFOffset */
		case 61447:
			_tiffIfd[ifd].offset = _reader->GetUInt() + base;
			if (!_tiffIfd[ifd].bps && _tiffIfd[ifd].offset > 0)
			{
				_reader->Seek(_tiffIfd[ifd].offset, SEEK_SET);
				JHead jh(*_reader, *this, true);
				if (jh._success)
				{
					_tiffIfd[ifd].comp = 6;
					_tiffIfd[ifd].width = jh._jdata.wide;
					_tiffIfd[ifd].height = jh._jdata.high;
					_tiffIfd[ifd].bps = jh._jdata.bits;
					_tiffIfd[ifd].samples = jh._jdata.clrs;
					if (!(jh._jdata.sraw || (jh._jdata.clrs & 1)))
						_tiffIfd[ifd].width *= jh._jdata.clrs;
					if ((_tiffIfd[ifd].width > 4 * _tiffIfd[ifd].height) & ~jh._jdata.clrs)
					{
						_tiffIfd[ifd].width /= 2;
						_tiffIfd[ifd].height *= 2;
					}
					i = _reader->GetOrder();
					ParseTiff(_tiffIfd[ifd].offset + 12);
					_reader->SetOrder(i);
				}
			}
			break;
		case 274:				/* Orientation */
			_tiffIfd[ifd].flip = "50132467"[_reader->GetUShort() & 7] - '0';
			break;
		case 277:				/* SamplesPerPixel */
			_tiffIfd[ifd].samples = _reader->GetUInt(type) & 7;
			break;
		case 279:				/* StripByteCounts */
		case 514:
		case 61448:
			_tiffIfd[ifd].bytes = _reader->GetUInt();
			break;
		case 61454:
			for (size_t c = 0; c < 3; c++)
				_camMul[(4 - c) % 3] = _reader->GetUInt(type);
			break;
		case 305:  case 11:		/* Software */
			_reader->GetString(software, 64);
			if (!strncmp(software, "Adobe", 5) ||
				!strncmp(software, "dcraw", 5) ||
				!strncmp(software, "UFRaw", 5) ||
				!strncmp(software, "Bibble", 6) ||
				!strncmp(software, "Nikon Scan", 10) ||
				!strcmp(software, "Digital Photo Professional"))
				_isRaw = 0;
			break;
		case 306:				/* DateTime */
			GetTimestamp(0);
			break;
		case 315:				/* Artist */
			_reader->Read(_artist, LenArtist, 1);
			break;
		case 322:				/* TileWidth */
			_tiffIfd[ifd].tile_width = _reader->GetUInt(type);
			break;
		case 323:				/* TileLength */
			_tiffIfd[ifd].tile_length = _reader->GetUInt(type);
			break;
		case 324:				/* TileOffsets */
			_tiffIfd[ifd].offset = len > 1 ? _reader->GetPosition() : _reader->GetUInt();
			if (len == 1)
				_tiffIfd[ifd].tile_width = _tiffIfd[ifd].tile_length = 0;
			if (len == 4)
			{
				_loadRaw = LoadRawType::Sinar4ShotLoadRaw;
				_isRaw = 5;
			}
			break;
		case 330:				/* SubIFDs */
			if (!strcmp(_model, "DSLR-A100") && _tiffIfd[ifd].width == 3872)
			{
				_loadRaw = LoadRawType::SonyArwLoadRaw;
				_dataOffset = _reader->GetUInt() + base;
				ifd++;  break;
			}
			while (len--)
			{
				i = _reader->GetPosition();
				_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
				if (ParseTiffIFD(base))
					break;
				_reader->Seek(i + 4, SEEK_SET);
			}
			break;
		case 400:
			strcpy_s(_make, LenMake, "Sarnoff");
			_maximum = 0xfff;
			break;
		case 28688:
			for (size_t c = 0; c < 4; c++)
				sony_curve[c + 1] = _reader->GetUShort() >> 2 & 0xfff;
			for (i = 0; i < 5; i++)
				for (j = sony_curve[i] + 1; j <= sony_curve[i + 1]; j++)
					_curve[j] = _curve[j - 1] + (1 << i);
			break;
		case 29184:
			sony_offset = _reader->GetUInt();
			break;
		case 29185:
			sony_length = _reader->GetUInt();
			break;
		case 29217:
			sony_key = _reader->GetUInt();
			break;
		case 29264:
			ParseMinolta(_reader->GetPosition());
			_rawWidth = 0;
			break;
		case 29443:
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c < 2)] = _reader->GetUShort();
			break;
		case 29459:
			for (size_t c = 0; c < 4; c++)
				_camMul[c] = _reader->GetUShort();
			i = (_camMul[1] == 1024 && _camMul[2] == 1024) << 1;
			SWAP(_camMul[i], _camMul[i + 1])
				break;
		case 33405:			/* Model2 */
			_reader->GetString(_model2, LenModel2);
			break;
		case 33421:			/* CFARepeatPatternDim */
			if (_reader->GetUShort() == 6 && _reader->GetUShort() == 6)
				_filters = 9;
			break;
		case 33422:			/* CFAPattern */
			if (_filters == 9)
			{
				for (size_t c = 0; c < 36; c++)
					((char *)_xtrans)[c] = _reader->GetChar() & 3;
				break;
			}
		case 64777:			/* Kodak P-series */
			if ((plen = len) > 16)
				plen = 16;
			_reader->Read(cfa_pat, 1, plen);
			_colors = cfa = 0;
			for (i = 0; i < plen && _colors < 4; i++)
			{
				_colors += !(cfa & (1 << cfa_pat[i]));
				cfa |= 1 << cfa_pat[i];
			}
			if (cfa == 070)
				memcpy(cfa_pc, "\003\004\005", 3);	/* CMY */
			if (cfa == 072)
				memcpy(cfa_pc, "\005\003\004\001", 4);	/* GMCY */
			goto guess_cfa_pc;
		case 33424:
		case 65024:
			_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
			ParseKodakIFD(base);
			break;
		case 33434:			/* ExposureTime */
			_tiffIfd[ifd].shutter = _shutter = _reader->GetReal(type);
			break;
		case 33437:			/* FNumber */
			_aperture = _reader->GetReal(type);
			break;
		case 34306:			/* Leaf _white balance */
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ 1] = 4096.0 / _reader->GetUShort();
			break;
		case 34307:			/* Leaf CatchLight color matrix */
			_reader->Read(software, 1, 7);
			if (strncmp(software, "MATRIX", 6))
				break;
			_colors = 4;
			for (_rawColor = i = 0; i < 3; i++)
			{
				for (size_t c = 0; c < 4; c++)
					_reader->GetScanf("%f", &_rgbCam[i][c ^ 1]);
				if (!_options->_useCameraWB)
					continue;
				num = 0;
				for (size_t c = 0; c < 4; c++)
					num += _rgbCam[i][c];
				for (size_t c = 0; c < 4; c++)
					_rgbCam[i][c] /= num;
			}
			break;
		case 34310:			/* Leaf metadata */
			ParseMos(_reader->GetPosition());
		case 34303:
			strcpy_s(_make, LenMake, "Leaf");
			break;
		case 34665:			/* EXIF tag */
			_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
			ParseExif(base);
			break;
		case 34853:			/* GPSInfo tag */
			_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
			ParseGps(base);
			break;
		case 34675:			/* InterColorProfile */
		case 50831:			/* AsShotICCProfile */
			_profileOffset = _reader->GetPosition();
			_profileLength = len;
			break;
		case 37122:			/* CompressedBitsPerPixel */
			_kodakCbpp = _reader->GetUInt();
			break;
		case 37386:			/* FocalLength */
			_focalLen = _reader->GetReal(type);
			break;
		case 37393:			/* ImageNumber */
			_shotOrder = _reader->GetUInt(type);
			break;
		case 37400:			/* old Kodak KDC tag */
			for (_rawColor = i = 0; i < 3; i++)
			{
				_reader->GetReal(type);
				for (size_t c = 0; c < 3; c++)
					_rgbCam[i][c] = _reader->GetReal(type);
			}
			break;
		case 40976:
			_stripOffset = _reader->GetUInt();
			switch (_tiffIfd[ifd].comp)
			{
			case 32770:
				_loadRaw = LoadRawType::SamsungLoadRaw;
				break;
			case 32772:
				_loadRaw = LoadRawType::Samsung2LoadRaw;
				break;
			case 32773:
				_loadRaw = LoadRawType::Samsung3LoadRaw;
				break;
			}
			break;
		case 46275:			/* Imacon tags */
			strcpy_s(_make, LenMake, "Imacon");
			_dataOffset = _reader->GetPosition();
			ima_len = len;
			break;
		case 46279:
			if (!ima_len)
				break;
			_reader->Seek(38, SEEK_CUR);
		case 46274:
			_reader->Seek(40, SEEK_CUR);
			_rawWidth = _reader->GetUInt();
			_rawHeight = _reader->GetUInt();
			_leftMargin = _reader->GetUInt() & 7;
			_width = _rawWidth - _leftMargin - (_reader->GetUInt() & 7);
			_topMargin = _reader->GetUInt() & 7;
			_height = _rawHeight - _topMargin - (_reader->GetUInt() & 7);
			if (_rawWidth == 7262)
			{
				_height = 5444;
				_width = 7244;
				_leftMargin = 7;
			}
			_reader->Seek(52, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = _reader->GetReal(11);
			_reader->Seek(114, SEEK_CUR);
			_flip = (_reader->GetUShort() >> 7) * 90;
			if (_width * _height * 6 == ima_len)
			{
				if (_flip % 180 == 90)
					SWAP(_width, _height);
				_rawWidth = _width;
				_rawHeight = _height;
				_leftMargin = _topMargin = _filters = _flip = 0;
			}
			sprintf_s(_model, LenModel, "Ixpress %d-Mp", _height*_width / 1000000);
			_loadRaw = LoadRawType::ImaconFullLoadRaw;
			if (_filters)
			{
				if (_leftMargin & 1) _filters = 0x61616161;
				_loadRaw = LoadRawType::UnpackedLoadRaw;
			}
			_maximum = 0xffff;
			break;
		case 50454:			/* Sinar tag */
		case 50455:
			if (!(cbuf = (char *)malloc(len)))
			{
				break;
			}
			else
			{
				CAutoFreeMemory(cbuf, false);
				_reader->Read(cbuf, 1, len);
				for (cp = cbuf - 1; cp && cp < cbuf + len; cp = strchr(cp, '\n'))
					if (!strncmp(++cp, "Neutral ", 8))
						sscanf_s(cp + 8, "%f %f %f", _camMul, _camMul + 1, _camMul + 2);
			}
			break;
		case 50458:
			if (!_make[0])
				strcpy_s(_make, LenMake, "Hasselblad");
			break;
		case 50459:			/* Hasselblad tag */
			i = _reader->GetOrder();
			j = _reader->GetPosition();
			c = _tiffNifds;
			_reader->SetOrder(_reader->GetUShort());
			_reader->Seek(j + (_reader->GetUShort(), _reader->GetUInt()), SEEK_SET);
			ParseTiffIFD(j);
			_maximum = 0xffff;
			_tiffNifds = c;
			_reader->SetOrder(i);
			break;
		case 50706:			/* DNGVersion */
			for (size_t c = 0; c < 4; c++)
				_dngVersion = (_dngVersion << 8) + _reader->GetChar();
			if (!_make[0])
				strcpy_s(_make, LenMake, "DNG");
			_isRaw = 1;
			break;
		case 50708:			/* UniqueCameraModel */
			if (_model[0])
				break;
			_reader->GetString(_make, LenMake);
			if ((cp = strchr(_make, ' ')))
			{
				strcpy_s(_model, LenModel, cp + 1);
				*cp = 0;
			}
			break;
		case 50710:			/* CFAPlaneColor */
			if (_filters == 9)
				break;
			if (len > 4)
				len = 4;
			_colors = len;
			_reader->Read(cfa_pc, 1, _colors);
		guess_cfa_pc:
			for (size_t c = 0; c < _colors; c++)
				tab[cfa_pc[c]] = c;
			_cdesc[_colors] = 0;
			for (i = 16; i--; )
				_filters = _filters << 2 | tab[cfa_pat[i % plen]];
			_filters -= !_filters;
			break;
		case 50711:			/* CFALayout */
			if (_reader->GetUShort() == 2)
				_fujiWidth = 1;
			break;
		case 291:
		case 50712:			/* LinearizationTable */
			LinearTable(len);
			break;
		case 50713:			/* BlackLevelRepeatDim */
			_cBlack[4] = _reader->GetUShort();
			_cBlack[5] = _reader->GetUShort();
			if (_cBlack[4] * _cBlack[5] > sizeof _cBlack / sizeof *_cBlack - 6)
				_cBlack[4] = _cBlack[5] = 1;
			break;
		case 61450:
			_cBlack[4] = _cBlack[5] = MIN(sqrt(len), 64);
		case 50714:			/* BlackLevel */
			if (!(_cBlack[4] * _cBlack[5]))
				_cBlack[4] = _cBlack[5] = 1;
			for (size_t c = 0; c < (_cBlack[4] * _cBlack[5]); c++)
				_cBlack[6 + c] = _reader->GetReal(type);
			_black = 0;
			break;
		case 50715:			/* BlackLevelDeltaH */
		case 50716:			/* BlackLevelDeltaV */
			for (num = i = 0; i < (len & 0xffff); i++)
				num += _reader->GetReal(type);
			_black += num / len + 0.5;
			break;
		case 50717:			/* WhiteLevel */
			_maximum = _reader->GetUInt(type);
			break;
		case 50718:			/* DefaultScale */
			_pixelAspect = _reader->GetReal(type);
			_pixelAspect /= _reader->GetReal(type);
			break;
		case 50721:			/* ColorMatrix1 */
		case 50722:			/* ColorMatrix2 */
			for (size_t c = 0; c < _colors; c++)
				for (j = 0; j < 3; j++)
					cm[c][j] = _reader->GetReal(type);
			use_cm = true;
			break;
		case 50723:			/* CameraCalibration1 */
		case 50724:			/* CameraCalibration2 */
			for (i = 0; i < _colors; i++)
				for (size_t c = 0; c < _colors; c++)
					cc[i][c] = _reader->GetReal(type);
			break;
		case 50727:			/* AnalogBalance */
			for (size_t c = 0; c < _colors; c++)
				ab[c] = _reader->GetReal(type);
			break;
		case 50728:			/* AsShotNeutral */
			for (size_t c = 0; c < _colors; c++)
				asn[c] = _reader->GetReal(type);
			break;
		case 50729:			/* AsShotWhiteXY */
			xyz[0] = _reader->GetReal(type);
			xyz[1] = _reader->GetReal(type);
			xyz[2] = 1 - xyz[0] - xyz[1];
			for (size_t c = 0; c < 3; c++)
				xyz[c] /= d65White[c];
			break;
		case 50740:			/* DNGPrivateData */
			if (_dngVersion)
				break;
			ParseMinolta(j = _reader->GetUInt() + base);
			_reader->Seek(j, SEEK_SET);
			ParseTiffIFD(base);
			break;
		case 50752:
			_reader->ReadShorts(_cr2Slice, 3);
			break;
		case 50829:			/* ActiveArea */
			_topMargin = _reader->GetUInt(type);
			_leftMargin = _reader->GetUInt(type);
			_height = _reader->GetUInt(type) - _topMargin;
			_width = _reader->GetUInt(type) - _leftMargin;
			break;
		case 50830:			/* MaskedAreas */
			for (i = 0; i < len && i < 32; i++)
				((int *)_mask)[i] = _reader->GetUInt(type);
			_black = 0;
			break;
		case 51009:			/* OpcodeList2 */
			_metaOffset = _reader->GetPosition();
			break;
		case 64772:			/* Kodak P-series */
			if (len < 13) break;
			_reader->Seek(16, SEEK_CUR);
			_dataOffset = _reader->GetUInt();
			_reader->Seek(28, SEEK_CUR);
			_dataOffset += _reader->GetUInt();
			_loadRaw = LoadRawType::PackedLoadRaw;
			break;
		case 65026:
			if (type == 2) _reader->GetString(_model2, LenModel2);
		}
		_reader->Seek(save, SEEK_SET);
	}
	if (sony_length)
	{
		unsigned* buf = (unsigned *)malloc(sony_length);
		if (buf)
		{
			CAutoFreeMemory(buf, false);
			_reader->Seek(sony_offset, SEEK_SET);
			_reader->Read(buf, sony_length, 1);
			SonyDecrypt(buf, sony_length / 4, 1, sony_key);

			CWriter* sfp = CWriter::CreateTempFile();
			sfp->Write(buf, sony_length, 1);
			sfp->Seek(0, SEEK_SET);
			ParseTiffIFD(-sony_offset);
			delete sfp;
		}
	}
	for (i = 0; i < _colors; i++)
		for (size_t c = 0; c < _colors; c++)
			cc[i][c] *= ab[i];
	if (use_cm)
	{
		for (size_t c = 0; c < _colors; c++)
			for (i = 0; i < 3; i++)
				for (cam_xyz[c][i] = j = 0; j < _colors; j++)
					cam_xyz[c][i] += cc[c][j] * cm[j][i] * xyz[i];
		CamXyzCoeff(_cMatrix, cam_xyz);
	}
	if (asn[0])
	{
		_camMul[3] = 0;
		for (size_t c = 0; c < _colors; c++)
			_camMul[c] = 1 / asn[c];
	}
	if (!use_cm)
		for (size_t c = 0; c < _colors; c++)
			_preMul[c] /= cc[c][c];
	return 0;
}

void CSimpleInfo::ParseExif(int base)
{
	unsigned tag, type, len, save;
	double expo;

	unsigned kodak = !strncmp(_make, "EASTMAN", 7) && _tiffNifds < 3;
	unsigned entries = _reader->GetUShort();
	while (entries--)
	{
		TiffGet(base, &tag, &type, &len, &save);
		switch (tag)
		{
		case 33434:
			_tiffIfd[_tiffNifds - 1].shutter = _shutter = _reader->GetReal(type);
			break;
		case 33437:
			_aperture = _reader->GetReal(type);
			break;
		case 34855:
			_isoSpeed = _reader->GetUShort();
			break;
		case 36867:
		case 36868:
			GetTimestamp(0);
			break;
		case 37377:
			if ((expo = -_reader->GetReal(type)) < 128)
				_tiffIfd[_tiffNifds - 1].shutter = _shutter = pow(2, expo);
			break;
		case 37378:
			_aperture = pow(2, _reader->GetReal(type) / 2);
			break;
		case 37386:
			_focalLen = _reader->GetReal(type);
			break;
		case 37500:
			ParseMakernote(base, 0);
			break;
		case 40962:
			if (kodak) _rawWidth = _reader->GetUInt();
			break;
		case 40963:
			if (kodak) _rawHeight = _reader->GetUInt();
			break;
		case 41730:
			if (_reader->GetUInt() == 0x20002)
			{
				_exifCfa = 0;
				for (size_t c = 0; c < 8; c += 2)
					_exifCfa |= _reader->GetChar() * 0x01010101 << c;
			}
		}
		_reader->Seek(save, SEEK_SET);
	}
}

void CSimpleInfo::ParseGps(int base)
{
	unsigned tag, type, len, save;

	unsigned entries = _reader->GetUShort();
	while (entries--)
	{
		TiffGet(base, &tag, &type, &len, &save);
		switch (tag)
		{
		case 1: case 3: case 5:
			_gpsData[29 + tag / 2] = _reader->GetChar();
			break;
		case 2: case 4: case 7:
			for (size_t c = 0; c < 6; c++)
				_gpsData[tag / 3 * 6 + c] = _reader->GetUInt();
			break;
		case 6:
			for (size_t c = 0; c < 6; c++)
				_gpsData[18 + c] = _reader->GetUInt();
			break;
		case 18: case 29:
			_reader->GetString((char *)(_gpsData + 14 + tag / 3), MIN(len, 12));
		}
		_reader->Seek(save, SEEK_SET);
	}
}
void CSimpleInfo::ParseMakernote(int base, int uptag)
{
	static const unsigned char xlat[2][256] = {
		{ 0xc1,0xbf,0x6d,0x0d,0x59,0xc5,0x13,0x9d,0x83,0x61,0x6b,0x4f,0xc7,0x7f,0x3d,0x3d,
		0x53,0x59,0xe3,0xc7,0xe9,0x2f,0x95,0xa7,0x95,0x1f,0xdf,0x7f,0x2b,0x29,0xc7,0x0d,
		0xdf,0x07,0xef,0x71,0x89,0x3d,0x13,0x3d,0x3b,0x13,0xfb,0x0d,0x89,0xc1,0x65,0x1f,
		0xb3,0x0d,0x6b,0x29,0xe3,0xfb,0xef,0xa3,0x6b,0x47,0x7f,0x95,0x35,0xa7,0x47,0x4f,
		0xc7,0xf1,0x59,0x95,0x35,0x11,0x29,0x61,0xf1,0x3d,0xb3,0x2b,0x0d,0x43,0x89,0xc1,
		0x9d,0x9d,0x89,0x65,0xf1,0xe9,0xdf,0xbf,0x3d,0x7f,0x53,0x97,0xe5,0xe9,0x95,0x17,
		0x1d,0x3d,0x8b,0xfb,0xc7,0xe3,0x67,0xa7,0x07,0xf1,0x71,0xa7,0x53,0xb5,0x29,0x89,
		0xe5,0x2b,0xa7,0x17,0x29,0xe9,0x4f,0xc5,0x65,0x6d,0x6b,0xef,0x0d,0x89,0x49,0x2f,
		0xb3,0x43,0x53,0x65,0x1d,0x49,0xa3,0x13,0x89,0x59,0xef,0x6b,0xef,0x65,0x1d,0x0b,
		0x59,0x13,0xe3,0x4f,0x9d,0xb3,0x29,0x43,0x2b,0x07,0x1d,0x95,0x59,0x59,0x47,0xfb,
		0xe5,0xe9,0x61,0x47,0x2f,0x35,0x7f,0x17,0x7f,0xef,0x7f,0x95,0x95,0x71,0xd3,0xa3,
		0x0b,0x71,0xa3,0xad,0x0b,0x3b,0xb5,0xfb,0xa3,0xbf,0x4f,0x83,0x1d,0xad,0xe9,0x2f,
		0x71,0x65,0xa3,0xe5,0x07,0x35,0x3d,0x0d,0xb5,0xe9,0xe5,0x47,0x3b,0x9d,0xef,0x35,
		0xa3,0xbf,0xb3,0xdf,0x53,0xd3,0x97,0x53,0x49,0x71,0x07,0x35,0x61,0x71,0x2f,0x43,
		0x2f,0x11,0xdf,0x17,0x97,0xfb,0x95,0x3b,0x7f,0x6b,0xd3,0x25,0xbf,0xad,0xc7,0xc5,
		0xc5,0xb5,0x8b,0xef,0x2f,0xd3,0x07,0x6b,0x25,0x49,0x95,0x25,0x49,0x6d,0x71,0xc7 },
		{ 0xa7,0xbc,0xc9,0xad,0x91,0xdf,0x85,0xe5,0xd4,0x78,0xd5,0x17,0x46,0x7c,0x29,0x4c,
		0x4d,0x03,0xe9,0x25,0x68,0x11,0x86,0xb3,0xbd,0xf7,0x6f,0x61,0x22,0xa2,0x26,0x34,
		0x2a,0xbe,0x1e,0x46,0x14,0x68,0x9d,0x44,0x18,0xc2,0x40,0xf4,0x7e,0x5f,0x1b,0xad,
		0x0b,0x94,0xb6,0x67,0xb4,0x0b,0xe1,0xea,0x95,0x9c,0x66,0xdc,0xe7,0x5d,0x6c,0x05,
		0xda,0xd5,0xdf,0x7a,0xef,0xf6,0xdb,0x1f,0x82,0x4c,0xc0,0x68,0x47,0xa1,0xbd,0xee,
		0x39,0x50,0x56,0x4a,0xdd,0xdf,0xa5,0xf8,0xc6,0xda,0xca,0x90,0xca,0x01,0x42,0x9d,
		0x8b,0x0c,0x73,0x43,0x75,0x05,0x94,0xde,0x24,0xb3,0x80,0x34,0xe5,0x2c,0xdc,0x9b,
		0x3f,0xca,0x33,0x45,0xd0,0xdb,0x5f,0xf5,0x52,0xc3,0x21,0xda,0xe2,0x22,0x72,0x6b,
		0x3e,0xd0,0x5b,0xa8,0x87,0x8c,0x06,0x5d,0x0f,0xdd,0x09,0x19,0x93,0xd0,0xb9,0xfc,
		0x8b,0x0f,0x84,0x60,0x33,0x1c,0x9b,0x45,0xf1,0xf0,0xa3,0x94,0x3a,0x12,0x77,0x33,
		0x4d,0x44,0x78,0x28,0x3c,0x9e,0xfd,0x65,0x57,0x16,0x94,0x6b,0xfb,0x59,0xd0,0xc8,
		0x22,0x36,0xdb,0xd2,0x63,0x98,0x43,0xa1,0x04,0x87,0x86,0xf7,0xa6,0x26,0xbb,0xd6,
		0x59,0x4d,0xbf,0x6a,0x2e,0xaa,0x2b,0xef,0xe6,0x78,0xb6,0x4e,0xe0,0x2f,0xdc,0x7c,
		0xbe,0x57,0x19,0x32,0x7e,0x2a,0xd0,0xb8,0xba,0x29,0x00,0x3c,0x52,0x7d,0xa8,0x49,
		0x3b,0x2d,0xeb,0x25,0x49,0xfa,0xa3,0xaa,0x39,0xa7,0xc5,0xa7,0x50,0x11,0x36,0xfb,
		0xc6,0x67,0x4a,0xf5,0xa5,0x12,0x65,0x7e,0xb0,0xdf,0xaf,0x4e,0xb3,0x61,0x7f,0x2f } };
	unsigned tag, type, len, save;
	unsigned ver97 = 0;
	unsigned serial = 0;
	unsigned wbi = 0;
	unsigned wb[4] = { 0,0,0,0 };
	unsigned i;
	unsigned char buf97[324];
	short sorder = _reader->GetOrder();
	char buf[10];
	/*
	The MakerNote might have its own TIFF header (possibly with
	its own byte-order!), or it might just be a table.
	*/
	if (!strcmp(_make, "Nokia")) return;
	_reader->Read(buf, 1, 10);
	if (!strncmp(buf, "KDK", 3) ||	/* these aren't TIFF tables */
		!strncmp(buf, "VER", 3) ||
		!strncmp(buf, "IIII", 4) ||
		!strncmp(buf, "MMMM", 4)) return;
	if (!strncmp(buf, "KC", 2) ||	/* Konica KD-400Z, KD-510Z */
		!strncmp(buf, "MLY", 3))
	{	/* Minolta DiMAGE G series */
		_reader->SetOrder(0x4d4d);
		while ((i = _reader->GetPosition()) < _dataOffset && i < 16384)
		{
			wb[0] = wb[2];  wb[2] = wb[1];  wb[1] = wb[3];
			wb[3] = _reader->GetUShort();
			if (wb[1] == 256 && wb[3] == 256 &&
				wb[0] > 256 && wb[0] < 640 && wb[2] > 256 && wb[2] < 640)
				for (size_t c = 0; c < 4; c++)
					_camMul[c] = wb[c];
		}
		goto quit;
	}
	if (!strcmp(buf, "Nikon"))
	{
		base = _reader->GetPosition();
		_reader->SetOrder(_reader->GetUShort());
		if (_reader->GetUShort() != 42)
			goto quit;
		unsigned offset = _reader->GetUInt();
		_reader->Seek(offset - 8, SEEK_CUR);
	}
	else if (!strcmp(buf, "OLYMPUS") || !strcmp(buf, "PENTAX "))
	{
		base = _reader->GetPosition() - 10;
		_reader->Seek(-2, SEEK_CUR);
		_reader->SetOrder(_reader->GetUShort());
		if (buf[0] == 'O')
			_reader->GetUShort();
	}
	else if (!strncmp(buf, "SONY", 4) || !strcmp(buf, "Panasonic"))
	{
		goto nf;
	}
	else if (!strncmp(buf, "FUJIFILM", 8))
	{
		base = _reader->GetPosition() - 10;
	nf: _reader->SetOrder(0x4949);
		_reader->Seek(2, SEEK_CUR);
	}
	else if (!strcmp(buf, "OLYMP") || !strcmp(buf, "LEICA") || !strcmp(buf, "Ricoh") || !strcmp(buf, "EPSON"))
		_reader->Seek(-2, SEEK_CUR);
	else if (!strcmp(buf, "AOC") || !strcmp(buf, "QVC"))
		_reader->Seek(-4, SEEK_CUR);
	else
	{
		_reader->Seek(-10, SEEK_CUR);
		if (!strncmp(_make, "SAMSUNG", 7))
			base = _reader->GetPosition();
	}
	unsigned entries = _reader->GetUShort();
	if (entries > 1000)
		return;
	short morder = _reader->GetOrder();
	while (entries--)
	{
		_reader->SetOrder(morder);
		TiffGet(base, &tag, &type, &len, &save);
		tag |= uptag << 16;
		if (tag == 2 && strstr(_make, "NIKON") && !_isoSpeed)
			_isoSpeed = (_reader->GetUShort(), _reader->GetUShort());
		if (tag == 4 && len > 26 && len < 35)
		{
			if ((i = (_reader->GetUInt(), _reader->GetUShort())) != 0x7fff && !_isoSpeed)
				_isoSpeed = 50 * pow(2, i / 32.0 - 4);
			if ((i = (_reader->GetUShort(), _reader->GetUShort())) != 0x7fff && !_aperture)
				_aperture = pow(2, i / 64.0);
			if ((i = _reader->GetUShort()) != 0xffff && !_shutter)
				_shutter = pow(2, (short)i / -32.0);
			wbi = (_reader->GetUShort(), _reader->GetUShort());
			_shotOrder = (_reader->GetUShort(), _reader->GetUShort());
		}
		if ((tag == 4 || tag == 0x114) && !strncmp(_make, "KONICA", 6))
		{
			_reader->Seek(tag == 4 ? 140 : 160, SEEK_CUR);
			switch (_reader->GetUShort())
			{
			case 72:  _flip = 0;  break;
			case 76:  _flip = 6;  break;
			case 82:  _flip = 5;  break;
			}
		}
		if (tag == 7 && type == 2 && len > 20)
			_reader->GetString(_model2, LenModel2);
		if (tag == 8 && type == 4)
			_shotOrder = _reader->GetUInt();
		if (tag == 9 && !strcmp(_make, "Canon"))
			_reader->Read(_artist, LenArtist, 1);
		if (tag == 0xc && len == 4)
			for (size_t c = 0; c < 3; c++)
				_camMul[(c << 1 | c >> 1) & 3] = _reader->GetReal(type);
		if (tag == 0xd && type == 7 && _reader->GetUShort() == 0xaaaa)
		{
			unsigned c = 2;
			for (i = 2; (unsigned short)c != 0xbbbb && i < len; i++)
				c = c << 8 | _reader->GetChar();
			while ((i += 4) < len - 5)
				if (_reader->GetUInt() == 257 && (i = len) && (c = (_reader->GetUInt(), _reader->GetChar())) < 3)
					_flip = "065"[c] - '0';
		}
		if (tag == 0x10 && type == 4)
			_uniqueId = _reader->GetUInt();
		if (tag == 0x11 && _isRaw && !strncmp(_make, "NIKON", 5))
		{
			_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
			ParseTiffIFD(base);
		}
		if (tag == 0x14 && type == 7)
		{
			if (len == 2560)
			{
				_reader->Seek(1248, SEEK_CUR);
				goto get2_256;
			}
			_reader->Read(buf, 1, 10);
			if (!strncmp(buf, "NRW ", 4))
			{
				_reader->Seek(strcmp(buf + 4, "0100") ? 46 : 1546, SEEK_CUR);
				_camMul[0] = _reader->GetUInt() << 2;
				_camMul[1] = _reader->GetUInt() + _reader->GetUInt();
				_camMul[2] = _reader->GetUInt() << 2;
			}
		}
		if (tag == 0x15 && type == 2 && _isRaw)
			_reader->Read(_model, LenModel, 1);
		if (strstr(_make, "PENTAX"))
		{
			if (tag == 0x1b) tag = 0x1018;
			if (tag == 0x1c) tag = 0x1017;
		}
		if (tag == 0x1d)
		{
			unsigned c;
			while ((c = _reader->GetChar()) && c != EOF)
				serial = serial * 10 + (isdigit(c) ? c - '0' : c % 10);
		}
		if (tag == 0x29 && type == 1)
		{
			unsigned y = wbi < 18 ? "012347800000005896"[wbi] - '0' : 0;
			_reader->Seek(8 + y * 32, SEEK_CUR);
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1) ^ 1] = _reader->GetUInt();
		}
		if (tag == 0x3d && type == 3 && len == 4)
			for (size_t c = 0; c < 4; c++)
				_cBlack[c ^ c >> 1] = _reader->GetUShort() >> (14 - _tiffBps);
		if (tag == 0x81 && type == 4)
		{
			_dataOffset = _reader->GetUInt();
			_reader->Seek(_dataOffset + 41, SEEK_SET);
			_rawHeight = _reader->GetUShort() * 2;
			_rawWidth = _reader->GetUShort();
			_filters = 0x61616161;
		}
		if ((tag == 0x81 && type == 7) ||
			(tag == 0x100 && type == 7) ||
			(tag == 0x280 && type == 1))
		{
			_thumbOffset = _reader->GetPosition();
			_thumbLength = len;
		}
		if (tag == 0x88 && type == 4 && (_thumbOffset = _reader->GetUInt()))
			_thumbOffset += base;
		if (tag == 0x89 && type == 4)
			_thumbLength = _reader->GetUInt();
		if (tag == 0x8c || tag == 0x96)
			_metaOffset = _reader->GetPosition();
		if (tag == 0x97)
		{
			for (i = 0; i < 4; i++)
				ver97 = ver97 * 10 + _reader->GetChar() - '0';
			switch (ver97)
			{
			case 100:
				_reader->Seek(68, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					_camMul[(c >> 1) | ((c & 1) << 1)] = _reader->GetUShort();
				break;
			case 102:
				_reader->Seek(6, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					_camMul[c ^ (c >> 1)] = _reader->GetUShort();
				break;
			case 103:
				_reader->Seek(16, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					_camMul[c] = _reader->GetUShort();
			}
			if (ver97 >= 200)
			{
				if (ver97 != 205) _reader->Seek(280, SEEK_CUR);
				_reader->Read(buf97, 324, 1);
			}
		}
		if (tag == 0xa1 && type == 7)
		{
			_reader->SetOrder(0x4949);
			_reader->Seek(140, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = _reader->GetUInt();
		}
		if (tag == 0xa4 && type == 3)
		{
			_reader->Seek(wbi * 48, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = _reader->GetUShort();
		}
		if (tag == 0xa7 && (unsigned)(ver97 - 200) < 17)
		{
			unsigned char ci = xlat[0][serial & 0xff];
			unsigned char cj = xlat[1][_reader->GetChar() ^ _reader->GetChar() ^ _reader->GetChar() ^ _reader->GetChar()];
			unsigned char ck = 0x60;
			for (i = 0; i < 324; i++)
				buf97[i] ^= (cj += ci * ck++);
			i = "66666>666;6A;:;55"[ver97 - 200] - '0';
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1) ^ (i & 1)] = _reader->GetUShort(buf97 + (i & -2) + c * 2);
		}
		if (tag == 0x200 && len == 3)
			_shotOrder = (_reader->GetUInt(), _reader->GetUInt());
		if (tag == 0x200 && len == 4)
			for (size_t c = 0; c < 4; c++)
				_cBlack[c ^ c >> 1] = _reader->GetUShort();
		if (tag == 0x201 && len == 4)
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1)] = _reader->GetUShort();
		if (tag == 0x220 && type == 7)
			_metaOffset = _reader->GetPosition();
		if (tag == 0x401 && type == 4 && len == 4)
			for (size_t c = 0; c < 4; c++)
				_cBlack[c ^ c >> 1] = _reader->GetUInt();
		if (tag == 0xe01)
		{		/* Nikon Capture Note */
			_reader->SetOrder(0x4949);
			_reader->Seek(22, SEEK_CUR);
			for (unsigned offset = 22; offset + 22 < len; offset += 22 + i)
			{
				tag = _reader->GetUInt();
				_reader->Seek(14, SEEK_CUR);
				i = _reader->GetUInt() - 4;
				if (tag == 0x76a43207) _flip = _reader->GetUShort();
				else _reader->Seek(i, SEEK_CUR);
			}
		}
		if (tag == 0xe80 && len == 256 && type == 7)
		{
			_reader->Seek(48, SEEK_CUR);
			_camMul[0] = _reader->GetUShort() * 508 * 1.078 / 0x10000;
			_camMul[2] = _reader->GetUShort() * 382 * 1.173 / 0x10000;
		}
		if (tag == 0xf00 && type == 7)
		{
			if (len == 614)
				_reader->Seek(176, SEEK_CUR);
			else if (len == 734 || len == 1502)
				_reader->Seek(148, SEEK_CUR);
			else goto next;
			goto get2_256;
		}
		if ((tag == 0x1011 && len == 9) || tag == 0x20400200)
			for (i = 0; i < 3; i++)
				for (size_t c = 0; c < 3; c++)
					_cMatrix[i][c] = ((short)_reader->GetUShort()) / 256.0;
		if ((tag == 0x1012 || tag == 0x20400600) && len == 4)
			for (size_t c = 0; c < 4; c++)
				_cBlack[c ^ c >> 1] = _reader->GetUShort();
		if (tag == 0x1017 || tag == 0x20400100)
			_camMul[0] = _reader->GetUShort() / 256.0;
		if (tag == 0x1018 || tag == 0x20400100)
			_camMul[2] = _reader->GetUShort() / 256.0;
		if (tag == 0x2011 && len == 2)
		{
		get2_256:
			_reader->SetOrder(0x4d4d);
			_camMul[0] = _reader->GetUShort() / 256.0;
			_camMul[2] = _reader->GetUShort() / 256.0;
		}
		if ((tag | 0x70) == 0x2070 && (type == 4 || type == 13))
			_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
		if (tag == 0x2020 && !strncmp(buf, "OLYMP", 5))
			ParseThumbNote(base, 257, 258);
		if (tag == 0x2040)
			ParseMakernote(base, 0x2040);
		if (tag == 0xb028)
		{
			_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
			ParseThumbNote(base, 136, 137);
		}
		if (tag == 0x4001 && len > 500)
		{
			i = len == 582 ? 50 : len == 653 ? 68 : len == 5120 ? 142 : 126;
			_reader->Seek(i, SEEK_CUR);
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1)] = _reader->GetUShort();
			for (i += 18; i <= len; i += 10)
			{
				_reader->GetUShort();
				for (size_t c = 0; c < 4; c++)
					_srawMul[c ^ (c >> 1)] = _reader->GetUShort();
				if (_srawMul[1] == 1170) break;
			}
		}
		if (tag == 0x4021 && _reader->GetUInt() && _reader->GetUInt())
			for (size_t c = 0; c < 4; c++)
				_camMul[c] = 1024;
		if (tag == 0xa021)
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1)] = _reader->GetUInt();
		if (tag == 0xa028)
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1)] -= _reader->GetUInt();
		if (tag == 0xb001)
			_uniqueId = _reader->GetUShort();
	next:
		_reader->Seek(save, SEEK_SET);
	}
quit:
	_reader->SetOrder(sorder);
}

void CSimpleInfo::ParseThumbNote(int base, unsigned toff, unsigned tlen)
{
	unsigned tag, type, len, save;

	unsigned entries = _reader->GetUShort();
	while (entries--)
	{
		TiffGet(base, &tag, &type, &len, &save);
		if (tag == toff) _thumbOffset = _reader->GetUInt() + base;
		if (tag == tlen) _thumbLength = _reader->GetUInt();
		_reader->Seek(save, SEEK_SET);
	}
}

void CSimpleInfo::GetTimestamp(int reversed)
{
	char str[20];
	str[19] = 0;
	if (reversed)
		for (size_t i = 19; i--; )
			str[i] = _reader->GetChar();
	else
		_reader->Read(str, 19, 1);

	tm t;
	memset(&t, 0, sizeof t);
	if (sscanf_s(str, "%d:%d:%d %d:%d:%d", &t.tm_year, &t.tm_mon,
		&t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec) != 6)
		return;
	t.tm_year -= 1900;
	t.tm_mon -= 1;
	t.tm_isdst = -1;
	if (mktime(&t) > 0)
		_timestamp = mktime(&t);
}

void CSimpleInfo::TiffGet(unsigned base, unsigned *tag, unsigned *type, unsigned *len, unsigned *save)
{
	*tag = _reader->GetUShort();
	*type = _reader->GetUShort();
	*len = _reader->GetUInt();
	*save = _reader->GetPosition() + 4;
	if (*len * ("11124811248484"[*type < 14 ? *type : 0] - '0') > 4)
		_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
}

void CSimpleInfo::ApplyTiff()
{
	int max_samp = 0, ties = 0, raw = -1, thm = -1;

	_thumbMisc = 16;
	if (_thumbOffset)
	{
		_reader->Seek(_thumbOffset, SEEK_SET);
		JHead jh(*_reader, *this, true);
		if (jh._success)
		{
			_thumbMisc = jh._jdata.bits;
			_thumbWidth = jh._jdata.wide;
			_thumbHeight = jh._jdata.high;
		}
	}
	for (size_t i = _tiffNifds; i--; )
	{
		if (_tiffIfd[i].shutter)
			_shutter = _tiffIfd[i].shutter;
		_tiffIfd[i].shutter = _shutter;
	}
	for (size_t i = 0; i < _tiffNifds; i++)
	{
		if (max_samp < _tiffIfd[i].samples)
			max_samp = _tiffIfd[i].samples;
		if (max_samp > 3) max_samp = 3;
		int os = _rawWidth*_rawHeight;
		int ns = _tiffIfd[i].width*_tiffIfd[i].height;
		if (_tiffBps)
		{
			os *= _tiffBps;
			ns *= _tiffIfd[i].bps;
		}
		if ((_tiffIfd[i].comp != 6 || _tiffIfd[i].samples != 3) &&
			(_tiffIfd[i].width | _tiffIfd[i].height) < 0x10000 &&
			ns && ((ns > os && (ties = 1)) ||
			(ns == os && _options->_shotSelect == ties++)))
		{
			_rawWidth = _tiffIfd[i].width;
			_rawHeight = _tiffIfd[i].height;
			_tiffBps = _tiffIfd[i].bps;
			_tiffCompress = _tiffIfd[i].comp;
			_dataOffset = _tiffIfd[i].offset;
			_tiffFlip = _tiffIfd[i].flip;
			_tiffSamples = _tiffIfd[i].samples;
			_tileWidth = _tiffIfd[i].tile_width;
			_tileLength = _tiffIfd[i].tile_length;
			_shutter = _tiffIfd[i].shutter;
			raw = i;
		}
	}
	if (_isRaw == 1 && ties)
		_isRaw = ties;
	if (!_tileWidth)
		_tileWidth = INT_MAX;
	if (!_tileLength)
		_tileLength = INT_MAX;
	for (size_t i = _tiffNifds; i--; )
		if (_tiffIfd[i].flip)
			_tiffFlip = _tiffIfd[i].flip;
	if (raw >= 0 && !_loadRaw)
	{
		switch (_tiffCompress)
		{
		case 32767:
			if (_tiffIfd[raw].bytes == _rawWidth*_rawHeight)
			{
				_tiffBps = 12;
				_loadRaw = LoadRawType::SonyArw2LoadRaw;
				break;
			}
			if (_tiffIfd[raw].bytes * 8 != _rawWidth*_rawHeight*_tiffBps)
			{
				_rawHeight += 8;
				_loadRaw = LoadRawType::SonyArwLoadRaw;
				break;
			}
			_loadFlags = 79;
		case 32769:
			_loadFlags++;
		case 32770:
		case 32773:
			goto slr;
		case 0:  case 1:
			if (!strncmp(_make, "OLYMPUS", 7) &&
				_tiffIfd[raw].bytes * 2 == _rawWidth*_rawHeight * 3)
				_loadFlags = 24;
			if (_tiffIfd[raw].bytes * 5 == _rawWidth*_rawHeight * 8)
			{
				_loadFlags = 81;
				_tiffBps = 12;
			} slr:
			switch (_tiffBps)
			{
			case  8:
				_loadRaw = LoadRawType::EightBitLoadRaw;
				break;
			case 12:
				if (_tiffIfd[raw].phint == 2)
					_loadFlags = 6;
				_loadRaw = LoadRawType::PackedLoadRaw;
				break;
			case 14:
				_loadFlags = 0;
			case 16:
				_loadRaw = LoadRawType::UnpackedLoadRaw;
				if (!strncmp(_make, "OLYMPUS", 7) && _tiffIfd[raw].bytes * 7 > _rawWidth*_rawHeight)
					_loadRaw = LoadRawType::OlympusLoadRaw;
			}
			break;
		case 6:  case 7:  case 99:
			_loadRaw = LoadRawType::LosslessJpegLoadRaw;
			break;
		case 262:
			_loadRaw = LoadRawType::Kodak262LoadRaw;
			break;
		case 34713:
			if ((_rawWidth + 9) / 10 * 16 * _rawHeight == _tiffIfd[raw].bytes)
			{
				_loadRaw = LoadRawType::PackedLoadRaw;
				_loadFlags = 1;
			}
			else if (_rawWidth*_rawHeight * 3 == _tiffIfd[raw].bytes * 2)
			{
				_loadRaw = LoadRawType::PackedLoadRaw;
				if (_model[0] == 'N') _loadFlags = 80;
			}
			else if (_rawWidth*_rawHeight * 3 == _tiffIfd[raw].bytes)
			{
				_loadRaw = LoadRawType::NikonYuvLoadRaw;
				GammaCurve(1 / 2.4, 12.92, 1, 4095);
				memset(_cBlack, 0, sizeof _cBlack);
				_filters = 0;
			}
			else if (_rawWidth*_rawHeight * 2 == _tiffIfd[raw].bytes)
			{
				_loadRaw = LoadRawType::UnpackedLoadRaw;
				_loadFlags = 4;
				_reader->SetOrder(0x4d4d);
			}
			else
				_loadRaw = LoadRawType::NikonLoadRaw;
			break;
		case 65535:
			_loadRaw = LoadRawType::PentaxLoadRaw;
			break;
		case 65000:
			switch (_tiffIfd[raw].phint)
			{
			case 2:
				_loadRaw = LoadRawType::KodakRgbLoadRaw;   _filters = 0;
				break;
			case 6:
				_loadRaw = LoadRawType::KodakYcbcrLoadRaw; _filters = 0;
				break;
			case 32803:
				_loadRaw = LoadRawType::Kodak65000LoadRaw;
			}
		case 32867: case 34892:
			break;
		default: _isRaw = 0;
		}
	}
	if (!_dngVersion)
	{
		if ((_tiffSamples == 3 && _tiffIfd[raw].bytes && _tiffBps != 14 &&
			(_tiffCompress & -16) != 32768)
			|| (_tiffBps == 8 && strncmp(_make, "Phase", 5) &&
				!strcasestr(_make, "Kodak") && !strstr(_model2, "DEBUG RAW")))
		{
			_isRaw = 0;
		}
	}
	for (size_t i = 0; i < _tiffNifds; i++)
	{
		if (i != raw && _tiffIfd[i].samples == max_samp &&
			_tiffIfd[i].width * _tiffIfd[i].height / (SQR(_tiffIfd[i].bps) + 1) >
			_thumbWidth *       _thumbHeight / (SQR(_thumbMisc) + 1)
			&& _tiffIfd[i].comp != 34892)
		{
			_thumbWidth = _tiffIfd[i].width;
			_thumbHeight = _tiffIfd[i].height;
			_thumbOffset = _tiffIfd[i].offset;
			_thumbLength = _tiffIfd[i].bytes;
			_thumbMisc = _tiffIfd[i].bps;
			thm = i;
		}
	}
	if (thm >= 0)
	{
		_thumbMisc |= _tiffIfd[thm].samples << 5;
		switch (_tiffIfd[thm].comp)
		{
		case 0:
			_writeThumb = WriteThumbType::LayerThumb;
			break;
		case 1:
			if (_tiffIfd[thm].bps <= 8)
				_writeThumb = WriteThumbType::PpmThumb;
			else if (!strcmp(_make, "Imacon"))
				_writeThumb = WriteThumbType::Ppm16Thumb;
			else
				_thumbLoadRaw = LoadRawType::LodakThumbLoadRaw;
			break;
		case 65000:
			_thumbLoadRaw = _tiffIfd[thm].phint == 6 ?
				LoadRawType::KodakYcbcrLoadRaw : LoadRawType::KodakRgbLoadRaw;
		}
	}
}

/*
Many cameras have a "debug mode" that writes JPEG and raw
at the same time.  The raw file has no header, so try to
to open the matching JPEG file and read its metadata.
*/
void CSimpleInfo::ParseExternalJpeg()
{
	const char* ext = strrchr(_reader->GetFileName(), '.');
	const char* file = strrchr(_reader->GetFileName(), '/');
	if (!file)
		file = strrchr(_reader->GetFileName(), '\\');
	if (!file)
		file = _reader->GetFileName() - 1;
	file++;
	if (!ext || strlen(ext) != 4 || ext - file != 8)
		return;
	size_t lenName = strlen(_reader->GetFileName()) + 1;
	char* jname = (char *)malloc(lenName);
	CAutoFreeMemory autoFree(jname);

	strcpy_s(jname, lenName, _reader->GetFileName());
	char* jfile = file - _reader->GetFileName() + jname;
	char* jext = ext - _reader->GetFileName() + jname;
	if (_stricmp(ext, ".jpg"))
	{
		strcpy_s(jext, strlen(jext) + 1, isupper(ext[1]) ? ".JPG" : ".jpg");
		if (isdigit(*file))
		{
			memcpy(jfile, file + 4, 4);
			memcpy(jfile + 4, file, 4);
		}
	}
	else
		while (isdigit(*--jext))
		{
			if (*jext != '9')
			{
				(*jext)++;
				break;
			}
			*jext = '0';
		}
	if (strcmp(jname, _reader->GetFileName()))
	{
		CReader* save = _reader;

		try
		{
			_reader = new CReader(jname);
		}
		catch (const CExceptionFile&)
		{
			_reader = save;
			throw;
		}

		ParseTiff(12);
		_thumbOffset = 0;
		_isRaw = 1;
		delete _reader;

		_reader = save;
	}
	if (!_timestamp)
		fprintf(stderr, ("Failed to read metadata from %s\n"), jname);
}

/*
All matrices are from Adobe DNG Converter unless otherwise noted.
*/
void CSimpleInfo::AdobeCoeff(const char *make, const char *model)
{
	static const struct
	{
		const char *prefix;
		short black, maximum, trans[12];
	} table[] = {
		{ "AgfaPhoto DC-833m", 0, 0,	/* DJC */
		{ 11438,-3762,-1115,-2409,9914,2497,-1227,2295,5300 } },
		{ "Apple QuickTake", 0, 0,		/* DJC */
		{ 21392,-5653,-3353,2406,8010,-415,7166,1427,2078 } },
		{ "Canon EOS D2000", 0, 0,
		{ 24542,-10860,-3401,-1490,11370,-297,2858,-605,3225 } },
		{ "Canon EOS D6000", 0, 0,
		{ 20482,-7172,-3125,-1033,10410,-285,2542,226,3136 } },
		{ "Canon EOS D30", 0, 0,
		{ 9805,-2689,-1312,-5803,13064,3068,-2438,3075,8775 } },
		{ "Canon EOS D60", 0, 0xfa0,
		{ 6188,-1341,-890,-7168,14489,2937,-2640,3228,8483 } },
		{ "Canon EOS 5DS", 0, 0x3c96,
		{ 6250,-711,-808,-5153,12794,2636,-1249,2198,5610 } },
		{ "Canon EOS 5D Mark III", 0, 0x3c80,
		{ 6722,-635,-963,-4287,12460,2028,-908,2162,5668 } },
		{ "Canon EOS 5D Mark II", 0, 0x3cf0,
		{ 4716,603,-830,-7798,15474,2480,-1496,1937,6651 } },
		{ "Canon EOS 5D", 0, 0xe6c,
		{ 6347,-479,-972,-8297,15954,2480,-1968,2131,7649 } },
		{ "Canon EOS 6D", 0, 0x3c82,
		{ 7034,-804,-1014,-4420,12564,2058,-851,1994,5758 } },
		{ "Canon EOS 7D Mark II", 0, 0x3510,
		{ 7268,-1082,-969,-4186,11839,2663,-825,2029,5839 } },
		{ "Canon EOS 7D", 0, 0x3510,
		{ 6844,-996,-856,-3876,11761,2396,-593,1772,6198 } },
		{ "Canon EOS 10D", 0, 0xfa0,
		{ 8197,-2000,-1118,-6714,14335,2592,-2536,3178,8266 } },
		{ "Canon EOS 20Da", 0, 0,
		{ 14155,-5065,-1382,-6550,14633,2039,-1623,1824,6561 } },
		{ "Canon EOS 20D", 0, 0xfff,
		{ 6599,-537,-891,-8071,15783,2424,-1983,2234,7462 } },
		{ "Canon EOS 30D", 0, 0,
		{ 6257,-303,-1000,-7880,15621,2396,-1714,1904,7046 } },
		{ "Canon EOS 40D", 0, 0x3f60,
		{ 6071,-747,-856,-7653,15365,2441,-2025,2553,7315 } },
		{ "Canon EOS 50D", 0, 0x3d93,
		{ 4920,616,-593,-6493,13964,2784,-1774,3178,7005 } },
		{ "Canon EOS 60D", 0, 0x2ff7,
		{ 6719,-994,-925,-4408,12426,2211,-887,2129,6051 } },
		{ "Canon EOS 70D", 0, 0x3bc7,
		{ 7034,-804,-1014,-4420,12564,2058,-851,1994,5758 } },
		{ "Canon EOS 80D", 0, 0,
		{ 7457,-671,-937,-4849,12495,2643,-1213,2354,5492 } },
		{ "Canon EOS 100D", 0, 0x350f,
		{ 6602,-841,-939,-4472,12458,2247,-975,2039,6148 } },
		{ "Canon EOS 300D", 0, 0xfa0,
		{ 8197,-2000,-1118,-6714,14335,2592,-2536,3178,8266 } },
		{ "Canon EOS 350D", 0, 0xfff,
		{ 6018,-617,-965,-8645,15881,2975,-1530,1719,7642 } },
		{ "Canon EOS 400D", 0, 0xe8e,
		{ 7054,-1501,-990,-8156,15544,2812,-1278,1414,7796 } },
		{ "Canon EOS 450D", 0, 0x390d,
		{ 5784,-262,-821,-7539,15064,2672,-1982,2681,7427 } },
		{ "Canon EOS 500D", 0, 0x3479,
		{ 4763,712,-646,-6821,14399,2640,-1921,3276,6561 } },
		{ "Canon EOS 550D", 0, 0x3dd7,
		{ 6941,-1164,-857,-3825,11597,2534,-416,1540,6039 } },
		{ "Canon EOS 600D", 0, 0x3510,
		{ 6461,-907,-882,-4300,12184,2378,-819,1944,5931 } },
		{ "Canon EOS 650D", 0, 0x354d,
		{ 6602,-841,-939,-4472,12458,2247,-975,2039,6148 } },
		{ "Canon EOS 700D", 0, 0x3c00,
		{ 6602,-841,-939,-4472,12458,2247,-975,2039,6148 } },
		{ "Canon EOS 750D", 0, 0x368e,
		{ 6362,-823,-847,-4426,12109,2616,-743,1857,5635 } },
		{ "Canon EOS 760D", 0, 0x350f,
		{ 6362,-823,-847,-4426,12109,2616,-743,1857,5635 } },
		{ "Canon EOS 1000D", 0, 0xe43,
		{ 6771,-1139,-977,-7818,15123,2928,-1244,1437,7533 } },
		{ "Canon EOS 1100D", 0, 0x3510,
		{ 6444,-904,-893,-4563,12308,2535,-903,2016,6728 } },
		{ "Canon EOS 1200D", 0, 0x37c2,
		{ 6461,-907,-882,-4300,12184,2378,-819,1944,5931 } },
		{ "Canon EOS 1300D", 0, 0x3510,
		{ 6939,-1016,-866,-4428,12473,2177,-1175,2178,6162 } },
		{ "Canon EOS M3", 0, 0,
		{ 6362,-823,-847,-4426,12109,2616,-743,1857,5635 } },
		{ "Canon EOS M10", 0, 0,
		{ 6400,-480,-888,-5294,13416,2047,-1296,2203,6137 } },
		{ "Canon EOS M", 0, 0,
		{ 6602,-841,-939,-4472,12458,2247,-975,2039,6148 } },
		{ "Canon EOS-1Ds Mark III", 0, 0x3bb0,
		{ 5859,-211,-930,-8255,16017,2353,-1732,1887,7448 } },
		{ "Canon EOS-1Ds Mark II", 0, 0xe80,
		{ 6517,-602,-867,-8180,15926,2378,-1618,1771,7633 } },
		{ "Canon EOS-1D Mark IV", 0, 0x3bb0,
		{ 6014,-220,-795,-4109,12014,2361,-561,1824,5787 } },
		{ "Canon EOS-1D Mark III", 0, 0x3bb0,
		{ 6291,-540,-976,-8350,16145,2311,-1714,1858,7326 } },
		{ "Canon EOS-1D Mark II N", 0, 0xe80,
		{ 6240,-466,-822,-8180,15825,2500,-1801,1938,8042 } },
		{ "Canon EOS-1D Mark II", 0, 0xe80,
		{ 6264,-582,-724,-8312,15948,2504,-1744,1919,8664 } },
		{ "Canon EOS-1DS", 0, 0xe20,
		{ 4374,3631,-1743,-7520,15212,2472,-2892,3632,8161 } },
		{ "Canon EOS-1D C", 0, 0x3c4e,
		{ 6847,-614,-1014,-4669,12737,2139,-1197,2488,6846 } },
		{ "Canon EOS-1D X Mark II", 0, 0,
		{ 7596,-978,-967,-4808,12571,2503,-1398,2567,5752 } },
		{ "Canon EOS-1D X", 0, 0x3c4e,
		{ 6847,-614,-1014,-4669,12737,2139,-1197,2488,6846 } },
		{ "Canon EOS-1D", 0, 0xe20,
		{ 6806,-179,-1020,-8097,16415,1687,-3267,4236,7690 } },
		{ "Canon EOS C500", 853, 0,		/* DJC */
		{ 17851,-10604,922,-7425,16662,763,-3660,3636,22278 } },
		{ "Canon PowerShot A530", 0, 0,
		{ 0 } },	/* don't want the A5 matrix */
		{ "Canon PowerShot A50", 0, 0,
		{ -5300,9846,1776,3436,684,3939,-5540,9879,6200,-1404,11175,217 } },
		{ "Canon PowerShot A5", 0, 0,
		{ -4801,9475,1952,2926,1611,4094,-5259,10164,5947,-1554,10883,547 } },
		{ "Canon PowerShot G10", 0, 0,
		{ 11093,-3906,-1028,-5047,12492,2879,-1003,1750,5561 } },
		{ "Canon PowerShot G11", 0, 0,
		{ 12177,-4817,-1069,-1612,9864,2049,-98,850,4471 } },
		{ "Canon PowerShot G12", 0, 0,
		{ 13244,-5501,-1248,-1508,9858,1935,-270,1083,4366 } },
		{ "Canon PowerShot G15", 0, 0,
		{ 7474,-2301,-567,-4056,11456,2975,-222,716,4181 } },
		{ "Canon PowerShot G16", 0, 0,
		{ 8020,-2687,-682,-3704,11879,2052,-965,1921,5556 } },
		{ "Canon PowerShot G1 X", 0, 0,
		{ 7378,-1255,-1043,-4088,12251,2048,-876,1946,5805 } },
		{ "Canon PowerShot G1", 0, 0,
		{ -4778,9467,2172,4743,-1141,4344,-5146,9908,6077,-1566,11051,557 } },
		{ "Canon PowerShot G2", 0, 0,
		{ 9087,-2693,-1049,-6715,14382,2537,-2291,2819,7790 } },
		{ "Canon PowerShot G3 X", 0, 0,
		{ 9701,-3857,-921,-3149,11537,1817,-786,1817,5147 } },
		{ "Canon PowerShot G3", 0, 0,
		{ 9212,-2781,-1073,-6573,14189,2605,-2300,2844,7664 } },
		{ "Canon PowerShot G5 X", 0, 0,
		{ 9602,-3823,-937,-2984,11495,1675,-407,1415,5049 } },
		{ "Canon PowerShot G5", 0, 0,
		{ 9757,-2872,-933,-5972,13861,2301,-1622,2328,7212 } },
		{ "Canon PowerShot G6", 0, 0,
		{ 9877,-3775,-871,-7613,14807,3072,-1448,1305,7485 } },
		{ "Canon PowerShot G7 X", 0, 0,
		{ 9602,-3823,-937,-2984,11495,1675,-407,1415,5049 } },
		{ "Canon PowerShot G9 X", 0, 0,
		{ 9602,-3823,-937,-2984,11495,1675,-407,1415,5049 } },
		{ "Canon PowerShot G9", 0, 0,
		{ 7368,-2141,-598,-5621,13254,2625,-1418,1696,5743 } },
		{ "Canon PowerShot Pro1", 0, 0,
		{ 10062,-3522,-999,-7643,15117,2730,-765,817,7323 } },
		{ "Canon PowerShot Pro70", 34, 0,
		{ -4155,9818,1529,3939,-25,4522,-5521,9870,6610,-2238,10873,1342 } },
		{ "Canon PowerShot Pro90", 0, 0,
		{ -4963,9896,2235,4642,-987,4294,-5162,10011,5859,-1770,11230,577 } },
		{ "Canon PowerShot S30", 0, 0,
		{ 10566,-3652,-1129,-6552,14662,2006,-2197,2581,7670 } },
		{ "Canon PowerShot S40", 0, 0,
		{ 8510,-2487,-940,-6869,14231,2900,-2318,2829,9013 } },
		{ "Canon PowerShot S45", 0, 0,
		{ 8163,-2333,-955,-6682,14174,2751,-2077,2597,8041 } },
		{ "Canon PowerShot S50", 0, 0,
		{ 8882,-2571,-863,-6348,14234,2288,-1516,2172,6569 } },
		{ "Canon PowerShot S60", 0, 0,
		{ 8795,-2482,-797,-7804,15403,2573,-1422,1996,7082 } },
		{ "Canon PowerShot S70", 0, 0,
		{ 9976,-3810,-832,-7115,14463,2906,-901,989,7889 } },
		{ "Canon PowerShot S90", 0, 0,
		{ 12374,-5016,-1049,-1677,9902,2078,-83,852,4683 } },
		{ "Canon PowerShot S95", 0, 0,
		{ 13440,-5896,-1279,-1236,9598,1931,-180,1001,4651 } },
		{ "Canon PowerShot S100", 0, 0,
		{ 7968,-2565,-636,-2873,10697,2513,180,667,4211 } },
		{ "Canon PowerShot S110", 0, 0,
		{ 8039,-2643,-654,-3783,11230,2930,-206,690,4194 } },
		{ "Canon PowerShot S120", 0, 0,
		{ 6961,-1685,-695,-4625,12945,1836,-1114,2152,5518 } },
		{ "Canon PowerShot SX1 IS", 0, 0,
		{ 6578,-259,-502,-5974,13030,3309,-308,1058,4970 } },
		{ "Canon PowerShot SX50 HS", 0, 0,
		{ 12432,-4753,-1247,-2110,10691,1629,-412,1623,4926 } },
		{ "Canon PowerShot SX60 HS", 0, 0,
		{ 13161,-5451,-1344,-1989,10654,1531,-47,1271,4955 } },
		{ "Canon PowerShot A3300", 0, 0,	/* DJC */
		{ 10826,-3654,-1023,-3215,11310,1906,0,999,4960 } },
		{ "Canon PowerShot A470", 0, 0,	/* DJC */
		{ 12513,-4407,-1242,-2680,10276,2405,-878,2215,4734 } },
		{ "Canon PowerShot A610", 0, 0,	/* DJC */
		{ 15591,-6402,-1592,-5365,13198,2168,-1300,1824,5075 } },
		{ "Canon PowerShot A620", 0, 0,	/* DJC */
		{ 15265,-6193,-1558,-4125,12116,2010,-888,1639,5220 } },
		{ "Canon PowerShot A630", 0, 0,	/* DJC */
		{ 14201,-5308,-1757,-6087,14472,1617,-2191,3105,5348 } },
		{ "Canon PowerShot A640", 0, 0,	/* DJC */
		{ 13124,-5329,-1390,-3602,11658,1944,-1612,2863,4885 } },
		{ "Canon PowerShot A650", 0, 0,	/* DJC */
		{ 9427,-3036,-959,-2581,10671,1911,-1039,1982,4430 } },
		{ "Canon PowerShot A720", 0, 0,	/* DJC */
		{ 14573,-5482,-1546,-1266,9799,1468,-1040,1912,3810 } },
		{ "Canon PowerShot S3 IS", 0, 0,	/* DJC */
		{ 14062,-5199,-1446,-4712,12470,2243,-1286,2028,4836 } },
		{ "Canon PowerShot SX110 IS", 0, 0,	/* DJC */
		{ 14134,-5576,-1527,-1991,10719,1273,-1158,1929,3581 } },
		{ "Canon PowerShot SX220", 0, 0,	/* DJC */
		{ 13898,-5076,-1447,-1405,10109,1297,-244,1860,3687 } },
		{ "Canon IXUS 160", 0, 0,		/* DJC */
		{ 11657,-3781,-1136,-3544,11262,2283,-160,1219,4700 } },
		{ "Casio EX-S20", 0, 0,		/* DJC */
		{ 11634,-3924,-1128,-4968,12954,2015,-1588,2648,7206 } },
		{ "Casio EX-Z750", 0, 0,		/* DJC */
		{ 10819,-3873,-1099,-4903,13730,1175,-1755,3751,4632 } },
		{ "Casio EX-Z10", 128, 0xfff,	/* DJC */
		{ 9790,-3338,-603,-2321,10222,2099,-344,1273,4799 } },
		{ "CINE 650", 0, 0,
		{ 3390,480,-500,-800,3610,340,-550,2336,1192 } },
		{ "CINE 660", 0, 0,
		{ 3390,480,-500,-800,3610,340,-550,2336,1192 } },
		{ "CINE", 0, 0,
		{ 20183,-4295,-423,-3940,15330,3985,-280,4870,9800 } },
		{ "Contax N Digital", 0, 0xf1e,
		{ 7777,1285,-1053,-9280,16543,2916,-3677,5679,7060 } },
		{ "DXO ONE", 0, 0,
		{ 6596,-2079,-562,-4782,13016,1933,-970,1581,5181 } },
		{ "Epson R-D1", 0, 0,
		{ 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
		{ "Fujifilm E550", 0, 0,
		{ 11044,-3888,-1120,-7248,15168,2208,-1531,2277,8069 } },
		{ "Fujifilm E900", 0, 0,
		{ 9183,-2526,-1078,-7461,15071,2574,-2022,2440,8639 } },
		{ "Fujifilm F5", 0, 0,
		{ 13690,-5358,-1474,-3369,11600,1998,-132,1554,4395 } },
		{ "Fujifilm F6", 0, 0,
		{ 13690,-5358,-1474,-3369,11600,1998,-132,1554,4395 } },
		{ "Fujifilm F77", 0, 0xfe9,
		{ 13690,-5358,-1474,-3369,11600,1998,-132,1554,4395 } },
		{ "Fujifilm F7", 0, 0,
		{ 10004,-3219,-1201,-7036,15047,2107,-1863,2565,7736 } },
		{ "Fujifilm F8", 0, 0,
		{ 13690,-5358,-1474,-3369,11600,1998,-132,1554,4395 } },
		{ "Fujifilm S100FS", 514, 0,
		{ 11521,-4355,-1065,-6524,13767,3058,-1466,1984,6045 } },
		{ "Fujifilm S1", 0, 0,
		{ 12297,-4882,-1202,-2106,10691,1623,-88,1312,4790 } },
		{ "Fujifilm S20Pro", 0, 0,
		{ 10004,-3219,-1201,-7036,15047,2107,-1863,2565,7736 } },
		{ "Fujifilm S20", 512, 0x3fff,
		{ 11401,-4498,-1312,-5088,12751,2613,-838,1568,5941 } },
		{ "Fujifilm S2Pro", 128, 0,
		{ 12492,-4690,-1402,-7033,15423,1647,-1507,2111,7697 } },
		{ "Fujifilm S3Pro", 0, 0,
		{ 11807,-4612,-1294,-8927,16968,1988,-2120,2741,8006 } },
		{ "Fujifilm S5Pro", 0, 0,
		{ 12300,-5110,-1304,-9117,17143,1998,-1947,2448,8100 } },
		{ "Fujifilm S5000", 0, 0,
		{ 8754,-2732,-1019,-7204,15069,2276,-1702,2334,6982 } },
		{ "Fujifilm S5100", 0, 0,
		{ 11940,-4431,-1255,-6766,14428,2542,-993,1165,7421 } },
		{ "Fujifilm S5500", 0, 0,
		{ 11940,-4431,-1255,-6766,14428,2542,-993,1165,7421 } },
		{ "Fujifilm S5200", 0, 0,
		{ 9636,-2804,-988,-7442,15040,2589,-1803,2311,8621 } },
		{ "Fujifilm S5600", 0, 0,
		{ 9636,-2804,-988,-7442,15040,2589,-1803,2311,8621 } },
		{ "Fujifilm S6", 0, 0,
		{ 12628,-4887,-1401,-6861,14996,1962,-2198,2782,7091 } },
		{ "Fujifilm S7000", 0, 0,
		{ 10190,-3506,-1312,-7153,15051,2238,-2003,2399,7505 } },
		{ "Fujifilm S9000", 0, 0,
		{ 10491,-3423,-1145,-7385,15027,2538,-1809,2275,8692 } },
		{ "Fujifilm S9500", 0, 0,
		{ 10491,-3423,-1145,-7385,15027,2538,-1809,2275,8692 } },
		{ "Fujifilm S9100", 0, 0,
		{ 12343,-4515,-1285,-7165,14899,2435,-1895,2496,8800 } },
		{ "Fujifilm S9600", 0, 0,
		{ 12343,-4515,-1285,-7165,14899,2435,-1895,2496,8800 } },
		{ "Fujifilm SL1000", 0, 0,
		{ 11705,-4262,-1107,-2282,10791,1709,-555,1713,4945 } },
		{ "Fujifilm IS-1", 0, 0,
		{ 21461,-10807,-1441,-2332,10599,1999,289,875,7703 } },
		{ "Fujifilm IS Pro", 0, 0,
		{ 12300,-5110,-1304,-9117,17143,1998,-1947,2448,8100 } },
		{ "Fujifilm HS10 HS11", 0, 0xf68,
		{ 12440,-3954,-1183,-1123,9674,1708,-83,1614,4086 } },
		{ "Fujifilm HS2", 0, 0,
		{ 13690,-5358,-1474,-3369,11600,1998,-132,1554,4395 } },
		{ "Fujifilm HS3", 0, 0,
		{ 13690,-5358,-1474,-3369,11600,1998,-132,1554,4395 } },
		{ "Fujifilm HS50EXR", 0, 0,
		{ 12085,-4727,-953,-3257,11489,2002,-511,2046,4592 } },
		{ "Fujifilm F900EXR", 0, 0,
		{ 12085,-4727,-953,-3257,11489,2002,-511,2046,4592 } },
		{ "Fujifilm X100S", 0, 0,
		{ 10592,-4262,-1008,-3514,11355,2465,-870,2025,6386 } },
		{ "Fujifilm X100T", 0, 0,
		{ 10592,-4262,-1008,-3514,11355,2465,-870,2025,6386 } },
		{ "Fujifilm X100", 0, 0,
		{ 12161,-4457,-1069,-5034,12874,2400,-795,1724,6904 } },
		{ "Fujifilm X10", 0, 0,
		{ 13509,-6199,-1254,-4430,12733,1865,-331,1441,5022 } },
		{ "Fujifilm X20", 0, 0,
		{ 11768,-4971,-1133,-4904,12927,2183,-480,1723,4605 } },
		{ "Fujifilm X30", 0, 0,
		{ 12328,-5256,-1144,-4469,12927,1675,-87,1291,4351 } },
		{ "Fujifilm X70", 0, 0,
		{ 10450,-4329,-878,-3217,11105,2421,-752,1758,6519 } },
		{ "Fujifilm X-Pro1", 0, 0,
		{ 10413,-3996,-993,-3721,11640,2361,-733,1540,6011 } },
		{ "Fujifilm X-Pro2", 0, 0,
		{ 11434,-4948,-1210,-3746,12042,1903,-666,1479,5235 } },
		{ "Fujifilm X-A1", 0, 0,
		{ 11086,-4555,-839,-3512,11310,2517,-815,1341,5940 } },
		{ "Fujifilm X-A2", 0, 0,
		{ 10763,-4560,-917,-3346,11311,2322,-475,1135,5843 } },
		{ "Fujifilm X-E1", 0, 0,
		{ 10413,-3996,-993,-3721,11640,2361,-733,1540,6011 } },
		{ "Fujifilm X-E2S", 0, 0,
		{ 11562,-5118,-961,-3022,11007,2311,-525,1569,6097 } },
		{ "Fujifilm X-E2", 0, 0,
		{ 8458,-2451,-855,-4597,12447,2407,-1475,2482,6526 } },
		{ "Fujifilm X-M1", 0, 0,
		{ 10413,-3996,-993,-3721,11640,2361,-733,1540,6011 } },
		{ "Fujifilm X-S1", 0, 0,
		{ 13509,-6199,-1254,-4430,12733,1865,-331,1441,5022 } },
		{ "Fujifilm X-T1", 0, 0,	/* also X-T10 */
		{ 8458,-2451,-855,-4597,12447,2407,-1475,2482,6526 } },
		{ "Fujifilm XF1", 0, 0,
		{ 13509,-6199,-1254,-4430,12733,1865,-331,1441,5022 } },
		{ "Fujifilm XQ", 0, 0,	/* XQ1 and XQ2 */
		{ 9252,-2704,-1064,-5893,14265,1717,-1101,2341,4349 } },
		{ "Imacon Ixpress", 0, 0,		/* DJC */
		{ 7025,-1415,-704,-5188,13765,1424,-1248,2742,6038 } },
		{ "Kodak NC2000", 0, 0,
		{ 13891,-6055,-803,-465,9919,642,2121,82,1291 } },
		{ "Kodak DCS315C", 8, 0,
		{ 17523,-4827,-2510,756,8546,-137,6113,1649,2250 } },
		{ "Kodak DCS330C", 8, 0,
		{ 20620,-7572,-2801,-103,10073,-396,3551,-233,2220 } },
		{ "Kodak DCS420", 0, 0,
		{ 10868,-1852,-644,-1537,11083,484,2343,628,2216 } },
		{ "Kodak DCS460", 0, 0,
		{ 10592,-2206,-967,-1944,11685,230,2206,670,1273 } },
		{ "Kodak EOSDCS1", 0, 0,
		{ 10592,-2206,-967,-1944,11685,230,2206,670,1273 } },
		{ "Kodak EOSDCS3B", 0, 0,
		{ 9898,-2700,-940,-2478,12219,206,1985,634,1031 } },
		{ "Kodak DCS520C", 178, 0,
		{ 24542,-10860,-3401,-1490,11370,-297,2858,-605,3225 } },
		{ "Kodak DCS560C", 177, 0,
		{ 20482,-7172,-3125,-1033,10410,-285,2542,226,3136 } },
		{ "Kodak DCS620C", 177, 0,
		{ 23617,-10175,-3149,-2054,11749,-272,2586,-489,3453 } },
		{ "Kodak DCS620X", 176, 0,
		{ 13095,-6231,154,12221,-21,-2137,895,4602,2258 } },
		{ "Kodak DCS660C", 173, 0,
		{ 18244,-6351,-2739,-791,11193,-521,3711,-129,2802 } },
		{ "Kodak DCS720X", 0, 0,
		{ 11775,-5884,950,9556,1846,-1286,-1019,6221,2728 } },
		{ "Kodak DCS760C", 0, 0,
		{ 16623,-6309,-1411,-4344,13923,323,2285,274,2926 } },
		{ "Kodak DCS Pro SLR", 0, 0,
		{ 5494,2393,-232,-6427,13850,2846,-1876,3997,5445 } },
		{ "Kodak DCS Pro 14nx", 0, 0,
		{ 5494,2393,-232,-6427,13850,2846,-1876,3997,5445 } },
		{ "Kodak DCS Pro 14", 0, 0,
		{ 7791,3128,-776,-8588,16458,2039,-2455,4006,6198 } },
		{ "Kodak ProBack645", 0, 0,
		{ 16414,-6060,-1470,-3555,13037,473,2545,122,4948 } },
		{ "Kodak ProBack", 0, 0,
		{ 21179,-8316,-2918,-915,11019,-165,3477,-180,4210 } },
		{ "Kodak P712", 0, 0,
		{ 9658,-3314,-823,-5163,12695,2768,-1342,1843,6044 } },
		{ "Kodak P850", 0, 0xf7c,
		{ 10511,-3836,-1102,-6946,14587,2558,-1481,1792,6246 } },
		{ "Kodak P880", 0, 0xfff,
		{ 12805,-4662,-1376,-7480,15267,2360,-1626,2194,7904 } },
		{ "Kodak EasyShare Z980", 0, 0,
		{ 11313,-3559,-1101,-3893,11891,2257,-1214,2398,4908 } },
		{ "Kodak EasyShare Z981", 0, 0,
		{ 12729,-4717,-1188,-1367,9187,2582,274,860,4411 } },
		{ "Kodak EasyShare Z990", 0, 0xfed,
		{ 11749,-4048,-1309,-1867,10572,1489,-138,1449,4522 } },
		{ "Kodak EASYSHARE Z1015", 0, 0xef1,
		{ 11265,-4286,-992,-4694,12343,2647,-1090,1523,5447 } },
		{ "Leaf CMost", 0, 0,
		{ 3952,2189,449,-6701,14585,2275,-4536,7349,6536 } },
		{ "Leaf Valeo 6", 0, 0,
		{ 3952,2189,449,-6701,14585,2275,-4536,7349,6536 } },
		{ "Leaf Aptus 54S", 0, 0,
		{ 8236,1746,-1314,-8251,15953,2428,-3673,5786,5771 } },
		{ "Leaf Aptus 65", 0, 0,
		{ 7914,1414,-1190,-8777,16582,2280,-2811,4605,5562 } },
		{ "Leaf Aptus 75", 0, 0,
		{ 7914,1414,-1190,-8777,16582,2280,-2811,4605,5562 } },
		{ "Leaf", 0, 0,
		{ 8236,1746,-1314,-8251,15953,2428,-3673,5786,5771 } },
		{ "Mamiya ZD", 0, 0,
		{ 7645,2579,-1363,-8689,16717,2015,-3712,5941,5961 } },
		{ "Micron 2010", 110, 0,		/* DJC */
		{ 16695,-3761,-2151,155,9682,163,3433,951,4904 } },
		{ "Minolta DiMAGE 5", 0, 0xf7d,
		{ 8983,-2942,-963,-6556,14476,2237,-2426,2887,8014 } },
		{ "Minolta DiMAGE 7Hi", 0, 0xf7d,
		{ 11368,-3894,-1242,-6521,14358,2339,-2475,3056,7285 } },
		{ "Minolta DiMAGE 7", 0, 0xf7d,
		{ 9144,-2777,-998,-6676,14556,2281,-2470,3019,7744 } },
		{ "Minolta DiMAGE A1", 0, 0xf8b,
		{ 9274,-2547,-1167,-8220,16323,1943,-2273,2720,8340 } },
		{ "Minolta DiMAGE A200", 0, 0,
		{ 8560,-2487,-986,-8112,15535,2771,-1209,1324,7743 } },
		{ "Minolta DiMAGE A2", 0, 0xf8f,
		{ 9097,-2726,-1053,-8073,15506,2762,-966,981,7763 } },
		{ "Minolta DiMAGE Z2", 0, 0,	/* DJC */
		{ 11280,-3564,-1370,-4655,12374,2282,-1423,2168,5396 } },
		{ "Minolta DYNAX 5", 0, 0xffb,
		{ 10284,-3283,-1086,-7957,15762,2316,-829,882,6644 } },
		{ "Minolta DYNAX 7", 0, 0xffb,
		{ 10239,-3104,-1099,-8037,15727,2451,-927,925,6871 } },
		{ "Motorola PIXL", 0, 0,		/* DJC */
		{ 8898,-989,-1033,-3292,11619,1674,-661,3178,5216 } },
		{ "Nikon D100", 0, 0,
		{ 5902,-933,-782,-8983,16719,2354,-1402,1455,6464 } },
		{ "Nikon D1H", 0, 0,
		{ 7577,-2166,-926,-7454,15592,1934,-2377,2808,8606 } },
		{ "Nikon D1X", 0, 0,
		{ 7702,-2245,-975,-9114,17242,1875,-2679,3055,8521 } },
		{ "Nikon D1", 0, 0, /* multiplied by 2.218750, 1.0, 1.148438 */
		{ 16772,-4726,-2141,-7611,15713,1972,-2846,3494,9521 } },
		{ "Nikon D200", 0, 0xfbc,
		{ 8367,-2248,-763,-8758,16447,2422,-1527,1550,8053 } },
		{ "Nikon D2H", 0, 0,
		{ 5710,-901,-615,-8594,16617,2024,-2975,4120,6830 } },
		{ "Nikon D2X", 0, 0,
		{ 10231,-2769,-1255,-8301,15900,2552,-797,680,7148 } },
		{ "Nikon D3000", 0, 0,
		{ 8736,-2458,-935,-9075,16894,2251,-1354,1242,8263 } },
		{ "Nikon D3100", 0, 0,
		{ 7911,-2167,-813,-5327,13150,2408,-1288,2483,7968 } },
		{ "Nikon D3200", 0, 0xfb9,
		{ 7013,-1408,-635,-5268,12902,2640,-1470,2801,7379 } },
		{ "Nikon D3300", 0, 0,
		{ 6988,-1384,-714,-5631,13410,2447,-1485,2204,7318 } },
		{ "Nikon D300", 0, 0,
		{ 9030,-1992,-715,-8465,16302,2255,-2689,3217,8069 } },
		{ "Nikon D3X", 0, 0,
		{ 7171,-1986,-648,-8085,15555,2718,-2170,2512,7457 } },
		{ "Nikon D3S", 0, 0,
		{ 8828,-2406,-694,-4874,12603,2541,-660,1509,7587 } },
		{ "Nikon D3", 0, 0,
		{ 8139,-2171,-663,-8747,16541,2295,-1925,2008,8093 } },
		{ "Nikon D40X", 0, 0,
		{ 8819,-2543,-911,-9025,16928,2151,-1329,1213,8449 } },
		{ "Nikon D40", 0, 0,
		{ 6992,-1668,-806,-8138,15748,2543,-874,850,7897 } },
		{ "Nikon D4S", 0, 0,
		{ 8598,-2848,-857,-5618,13606,2195,-1002,1773,7137 } },
		{ "Nikon D4", 0, 0,
		{ 8598,-2848,-857,-5618,13606,2195,-1002,1773,7137 } },
		{ "Nikon Df", 0, 0,
		{ 8598,-2848,-857,-5618,13606,2195,-1002,1773,7137 } },
		{ "Nikon D5000", 0, 0xf00,
		{ 7309,-1403,-519,-8474,16008,2622,-2433,2826,8064 } },
		{ "Nikon D5100", 0, 0x3de6,
		{ 8198,-2239,-724,-4871,12389,2798,-1043,2050,7181 } },
		{ "Nikon D5200", 0, 0,
		{ 8322,-3112,-1047,-6367,14342,2179,-988,1638,6394 } },
		{ "Nikon D5300", 0, 0,
		{ 6988,-1384,-714,-5631,13410,2447,-1485,2204,7318 } },
		{ "Nikon D5500", 0, 0,
		{ 8821,-2938,-785,-4178,12142,2287,-824,1651,6860 } },
		{ "Nikon D500", 0, 0,
		{ 8813,-3210,-1036,-4703,12868,2021,-1054,1940,6129 } },
		{ "Nikon D50", 0, 0,
		{ 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
		{ "Nikon D5", 0, 0,
		{ 9200,-3522,-992,-5755,13803,2117,-753,1486,6338 } },
		{ "Nikon D600", 0, 0x3e07,
		{ 8178,-2245,-609,-4857,12394,2776,-1207,2086,7298 } },
		{ "Nikon D610", 0, 0,
		{ 8178,-2245,-609,-4857,12394,2776,-1207,2086,7298 } },
		{ "Nikon D60", 0, 0,
		{ 8736,-2458,-935,-9075,16894,2251,-1354,1242,8263 } },
		{ "Nikon D7000", 0, 0,
		{ 8198,-2239,-724,-4871,12389,2798,-1043,2050,7181 } },
		{ "Nikon D7100", 0, 0,
		{ 8322,-3112,-1047,-6367,14342,2179,-988,1638,6394 } },
		{ "Nikon D7200", 0, 0,
		{ 8322,-3112,-1047,-6367,14342,2179,-988,1638,6394 } },
		{ "Nikon D750", 0, 0,
		{ 9020,-2890,-715,-4535,12436,2348,-934,1919,7086 } },
		{ "Nikon D700", 0, 0,
		{ 8139,-2171,-663,-8747,16541,2295,-1925,2008,8093 } },
		{ "Nikon D70", 0, 0,
		{ 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
		{ "Nikon D810", 0, 0,
		{ 9369,-3195,-791,-4488,12430,2301,-893,1796,6872 } },
		{ "Nikon D800", 0, 0,
		{ 7866,-2108,-555,-4869,12483,2681,-1176,2069,7501 } },
		{ "Nikon D80", 0, 0,
		{ 8629,-2410,-883,-9055,16940,2171,-1490,1363,8520 } },
		{ "Nikon D90", 0, 0xf00,
		{ 7309,-1403,-519,-8474,16008,2622,-2434,2826,8064 } },
		{ "Nikon E700", 0, 0x3dd,		/* DJC */
		{ -3746,10611,1665,9621,-1734,2114,-2389,7082,3064,3406,6116,-244 } },
		{ "Nikon E800", 0, 0x3dd,		/* DJC */
		{ -3746,10611,1665,9621,-1734,2114,-2389,7082,3064,3406,6116,-244 } },
		{ "Nikon E950", 0, 0x3dd,		/* DJC */
		{ -3746,10611,1665,9621,-1734,2114,-2389,7082,3064,3406,6116,-244 } },
		{ "Nikon E995", 0, 0,	/* copied from E5000 */
		{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "Nikon E2100", 0, 0,	/* copied from Z2, new _white balance */
		{ 13142,-4152,-1596,-4655,12374,2282,-1769,2696,6711 } },
		{ "Nikon E2500", 0, 0,
		{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "Nikon E3200", 0, 0,		/* DJC */
		{ 9846,-2085,-1019,-3278,11109,2170,-774,2134,5745 } },
		{ "Nikon E4300", 0, 0,	/* copied from Minolta DiMAGE Z2 */
		{ 11280,-3564,-1370,-4655,12374,2282,-1423,2168,5396 } },
		{ "Nikon E4500", 0, 0,
		{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "Nikon E5000", 0, 0,
		{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "Nikon E5400", 0, 0,
		{ 9349,-2987,-1001,-7919,15766,2266,-2098,2680,6839 } },
		{ "Nikon E5700", 0, 0,
		{ -5368,11478,2368,5537,-113,3148,-4969,10021,5782,778,9028,211 } },
		{ "Nikon E8400", 0, 0,
		{ 7842,-2320,-992,-8154,15718,2599,-1098,1342,7560 } },
		{ "Nikon E8700", 0, 0,
		{ 8489,-2583,-1036,-8051,15583,2643,-1307,1407,7354 } },
		{ "Nikon E8800", 0, 0,
		{ 7971,-2314,-913,-8451,15762,2894,-1442,1520,7610 } },
		{ "Nikon COOLPIX A", 0, 0,
		{ 8198,-2239,-724,-4871,12389,2798,-1043,2050,7181 } },
		{ "Nikon COOLPIX P330", 200, 0,
		{ 10321,-3920,-931,-2750,11146,1824,-442,1545,5539 } },
		{ "Nikon COOLPIX P340", 200, 0,
		{ 10321,-3920,-931,-2750,11146,1824,-442,1545,5539 } },
		{ "Nikon COOLPIX P6000", 0, 0,
		{ 9698,-3367,-914,-4706,12584,2368,-837,968,5801 } },
		{ "Nikon COOLPIX P7000", 0, 0,
		{ 11432,-3679,-1111,-3169,11239,2202,-791,1380,4455 } },
		{ "Nikon COOLPIX P7100", 0, 0,
		{ 11053,-4269,-1024,-1976,10182,2088,-526,1263,4469 } },
		{ "Nikon COOLPIX P7700", 200, 0,
		{ 10321,-3920,-931,-2750,11146,1824,-442,1545,5539 } },
		{ "Nikon COOLPIX P7800", 200, 0,
		{ 10321,-3920,-931,-2750,11146,1824,-442,1545,5539 } },
		{ "Nikon 1 V3", 0, 0,
		{ 5958,-1559,-571,-4021,11453,2939,-634,1548,5087 } },
		{ "Nikon 1 J4", 0, 0,
		{ 5958,-1559,-571,-4021,11453,2939,-634,1548,5087 } },
		{ "Nikon 1 J5", 0, 0,
		{ 7520,-2518,-645,-3844,12102,1945,-913,2249,6835 } },
		{ "Nikon 1 S2", 200, 0,
		{ 6612,-1342,-618,-3338,11055,2623,-174,1792,5075 } },
		{ "Nikon 1 V2", 0, 0,
		{ 6588,-1305,-693,-3277,10987,2634,-355,2016,5106 } },
		{ "Nikon 1 J3", 0, 0,
		{ 6588,-1305,-693,-3277,10987,2634,-355,2016,5106 } },
		{ "Nikon 1 AW1", 0, 0,
		{ 6588,-1305,-693,-3277,10987,2634,-355,2016,5106 } },
		{ "Nikon 1 ", 0, 0,		/* J1, J2, S1, V1 */
		{ 8994,-2667,-865,-4594,12324,2552,-699,1786,6260 } },
		{ "Olympus AIR A01", 0, 0,
		{ 8992,-3093,-639,-2563,10721,2122,-437,1270,5473 } },
		{ "Olympus C5050", 0, 0,
		{ 10508,-3124,-1273,-6079,14294,1901,-1653,2306,6237 } },
		{ "Olympus C5060", 0, 0,
		{ 10445,-3362,-1307,-7662,15690,2058,-1135,1176,7602 } },
		{ "Olympus C7070", 0, 0,
		{ 10252,-3531,-1095,-7114,14850,2436,-1451,1723,6365 } },
		{ "Olympus C70", 0, 0,
		{ 10793,-3791,-1146,-7498,15177,2488,-1390,1577,7321 } },
		{ "Olympus C80", 0, 0,
		{ 8606,-2509,-1014,-8238,15714,2703,-942,979,7760 } },
		{ "Olympus E-10", 0, 0xffc,
		{ 12745,-4500,-1416,-6062,14542,1580,-1934,2256,6603 } },
		{ "Olympus E-1", 0, 0,
		{ 11846,-4767,-945,-7027,15878,1089,-2699,4122,8311 } },
		{ "Olympus E-20", 0, 0xffc,
		{ 13173,-4732,-1499,-5807,14036,1895,-2045,2452,7142 } },
		{ "Olympus E-300", 0, 0,
		{ 7828,-1761,-348,-5788,14071,1830,-2853,4518,6557 } },
		{ "Olympus E-330", 0, 0,
		{ 8961,-2473,-1084,-7979,15990,2067,-2319,3035,8249 } },
		{ "Olympus E-30", 0, 0xfbc,
		{ 8144,-1861,-1111,-7763,15894,1929,-1865,2542,7607 } },
		{ "Olympus E-3", 0, 0xf99,
		{ 9487,-2875,-1115,-7533,15606,2010,-1618,2100,7389 } },
		{ "Olympus E-400", 0, 0,
		{ 6169,-1483,-21,-7107,14761,2536,-2904,3580,8568 } },
		{ "Olympus E-410", 0, 0xf6a,
		{ 8856,-2582,-1026,-7761,15766,2082,-2009,2575,7469 } },
		{ "Olympus E-420", 0, 0xfd7,
		{ 8746,-2425,-1095,-7594,15612,2073,-1780,2309,7416 } },
		{ "Olympus E-450", 0, 0xfd2,
		{ 8745,-2425,-1095,-7594,15613,2073,-1780,2309,7416 } },
		{ "Olympus E-500", 0, 0,
		{ 8136,-1968,-299,-5481,13742,1871,-2556,4205,6630 } },
		{ "Olympus E-510", 0, 0xf6a,
		{ 8785,-2529,-1033,-7639,15624,2112,-1783,2300,7817 } },
		{ "Olympus E-520", 0, 0xfd2,
		{ 8344,-2322,-1020,-7596,15635,2048,-1748,2269,7287 } },
		{ "Olympus E-5", 0, 0xeec,
		{ 11200,-3783,-1325,-4576,12593,2206,-695,1742,7504 } },
		{ "Olympus E-600", 0, 0xfaf,
		{ 8453,-2198,-1092,-7609,15681,2008,-1725,2337,7824 } },
		{ "Olympus E-620", 0, 0xfaf,
		{ 8453,-2198,-1092,-7609,15681,2008,-1725,2337,7824 } },
		{ "Olympus E-P1", 0, 0xffd,
		{ 8343,-2050,-1021,-7715,15705,2103,-1831,2380,8235 } },
		{ "Olympus E-P2", 0, 0xffd,
		{ 8343,-2050,-1021,-7715,15705,2103,-1831,2380,8235 } },
		{ "Olympus E-P3", 0, 0,
		{ 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
		{ "Olympus E-P5", 0, 0,
		{ 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
		{ "Olympus E-PL1s", 0, 0,
		{ 11409,-3872,-1393,-4572,12757,2003,-709,1810,7415 } },
		{ "Olympus E-PL1", 0, 0,
		{ 11408,-4289,-1215,-4286,12385,2118,-387,1467,7787 } },
		{ "Olympus E-PL2", 0, 0xcf3,
		{ 15030,-5552,-1806,-3987,12387,1767,-592,1670,7023 } },
		{ "Olympus E-PL3", 0, 0,
		{ 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
		{ "Olympus E-PL5", 0, 0xfcb,
		{ 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
		{ "Olympus E-PL6", 0, 0,
		{ 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
		{ "Olympus E-PL7", 0, 0,
		{ 9197,-3190,-659,-2606,10830,2039,-458,1250,5458 } },
		{ "Olympus E-PM1", 0, 0,
		{ 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
		{ "Olympus E-PM2", 0, 0,
		{ 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
		{ "Olympus E-M10", 0, 0,	/* also E-M10 Mark II */
		{ 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
		{ "Olympus E-M1", 0, 0,
		{ 7687,-1984,-606,-4327,11928,2721,-1381,2339,6452 } },
		{ "Olympus E-M5MarkII", 0, 0,
		{ 9422,-3258,-711,-2655,10898,2015,-512,1354,5512 } },
		{ "Olympus E-M5", 0, 0xfe1,
		{ 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
		{ "Olympus PEN-F", 0, 0,
		{ 9476,-3182,-765,-2613,10958,1893,-449,1315,5268 } },
		{ "Olympus SH-2", 0, 0,
		{ 10156,-3425,-1077,-2611,11177,1624,-385,1592,5080 } },
		{ "Olympus SP350", 0, 0,
		{ 12078,-4836,-1069,-6671,14306,2578,-786,939,7418 } },
		{ "Olympus SP3", 0, 0,
		{ 11766,-4445,-1067,-6901,14421,2707,-1029,1217,7572 } },
		{ "Olympus SP500UZ", 0, 0xfff,
		{ 9493,-3415,-666,-5211,12334,3260,-1548,2262,6482 } },
		{ "Olympus SP510UZ", 0, 0xffe,
		{ 10593,-3607,-1010,-5881,13127,3084,-1200,1805,6721 } },
		{ "Olympus SP550UZ", 0, 0xffe,
		{ 11597,-4006,-1049,-5432,12799,2957,-1029,1750,6516 } },
		{ "Olympus SP560UZ", 0, 0xff9,
		{ 10915,-3677,-982,-5587,12986,2911,-1168,1968,6223 } },
		{ "Olympus SP570UZ", 0, 0,
		{ 11522,-4044,-1146,-4736,12172,2904,-988,1829,6039 } },
		{ "Olympus STYLUS1", 0, 0,
		{ 8360,-2420,-880,-3928,12353,1739,-1381,2416,5173 } },
		{ "Olympus TG-4", 0, 0,
		{ 11426,-4159,-1126,-2066,10678,1593,-120,1327,4998 } },
		{ "Olympus XZ-10", 0, 0,
		{ 9777,-3483,-925,-2886,11297,1800,-602,1663,5134 } },
		{ "Olympus XZ-1", 0, 0,
		{ 10901,-4095,-1074,-1141,9208,2293,-62,1417,5158 } },
		{ "Olympus XZ-2", 0, 0,
		{ 9777,-3483,-925,-2886,11297,1800,-602,1663,5134 } },
		{ "OmniVision", 0, 0,		/* DJC */
		{ 12782,-4059,-379,-478,9066,1413,1340,1513,5176 } },
		{ "Pentax *ist DL2", 0, 0,
		{ 10504,-2438,-1189,-8603,16207,2531,-1022,863,12242 } },
		{ "Pentax *ist DL", 0, 0,
		{ 10829,-2838,-1115,-8339,15817,2696,-837,680,11939 } },
		{ "Pentax *ist DS2", 0, 0,
		{ 10504,-2438,-1189,-8603,16207,2531,-1022,863,12242 } },
		{ "Pentax *ist DS", 0, 0,
		{ 10371,-2333,-1206,-8688,16231,2602,-1230,1116,11282 } },
		{ "Pentax *ist D", 0, 0,
		{ 9651,-2059,-1189,-8881,16512,2487,-1460,1345,10687 } },
		{ "Pentax K10D", 0, 0,
		{ 9566,-2863,-803,-7170,15172,2112,-818,803,9705 } },
		{ "Pentax K1", 0, 0,
		{ 11095,-3157,-1324,-8377,15834,2720,-1108,947,11688 } },
		{ "Pentax K20D", 0, 0,
		{ 9427,-2714,-868,-7493,16092,1373,-2199,3264,7180 } },
		{ "Pentax K200D", 0, 0,
		{ 9186,-2678,-907,-8693,16517,2260,-1129,1094,8524 } },
		{ "Pentax K2000", 0, 0,
		{ 11057,-3604,-1155,-5152,13046,2329,-282,375,8104 } },
		{ "Pentax K-m", 0, 0,
		{ 11057,-3604,-1155,-5152,13046,2329,-282,375,8104 } },
		{ "Pentax K-x", 0, 0,
		{ 8843,-2837,-625,-5025,12644,2668,-411,1234,7410 } },
		{ "Pentax K-r", 0, 0,
		{ 9895,-3077,-850,-5304,13035,2521,-883,1768,6936 } },
		{ "Pentax K-1", 0, 0,
		{ 8566,-2746,-1201,-3612,12204,1550,-893,1680,6264 } },
		{ "Pentax K-30", 0, 0,
		{ 8710,-2632,-1167,-3995,12301,1881,-981,1719,6535 } },
		{ "Pentax K-3 II", 0, 0,
		{ 8626,-2607,-1155,-3995,12301,1881,-1039,1822,6925 } },
		{ "Pentax K-3", 0, 0,
		{ 7415,-2052,-721,-5186,12788,2682,-1446,2157,6773 } },
		{ "Pentax K-5 II", 0, 0,
		{ 8170,-2725,-639,-4440,12017,2744,-771,1465,6599 } },
		{ "Pentax K-5", 0, 0,
		{ 8713,-2833,-743,-4342,11900,2772,-722,1543,6247 } },
		{ "Pentax K-7", 0, 0,
		{ 9142,-2947,-678,-8648,16967,1663,-2224,2898,8615 } },
		{ "Pentax K-S1", 0, 0,
		{ 8512,-3211,-787,-4167,11966,2487,-638,1288,6054 } },
		{ "Pentax K-S2", 0, 0,
		{ 8662,-3280,-798,-3928,11771,2444,-586,1232,6054 } },
		{ "Pentax Q-S1", 0, 0,
		{ 12995,-5593,-1107,-1879,10139,2027,-64,1233,4919 } },
		{ "Pentax 645D", 0, 0x3e00,
		{ 10646,-3593,-1158,-3329,11699,1831,-667,2874,6287 } },
		{ "Panasonic DMC-CM1", 15, 0,
		{ 8770,-3194,-820,-2871,11281,1803,-513,1552,4434 } },
		{ "Panasonic DMC-FZ8", 0, 0xf7f,
		{ 8986,-2755,-802,-6341,13575,3077,-1476,2144,6379 } },
		{ "Panasonic DMC-FZ18", 0, 0,
		{ 9932,-3060,-935,-5809,13331,2753,-1267,2155,5575 } },
		{ "Panasonic DMC-FZ28", 15, 0xf96,
		{ 10109,-3488,-993,-5412,12812,2916,-1305,2140,5543 } },
		{ "Panasonic DMC-FZ330", 15, 0,
		{ 8378,-2798,-769,-3068,11410,1877,-538,1792,4623 } },
		{ "Panasonic DMC-FZ300", 15, 0,
		{ 8378,-2798,-769,-3068,11410,1877,-538,1792,4623 } },
		{ "Panasonic DMC-FZ30", 0, 0xf94,
		{ 10976,-4029,-1141,-7918,15491,2600,-1670,2071,8246 } },
		{ "Panasonic DMC-FZ3", 15, 0,
		{ 9938,-2780,-890,-4604,12393,2480,-1117,2304,4620 } },
		{ "Panasonic DMC-FZ4", 15, 0,
		{ 13639,-5535,-1371,-1698,9633,2430,316,1152,4108 } },
		{ "Panasonic DMC-FZ50", 0, 0,
		{ 7906,-2709,-594,-6231,13351,3220,-1922,2631,6537 } },
		{ "Panasonic DMC-FZ7", 15, 0,
		{ 11532,-4324,-1066,-2375,10847,1749,-564,1699,4351 } },
		{ "Leica V-LUX1", 0, 0,
		{ 7906,-2709,-594,-6231,13351,3220,-1922,2631,6537 } },
		{ "Panasonic DMC-L10", 15, 0xf96,
		{ 8025,-1942,-1050,-7920,15904,2100,-2456,3005,7039 } },
		{ "Panasonic DMC-L1", 0, 0xf7f,
		{ 8054,-1885,-1025,-8349,16367,2040,-2805,3542,7629 } },
		{ "Leica DIGILUX 3", 0, 0xf7f,
		{ 8054,-1885,-1025,-8349,16367,2040,-2805,3542,7629 } },
		{ "Panasonic DMC-LC1", 0, 0,
		{ 11340,-4069,-1275,-7555,15266,2448,-2960,3426,7685 } },
		{ "Leica DIGILUX 2", 0, 0,
		{ 11340,-4069,-1275,-7555,15266,2448,-2960,3426,7685 } },
		{ "Panasonic DMC-LX100", 15, 0,
		{ 8844,-3538,-768,-3709,11762,2200,-698,1792,5220 } },
		{ "Leica D-LUX (Typ 109)", 15, 0,
		{ 8844,-3538,-768,-3709,11762,2200,-698,1792,5220 } },
		{ "Panasonic DMC-LF1", 15, 0,
		{ 9379,-3267,-816,-3227,11560,1881,-926,1928,5340 } },
		{ "Leica C (Typ 112)", 15, 0,
		{ 9379,-3267,-816,-3227,11560,1881,-926,1928,5340 } },
		{ "Panasonic DMC-LX1", 0, 0xf7f,
		{ 10704,-4187,-1230,-8314,15952,2501,-920,945,8927 } },
		{ "Leica D-LUX2", 0, 0xf7f,
		{ 10704,-4187,-1230,-8314,15952,2501,-920,945,8927 } },
		{ "Panasonic DMC-LX2", 0, 0,
		{ 8048,-2810,-623,-6450,13519,3272,-1700,2146,7049 } },
		{ "Leica D-LUX3", 0, 0,
		{ 8048,-2810,-623,-6450,13519,3272,-1700,2146,7049 } },
		{ "Panasonic DMC-LX3", 15, 0,
		{ 8128,-2668,-655,-6134,13307,3161,-1782,2568,6083 } },
		{ "Leica D-LUX 4", 15, 0,
		{ 8128,-2668,-655,-6134,13307,3161,-1782,2568,6083 } },
		{ "Panasonic DMC-LX5", 15, 0,
		{ 10909,-4295,-948,-1333,9306,2399,22,1738,4582 } },
		{ "Leica D-LUX 5", 15, 0,
		{ 10909,-4295,-948,-1333,9306,2399,22,1738,4582 } },
		{ "Panasonic DMC-LX7", 15, 0,
		{ 10148,-3743,-991,-2837,11366,1659,-701,1893,4899 } },
		{ "Leica D-LUX 6", 15, 0,
		{ 10148,-3743,-991,-2837,11366,1659,-701,1893,4899 } },
		{ "Panasonic DMC-FZ1000", 15, 0,
		{ 7830,-2696,-763,-3325,11667,1866,-641,1712,4824 } },
		{ "Leica V-LUX (Typ 114)", 15, 0,
		{ 7830,-2696,-763,-3325,11667,1866,-641,1712,4824 } },
		{ "Panasonic DMC-FZ100", 15, 0xfff,
		{ 16197,-6146,-1761,-2393,10765,1869,366,2238,5248 } },
		{ "Leica V-LUX 2", 15, 0xfff,
		{ 16197,-6146,-1761,-2393,10765,1869,366,2238,5248 } },
		{ "Panasonic DMC-FZ150", 15, 0xfff,
		{ 11904,-4541,-1189,-2355,10899,1662,-296,1586,4289 } },
		{ "Leica V-LUX 3", 15, 0xfff,
		{ 11904,-4541,-1189,-2355,10899,1662,-296,1586,4289 } },
		{ "Panasonic DMC-FZ200", 15, 0xfff,
		{ 8112,-2563,-740,-3730,11784,2197,-941,2075,4933 } },
		{ "Leica V-LUX 4", 15, 0xfff,
		{ 8112,-2563,-740,-3730,11784,2197,-941,2075,4933 } },
		{ "Panasonic DMC-FX150", 15, 0xfff,
		{ 9082,-2907,-925,-6119,13377,3058,-1797,2641,5609 } },
		{ "Panasonic DMC-G10", 0, 0,
		{ 10113,-3400,-1114,-4765,12683,2317,-377,1437,6710 } },
		{ "Panasonic DMC-G1", 15, 0xf94,
		{ 8199,-2065,-1056,-8124,16156,2033,-2458,3022,7220 } },
		{ "Panasonic DMC-G2", 15, 0xf3c,
		{ 10113,-3400,-1114,-4765,12683,2317,-377,1437,6710 } },
		{ "Panasonic DMC-G3", 15, 0xfff,
		{ 6763,-1919,-863,-3868,11515,2684,-1216,2387,5879 } },
		{ "Panasonic DMC-G5", 15, 0xfff,
		{ 7798,-2562,-740,-3879,11584,2613,-1055,2248,5434 } },
		{ "Panasonic DMC-G6", 15, 0xfff,
		{ 8294,-2891,-651,-3869,11590,2595,-1183,2267,5352 } },
		{ "Panasonic DMC-G7", 15, 0xfff,
		{ 7610,-2780,-576,-4614,12195,2733,-1375,2393,6490 } },
		{ "Panasonic DMC-GF1", 15, 0xf92,
		{ 7888,-1902,-1011,-8106,16085,2099,-2353,2866,7330 } },
		{ "Panasonic DMC-GF2", 15, 0xfff,
		{ 7888,-1902,-1011,-8106,16085,2099,-2353,2866,7330 } },
		{ "Panasonic DMC-GF3", 15, 0xfff,
		{ 9051,-2468,-1204,-5212,13276,2121,-1197,2510,6890 } },
		{ "Panasonic DMC-GF5", 15, 0xfff,
		{ 8228,-2945,-660,-3938,11792,2430,-1094,2278,5793 } },
		{ "Panasonic DMC-GF6", 15, 0,
		{ 8130,-2801,-946,-3520,11289,2552,-1314,2511,5791 } },
		{ "Panasonic DMC-GF7", 15, 0,
		{ 7610,-2780,-576,-4614,12195,2733,-1375,2393,6490 } },
		{ "Panasonic DMC-GF8", 15, 0,
		{ 7610,-2780,-576,-4614,12195,2733,-1375,2393,6490 } },
		{ "Panasonic DMC-GH1", 15, 0xf92,
		{ 6299,-1466,-532,-6535,13852,2969,-2331,3112,5984 } },
		{ "Panasonic DMC-GH2", 15, 0xf95,
		{ 7780,-2410,-806,-3913,11724,2484,-1018,2390,5298 } },
		{ "Panasonic DMC-GH3", 15, 0,
		{ 6559,-1752,-491,-3672,11407,2586,-962,1875,5130 } },
		{ "Panasonic DMC-GH4", 15, 0,
		{ 7122,-2108,-512,-3155,11201,2231,-541,1423,5045 } },
		{ "Panasonic DMC-GM1", 15, 0,
		{ 6770,-1895,-744,-5232,13145,2303,-1664,2691,5703 } },
		{ "Panasonic DMC-GM5", 15, 0,
		{ 8238,-3244,-679,-3921,11814,2384,-836,2022,5852 } },
		{ "Panasonic DMC-GX1", 15, 0,
		{ 6763,-1919,-863,-3868,11515,2684,-1216,2387,5879 } },
		{ "Panasonic DMC-GX7", 15, 0,
		{ 7610,-2780,-576,-4614,12195,2733,-1375,2393,6490 } },
		{ "Panasonic DMC-GX8", 15, 0,
		{ 7564,-2263,-606,-3148,11239,2177,-540,1435,4853 } },
		{ "Panasonic DMC-TZ1", 15, 0,
		{ 7790,-2736,-755,-3452,11870,1769,-628,1647,4898 } },
		{ "Panasonic DMC-ZS1", 15, 0,
		{ 7790,-2736,-755,-3452,11870,1769,-628,1647,4898 } },
		{ "Panasonic DMC-TZ6", 15, 0,
		{ 8607,-2822,-808,-3755,11930,2049,-820,2060,5224 } },
		{ "Panasonic DMC-ZS4", 15, 0,
		{ 8607,-2822,-808,-3755,11930,2049,-820,2060,5224 } },
		{ "Panasonic DMC-TZ7", 15, 0,
		{ 8802,-3135,-789,-3151,11468,1904,-550,1745,4810 } },
		{ "Panasonic DMC-ZS5", 15, 0,
		{ 8802,-3135,-789,-3151,11468,1904,-550,1745,4810 } },
		{ "Panasonic DMC-TZ8", 15, 0,
		{ 8550,-2908,-842,-3195,11529,1881,-338,1603,4631 } },
		{ "Panasonic DMC-ZS6", 15, 0,
		{ 8550,-2908,-842,-3195,11529,1881,-338,1603,4631 } },
		{ "Leica S (Typ 007)", 0, 0,
		{ 6063,-2234,-231,-5210,13787,1500,-1043,2866,6997 } },
		{ "Leica X", 0, 0,		/* X and X-U, both (Typ 113) */
		{ 7712,-2059,-653,-3882,11494,2726,-710,1332,5958 } },
		{ "Leica Q (Typ 116)", 0, 0,
		{ 11865,-4523,-1441,-5423,14458,935,-1587,2687,4830 } },
		{ "Leica M (Typ 262)", 0, 0,
		{ 6653,-1486,-611,-4221,13303,929,-881,2416,7226 } },
		{ "Leica SL (Typ 601)", 0, 0,
		{ 11865,-4523,-1441,-5423,14458,935,-1587,2687,4830 } },
		{ "Phase One H 20", 0, 0,		/* DJC */
		{ 1313,1855,-109,-6715,15908,808,-327,1840,6020 } },
		{ "Phase One H 25", 0, 0,
		{ 2905,732,-237,-8134,16626,1476,-3038,4253,7517 } },
		{ "Phase One P 2", 0, 0,
		{ 2905,732,-237,-8134,16626,1476,-3038,4253,7517 } },
		{ "Phase One P 30", 0, 0,
		{ 4516,-245,-37,-7020,14976,2173,-3206,4671,7087 } },
		{ "Phase One P 45", 0, 0,
		{ 5053,-24,-117,-5684,14076,1702,-2619,4492,5849 } },
		{ "Phase One P40", 0, 0,
		{ 8035,435,-962,-6001,13872,2320,-1159,3065,5434 } },
		{ "Phase One P65", 0, 0,
		{ 8035,435,-962,-6001,13872,2320,-1159,3065,5434 } },
		{ "Photron BC2-HD", 0, 0,		/* DJC */
		{ 14603,-4122,-528,-1810,9794,2017,-297,2763,5936 } },
		{ "Red One", 704, 0xffff,		/* DJC */
		{ 21014,-7891,-2613,-3056,12201,856,-2203,5125,8042 } },
		{ "Ricoh GR II", 0, 0,
		{ 4630,-834,-423,-4977,12805,2417,-638,1467,6115 } },
		{ "Ricoh GR", 0, 0,
		{ 3708,-543,-160,-5381,12254,3556,-1471,1929,8234 } },
		{ "Samsung EX1", 0, 0x3e00,
		{ 8898,-2498,-994,-3144,11328,2066,-760,1381,4576 } },
		{ "Samsung EX2F", 0, 0x7ff,
		{ 10648,-3897,-1055,-2022,10573,1668,-492,1611,4742 } },
		{ "Samsung EK-GN120", 0, 0,
		{ 7557,-2522,-739,-4679,12949,1894,-840,1777,5311 } },
		{ "Samsung NX mini", 0, 0,
		{ 5222,-1196,-550,-6540,14649,2009,-1666,2819,5657 } },
		{ "Samsung NX3300", 0, 0,
		{ 8060,-2933,-761,-4504,12890,1762,-630,1489,5227 } },
		{ "Samsung NX3000", 0, 0,
		{ 8060,-2933,-761,-4504,12890,1762,-630,1489,5227 } },
		{ "Samsung NX30", 0, 0,	/* NX30, NX300, NX300M */
		{ 7557,-2522,-739,-4679,12949,1894,-840,1777,5311 } },
		{ "Samsung NX2000", 0, 0,
		{ 7557,-2522,-739,-4679,12949,1894,-840,1777,5311 } },
		{ "Samsung NX2", 0, 0xfff,	/* NX20, NX200, NX210 */
		{ 6933,-2268,-753,-4921,13387,1647,-803,1641,6096 } },
		{ "Samsung NX1000", 0, 0,
		{ 6933,-2268,-753,-4921,13387,1647,-803,1641,6096 } },
		{ "Samsung NX1100", 0, 0,
		{ 6933,-2268,-753,-4921,13387,1647,-803,1641,6096 } },
		{ "Samsung NX11", 0, 0,
		{ 10332,-3234,-1168,-6111,14639,1520,-1352,2647,8331 } },
		{ "Samsung NX10", 0, 0,	/* also NX100 */
		{ 10332,-3234,-1168,-6111,14639,1520,-1352,2647,8331 } },
		{ "Samsung NX500", 0, 0,
		{ 10686,-4042,-1052,-3595,13238,276,-464,1259,5931 } },
		{ "Samsung NX5", 0, 0,
		{ 10332,-3234,-1168,-6111,14639,1520,-1352,2647,8331 } },
		{ "Samsung NX1", 0, 0,
		{ 10686,-4042,-1052,-3595,13238,276,-464,1259,5931 } },
		{ "Samsung WB2000", 0, 0xfff,
		{ 12093,-3557,-1155,-1000,9534,1733,-22,1787,4576 } },
		{ "Samsung GX-1", 0, 0,
		{ 10504,-2438,-1189,-8603,16207,2531,-1022,863,12242 } },
		{ "Samsung GX20", 0, 0,	/* copied from Pentax K20D */
		{ 9427,-2714,-868,-7493,16092,1373,-2199,3264,7180 } },
		{ "Samsung S85", 0, 0,		/* DJC */
		{ 11885,-3968,-1473,-4214,12299,1916,-835,1655,5549 } },
		{ "Sinar", 0, 0,			/* DJC */
		{ 16442,-2956,-2422,-2877,12128,750,-1136,6066,4559 } },
		{ "Sony DSC-F828", 0, 0,
		{ 7924,-1910,-777,-8226,15459,2998,-1517,2199,6818,-7242,11401,3481 } },
		{ "Sony DSC-R1", 0, 0,
		{ 8512,-2641,-694,-8042,15670,2526,-1821,2117,7414 } },
		{ "Sony DSC-V3", 0, 0,
		{ 7511,-2571,-692,-7894,15088,3060,-948,1111,8128 } },
		{ "Sony DSC-RX100M", 0, 0,		/* M2, M3, and M4 */
		{ 6596,-2079,-562,-4782,13016,1933,-970,1581,5181 } },
		{ "Sony DSC-RX100", 0, 0,
		{ 8651,-2754,-1057,-3464,12207,1373,-568,1398,4434 } },
		{ "Sony DSC-RX10", 0, 0,		/* also RX10M2 */
		{ 6679,-1825,-745,-5047,13256,1953,-1580,2422,5183 } },
		{ "Sony DSC-RX1RM2", 0, 0,
		{ 6629,-1900,-483,-4618,12349,2550,-622,1381,6514 } },
		{ "Sony DSC-RX1", 0, 0,
		{ 6344,-1612,-462,-4863,12477,2681,-865,1786,6899 } },
		{ "Sony DSLR-A100", 0, 0xfeb,
		{ 9437,-2811,-774,-8405,16215,2290,-710,596,7181 } },
		{ "Sony DSLR-A290", 0, 0,
		{ 6038,-1484,-579,-9145,16746,2512,-875,746,7218 } },
		{ "Sony DSLR-A2", 0, 0,
		{ 9847,-3091,-928,-8485,16345,2225,-715,595,7103 } },
		{ "Sony DSLR-A300", 0, 0,
		{ 9847,-3091,-928,-8485,16345,2225,-715,595,7103 } },
		{ "Sony DSLR-A330", 0, 0,
		{ 9847,-3091,-929,-8485,16346,2225,-714,595,7103 } },
		{ "Sony DSLR-A350", 0, 0xffc,
		{ 6038,-1484,-578,-9146,16746,2513,-875,746,7217 } },
		{ "Sony DSLR-A380", 0, 0,
		{ 6038,-1484,-579,-9145,16746,2512,-875,746,7218 } },
		{ "Sony DSLR-A390", 0, 0,
		{ 6038,-1484,-579,-9145,16746,2512,-875,746,7218 } },
		{ "Sony DSLR-A450", 0, 0xfeb,
		{ 4950,-580,-103,-5228,12542,3029,-709,1435,7371 } },
		{ "Sony DSLR-A580", 0, 0xfeb,
		{ 5932,-1492,-411,-4813,12285,2856,-741,1524,6739 } },
		{ "Sony DSLR-A500", 0, 0xfeb,
		{ 6046,-1127,-278,-5574,13076,2786,-691,1419,7625 } },
		{ "Sony DSLR-A5", 0, 0xfeb,
		{ 4950,-580,-103,-5228,12542,3029,-709,1435,7371 } },
		{ "Sony DSLR-A700", 0, 0,
		{ 5775,-805,-359,-8574,16295,2391,-1943,2341,7249 } },
		{ "Sony DSLR-A850", 0, 0,
		{ 5413,-1162,-365,-5665,13098,2866,-608,1179,8440 } },
		{ "Sony DSLR-A900", 0, 0,
		{ 5209,-1072,-397,-8845,16120,2919,-1618,1803,8654 } },
		{ "Sony ILCA-68", 0, 0,
		{ 6435,-1903,-536,-4722,12449,2550,-663,1363,6517 } },
		{ "Sony ILCA-77M2", 0, 0,
		{ 5991,-1732,-443,-4100,11989,2381,-704,1467,5992 } },
		{ "Sony ILCE-6300", 0, 0,
		{ 5973,-1695,-419,-3826,11797,2293,-639,1398,5789 } },
		{ "Sony ILCE-7M2", 0, 0,
		{ 5271,-712,-347,-6153,13653,2763,-1601,2366,7242 } },
		{ "Sony ILCE-7S", 0, 0,	/* also ILCE-7SM2 */
		{ 5838,-1430,-246,-3497,11477,2297,-748,1885,5778 } },
		{ "Sony ILCE-7RM2", 0, 0,
		{ 6629,-1900,-483,-4618,12349,2550,-622,1381,6514 } },
		{ "Sony ILCE-7R", 0, 0,
		{ 4913,-541,-202,-6130,13513,2906,-1564,2151,7183 } },
		{ "Sony ILCE-7", 0, 0,
		{ 5271,-712,-347,-6153,13653,2763,-1601,2366,7242 } },
		{ "Sony ILCE", 0, 0,	/* 3000, 5000, 5100, 6000, and QX1 */
		{ 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
		{ "Sony NEX-5N", 0, 0,
		{ 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
		{ "Sony NEX-5R", 0, 0,
		{ 6129,-1545,-418,-4930,12490,2743,-977,1693,6615 } },
		{ "Sony NEX-5T", 0, 0,
		{ 6129,-1545,-418,-4930,12490,2743,-977,1693,6615 } },
		{ "Sony NEX-3N", 0, 0,
		{ 6129,-1545,-418,-4930,12490,2743,-977,1693,6615 } },
		{ "Sony NEX-3", 138, 0,		/* DJC */
		{ 6907,-1256,-645,-4940,12621,2320,-1710,2581,6230 } },
		{ "Sony NEX-5", 116, 0,		/* DJC */
		{ 6807,-1350,-342,-4216,11649,2567,-1089,2001,6420 } },
		{ "Sony NEX-3", 0, 0,		/* Adobe */
		{ 6549,-1550,-436,-4880,12435,2753,-854,1868,6976 } },
		{ "Sony NEX-5", 0, 0,		/* Adobe */
		{ 6549,-1550,-436,-4880,12435,2753,-854,1868,6976 } },
		{ "Sony NEX-6", 0, 0,
		{ 6129,-1545,-418,-4930,12490,2743,-977,1693,6615 } },
		{ "Sony NEX-7", 0, 0,
		{ 5491,-1192,-363,-4951,12342,2948,-911,1722,7192 } },
		{ "Sony NEX", 0, 0,	/* NEX-C3, NEX-F3 */
		{ 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
		{ "Sony SLT-A33", 0, 0,
		{ 6069,-1221,-366,-5221,12779,2734,-1024,2066,6834 } },
		{ "Sony SLT-A35", 0, 0,
		{ 5986,-1618,-415,-4557,11820,3120,-681,1404,6971 } },
		{ "Sony SLT-A37", 0, 0,
		{ 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
		{ "Sony SLT-A55", 0, 0,
		{ 5932,-1492,-411,-4813,12285,2856,-741,1524,6739 } },
		{ "Sony SLT-A57", 0, 0,
		{ 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
		{ "Sony SLT-A58", 0, 0,
		{ 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
		{ "Sony SLT-A65", 0, 0,
		{ 5491,-1192,-363,-4951,12342,2948,-911,1722,7192 } },
		{ "Sony SLT-A77", 0, 0,
		{ 5491,-1192,-363,-4951,12342,2948,-911,1722,7192 } },
		{ "Sony SLT-A99", 0, 0,
		{ 6344,-1612,-462,-4863,12477,2681,-865,1786,6899 } },
	};
	double cam_xyz[4][3];
	char name[130];

	sprintf_s(name, 130, "%s %s", make, model);
	for (size_t i = 0; i < sizeof table / sizeof *table; i++)
	{
		if (!strncmp(name, table[i].prefix, strlen(table[i].prefix)))
		{
			if (table[i].black)   _black = (unsigned short)table[i].black;
			if (table[i].maximum) _maximum = (unsigned short)table[i].maximum;
			if (table[i].trans[0])
			{
				_rawColor = 0;
				for (size_t j = 0; j < 12; j++)
					((double *)cam_xyz)[j] = table[i].trans[j] / 10000.0;
				CamXyzCoeff(_rgbCam, cam_xyz);
			}
			break;
		}
	}
}

void CSimpleInfo::CamXyzCoeff(float _rgbCam[3][4], double cam_xyz[4][3])
{
	double cam_rgb[4][3];

	for (size_t i = 0; i < _colors; i++)		/* Multiply out XYZ colorspace */
	{
		for (size_t j = 0; j < 3; j++)
		{
			cam_rgb[i][j] = 0;
			for (size_t k = 0; k < 3; k++)
			{
				cam_rgb[i][j] += cam_xyz[i][k] * xyzRGB[k][j];
			}
		}
	}

	for (size_t i = 0; i < _colors; i++)
	{		/* Normalize cam_rgb so that */
		double num = 0;
		for (size_t j = 0; j < 3; j++)		/* cam_rgb * (1,1,1) is (1,1,1,1) */
			num += cam_rgb[i][j];
		for (size_t j = 0; j < 3; j++)
			cam_rgb[i][j] /= num;
		_preMul[i] = 1 / num;
	}
	double inverse[4][3];
	PseudoInverse(cam_rgb, inverse, _colors);
	for (size_t i = 0; i < 3; i++)
		for (size_t j = 0; j < _colors; j++)
			_rgbCam[i][j] = inverse[j][i];
}

void CSimpleInfo::PseudoInverse(double(*in)[3], double(*out)[3], int size)
{
	double work[3][6];

	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 6; j++)
			work[i][j] = j == i + 3;
		for (size_t j = 0; j < 3; j++)
			for (size_t k = 0; k < size; k++)
				work[i][j] += in[k][i] * in[k][j];
	}
	for (size_t i = 0; i < 3; i++)
	{
		double num = work[i][i];
		for (size_t j = 0; j < 6; j++)
			work[i][j] /= num;
		for (size_t k = 0; k < 3; k++)
		{
			if (k == i) continue;
			num = work[k][i];
			for (size_t j = 0; j < 6; j++)
				work[k][j] -= work[i][j] * num;
		}
	}
	for (size_t i = 0; i < size; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			out[i][j] = 0;
			for (size_t k = 0; k < 3; k++)
			{
				out[i][j] += work[j][k + 3] * in[i][k];
			}
		}
	}
}

float CSimpleInfo::FindGreen(int bps, int bite, int off0, int off1)
{
	UINT64 bitbuf = 0;
	unsigned short img[2][2064];

	for (size_t c = 0; c < 2; c++)
	{
		_reader->Seek(c ? off1 : off0, SEEK_SET);
		int vbits = 0;
		for (int col = 0; col < _width; col++)
		{
			for (vbits -= bps; vbits < 0; vbits += bite)
			{
				bitbuf <<= bite;
				for (int i = 0; i < bite; i += 8)
					bitbuf |= (unsigned)(_reader->GetChar() << i);
			}
			img[c][col] = bitbuf << (64 - bps - vbits) >> (64 - bps);
		}
	}
	double sum[] = { 0, 0 };
	for (size_t c = 0; c < (_width - 1); c++)
	{
		sum[c & 1] += ABS(img[0][c] - img[1][c + 1]);
		sum[~c & 1] += ABS(img[1][c] - img[0][c + 1]);
	}
	return 100 * log(sum[0] / sum[1]);
}

void CSimpleInfo::SimpleCoeff(int index)
{
	static const float table[][12] = {
		/* index 0 -- all Foveon cameras */
		{ 1.4032,-0.2231,-0.1016,-0.5263,1.4816,0.017,-0.0112,0.0183,0.9113 },
		/* index 1 -- Kodak DC20 and DC25 */
		{ 2.25,0.75,-1.75,-0.25,-0.25,0.75,0.75,-0.25,-0.25,-1.75,0.75,2.25 },
		/* index 2 -- Logitech Fotoman Pixtura */
		{ 1.893,-0.418,-0.476,-0.495,1.773,-0.278,-1.017,-0.655,2.672 },
		/* index 3 -- Nikon E880, E900, and E990 */
		{ -1.936280,  1.800443, -1.448486,  2.584324,
		1.405365, -0.524955, -0.289090,  0.408680,
		-1.204965,  1.082304,  2.941367, -1.818705 }
	};

	_rawColor = 0;
	for (size_t i = 0; i < 3; i++)
		for (size_t c = 0; c < _colors; c++)
			_rgbCam[i][c] = table[index][i*_colors + c];
}

void CSimpleInfo::GammaCurve(double pwr, double ts, int mode, int imax)
{
	double g[6], bnd[2] = { 0,0 };

	g[0] = pwr;
	g[1] = ts;
	g[2] = g[3] = g[4] = 0;
	bnd[g[1] >= 1] = 1;
	if (g[1] && (g[1] - 1)*(g[0] - 1) <= 0)
	{
		for (size_t i = 0; i < 48; i++)
		{
			g[2] = (bnd[0] + bnd[1]) / 2;
			if (g[0]) bnd[(pow(g[2] / g[1], -g[0]) - 1) / g[0] - 1 / g[2] > -1] = g[2];
			else	bnd[g[2] / exp(1 - 1 / g[2]) < g[1]] = g[2];
		}
		g[3] = g[2] / g[1];
		if (g[0]) g[4] = g[2] * (1 / g[0] - 1);
	}
	if (g[0]) g[5] = 1 / (g[1] * SQR(g[3]) / 2 - g[4] * (1 - g[3]) +
		(1 - pow(g[3], 1 + g[0]))*(1 + g[4]) / (1 + g[0])) - 1;
	else      g[5] = 1 / (g[1] * SQR(g[3]) / 2 + 1
		- g[2] - g[3] - g[2] * g[3] * (log(g[3]) - 1)) - 1;
	if (!mode--)
	{
		memcpy(_gamma, g, sizeof _gamma);
		return;
	}
	for (size_t i = 0; i < 0x10000; i++)
	{
		_curve[i] = 0xffff;
		double r;
		if ((r = (double)i / imax) < 1)
			_curve[i] = 0x10000 * (mode
				? (r < g[3] ? r*g[1] : (g[0] ? pow(r, g[0])*(1 + g[4]) - g[4] : log(r)*g[2] + 1))
				: (r < g[2] ? r / g[1] : (g[0] ? pow((r + g[4]) / (1 + g[4]), 1 / g[0]) : exp((r - 1) / g[2]))));
	}
}

short CSimpleInfo::GuessByteOrder(int words)
{
	unsigned char test[4][2];
	int t = 2;
	double sum[2] = { 0,0 };

	_reader->Read(test[0], 2, 2);
	for (words -= 2; words--; )
	{
		_reader->Read(test[t], 2, 1);
		for (size_t msb = 0; msb < 2; msb++)
		{
			double diff = (test[t ^ 2][msb] << 8 | test[t ^ 2][!msb]) - (test[t][msb] << 8 | test[t][!msb]);
			sum[msb] += diff*diff;
		}
		t = (t + 1) & 3;
	}
	return sum[0] < sum[1] ? 0x4d4d : 0x4949;
}

int CSimpleInfo::CanonS2is()
{
	for (size_t row = 0; row < 100; row++)
	{
		_reader->Seek(row * 3340 + 3284, SEEK_SET);
		if (_reader->GetChar() > 15)
			return 1;
	}
	return 0;
}

/*
Returns 1 for a Coolpix 995, 0 for anything else.
*/
int CSimpleInfo::NikonE995()
{
	const unsigned char often[] = { 0x00, 0x55, 0xaa, 0xff };

	int histo[256];
	memset(histo, 0, sizeof histo);
	_reader->Seek(-2000, SEEK_END);
	for (size_t i = 0; i < 2000; i++)
		histo[_reader->GetChar()]++;
	for (size_t i = 0; i < 4; i++)
		if (histo[often[i]] < 200)
			return 0;
	return 1;
}

/*
Returns 1 for a Coolpix 2100, 0 for anything else.
*/
int CSimpleInfo::NikonE2100()
{
	unsigned char t[12];

	_reader->Seek(0, SEEK_SET);
	for (size_t i = 0; i < 1024; i++)
	{
		_reader->Read(t, 1, 12);
		if (((t[2] & t[4] & t[7] & t[9]) >> 4
			& t[1] & t[6] & t[8] & t[11] & 3) != 3)
			return 0;
	}
	return 1;
}

void CSimpleInfo::Nikon3700()
{
	static const struct
	{
		int bits;
		char make[12], model[15];
	} table[] = {
		{ 0x00, "Pentax",  "Optio 33WR" },
		{ 0x03, "Nikon",   "E3200" },
		{ 0x32, "Nikon",   "E3700" },
		{ 0x33, "Olympus", "C740UZ" } };

	unsigned char dp[24];

	_reader->Seek(3072, SEEK_SET);
	_reader->Read(dp, 1, 24);
	int bits = (dp[8] & 3) << 4 | (dp[20] & 3);
	for (size_t i = 0; i < sizeof table / sizeof *table; i++)
		if (bits == table[i].bits)
		{
			strcpy_s(_make, LenMake, table[i].make);
			strcpy_s(_model, LenModel, table[i].model);
		}
}

/*
Separates a Minolta DiMAGE Z2 from a Nikon E4300.
*/
int CSimpleInfo::MinoltaZ2()
{
	char tail[424];

	_reader->Seek(-sizeof tail, SEEK_END);
	_reader->Read(tail, 1, sizeof tail);
	int nz = 0;
	for (size_t i = 0; i < sizeof tail; i++)
		if (tail[i]) nz++;
	return nz > 20;
}

void CSimpleInfo::ParsePhaseOne(int base)
{
	float romm_cam[3][3];
	char *cp;

	memset(&_ph1, 0, sizeof _ph1);
	_reader->Seek(base, SEEK_SET);
	_reader->SetOrder(_reader->GetUInt() & 0xffff);
	if (_reader->GetUInt() >> 8 != 0x526177)
		return;		/* "Raw" */
	_reader->Seek(_reader->GetUInt() + base, SEEK_SET);
	unsigned entries = _reader->GetUInt();
	_reader->GetUInt();
	while (entries--)
	{
		unsigned tag = _reader->GetUInt();
		unsigned type = _reader->GetUInt();
		unsigned len = _reader->GetUInt();
		unsigned data = _reader->GetUInt();
		unsigned save = _reader->GetPosition();
		_reader->Seek(base + data, SEEK_SET);
		switch (tag)
		{
		case 0x100:
			_flip = "0653"[data & 3] - '0';
			break;
		case 0x106:
			for (size_t i = 0; i < 9; i++)
				((float *)romm_cam)[i] = _reader->GetReal(11);
			RommCoeff(romm_cam);
			break;
		case 0x107:
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = _reader->GetReal(11);
			break;
		case 0x108:
			_rawWidth = data;
			break;
		case 0x109:
			_rawHeight = data;
			break;
		case 0x10a:
			_leftMargin = data;
			break;
		case 0x10b:
			_topMargin = data;
			break;
		case 0x10c:
			_width = data;
			break;
		case 0x10d:
			_height = data;
			break;
		case 0x10e:
			_ph1.format = data;
			break;
		case 0x10f:
			_dataOffset = data + base;
			break;
		case 0x110:
			_metaOffset = data + base;
			_metaLength = len;
			break;
		case 0x112:
			_ph1.key_off = save - 4;
			break;
		case 0x210:
			_ph1.tag_210 = _reader->IntToFloat(data);
			break;
		case 0x21a:
			_ph1.tag_21a = data;
			break;
		case 0x21c:
			_stripOffset = data + base;
			break;
		case 0x21d:
			_ph1.black = data;
			break;
		case 0x222:
			_ph1.split_col = data;
			break;
		case 0x223:
			_ph1.black_col = data + base;
			break;
		case 0x224:
			_ph1.split_row = data;
			break;
		case 0x225:
			_ph1.black_row = data + base;
			break;
		case 0x301:
			_model[63] = 0;
			_reader->Read(_model, 1, 63);
			if ((cp = strstr(_model, " camera"))) *cp = 0;
		}
		_reader->Seek(save, SEEK_SET);
	}
	_loadRaw = _ph1.format < 3 ? LoadRawType::PhaseOneLoadRaw : LoadRawType::PhaseOneLoadRawC;
	_maximum = 0xffff;
	strcpy_s(_make, LenMake, "Phase One");
	if (_model[0]) return;
	switch (_rawHeight)
	{
	case 2060:
		strcpy_s(_model, LenModel, "LightPhase");
		break;
	case 2682:
		strcpy_s(_model, LenModel, "H 10");
		break;
	case 4128:
		strcpy_s(_model, LenModel, "H 20");
		break;
	case 5488:
		strcpy_s(_model, LenModel, "H 25");
		break;
	}
}

/*
Parse a CIFF file, better known as Canon CRW format.
*/
void CSimpleInfo::ParseCiff(int offset, int length, int depth)
{
	int wbi = -1;
	unsigned short key[] = { 0x410, 0x45f3 };

	_reader->Seek(offset + length - 4, SEEK_SET);
	int tboff = _reader->GetUInt() + offset;
	_reader->Seek(tboff, SEEK_SET);
	int nrecs = _reader->GetUShort();
	if ((nrecs | depth) > 127) return;
	while (nrecs--)
	{
		int type = _reader->GetUShort();
		int len = _reader->GetUInt();
		int save = _reader->GetPosition() + 4;
		_reader->Seek(offset + _reader->GetUInt(), SEEK_SET);
		if ((((type >> 8) + 8) | 8) == 0x38)
			ParseCiff(_reader->GetPosition(), len, depth + 1); /* Parse a sub-table */
		if (type == 0x0810)
			_reader->Read(_artist, 64, 1);
		if (type == 0x080a)
		{
			_reader->Read(_make, 64, 1);
			_reader->Seek(strlen(_make) - 63, SEEK_CUR);
			_reader->Read(_model, 64, 1);
		}
		if (type == 0x1810)
		{
			_width = _reader->GetUInt();
			_height = _reader->GetUInt();
			_pixelAspect = _reader->IntToFloat(_reader->GetUInt());
			_flip = _reader->GetUInt();
		}
		if (type == 0x1835)			/* Get the decoder table */
			_tiffCompress = _reader->GetUInt();
		if (type == 0x2007)
		{
			_thumbOffset = _reader->GetPosition();
			_thumbLength = len;
		}
		if (type == 0x1818)
		{
			_shutter = pow(2, -_reader->IntToFloat((_reader->GetUInt(), _reader->GetUInt())));
			_aperture = pow(2, _reader->IntToFloat(_reader->GetUInt()) / 2);
		}
		if (type == 0x102a)
		{
			_isoSpeed = pow(2, (_reader->GetUInt(), _reader->GetUShort()) / 32.0 - 4) * 50;
			_aperture = pow(2, (_reader->GetUShort(), (short)_reader->GetUShort()) / 64.0);
			_shutter = pow(2, -((short)_reader->GetUShort()) / 32.0);
			wbi = (_reader->GetUShort(), _reader->GetUShort());
			if (wbi > 17)
				wbi = 0;
			_reader->Seek(32, SEEK_CUR);
			if (_shutter > 1e6)
				_shutter = _reader->GetUShort() / 10.0;
		}
		if (type == 0x102c)
		{
			if (_reader->GetUShort() > 512)
			{		/* Pro90, G1 */
				_reader->Seek(118, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					_camMul[c ^ 2] = _reader->GetUShort();
			}
			else
			{				/* G2, S30, S40 */
				_reader->Seek(98, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					_camMul[c ^ (c >> 1) ^ 1] = _reader->GetUShort();
			}
		}
		if (type == 0x0032)
		{
			if (len == 768)
			{			/* EOS D30 */
				_reader->Seek(72, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					_camMul[c ^ (c >> 1)] = 1024.0 / _reader->GetUShort();
				if (!wbi) _camMul[0] = -1;	/* use my auto _white balance */
			}
			else if (!_camMul[0])
			{
				int x;
				if (_reader->GetUShort() == key[0])		/* Pro1, G6, S60, S70 */
				{
					x = (strstr(_model, "Pro1") ? "012346000000000000" : "01345:000000006008")[wbi] - '0' + 2;
				}
				else
				{				/* G3, G5, S45, S50 */
					x = "023457000000006000"[wbi] - '0';
					key[0] = key[1] = 0;
				}
				_reader->Seek(78 + x * 8, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					_camMul[c ^ (c >> 1) ^ 1] = _reader->GetUShort() ^ key[c & 1];
				if (!wbi) _camMul[0] = -1;
			}
		}
		if (type == 0x10a9)
		{		/* D60, 10D, 300D, and clones */
			if (len > 66) wbi = "0134567028"[wbi] - '0';
			_reader->Seek(2 + wbi * 8, SEEK_CUR);
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1)] = _reader->GetUShort();
		}
		if (type == 0x1030 && (0x18040 >> wbi & 1))
			CiffBlock1030();		/* all that don't have 0x10a9 */
		if (type == 0x1031)
		{
			_rawWidth = (_reader->GetUShort(), _reader->GetUShort());
			_rawHeight = _reader->GetUShort();
		}
		if (type == 0x5029)
		{
			_focalLen = len >> 16;
			if ((len & 0xffff) == 2) _focalLen /= 32;
		}
		if (type == 0x5813)
			_flashUsed = _reader->IntToFloat(len);
		if (type == 0x5814)
			_canonEV = _reader->IntToFloat(len);
		if (type == 0x5817)
			_shotOrder = len;
		if (type == 0x5834)
			_uniqueId = len;
		if (type == 0x580e)
			_timestamp = len;
		if (type == 0x180e)
			_timestamp = _reader->GetUInt();
#ifdef LOCALTIME
		if ((type | 0x4000) == 0x580e)
			_timestamp = mktime(gmtime(&_timestamp));
#endif
		_reader->Seek(save, SEEK_SET);
	}
}

/*
CIFF block 0x1030 contains an 8x8 _white sample.
Load this into _white[][] for use in ScaleColors().
*/
void CSimpleInfo::CiffBlock1030()
{
	static const unsigned short key[] = { 0x410, 0x45f3 };

	if ((_reader->GetUShort(), _reader->GetUInt()) != 0x80008 || !_reader->GetUInt())
		return;
	int bpp = _reader->GetUShort();
	if (bpp != 10 && bpp != 12)
		return;
	int i = 0;
	int vbits = 0;
	unsigned long bitbuf = 0;
	for (size_t row = 0; row < 8; row++)
	{
		for (size_t col = 0; col < 8; col++)
		{
			if (vbits < bpp)
			{
				bitbuf = bitbuf << 16 | (_reader->GetUShort() ^ key[i++ & 1]);
				vbits += 16;
			}
			_white[row][col] = bitbuf >> (vbits -= bpp) & ~(-1 << bpp);
		}
	}
}

void CSimpleInfo::ParseFuji(int offset)
{
	_reader->Seek(offset, SEEK_SET);
	unsigned entries = _reader->GetUInt();
	if (entries > 255)
		return;
	while (entries--)
	{
		unsigned tag = _reader->GetUShort();
		unsigned len = _reader->GetUShort();
		unsigned save = _reader->GetPosition();
		if (tag == 0x100)
		{
			_rawHeight = _reader->GetUShort();
			_rawWidth = _reader->GetUShort();
		}
		else if (tag == 0x121)
		{
			_height = _reader->GetUShort();
			if ((_width = _reader->GetUShort()) == 4284)
				_width += 3;
		}
		else if (tag == 0x130)
		{
			_fujiLayout = _reader->GetChar() >> 7;
			_fujiWidth = !(_reader->GetChar() & 8);
		}
		else if (tag == 0x131)
		{
			_filters = 9;
			for (size_t c = 0; c < 36; c++)
				_xtransAbs[0][35 - c] = _reader->GetChar() & 3;
		}
		else if (tag == 0x2ff0)
		{
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ 1] = _reader->GetUShort();
		}
		else if (tag == 0xc000)
		{
			short c = _reader->GetOrder();
			_reader->SetOrder(0x4949);
			while ((tag = _reader->GetUInt()) > _rawWidth);
			_width = tag;
			_height = _reader->GetUInt();
			_reader->SetOrder(c);
		}
		_reader->Seek(save + len, SEEK_SET);
	}
	_height <<= _fujiLayout;
	_width >>= _fujiLayout;
}

int CSimpleInfo::ParseJpeg(int offset)
{
	int len, save, hlen, mark;

	_reader->Seek(offset, SEEK_SET);
	if (_reader->GetChar() != 0xff || _reader->GetChar() != 0xd8)
		return 0;

	while (_reader->GetChar() == 0xff && (mark = _reader->GetChar()) != 0xda)
	{
		_reader->SetOrder(0x4d4d);
		len = _reader->GetUShort() - 2;
		save = _reader->GetPosition();
		if (mark == 0xc0 || mark == 0xc3 || mark == 0xc9)
		{
			_reader->GetChar();
			_rawHeight = _reader->GetUShort();
			_rawWidth = _reader->GetUShort();
		}
		_reader->SetOrder(_reader->GetUShort());
		hlen = _reader->GetUInt();
		if (_reader->GetUInt() == 0x48454150)		/* "HEAP" */
			ParseCiff(save + hlen, len - hlen, 0);
		if (ParseTiff(save + 6)) ApplyTiff();
		_reader->Seek(save + len, SEEK_SET);
	}
	return 1;
}

void CSimpleInfo::ParseRiff()
{
	char tag[4];
	static const char mon[12][4] =
	{ "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

	_reader->SetOrder(0x4949);
	_reader->Read(tag, 4, 1);
	unsigned size = _reader->GetUInt();
	unsigned end = _reader->GetPosition() + size;
	if (!memcmp(tag, "RIFF", 4) || !memcmp(tag, "LIST", 4))
	{
		_reader->GetUInt();
		while (_reader->GetPosition() + 7 < end && !_reader->Eof())
			ParseRiff();
	}
	else if (!memcmp(tag, "nctg", 4))
	{
		while (_reader->GetPosition() + 7 < end)
		{
			unsigned i = _reader->GetUShort();
			size = _reader->GetUShort();
			if ((i + 1) >> 1 == 10 && size == 20)
				GetTimestamp(0);
			else _reader->Seek(size, SEEK_CUR);
		}
	}
	else if (!memcmp(tag, "IDIT", 4) && size < 64)
	{
		char date[64], month[64];

		_reader->Read(date, 64, 1);
		date[size] = 0;
		tm t;
		memset(&t, 0, sizeof t);
		if (sscanf(date, "%*s %s %d %d:%d:%d %d", month, &t.tm_mday,
			&t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year) == 6)
		{
			unsigned i;
			for (i = 0; i < 12 && _stricmp(mon[i], month); i++);
			t.tm_mon = i;
			t.tm_year -= 1900;
			if (mktime(&t) > 0)
				_timestamp = mktime(&t);
		}
	}
	else
		_reader->Seek(size, SEEK_CUR);
}

void CSimpleInfo::ParseQt(int end)
{
	unsigned size;
	char tag[4];

	_reader->SetOrder(0x4d4d);
	while (_reader->GetPosition() + 7 < end)
	{
		unsigned save = _reader->GetPosition();
		if ((size = _reader->GetUInt()) < 8)
			return;
		_reader->Read(tag, 4, 1);
		if (!memcmp(tag, "moov", 4) ||
			!memcmp(tag, "udta", 4) ||
			!memcmp(tag, "CNTH", 4))
			ParseQt(save + size);
		if (!memcmp(tag, "CNDA", 4))
			ParseJpeg(_reader->GetPosition());
		_reader->Seek(save + size, SEEK_SET);
	}
}

void CSimpleInfo::ParseSmal(int offset, int fsize)
{
	_reader->Seek(offset + 2, SEEK_SET);
	_reader->SetOrder(0x4949);
	int ver = _reader->GetChar();
	if (ver == 6)
		_reader->Seek(5, SEEK_CUR);
	if (_reader->GetUInt() != fsize)
		return;
	if (ver > 6)
		_dataOffset = _reader->GetUInt();
	_rawHeight = _height = _reader->GetUShort();
	_rawWidth = _width = _reader->GetUShort();
	strcpy_s(_make, LenMake, "SMaL");
	sprintf_s(_model, LenModel, "v%d %dx%d", ver, _width, _height);
	if (ver == 6)
		_loadRaw = LoadRawType::SmalV6LoadRaw;
	if (ver == 9)
		_loadRaw = LoadRawType::SmalV9LoadRaw;
}

void CSimpleInfo::ParseCine()
{
	_reader->SetOrder(0x4949);
	_reader->Seek(4, SEEK_SET);
	_isRaw = _reader->GetUShort() == 2;
	_reader->Seek(14, SEEK_CUR);
	_isRaw *= _reader->GetUInt();
	unsigned off_head = _reader->GetUInt();
	unsigned off_setup = _reader->GetUInt();
	unsigned off_image = _reader->GetUInt();
	_timestamp = _reader->GetUInt();
	unsigned i = _reader->GetUInt();
	if (i)
		_timestamp = i;
	_reader->Seek(off_head + 4, SEEK_SET);
	_rawWidth = _reader->GetUInt();
	_rawHeight = _reader->GetUInt();
	switch (_reader->GetUShort(), _reader->GetUShort())
	{
	case 8:
		_loadRaw = LoadRawType::EightBitLoadRaw;
		break;
	case 16:
		_loadRaw = LoadRawType::UnpackedLoadRaw;
	}
	_reader->Seek(off_setup + 792, SEEK_SET);
	strcpy_s(_make, LenMake, "CINE");
	sprintf_s(_model, LenModel, "%d", _reader->GetUInt());
	_reader->Seek(12, SEEK_CUR);
	switch ((i = _reader->GetUInt()) & 0xffffff)
	{
	case 3:
		_filters = 0x94949494;
		break;
	case 4:
		_filters = 0x49494949;
		break;
	default:  _isRaw = 0;
	}
	_reader->Seek(72, SEEK_CUR);
	switch ((_reader->GetUInt() + 3600) % 360)
	{
	case 270:
		_flip = 4;
		break;
	case 180:
		_flip = 1;
		break;
	case  90:
		_flip = 7;
		break;
	case   0:
		_flip = 2;
	}
	_camMul[0] = _reader->GetReal(11);
	_camMul[2] = _reader->GetReal(11);
	_maximum = ~(-1 << _reader->GetUInt());
	_reader->Seek(668, SEEK_CUR);
	_shutter = _reader->GetUInt() / 1000000000.0;
	_reader->Seek(off_image, SEEK_SET);
	if (_options->_shotSelect < _isRaw)
		_reader->Seek(_options->_shotSelect * 8, SEEK_CUR);
	_dataOffset = (INT64)_reader->GetUInt() + 8;
	_dataOffset += (INT64)_reader->GetUInt() << 32;
}

void CSimpleInfo::ParseRedcine()
{
	_reader->SetOrder(0x4d4d);
	_isRaw = 0;
	_reader->Seek(52, SEEK_SET);
	_width = _reader->GetUInt();
	_height = _reader->GetUInt();
	_reader->Seek(0, SEEK_END);
	unsigned i = _reader->GetPosition() & 511;
	_reader->Seek(-(i), SEEK_CUR);
	if (_reader->GetUInt() != i || _reader->GetUInt() != 0x52454f42)
	{
		fprintf(stderr, ("%s: Tail is missing, parsing from head...\n"), _reader->GetFileName());
		_reader->Seek(0, SEEK_SET);
		unsigned len;
		while ((len = _reader->GetUInt()) != EOF)
		{
			if (_reader->GetUInt() == 0x52454456)
				if (_isRaw++ == _options->_shotSelect)
					_dataOffset = _reader->GetPosition() - 8;
			_reader->Seek(len - 8, SEEK_CUR);
		}
	}
	else
	{
		unsigned rdvo = _reader->GetUInt();
		_reader->Seek(12, SEEK_CUR);
		_isRaw = _reader->GetUInt();
		_reader->Seek(rdvo + 8 + _options->_shotSelect * 4, SEEK_SET);
		_dataOffset = _reader->GetUInt();
	}
}

void CSimpleInfo::ParseRollei()
{
	char line[128];
	char* val;
	tm t;

	_reader->Seek(0, SEEK_SET);
	memset(&t, 0, sizeof t);
	do
	{
		_reader->GetString(line, 128);
		if ((val = strchr(line, '=')))
			*val++ = 0;
		else
			val = line + strlen(line);
		if (!strcmp(line, "DAT"))
			sscanf(val, "%d.%d.%d", &t.tm_mday, &t.tm_mon, &t.tm_year);
		if (!strcmp(line, "TIM"))
			sscanf(val, "%d:%d:%d", &t.tm_hour, &t.tm_min, &t.tm_sec);
		if (!strcmp(line, "HDR"))
			_thumbOffset = atoi(val);
		if (!strcmp(line, "X  "))
			_rawWidth = atoi(val);
		if (!strcmp(line, "Y  "))
			_rawHeight = atoi(val);
		if (!strcmp(line, "TX "))
			_thumbWidth = atoi(val);
		if (!strcmp(line, "TY "))
			_thumbHeight = atoi(val);
	} while (strncmp(line, "EOHD", 4));
	_dataOffset = _thumbOffset + _thumbWidth * _thumbHeight * 2;
	t.tm_year -= 1900;
	t.tm_mon -= 1;
	if (mktime(&t) > 0)
		_timestamp = mktime(&t);
	strcpy_s(_make, LenMake, "Rollei");
	strcpy_s(_model, LenModel, "d530flex");
	_writeThumb = WriteThumbType::RolleiThumb;
}

void CSimpleInfo::ParseSinarIA()
{
	_reader->SetOrder(0x4949);
	_reader->Seek(4, SEEK_SET);
	int entries = _reader->GetUInt();
	_reader->Seek(_reader->GetUInt(), SEEK_SET);
	char str[8];
	while (entries--)
	{
		int off = _reader->GetUInt();
		_reader->GetUInt();
		_reader->Read(str, 8, 1);
		if (!strcmp(str, "META"))   _metaOffset = off;
		if (!strcmp(str, "THUMB")) _thumbOffset = off;
		if (!strcmp(str, "RAW0"))   _dataOffset = off;
	}
	_reader->Seek(_metaOffset + 20, SEEK_SET);
	_reader->Read(_make, 64, 1);
	_make[63] = 0;

	char* cp;
	if ((cp = strchr(_make, ' ')))
	{
		strcpy_s(_model, LenModel, cp + 1);
		*cp = 0;
	}
	_rawWidth = _reader->GetUShort();
	_rawHeight = _reader->GetUShort();
	_loadRaw = LoadRawType::UnpackedLoadRaw;
	_thumbWidth = (_reader->GetUInt(), _reader->GetUShort());
	_thumbHeight = _reader->GetUShort();
	_writeThumb = WriteThumbType::PpmThumb;
	_maximum = 0x3fff;
}

void CSimpleInfo::ParseMinolta(int base)
{
	int high = 0, wide = 0;
	short sorder = _reader->GetOrder();

	_reader->Seek(base, SEEK_SET);
	if (_reader->GetChar() || _reader->GetChar() - 'M' || _reader->GetChar() - 'R')
		return;
	_reader->SetOrder(_reader->GetChar() * 0x101);
	int offset = base + _reader->GetUInt() + 8;
	int save;
	while ((save = _reader->GetPosition()) < offset)
	{
		int tag = 0;
		for (size_t i = 0; i < 4; i++)
			tag = tag << 8 | _reader->GetChar();
		int len = _reader->GetUInt();
		int i;
		switch (tag)
		{
		case 0x505244:				/* PRD */
			_reader->Seek(8, SEEK_CUR);
			high = _reader->GetUShort();
			wide = _reader->GetUShort();
			break;
		case 0x574247:				/* WBG */
			_reader->GetUInt();
			i = strcmp(_model, "DiMAGE A200") ? 0 : 3;
			for (size_t c = 0; c < 4; c++)
				_camMul[c ^ (c >> 1) ^ i] = _reader->GetUShort();
			break;
		case 0x545457:				/* TTW */
			ParseTiff(_reader->GetPosition());
			_dataOffset = offset;
		}
		_reader->Seek(save + len + 8, SEEK_SET);
	}
	_rawHeight = high;
	_rawWidth = wide;
	_reader->SetOrder(sorder);
}

void CSimpleInfo::ParseFoveon()
{
	int entries, img = 0, off, len, tag, save, i, wide, high, pent, poff[256][2];
	char name[64], value[64];

	_reader->SetOrder(0x4949);			/* Little-endian */
	_reader->Seek(36, SEEK_SET);
	_flip = _reader->GetUInt();
	_reader->Seek(-4, SEEK_END);
	_reader->Seek(_reader->GetUInt(), SEEK_SET);
	if (_reader->GetUInt() != 0x64434553)
		return;	/* SECd */
	entries = (_reader->GetUInt(), _reader->GetUInt());
	while (entries--)
	{
		off = _reader->GetUInt();
		len = _reader->GetUInt();
		tag = _reader->GetUInt();
		save = _reader->GetPosition();
		_reader->Seek(off, SEEK_SET);
		if (_reader->GetUInt() != (0x20434553 | (tag << 24)))
			return;
		switch (tag)
		{
		case 0x47414d49:			/* IMAG */
		case 0x32414d49:			/* IMA2 */
			_reader->Seek(8, SEEK_CUR);
			pent = _reader->GetUInt();
			wide = _reader->GetUInt();
			high = _reader->GetUInt();
			if (wide > _rawWidth && high > _rawHeight)
			{
				switch (pent)
				{
				case  5:
					_loadFlags = 1;
				case  6:
					_loadRaw = LoadRawType::FoveonSdLoadRaw;
					break;
				case 30:
					_loadRaw = LoadRawType::FoveonDpLoadRaw;
					break;
				default:
					_loadRaw = LoadRawType::UnknownLoadRaw;
				}
				_rawWidth = wide;
				_rawHeight = high;
				_dataOffset = off + 28;
				_isFoveon = 1;
			}
			_reader->Seek(off + 28, SEEK_SET);
			if (_reader->GetChar() == 0xff && _reader->GetChar() == 0xd8
				&& _thumbLength < len - 28)
			{
				_thumbOffset = off + 28;
				_thumbLength = len - 28;
				_writeThumb = WriteThumbType::JpegThumb;
			}
			if (++img == 2 && !_thumbLength)
			{
				_thumbOffset = off + 24;
				_thumbWidth = wide;
				_thumbHeight = high;
				_writeThumb = WriteThumbType::FoveonThumb;
			}
			break;
		case 0x464d4143:			/* CAMF */
			_metaOffset = off + 8;
			_metaLength = len - 28;
			break;
		case 0x504f5250:			/* PROP */
			pent = (_reader->GetUInt(), _reader->GetUInt());
			_reader->Seek(12, SEEK_CUR);
			off += pent * 8 + 24;
			if ((unsigned)pent > 256) pent = 256;
			for (i = 0; i < pent * 2; i++)
				((int *)poff)[i] = off + _reader->GetUInt() * 2;
			for (i = 0; i < pent; i++)
			{
				FoveonGets(poff[i][0], name, 64);
				FoveonGets(poff[i][1], value, 64);
				if (!strcmp(name, "ISO"))
					_isoSpeed = atoi(value);
				if (!strcmp(name, "CAMMANUF"))
					strcpy_s(_make, LenMake, value);
				if (!strcmp(name, "CAMMODEL"))
					strcpy_s(_model, LenModel, value);
				if (!strcmp(name, "WB_DESC"))
					strcpy_s(_model2, LenModel2, value);
				if (!strcmp(name, "TIME"))
					_timestamp = atoi(value);
				if (!strcmp(name, "EXPTIME"))
					_shutter = atoi(value) / 1000000.0;
				if (!strcmp(name, "APERTURE"))
					_aperture = atof(value);
				if (!strcmp(name, "FLENGTH"))
					_focalLen = atof(value);
			}
#ifdef LOCALTIME
			_timestamp = mktime(gmtime(&_timestamp));
#endif
		}
		_reader->Seek(save, SEEK_SET);
	}
}

char* CSimpleInfo::FoveonGets(int offset, char* str, int len)
{
	_reader->Seek(offset, SEEK_SET);
	size_t i;
	for (i = 0; i < len - 1; i++)
		if ((str[i] = _reader->GetUShort()) == 0)
			break;
	str[i] = 0;
	return str;
}

void CSimpleInfo::ParseKodakIFD(int base)
{
	int wbi = -2, wbtemp = 6500;
	float mul[3] = { 1,1,1 };
	static const int wbtag[] = { 64037,64040,64039,64041,-1,-1,64042 };

	unsigned entries = _reader->GetUShort();
	if (entries > 1024)
		return;

	unsigned tag, type, len, save;
	while (entries--)
	{
		TiffGet(base, &tag, &type, &len, &save);
		if (tag == 1020)
			wbi = _reader->GetUInt(type);
		if (tag == 1021 && len == 72)
		{		/* WB set in software */
			_reader->Seek(40, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = 2048.0 / _reader->GetUShort();
			wbi = -2;
		}
		if (tag == 2118)
			wbtemp = _reader->GetUInt(type);
		if (tag == 2120 + wbi && wbi >= 0)
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = 2048.0 / _reader->GetReal(type);
		if (tag == 2130 + wbi)
			for (size_t c = 0; c < 3; c++)
				mul[c] = _reader->GetReal(type);
		if (tag == 2140 + wbi && wbi >= 0)
		{
			for (size_t c = 0; c < 3; c++)
			{
				float num = 0;
				for (int i = 0; i < 4; i++)
					num += _reader->GetReal(type) * pow(wbtemp / 100.0, i);
				_camMul[c] = 2048 / (num * mul[c]);
			}
		}
		if (tag == 2317)
			LinearTable(len);
		if (tag == 6020)
			_isoSpeed = _reader->GetUInt(type);
		if (tag == 64013)
			wbi = _reader->GetChar();
		if ((unsigned)wbi < 7 && tag == wbtag[wbi])
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = _reader->GetUInt();
		if (tag == 64019)
			_width = _reader->GetUInt(type);
		if (tag == 64020)
			_height = (_reader->GetUInt(type) + 1) & -2;
		_reader->Seek(save, SEEK_SET);
	}
}

void CSimpleInfo::ParseMos(int offset)
{
	char data[40];
	int i, neut[4], planes = 0, frot = 0;
	static const char *mod[] =
	{ "","DCB2","Volare","Cantare","CMost","Valeo 6","Valeo 11","Valeo 22",
		"Valeo 11p","Valeo 17","","Aptus 17","Aptus 22","Aptus 75","Aptus 65",
		"Aptus 54S","Aptus 65S","Aptus 75S","AFi 5","AFi 6","AFi 7",
		"AFi-II 7","Aptus-II 7","","Aptus-II 6","","","Aptus-II 10","Aptus-II 5",
		"","","","","Aptus-II 10R","Aptus-II 8","","Aptus-II 12","","AFi-II 12" };
	float romm_cam[3][3];

	_reader->Seek(offset, SEEK_SET);
	while (1)
	{
		if (_reader->GetUInt() != 0x504b5453)
			break;
		_reader->GetUInt();
		_reader->Read(data, 1, 40);
		int skip = _reader->GetUInt();
		int from = _reader->GetPosition();
		if (!strcmp(data, "JPEG_preview_data"))
		{
			_thumbOffset = from;
			_thumbLength = skip;
		}
		if (!strcmp(data, "icc_camera_profile"))
		{
			_profileOffset = from;
			_profileLength = skip;
		}
		if (!strcmp(data, "ShootObj_back_type"))
		{
			_reader->GetScanf("%d", &i);
			if ((unsigned)i < sizeof mod / sizeof(*mod))
				strcpy_s(_model, LenModel, mod[i]);
		}
		if (!strcmp(data, "icc_camera_to_tone_matrix"))
		{
			for (size_t i = 0; i < 9; i++)
				((float *)romm_cam)[i] = _reader->IntToFloat(_reader->GetUInt());
			RommCoeff(romm_cam);
		}
		if (!strcmp(data, "CaptProf_color_matrix"))
		{
			for (size_t i = 0; i < 9; i++)
				_reader->GetScanf("%f", (float *)romm_cam + i);
			RommCoeff(romm_cam);
		}
		if (!strcmp(data, "CaptProf_number_of_planes"))
			_reader->GetScanf("%d", &planes);
		if (!strcmp(data, "CaptProf_raw_data_rotation"))
			_reader->GetScanf("%d", &_flip);
		if (!strcmp(data, "CaptProf_mosaic_pattern"))
		{
			for (size_t c = 0; c < 4; c++)
			{
				_reader->GetScanf("%d", &i);
				if (i == 1) frot = c ^ (c >> 1);
			}
			if (!strcmp(data, "ImgProf_rotation_angle"))
			{
				_reader->GetScanf("%d", &i);
				_flip = i - _flip;
			}
		}
		if (!strcmp(data, "NeutObj_neutrals") && !_camMul[0])
		{
			for (size_t c = 0; c < 4; c++)
				_reader->GetScanf("%d", neut + c);
			for (size_t c = 0; c < 3; c++)
				_camMul[c] = (float)neut[0] / neut[c + 1];
		}
		if (!strcmp(data, "Rows_data"))
			_loadFlags = _reader->GetUInt();
		ParseMos(from);
		_reader->Seek(skip + from, SEEK_SET);
	}
	if (planes)
		_filters = (planes == 1) * 0x01010101 *
		(unsigned char) "\x94\x61\x16\x49"[(_flip / 90 + frot) & 3];
}



void CSimpleInfo::SonyDecrypt(unsigned *data, int len, int start, int key)
{
	static unsigned pad[128];
	static unsigned p;

	if (start)
	{
		for (p = 0; p < 4; p++)
			pad[p] = key = key * 48828125 + 1;
		pad[3] = pad[3] << 1 | (pad[0] ^ pad[2]) >> 31;
		for (p = 4; p < 127; p++)
			pad[p] = (pad[p - 4] ^ pad[p - 2]) << 1 | (pad[p - 3] ^ pad[p - 1]) >> 31;
		for (p = 0; p < 127; p++)
			pad[p] = htonl(pad[p]);
	}
	while (len-- && p++)
		*data++ ^= pad[(p - 1) & 127] = pad[p & 127] ^ pad[(p + 64) & 127];
}

void CSimpleInfo::LinearTable(unsigned len)
{
	if (len > 0x1000)
		len = 0x1000;
	_reader->ReadShorts(_curve, len);
	for (size_t i = len; i < 0x1000; i++)
		_curve[i] = _curve[i - 1];
	_maximum = _curve[0xfff];
}

void CSimpleInfo::RommCoeff(float romm_cam[3][3])
{
	static const float rgb_romm[3][3] =	/* ROMM == Kodak ProPhoto */
	{ { 2.034193, -0.727420, -0.306766 },
	{ -0.228811,  1.231729, -0.002922 },
	{ -0.008565, -0.153273,  1.161839 } };
	int i, j, k;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			for (_cMatrix[i][j] = k = 0; k < 3; k++)
				_cMatrix[i][j] += rgb_romm[i][k] * romm_cam[k][j];
}



char* CSimpleInfo::memmem(char* haystack, size_t haystacklen, char* needle, size_t needlelen)
{
	for (char* c = haystack; c <= haystack + haystacklen - needlelen; c++)
		if (!memcmp(c, needle, needlelen))
			return c;
	return 0;
}

char* CSimpleInfo::strcasestr(char* haystack, const char* needle)
{
	for (char* c = haystack; *c; c++)
		if (!_strnicmp(c, needle, strlen(needle)))
			return c;
	return 0;
}
#pragma endregion

unsigned CSimpleInfo::GetBitHuff(int nbits, unsigned short *huff)
{
	static unsigned bitbuf = 0;
	static int vbits = 0;
	static int reset = 0;

	if (nbits > 25)
		return 0;

	if (nbits < 0)
		return bitbuf = vbits = reset = 0;

	if (nbits == 0 || vbits < 0)
		return 0;

	unsigned c;
	while (!reset && vbits < nbits && (c = _reader->GetChar()) != EOF &&
		!(reset = _zeroAfterFF && c == 0xff && _reader->GetChar()))
	{
		bitbuf = (bitbuf << 8) + (unsigned char)c;
		vbits += 8;
	}

	c = bitbuf << (32 - vbits) >> (32 - nbits);
	if (huff != nullptr)
	{
		vbits -= huff[c] >> 8;
		c = (unsigned char)huff[c];
	}
	else
	{
		vbits -= nbits;
	}

	if (vbits < 0)
		throw CExceptionFile();

	return c;
}

