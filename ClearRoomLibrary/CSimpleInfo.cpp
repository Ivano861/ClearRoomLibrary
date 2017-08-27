#include "Stdafx.h"
#include "CSimpleInfo.h"
#include "jhead.h"
#include "CException.h"
#include "CAutoFreeMemory.h"
#include <time.h>
#include "Macro.h"

using namespace Unmanaged;

#pragma region Costructors
CSimpleInfo::CSimpleInfo(CReader* reader) : _reader(reader), meta_data(nullptr), raw_image(nullptr), image(nullptr)
{
}
#pragma endregion

#pragma region Destructor
CSimpleInfo::~CSimpleInfo()
{
	if (meta_data)
	{
		free(meta_data);
		meta_data = nullptr;
	}
	if (raw_image)
	{
		free(raw_image);
		raw_image = nullptr;
	}
	if (image)
	{
		free(image);
		image = nullptr;
	}
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

	tiff_flip = flip = filters = UINT_MAX;	/* unknown */
	raw_height = raw_width = fuji_width = fuji_layout = cr2_slice[0] = 0;
	maximum = height = width = top_margin = left_margin = 0;
	cdesc[0] = desc[0] = artist[0] = make[0] = model[0] = model2[0] = 0;
	iso_speed = shutter = aperture = focal_len = 0;
	unique_id = 0;
	tiff_nifds = 0;
	memset(tiff_ifd, 0, sizeof tiff_ifd);
	memset(gpsdata, 0, sizeof gpsdata);
	memset(cblack, 0, sizeof cblack);
	memset(white, 0, sizeof white);
	memset(mask, 0, sizeof mask);
	thumb_offset = thumb_length = thumb_width = thumb_height = 0;
	load_raw = LoadRawType::unknown_load_raw;
	thumb_load_raw = LoadRawType::unknown_load_raw;
	write_thumb = WriteThumbType::jpeg_thumb;
	data_offset = meta_offset = meta_length = tiff_bps = tiff_compress = 0;
	kodak_cbpp = zero_after_ff = dng_version = load_flags = 0;
	timestamp = shot_order = tiff_samples = black = is_foveon = 0;
	mix_green = profile_length = data_error = zero_is_bad = 0;
	pixel_aspect = is_raw = raw_color = 1;
	tile_width = tile_length = 0;
	for (size_t i = 0; i < 4; i++)
	{
		cam_mul[i] = i == 1;
		pre_mul[i] = i < 3;
		for (size_t c = 0; c < 3; c++)
			cmatrix[c][i] = 0;
		for (size_t c = 0; c < 3; c++)
			rgb_cam[c][i] = c == i;
	}
	colors = 3;
	for (size_t i = 0; i < 0x10000; i++)
		curve[i] = static_cast<unsigned short>(i);

	tiff_nifds = 0;

	_reader->SetOrder(_reader->get2());
	int hlen = _reader->get4();

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
		parse_phase_one(cp - head);
		if (cp - head && parse_tiff(0))
			apply_tiff();
	}
	else if (_reader->GetOrder() == 0x4949 || _reader->GetOrder() == 0x4d4d)
	{
		if (!memcmp(head + 6, "HEAPCCDR", 8))
		{
			data_offset = hlen;
			parse_ciff(hlen, flen - hlen, 0);
			load_raw = LoadRawType::canon_load_raw;
		}
		else if (parse_tiff(0))
			apply_tiff();
	}
	else if (!memcmp(head, "\xff\xd8\xff\xe1", 4) &&
		!memcmp(head + 6, "Exif", 4))
	{
		_reader->Seek(4, SEEK_SET);
		data_offset = 4 + _reader->get2();
		_reader->Seek(data_offset, SEEK_SET);
		if (_reader->GetChar() != 0xff)
			parse_tiff(12);
		thumb_offset = 0;
	}
	else if (!memcmp(head + 25, "ARECOYK", 7))
	{
		strcpy_s(make, LenMake, "Contax");
		strcpy_s(model, LenModel, "N Digital");
		_reader->Seek(33, SEEK_SET);
		get_timestamp(1);
		_reader->Seek(60, SEEK_SET);
		for (size_t c = 0; c < 4; c++)
			cam_mul[c ^ (c >> 1)] = _reader->get4();
	}
	else if (!strcmp(head, "PXN"))
	{
		strcpy_s(make, LenMake, "Logitech");
		strcpy_s(model, LenModel, "Fotoman Pixtura");
	}
	else if (!strcmp(head, "qktk"))
	{
		strcpy_s(make, LenMake, "Apple");
		strcpy_s(model, LenModel, "QuickTake 100");
		load_raw = LoadRawType::quicktake_100_load_raw;
	}
	else if (!strcmp(head, "qktn"))
	{
		strcpy_s(make, LenMake, "Apple");
		strcpy_s(model, LenModel, "QuickTake 150");
		load_raw = LoadRawType::kodak_radc_load_raw;
	}
	else if (!memcmp(head, "FUJIFILM", 8))
	{
		_reader->Seek(84, SEEK_SET);
		thumb_offset = _reader->get4();
		thumb_length = _reader->get4();
		_reader->Seek(92, SEEK_SET);
		parse_fuji(_reader->get4());
		if (thumb_offset > 120)
		{
			int i;
			_reader->Seek(120, SEEK_SET);
			is_raw += (i = _reader->get4()) && 1;
			if (is_raw == 2 && shot_select)
				parse_fuji(i);
		}
		load_raw = LoadRawType::unpacked_load_raw;
		_reader->Seek(100 + 28 * (shot_select > 0), SEEK_SET);
		parse_tiff(data_offset = _reader->get4());
		parse_tiff(thumb_offset + 12);
		apply_tiff();
	}
	else if (!memcmp(head, "RIFF", 4))
	{
		_reader->Seek(0, SEEK_SET);
		parse_riff();
	}
	else if (!memcmp(head + 4, "ftypqt   ", 9))
	{
		_reader->Seek(0, SEEK_SET);
		parse_qt(fsize);
		is_raw = 0;
	}
	else if (!memcmp(head, "\0\001\0\001\0@", 6))
	{
		_reader->Seek(6, SEEK_SET);
		_reader->Read(make, 1, 8);
		_reader->Read(model, 1, 8);
		_reader->Read(model2, 1, 16);
		data_offset = _reader->get2();
		_reader->get2();
		raw_width = _reader->get2();
		raw_height = _reader->get2();
		load_raw = LoadRawType::nokia_load_raw;
		filters = 0x61616161;
	}
	else if (!memcmp(head, "NOKIARAW", 8))
	{
		strcpy_s(make, LenMake, "NOKIA");
		_reader->SetOrder(0x4949);
		_reader->Seek(300, SEEK_SET);
		data_offset = _reader->get4();
		unsigned int i = _reader->get4();
		width = _reader->get2();
		height = _reader->get2();
		tiff_bps = i * 8 / (width * height);
		switch (tiff_bps)
		{
		case  8:
			load_raw = LoadRawType::eight_bit_load_raw;
			break;
		case 10:
			load_raw = LoadRawType::nokia_load_raw;
		}
		top_margin = i / (width * tiff_bps / 8) - height;
		raw_height = height + top_margin;
		mask[0][3] = 1;
		filters = 0x61616161;
	}
	else if (!memcmp(head, "ARRI", 4))
	{
		_reader->SetOrder(0x4949);
		_reader->Seek(20, SEEK_SET);
		width = _reader->get4();
		height = _reader->get4();
		strcpy_s(make, LenMake, "ARRI");
		_reader->Seek(668, SEEK_SET);
		_reader->Read(model, 1, LenModel);
		data_offset = 4096;
		load_raw = LoadRawType::packed_load_raw;
		load_flags = 88;
		filters = 0x61616161;
	}
	else if (!memcmp(head, "XPDS", 4))
	{
		_reader->SetOrder( 0x4949);
		_reader->Seek(0x800, SEEK_SET);
		_reader->Read(make, 1, 41);
		raw_height = _reader->get2();
		raw_width = _reader->get2();
		_reader->Seek(56, SEEK_CUR);
		_reader->Read(model, 1, 30);
		data_offset = 0x10000;
		load_raw = LoadRawType::canon_rmf_load_raw;
		gamma_curve(0, 12.25, 1, 1023);
	}
	else if (!memcmp(head + 4, "RED1", 4))
	{
		strcpy_s(make, LenMake, "Red");
		strcpy_s(model, LenModel, "One");
		parse_redcine();
		load_raw = LoadRawType::redcine_load_raw;
		gamma_curve(1 / 2.4, 12.92, 1, 4095);
		filters = 0x49494949;
	}
	else if (!memcmp(head, "DSC-Image", 9))
		parse_rollei();
	else if (!memcmp(head, "PWAD", 4))
		parse_sinar_ia();
	else if (!memcmp(head, "\0MRM", 4))
		parse_minolta(0);
	else if (!memcmp(head, "FOVb", 4))
		parse_foveon();
	else if (!memcmp(head, "CI", 2))
		parse_cine();

	int zero_fsize = 1;
	if (make[0] == 0)
	{
		zero_fsize = 0;
		for (size_t i = 0; i < sizeof table / sizeof *table; i++)
		{
			if (fsize == table[i].fsize)
			{
				strcpy_s(make, LenMake, table[i].make);
				strcpy_s(model, LenModel, table[i].model);
				flip = table[i].flags >> 2;
				zero_is_bad = table[i].flags & 2;
				if (table[i].flags & 1)
					parse_external_jpeg();
				data_offset = table[i].offset;
				raw_width = table[i].rw;
				raw_height = table[i].rh;
				left_margin = table[i].lm;
				top_margin = table[i].tm;
				width = raw_width - left_margin - table[i].rm;
				height = raw_height - top_margin - table[i].bm;
				filters = 0x1010101 * table[i].cf;
				colors = 4 - !((filters & filters >> 1) & 0x5555);
				load_flags = table[i].lf;
				tiff_bps = (fsize - data_offset) * 8 / (raw_width*raw_height);
				switch (tiff_bps)
				{
				case 6:
					load_raw = LoadRawType::minolta_rd175_load_raw;
					break;
				case 8:
					load_raw = LoadRawType::eight_bit_load_raw;
					break;
				case 10: case 12:
					load_flags |= 128;
					load_raw = LoadRawType::packed_load_raw;
					break;
				case 16:
					_reader->SetOrder(0x4949 | 0x404 * (load_flags & 1));
					tiff_bps -= load_flags >> 4;
					tiff_bps -= load_flags = load_flags >> 1 & 7;
					load_raw = LoadRawType::unpacked_load_raw;
				}
				maximum = (1 << tiff_bps) - (1 << table[i].max);
			}
		}
	}
	if (zero_fsize)
		fsize = 0;
	if (make[0] == 0)
	{
		parse_smal(0, flen);
	}
	if (make[0] == 0)
	{
		parse_jpeg(0);
		if (!(strncmp(model, "ov", 2) && strncmp(model, "RP_OV", 5)) &&
			!_reader->Seek(-6404096, SEEK_END) &&
			_reader->Read(head, 1, 32) && !strcmp(head, "BRCMn"))
		{
			strcpy_s(make, LenMake, "OmniVision");
			data_offset = _reader->GetPosition() + 0x8000 - 32;
			width = raw_width;
			raw_width = 2611;
			load_raw = LoadRawType::nokia_load_raw;
			filters = 0x16161616;
		}
		else
		{
			is_raw = 0;
		}
	}

	for (size_t i = 0; i < sizeof corp / sizeof *corp; i++)
		if (strcasestr(make, corp[i]))	/* Simplify company names */
			strcpy_s(make, LenMake, corp[i]);
	if ((!strcmp(make, "Kodak") || !strcmp(make, "Leica")) &&
		((cp = strcasestr(model, " DIGITAL CAMERA")) ||
		(cp = strstr(model, "FILE VERSION"))))
	{
		*cp = 0;
	}
	if (!_strnicmp(model, "PENTAX", 6))
		strcpy_s(make, LenMake, "Pentax");
	cp = make + strlen(make);		/* Remove trailing spaces */
	while (*--cp == ' ')
		*cp = 0;
	cp = model + strlen(model);
	while (*--cp == ' ')
		*cp = 0;
	size_t ii = strlen(make);			/* Remove make from model */
	if (!_strnicmp(model, make, ii) && model[ii++] == ' ')
		memmove(model, model + ii, LenModel - ii);
	if (!strncmp(model, "FinePix ", 8))
		strcpy_s(model, LenModel, model + 8);
	if (!strncmp(model, "Digital Camera ", 15))
		strcpy_s(model, LenModel, model + 15);
	desc[511] = artist[63] = make[63] = model[63] = model2[63] = 0;
	if (!is_raw)
		goto notraw;

	if (!height)
		height = raw_height;
	if (!width)
		width = raw_width;
	if (height == 2624 && width == 3936)	/* Pentax K10D and Samsung GX10 */
	{
		height = 2616;   width = 3896;
	}
	if (height == 3136 && width == 4864)  /* Pentax K20D and Samsung GX20 */
	{
		height = 3124;   width = 4688; filters = 0x16161616;
	}
	if (width == 4352 && (!strcmp(model, "K-r") || !strcmp(model, "K-x")))
	{
		width = 4309; filters = 0x16161616;
	}
	if (width >= 4960 && !strncmp(model, "K-5", 3))
	{
		left_margin = 10; width = 4950; filters = 0x16161616;
	}
	if (width == 4736 && !strcmp(model, "K-7"))
	{
		height = 3122;   width = 4684; filters = 0x16161616; top_margin = 2;
	}
	if (width == 6080 && !strcmp(model, "K-3"))
	{
		left_margin = 4;  width = 6040;
	}
	if (width == 7424 && !strcmp(model, "645D"))
	{
		height = 5502;   width = 7328; filters = 0x61616161; top_margin = 29;
		left_margin = 48;
	}
	if (height == 3014 && width == 4096)	/* Ricoh GX200 */
		width = 4014;
	if (dng_version)
	{
		if (filters == UINT_MAX)
			filters = 0;
		if (filters)
			is_raw *= tiff_samples;
		else
			colors = tiff_samples;
		switch (tiff_compress)
		{
		case 0:
		case 1:
			load_raw = LoadRawType::packed_dng_load_raw;
			break;
		case 7:
			load_raw = LoadRawType::lossless_dng_load_raw;
			break;
		case 34892:
			load_raw = LoadRawType::lossy_dng_load_raw;
			break;
		default:
			load_raw = LoadRawType::unknown_load_raw;
		}
		goto dng_skip;
	}
	if (!strcmp(make, "Canon") && !fsize && tiff_bps != 15)
	{
		if (!load_raw)
			load_raw = LoadRawType::lossless_jpeg_load_raw;
		for (size_t i = 0; i < sizeof canon / sizeof *canon; i++)
		{
			if (raw_width == canon[i][0] && raw_height == canon[i][1])
			{
				width = raw_width - (left_margin = canon[i][2]);
				height = raw_height - (top_margin = canon[i][3]);
				width -= canon[i][4];
				height -= canon[i][5];
				mask[0][1] = canon[i][6];
				mask[0][3] = -canon[i][7];
				mask[1][1] = canon[i][8];
				mask[1][3] = -canon[i][9];
				if (canon[i][10])
					filters = canon[i][10] * 0x01010101;
			}
		}
		if ((unique_id | 0x20000) == 0x2720000)
		{
			left_margin = 8;
			top_margin = 16;
		}
	}
	for (size_t i = 0; i < sizeof unique / sizeof *unique; i++)
	{
		if (unique_id == 0x80000000 + unique[i].id)
		{
			adobe_coeff("Canon", unique[i].model);
			if (model[4] == 'K' && strlen(model) == 8)
				strcpy_s(model, LenModel, unique[i].model);
		}
	}
	for (size_t i = 0; i < sizeof sonique / sizeof *sonique; i++)
	{
		if (unique_id == sonique[i].id)
			strcpy_s(model, LenModel, sonique[i].model);
	}
	if (!strcmp(make, "Nikon"))
	{
		if (!load_raw)
			load_raw = LoadRawType::packed_load_raw;
		if (model[0] == 'E')
			load_flags |= !data_offset << 2 | 2;
	}

	/* Set parameters based on camera name (for non-DNG files). */

	if (!strcmp(model, "KAI-0340")
		&& find_green(16, 16, 3840, 5120) < 25)
	{
		height = 480;
		top_margin = filters = 0;
		strcpy_s(model, LenModel, "C603");
	}
	if (!strcmp(make, "Sony") && raw_width > 3888)
		black = 128 << (tiff_bps - 12);
	if (is_foveon)
	{
		if (height * 2 < width) pixel_aspect = 0.5;
		if (height   > width) pixel_aspect = 2;
		filters = 0;
		simple_coeff(0);
	}
	else if (!strcmp(make, "Canon") && tiff_bps == 15)
	{
		switch (width)
		{
		case 3344: width -= 66;
		case 3872: width -= 6;
		}
		if (height > width)
		{
			SWAP(height, width);
			SWAP(raw_height, raw_width);
		}
		if (width == 7200 && height == 3888)
		{
			raw_width = width = 6480;
			raw_height = height = 4320;
		}
		filters = 0;
		tiff_samples = colors = 3;
		load_raw = LoadRawType::canon_sraw_load_raw;
	}
	else if (!strcmp(model, "PowerShot 600"))
	{
		height = 613;
		width = 854;
		raw_width = 896;
		colors = 4;
		filters = 0xe1e4e1e4;
		load_raw = LoadRawType::canon_600_load_raw;
	}
	else if (!strcmp(model, "PowerShot A5") ||
		!strcmp(model, "PowerShot A5 Zoom"))
	{
		height = 773;
		width = 960;
		raw_width = 992;
		pixel_aspect = 256 / 235.0;
		filters = 0x1e4e1e4e;
		goto canon_a5;
	}
	else if (!strcmp(model, "PowerShot A50"))
	{
		height = 968;
		width = 1290;
		raw_width = 1320;
		filters = 0x1b4e4b1e;
		goto canon_a5;
	}
	else if (!strcmp(model, "PowerShot Pro70"))
	{
		height = 1024;
		width = 1552;
		filters = 0x1e4b4e1b;
	canon_a5:
		colors = 4;
		tiff_bps = 10;
		load_raw = LoadRawType::packed_load_raw;
		load_flags = 40;
	}
	else if (!strcmp(model, "PowerShot Pro90 IS") ||
		!strcmp(model, "PowerShot G1"))
	{
		colors = 4;
		filters = 0xb4b4b4b4;
	}
	else if (!strcmp(model, "PowerShot A610"))
	{
		if (canon_s2is())
			strcpy_s(model + 10, LenModel - 10, "S2 IS");
	}
	else if (!strcmp(model, "PowerShot SX220 HS"))
	{
		mask[1][3] = -4;
	}
	else if (!strcmp(model, "EOS D2000C"))
	{
		filters = 0x61616161;
		black = curve[200];
	}
	else if (!strcmp(model, "D1"))
	{
		cam_mul[0] *= 256 / 527.0;
		cam_mul[2] *= 256 / 317.0;
	}
	else if (!strcmp(model, "D1X"))
	{
		width -= 4;
		pixel_aspect = 0.5;
	}
	else if (!strcmp(model, "D40X") ||
		!strcmp(model, "D60") ||
		!strcmp(model, "D80") ||
		!strcmp(model, "D3000"))
	{
		height -= 3;
		width -= 4;
	}
	else if (!strcmp(model, "D3") ||
		!strcmp(model, "D3S") ||
		!strcmp(model, "D700"))
	{
		width -= 4;
		left_margin = 2;
	}
	else if (!strcmp(model, "D3100"))
	{
		width -= 28;
		left_margin = 6;
	}
	else if (!strcmp(model, "D5000") ||
		!strcmp(model, "D90"))
	{
		width -= 42;
	}
	else if (!strcmp(model, "D5100") ||
		!strcmp(model, "D7000") ||
		!strcmp(model, "COOLPIX A"))
	{
		width -= 44;
	}
	else if (!strcmp(model, "D3200") ||
		!strncmp(model, "D6", 2) ||
		!strncmp(model, "D800", 4))
	{
		width -= 46;
	}
	else if (!strcmp(model, "D4") ||
		!strcmp(model, "Df"))
	{
		width -= 52;
		left_margin = 2;
	}
	else if (!strncmp(model, "D40", 3) ||
		!strncmp(model, "D50", 3) ||
		!strncmp(model, "D70", 3))
	{
		width--;
	}
	else if (!strcmp(model, "D100"))
	{
		if (load_flags)
			raw_width = (width += 3) + 3;
	}
	else if (!strcmp(model, "D200"))
	{
		left_margin = 1;
		width -= 4;
		filters = 0x94949494;
	}
	else if (!strncmp(model, "D2H", 3))
	{
		left_margin = 6;
		width -= 14;
	}
	else if (!strncmp(model, "D2X", 3))
	{
		if (width == 3264) width -= 32;
		else width -= 8;
	}
	else if (!strncmp(model, "D300", 4))
	{
		width -= 32;
	}
	else if (!strncmp(model, "COOLPIX P", 9) && raw_width != 4032)
	{
		load_flags = 24;
		filters = 0x94949494;
		if (model[9] == '7' && iso_speed >= 400)
			black = 255;
	}
	else if (!strncmp(model, "1 ", 2))
	{
		height -= 2;
	}
	else if (fsize == 1581060)
	{
		simple_coeff(3);
		pre_mul[0] = 1.2085;
		pre_mul[1] = 1.0943;
		pre_mul[3] = 1.1103;
	}
	else if (fsize == 3178560)
	{
		cam_mul[0] *= 4;
		cam_mul[2] *= 4;
	}
	else if (fsize == 4771840)
	{
		if (!timestamp && nikon_e995())
			strcpy_s(model, LenModel, "E995");
		if (strcmp(model, "E995"))
		{
			filters = 0xb4b4b4b4;
			simple_coeff(3);
			pre_mul[0] = 1.196;
			pre_mul[1] = 1.246;
			pre_mul[2] = 1.018;
		}
	}
	else if (fsize == 2940928)
	{
		if (!timestamp && !nikon_e2100())
			strcpy_s(model, LenModel, "E2500");
		if (!strcmp(model, "E2500"))
		{
			height -= 2;
			load_flags = 6;
			colors = 4;
			filters = 0x4b4b4b4b;
		}
	}
	else if (fsize == 4775936)
	{
		if (!timestamp)
			nikon_3700();
		if (model[0] == 'E' && atoi(model + 1) < 3700)
			filters = 0x49494949;
		if (!strcmp(model, "Optio 33WR"))
		{
			flip = 1;
			filters = 0x16161616;
		}
		if (make[0] == 'O')
		{
			int i = find_green(12, 32, 1188864, 3576832);
			int c = find_green(12, 32, 2383920, 2387016);
			if (abs(i) < abs(c))
			{
				SWAP(i, c);
				load_flags = 24;
			}
			if (i < 0) filters = 0x61616161;
		}
	}
	else if (fsize == 5869568)
	{
		if (!timestamp && minolta_z2())
		{
			strcpy_s(make, LenMake, "Minolta");
			strcpy_s(model, LenModel, "DiMAGE Z2");
		}
		load_flags = 6 + 24 * (make[0] == 'M');
	}
	else if (fsize == 6291456)
	{
		_reader->Seek(0x300000, SEEK_SET);
		_reader->SetOrder(guess_byte_order(0x10000));
		if (_reader->GetOrder() == 0x4d4d)
		{
			height -= (top_margin = 16);
			width -= (left_margin = 28);
			maximum = 0xf5c0;
			strcpy_s(make, LenMake, "ISG");
			model[0] = 0;
		}
	}
	else if (!strcmp(make, "Fujifilm"))
	{
		if (!strcmp(model + 7, "S2Pro"))
		{
			strcpy_s(model, LenModel, "S2Pro");
			height = 2144;
			width = 2880;
			flip = 6;
		}
		else if (load_raw != LoadRawType::packed_load_raw)
			maximum = (is_raw == 2 && shot_select) ? 0x2f00 : 0x3e00;
		top_margin = (raw_height - height) >> 2 << 1;
		left_margin = (raw_width - width) >> 2 << 1;
		if (width == 2848 || width == 3664) filters = 0x16161616;
		if (width == 4032 || width == 4952 || width == 6032) left_margin = 0;
		if (width == 3328 && (width -= 66)) left_margin = 34;
		if (width == 4936) left_margin = 4;
		if (!strcmp(model, "HS50EXR") ||
			!strcmp(model, "F900EXR"))
		{
			width += 2;
			left_margin = 0;
			filters = 0x16161616;
		}
		if (fuji_layout) raw_width *= is_raw;
		if (filters == 9)
			for (size_t c = 0; c < 36; c++)
				((char *)xtrans)[c] = xtrans_abs[(c / 6 + top_margin) % 6][(c + left_margin) % 6];
	}
	else if (!strcmp(model, "KD-400Z"))
	{
		height = 1712;
		width = 2312;
		raw_width = 2336;
		goto konica_400z;
	}
	else if (!strcmp(model, "KD-510Z"))
	{
		goto konica_510z;
	}
	else if (!_stricmp(make, "Minolta"))
	{
		if (!load_raw && (maximum = 0xfff))
			load_raw = LoadRawType::unpacked_load_raw;
		if (!strncmp(model, "DiMAGE A", 8))
		{
			if (!strcmp(model, "DiMAGE A200"))
				filters = 0x49494949;
			tiff_bps = 12;
			load_raw = LoadRawType::packed_load_raw;
		}
		else if (!strncmp(model, "ALPHA", 5) ||
			!strncmp(model, "DYNAX", 5) ||
			!strncmp(model, "MAXXUM", 6))
		{
			sprintf_s(model + 20, LenModel - 20, "DYNAX %-10s", model + 6 + (model[0] == 'M'));
			adobe_coeff(make, model + 20);
			load_raw = LoadRawType::packed_load_raw;
		}
		else if (!strncmp(model, "DiMAGE G", 8))
		{
			if (model[8] == '4')
			{
				height = 1716;
				width = 2304;
			}
			else if (model[8] == '5')
			{
			konica_510z:
				height = 1956;
				width = 2607;
				raw_width = 2624;
			}
			else if (model[8] == '6')
			{
				height = 2136;
				width = 2848;
			}
			data_offset += 14;
			filters = 0x61616161;
		konica_400z:
			load_raw = LoadRawType::unpacked_load_raw;
			maximum = 0x3df;
			_reader->SetOrder(0x4d4d);
		}
	}
	else if (!strcmp(model, "*ist D"))
	{
		load_raw = LoadRawType::unpacked_load_raw;
		data_error = -1;	// TODO: ignore first file error
	}
	else if (!strcmp(model, "*ist DS"))
	{
		height -= 2;
	}
	else if (!strcmp(make, "Samsung") && raw_width == 4704)
	{
		height -= top_margin = 8;
		width -= 2 * (left_margin = 8);
		load_flags = 32;
	}
	else if (!strcmp(make, "Samsung") && raw_height == 3714)
	{
		height -= top_margin = 18;
		left_margin = raw_width - (width = 5536);
		if (raw_width != 5600)
			left_margin = top_margin = 0;
		filters = 0x61616161;
		colors = 3;
	}
	else if (!strcmp(make, "Samsung") && raw_width == 5632)
	{
		_reader->SetOrder(0x4949);
		height = 3694;
		top_margin = 2;
		width = 5574 - (left_margin = 32 + tiff_bps);
		if (tiff_bps == 12) load_flags = 80;
	}
	else if (!strcmp(make, "Samsung") && raw_width == 5664)
	{
		height -= top_margin = 17;
		left_margin = 96;
		width = 5544;
		filters = 0x49494949;
	}
	else if (!strcmp(make, "Samsung") && raw_width == 6496)
	{
		filters = 0x61616161;
		black = 1 << (tiff_bps - 7);
	}
	else if (!strcmp(model, "EX1"))
	{
		_reader->SetOrder(0x4949);
		height -= 20;
		top_margin = 2;
		if ((width -= 6) > 3682)
		{
			height -= 10;
			width -= 46;
			top_margin = 8;
		}
	}
	else if (!strcmp(model, "WB2000"))
	{
		_reader->SetOrder(0x4949);
		height -= 3;
		top_margin = 2;
		if ((width -= 10) > 3718)
		{
			height -= 28;
			width -= 56;
			top_margin = 8;
		}
	}
	else if (strstr(model, "WB550"))
	{
		strcpy_s(model, LenModel, "WB550");
	}
	else if (!strcmp(model, "EX2F"))
	{
		height = 3045;
		width = 4070;
		top_margin = 3;
		_reader->SetOrder(0x4949);
		filters = 0x49494949;
		load_raw = LoadRawType::unpacked_load_raw;
	}
	else if (!strcmp(model, "STV680 VGA"))
	{
		black = 16;
	}
	else if (!strcmp(model, "N95"))
	{
		height = raw_height - (top_margin = 2);
	}
	else if (!strcmp(model, "640x480"))
	{
		gamma_curve(0.45, 4.5, 1, 255);
	}
	else if (!strcmp(make, "Hasselblad"))
	{
		if (load_raw == LoadRawType::lossless_jpeg_load_raw)
			load_raw = LoadRawType::hasselblad_load_raw;
		if (raw_width == 7262)
		{
			height = 5444;
			width = 7248;
			top_margin = 4;
			left_margin = 7;
			filters = 0x61616161;
		}
		else if (raw_width == 7410 || raw_width == 8282)
		{
			height -= 84;
			width -= 82;
			top_margin = 4;
			left_margin = 41;
			filters = 0x61616161;
		}
		else if (raw_width == 9044)
		{
			height = 6716;
			width = 8964;
			top_margin = 8;
			left_margin = 40;
			black += load_flags = 256;
			maximum = 0x8101;
		}
		else if (raw_width == 4090)
		{
			strcpy_s(model, LenModel, "V96C");
			height -= (top_margin = 6);
			width -= (left_margin = 3) + 7;
			filters = 0x61616161;
		}
		if (tiff_samples > 1)
		{
			is_raw = tiff_samples + 1;
			if (!shot_select && !half_size) filters = 0;
		}
	}
	else if (!strcmp(make, "Sinar"))
	{
		if (!load_raw) load_raw = LoadRawType::unpacked_load_raw;
		if (is_raw > 1 && !shot_select && !half_size) filters = 0;
		maximum = 0x3fff;
	}
	else if (!strcmp(make, "Leaf"))
	{
		maximum = 0x3fff;
		_reader->Seek(data_offset, SEEK_SET);
		jhead jh(*_reader, *this, true);
		if (jh._success && jh.jh.bits == 15)
			maximum = 0x1fff;
		if (tiff_samples > 1) filters = 0;
		if (tiff_samples > 1 || tile_length < raw_height)
		{
			load_raw = LoadRawType::leaf_hdr_load_raw;
			raw_width = tile_width;
		}
		if ((width | height) == 2048)
		{
			if (tiff_samples == 1)
			{
				filters = 1;
				strcpy_s(cdesc, LenCDesc, "RBTG");
				strcpy_s(model, LenModel, "CatchLight");
				top_margin = 8; left_margin = 18; height = 2032; width = 2016;
			}
			else
			{
				strcpy_s(model, LenModel, "DCB2");
				top_margin = 10; left_margin = 16; height = 2028; width = 2022;
			}
		}
		else if (width + height == 3144 + 2060)
		{
			if (!model[0])
				strcpy_s(model, LenModel, "Cantare");
			if (width > height)
			{
				top_margin = 6; left_margin = 32; height = 2048;  width = 3072;
				filters = 0x61616161;
			}
			else
			{
				left_margin = 6;  top_margin = 32;  width = 2048; height = 3072;
				filters = 0x16161616;
			}
			if (!cam_mul[0] || model[0] == 'V') filters = 0;
			else is_raw = tiff_samples;
		}
		else if (width == 2116)
		{
			strcpy_s(model, LenModel, "Valeo 6");
			height -= 2 * (top_margin = 30);
			width -= 2 * (left_margin = 55);
			filters = 0x49494949;
		}
		else if (width == 3171)
		{
			strcpy_s(model, LenModel, "Valeo 6");
			height -= 2 * (top_margin = 24);
			width -= 2 * (left_margin = 24);
			filters = 0x16161616;
		}
	}
	else if (!strcmp(make, "Leica") || !strcmp(make, "Panasonic"))
	{
		if ((flen - data_offset) / (raw_width * 8 / 7) == raw_height)
			load_raw = LoadRawType::panasonic_load_raw;
		if (!load_raw)
		{
			load_raw = LoadRawType::unpacked_load_raw;
			load_flags = 4;
		}
		zero_is_bad = 1;
		if ((height += 12) > raw_height) height = raw_height;
		for (size_t i = 0; i < sizeof pana / sizeof *pana; i++)
			if (raw_width == pana[i][0] && raw_height == pana[i][1])
			{
				left_margin = pana[i][2];
				top_margin = pana[i][3];
				width += pana[i][4];
				height += pana[i][5];
			}
		filters = 0x01010101 * (unsigned char) "\x94\x61\x49\x16"
			[((filters - 1) ^ (left_margin & 1) ^ (top_margin << 1)) & 3];
	}
	else if (!strcmp(model, "C770UZ"))
	{
		height = 1718;
		width = 2304;
		filters = 0x16161616;
		load_raw = LoadRawType::packed_load_raw;
		load_flags = 30;
	}
	else if (!strcmp(make, "Olympus"))
	{
		height += height & 1;
		if (exif_cfa) filters = exif_cfa;
		if (width == 4100) width -= 4;
		if (width == 4080) width -= 24;
		if (width == 9280)
		{
			width -= 6; height -= 6;
		}
		if (load_raw == LoadRawType::unpacked_load_raw)
			load_flags = 4;
		tiff_bps = 12;
		if (!strcmp(model, "E-300") ||
			!strcmp(model, "E-500"))
		{
			width -= 20;
			if (load_raw == LoadRawType::unpacked_load_raw)
			{
				maximum = 0xfc3;
				memset(cblack, 0, sizeof cblack);
			}
		}
		else if (!strcmp(model, "E-330"))
		{
			width -= 30;
			if (load_raw == LoadRawType::unpacked_load_raw)
				maximum = 0xf79;
		}
		else if (!strcmp(model, "SP550UZ"))
		{
			thumb_length = flen - (thumb_offset = 0xa39800);
			thumb_height = 480;
			thumb_width = 640;
		}
		else if (!strcmp(model, "TG-4"))
		{
			width -= 16;
		}
	}
	else if (!strcmp(model, "N Digital"))
	{
		height = 2047;
		width = 3072;
		filters = 0x61616161;
		data_offset = 0x1a00;
		load_raw = LoadRawType::packed_load_raw;
	}
	else if (!strcmp(model, "DSC-F828"))
	{
		width = 3288;
		left_margin = 5;
		mask[1][3] = -17;
		data_offset = 862144;
		load_raw = LoadRawType::sony_load_raw;
		filters = 0x9c9c9c9c;
		colors = 4;
		strcpy_s(cdesc, LenCDesc, "RGBE");
	}
	else if (!strcmp(model, "DSC-V3"))
	{
		width = 3109;
		left_margin = 59;
		mask[0][1] = 9;
		data_offset = 787392;
		load_raw = LoadRawType::sony_load_raw;
	}
	else if (!strcmp(make, "Sony") && raw_width == 3984)
	{
		width = 3925;
		_reader->SetOrder(0x4d4d);
	}
	else if (!strcmp(make, "Sony") && raw_width == 4288)
	{
		width -= 32;
	}
	else if (!strcmp(make, "Sony") && raw_width == 4600)
	{
		if (!strcmp(model, "DSLR-A350"))
			height -= 4;
		black = 0;
	}
	else if (!strcmp(make, "Sony") && raw_width == 4928)
	{
		if (height < 3280) width -= 8;
	}
	else if (!strcmp(make, "Sony") && raw_width == 5504)
	{
		width -= height > 3664 ? 8 : 32;
		if (!strncmp(model, "DSC", 3))
			black = 200 << (tiff_bps - 12);
	}
	else if (!strcmp(make, "Sony") && raw_width == 6048)
	{
		width -= 24;
		if (strstr(model, "RX1") || strstr(model, "A99"))
			width -= 6;
	}
	else if (!strcmp(make, "Sony") && raw_width == 7392)
	{
		width -= 30;
	}
	else if (!strcmp(make, "Sony") && raw_width == 8000)
	{
		width -= 32;
		if (!strncmp(model, "DSC", 3))
		{
			tiff_bps = 14;
			load_raw = LoadRawType::unpacked_load_raw;
			black = 512;
		}
	}
	else if (!strcmp(model, "DSLR-A100"))
	{
		if (width == 3880)
		{
			height--;
			width = ++raw_width;
		}
		else
		{
			height -= 4;
			width -= 4;
			_reader->SetOrder(0x4d4d);
			load_flags = 2;
		}
		filters = 0x61616161;
	}
	else if (!strcmp(model, "PIXL"))
	{
		height -= top_margin = 4;
		width -= left_margin = 32;
		gamma_curve(0, 7, 1, 255);
	}
	else if (!strcmp(model, "C603") || !strcmp(model, "C330")
		|| !strcmp(model, "12MP"))
	{
		_reader->SetOrder(0x4949);
		if (filters && data_offset)
		{
			_reader->Seek(data_offset < 4096 ? 168 : 5252, SEEK_SET);
			_reader->read_shorts(curve, 256);
		}
		else gamma_curve(0, 3.875, 1, 255);
		load_raw = filters ? LoadRawType::eight_bit_load_raw :
			strcmp(model, "C330") ? LoadRawType::kodak_c603_load_raw :
			LoadRawType::kodak_c330_load_raw;
		load_flags = tiff_bps > 16;
		tiff_bps = 8;
	}
	else if (!_strnicmp(model, "EasyShare", 9))
	{
		data_offset = data_offset < 0x15000 ? 0x15000 : 0x17000;
		load_raw = LoadRawType::packed_load_raw;
	}
	else if (!_stricmp(make, "Kodak"))
	{
		if (filters == UINT_MAX) filters = 0x61616161;
		if (!strncmp(model, "NC2000", 6) ||
			!strncmp(model, "EOSDCS", 6) ||
			!strncmp(model, "DCS4", 4))
		{
			width -= 4;
			left_margin = 2;
			if (model[6] == ' ') model[6] = 0;
			if (!strcmp(model, "DCS460A")) goto bw;
		}
		else if (!strcmp(model, "DCS660M"))
		{
			black = 214;
			goto bw;
		}
		else if (!strcmp(model, "DCS760M"))
		{
		bw:   colors = 1;
			filters = 0;
		}
		if (!strcmp(model + 4, "20X"))
			strcpy_s(cdesc, LenCDesc, "MYCY");
		if (strstr(model, "DC25"))
		{
			strcpy_s(model, LenModel, "DC25");
			data_offset = 15424;
		}
		if (!strncmp(model, "DC2", 3))
		{
			raw_height = 2 + (height = 242);
			if (flen < 100000)
			{
				raw_width = 256; width = 249;
				pixel_aspect = (4.0*height) / (3.0*width);
			}
			else
			{
				raw_width = 512; width = 501;
				pixel_aspect = (493.0*height) / (373.0*width);
			}
			top_margin = left_margin = 1;
			colors = 4;
			filters = 0x8d8d8d8d;
			simple_coeff(1);
			pre_mul[1] = 1.179;
			pre_mul[2] = 1.209;
			pre_mul[3] = 1.036;
			load_raw = LoadRawType::eight_bit_load_raw;
		}
		else if (!strcmp(model, "40"))
		{
			strcpy_s(model, LenModel, "DC40");
			height = 512;
			width = 768;
			data_offset = 1152;
			load_raw = LoadRawType::kodak_radc_load_raw;
			tiff_bps = 12;
		}
		else if (strstr(model, "DC50"))
		{
			strcpy_s(model, LenModel, "DC50");
			height = 512;
			width = 768;
			data_offset = 19712;
			load_raw = LoadRawType::kodak_radc_load_raw;
		}
		else if (strstr(model, "DC120"))
		{
			strcpy_s(model, LenModel, "DC120");
			height = 976;
			width = 848;
			pixel_aspect = height / 0.75 / width;
			load_raw = tiff_compress == 7 ? LoadRawType::kodak_jpeg_load_raw : LoadRawType::kodak_dc120_load_raw;
		}
		else if (!strcmp(model, "DCS200"))
		{
			thumb_height = 128;
			thumb_width = 192;
			thumb_offset = 6144;
			thumb_misc = 360;
			write_thumb = WriteThumbType::layer_thumb;
			black = 17;
		}
	}
	else if (!strcmp(model, "Fotoman Pixtura"))
	{
		height = 512;
		width = 768;
		data_offset = 3632;
		load_raw = LoadRawType::kodak_radc_load_raw;
		filters = 0x61616161;
		simple_coeff(2);
	}
	else if (!strncmp(model, "QuickTake", 9))
	{
		if (head[5])
			strcpy_s(model + 10, LenModel - 10, "200");
		_reader->Seek(544, SEEK_SET);
		height = _reader->get2();
		width = _reader->get2();
		data_offset = (_reader->get4(), _reader->get2()) == 30 ? 738 : 736;
		if (height > width)
		{
			SWAP(height, width);
			_reader->Seek(data_offset - 6, SEEK_SET);
			flip = ~_reader->get2() & 3 ? 5 : 6;
		}
		filters = 0x61616161;
	}
	else if (!strcmp(make, "Rollei") && !load_raw)
	{
		switch (raw_width)
		{
		case 1316:
			height = 1030;
			width = 1300;
			top_margin = 1;
			left_margin = 6;
			break;
		case 2568:
			height = 1960;
			width = 2560;
			top_margin = 2;
			left_margin = 8;
		}
		filters = 0x16161616;
		load_raw = LoadRawType::rollei_load_raw;
	}
	if (!model[0])
		sprintf_s(model, LenModel, "%dx%d", width, height);
	if (filters == UINT_MAX) filters = 0x94949494;
	if (thumb_offset && !thumb_height)
	{
		_reader->Seek(thumb_offset, SEEK_SET);
		jhead jh(*_reader, *this, true);
		if (jh._success)
		{
			thumb_width = jh.jh.wide;
			thumb_height = jh.jh.high;
		}
	}
dng_skip:
	if ((use_camera_matrix & (use_camera_wb || dng_version))
		&& cmatrix[0][0] > 0.125)
	{
		memcpy(rgb_cam, cmatrix, sizeof cmatrix);
		raw_color = 0;
	}
	if (raw_color) adobe_coeff(make, model);
	if (load_raw == LoadRawType::kodak_radc_load_raw)
		if (raw_color) adobe_coeff("Apple", "Quicktake");
	if (fuji_width)
	{
		fuji_width = width >> !fuji_layout;
		filters = fuji_width & 1 ? 0x94949494 : 0x49494949;
		width = (height >> fuji_layout) + fuji_width;
		height = width - 1;
		pixel_aspect = 1;
	}
	else
	{
		if (raw_height < height) raw_height = height;
		if (raw_width  < width) raw_width = width;
	}
	if (!tiff_bps) tiff_bps = 12;
	if (!maximum) maximum = (1 << tiff_bps) - 1;
	if (!load_raw || height < 22 || width < 22 ||
		tiff_bps > 16 || tiff_samples > 6 || colors > 4)
		is_raw = 0;
	if (!cdesc[0])
		strcpy_s(cdesc, LenCDesc, colors == 3 ? "RGBG" : "GMCY");
	if (!raw_height) raw_height = height;
	if (!raw_width) raw_width = width;
	if (filters > 999 && colors == 3)
		filters |= ((filters >> 2 & 0x22222222) |
		(filters << 2 & 0x88888888)) & filters << 1;

notraw:
	if (flip == UINT_MAX)
		flip = tiff_flip;
	if (flip == UINT_MAX)
		flip = 0;
}
#pragma endregion

#pragma region Private methods
int CSimpleInfo::parse_tiff(int base)
{
	_reader->Seek(base, SEEK_SET);
	_reader->SetOrder(_reader->get2());
	if (_reader->GetOrder() != 0x4949 && _reader->GetOrder() != 0x4d4d)
		return 0;
	_reader->get2();
	int doff;
	while ((doff = _reader->get4()))
	{
		_reader->Seek(doff + base, SEEK_SET);
		if (parse_tiff_ifd(base)) break;
	}
	return 1;
}


int CSimpleInfo::parse_tiff_ifd(int base)
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

	if (tiff_nifds >= sizeof tiff_ifd / sizeof tiff_ifd[0])
		return 1;
	int ifd = tiff_nifds++;
	for (j = 0; j < 4; j++)
		for (i = 0; i < 4; i++)
			cc[j][i] = i == j;
	unsigned entries = _reader->get2();
	if (entries > 512)
		return 1;

	bool use_cm = false;

	while (entries--)
	{
		tiff_get(base, &tag, &type, &len, &save);
		switch (tag)
		{
		case 5:
			width = _reader->get2();
			break;
		case 6:
			height = _reader->get2();
			break;
		case 7:
			width += _reader->get2();
			break;
		case 9:
			if ((i = _reader->get2()))
				filters = i;
			break;
		case 17:
		case 18:
			if (type == 3 && len == 1)
				cam_mul[(tag - 17) * 2] = _reader->get2() / 256.0;
			break;
		case 23:
			if (type == 3)
				iso_speed = _reader->get2();
			break;
		case 28: case 29: case 30:
			cblack[tag - 28] = _reader->get2();
			cblack[3] = cblack[1];
			break;
		case 36: case 37: case 38:
			cam_mul[tag - 36] = _reader->get2();
			break;
		case 39:
			if (len < 50 || cam_mul[0])
				break;
			_reader->Seek(12, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = _reader->get2();
			break;
		case 46:
			if (type != 7 || _reader->GetChar() != 0xff || _reader->GetChar() != 0xd8) break;
			thumb_offset = _reader->GetPosition() - 2;
			thumb_length = len;
			break;
		case 61440:			/* Fuji HS10 table */
			_reader->Seek(_reader->get4() + base, SEEK_SET);
			parse_tiff_ifd(base);
			break;
		case 2: case 256: case 61441:	/* ImageWidth */
			tiff_ifd[ifd].width = _reader->getint(type);
			break;
		case 3: case 257: case 61442:	/* ImageHeight */
			tiff_ifd[ifd].height = _reader->getint(type);
			break;
		case 258:				/* BitsPerSample */
		case 61443:
			tiff_ifd[ifd].samples = len & 7;
			tiff_ifd[ifd].bps = _reader->getint(type);
			if (tiff_bps < tiff_ifd[ifd].bps)
				tiff_bps = tiff_ifd[ifd].bps;
			break;
		case 61446:
			raw_height = 0;
			if (tiff_ifd[ifd].bps > 12)
				break;
			load_raw = LoadRawType::packed_load_raw;
			load_flags = _reader->get4() ? 24 : 80;
			break;
		case 259:				/* Compression */
			tiff_ifd[ifd].comp = _reader->getint(type);
			break;
		case 262:				/* PhotometricInterpretation */
			tiff_ifd[ifd].phint = _reader->get2();
			break;
		case 270:				/* ImageDescription */
			_reader->Read(desc, 512, 1);
			break;
		case 271:				/* Make */
			_reader->GetString(make, LenMake);
			break;
		case 272:				/* Model */
			_reader->GetString(model, LenModel);
			break;
		case 280:				/* Panasonic RW2 offset */
			if (type != 4) break;
			load_raw = LoadRawType::panasonic_load_raw;
			load_flags = 0x2008;
		case 273:				/* StripOffset */
		case 513:				/* JpegIFOffset */
		case 61447:
			tiff_ifd[ifd].offset = _reader->get4() + base;
			if (!tiff_ifd[ifd].bps && tiff_ifd[ifd].offset > 0)
			{
				_reader->Seek(tiff_ifd[ifd].offset, SEEK_SET);
				jhead jh(*_reader, *this, true);
				if (jh._success)
				{
					tiff_ifd[ifd].comp = 6;
					tiff_ifd[ifd].width = jh.jh.wide;
					tiff_ifd[ifd].height = jh.jh.high;
					tiff_ifd[ifd].bps = jh.jh.bits;
					tiff_ifd[ifd].samples = jh.jh.clrs;
					if (!(jh.jh.sraw || (jh.jh.clrs & 1)))
						tiff_ifd[ifd].width *= jh.jh.clrs;
					if ((tiff_ifd[ifd].width > 4 * tiff_ifd[ifd].height) & ~jh.jh.clrs)
					{
						tiff_ifd[ifd].width /= 2;
						tiff_ifd[ifd].height *= 2;
					}
					i = _reader->GetOrder();
					parse_tiff(tiff_ifd[ifd].offset + 12);
					_reader->SetOrder(i);
				}
			}
			break;
		case 274:				/* Orientation */
			tiff_ifd[ifd].flip = "50132467"[_reader->get2() & 7] - '0';
			break;
		case 277:				/* SamplesPerPixel */
			tiff_ifd[ifd].samples = _reader->getint(type) & 7;
			break;
		case 279:				/* StripByteCounts */
		case 514:
		case 61448:
			tiff_ifd[ifd].bytes = _reader->get4();
			break;
		case 61454:
			for (size_t c = 0; c < 3; c++)
				cam_mul[(4 - c) % 3] = _reader->getint(type);
			break;
		case 305:  case 11:		/* Software */
			_reader->GetString(software, 64);
			if (!strncmp(software, "Adobe", 5) ||
				!strncmp(software, "dcraw", 5) ||
				!strncmp(software, "UFRaw", 5) ||
				!strncmp(software, "Bibble", 6) ||
				!strncmp(software, "Nikon Scan", 10) ||
				!strcmp(software, "Digital Photo Professional"))
				is_raw = 0;
			break;
		case 306:				/* DateTime */
			get_timestamp(0);
			break;
		case 315:				/* Artist */
			_reader->Read(artist, LenArtist, 1);
			break;
		case 322:				/* TileWidth */
			tiff_ifd[ifd].tile_width = _reader->getint(type);
			break;
		case 323:				/* TileLength */
			tiff_ifd[ifd].tile_length = _reader->getint(type);
			break;
		case 324:				/* TileOffsets */
			tiff_ifd[ifd].offset = len > 1 ? _reader->GetPosition() : _reader->get4();
			if (len == 1)
				tiff_ifd[ifd].tile_width = tiff_ifd[ifd].tile_length = 0;
			if (len == 4)
			{
				load_raw = LoadRawType::sinar_4shot_load_raw;
				is_raw = 5;
			}
			break;
		case 330:				/* SubIFDs */
			if (!strcmp(model, "DSLR-A100") && tiff_ifd[ifd].width == 3872)
			{
				load_raw = LoadRawType::sony_arw_load_raw;
				data_offset = _reader->get4() + base;
				ifd++;  break;
			}
			while (len--)
			{
				i = _reader->GetPosition();
				_reader->Seek(_reader->get4() + base, SEEK_SET);
				if (parse_tiff_ifd(base))
					break;
				_reader->Seek(i + 4, SEEK_SET);
			}
			break;
		case 400:
			strcpy_s(make, LenMake, "Sarnoff");
			maximum = 0xfff;
			break;
		case 28688:
			for (size_t c = 0; c < 4; c++)
				sony_curve[c + 1] = _reader->get2() >> 2 & 0xfff;
			for (i = 0; i < 5; i++)
				for (j = sony_curve[i] + 1; j <= sony_curve[i + 1]; j++)
					curve[j] = curve[j - 1] + (1 << i);
			break;
		case 29184:
			sony_offset = _reader->get4();
			break;
		case 29185:
			sony_length = _reader->get4();
			break;
		case 29217:
			sony_key = _reader->get4();
			break;
		case 29264:
			parse_minolta(_reader->GetPosition());
			raw_width = 0;
			break;
		case 29443:
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ (c < 2)] = _reader->get2();
			break;
		case 29459:
			for (size_t c = 0; c < 4; c++)
				cam_mul[c] = _reader->get2();
			i = (cam_mul[1] == 1024 && cam_mul[2] == 1024) << 1;
			SWAP(cam_mul[i], cam_mul[i + 1])
				break;
		case 33405:			/* Model2 */
			_reader->GetString(model2, LenModel2);
			break;
		case 33421:			/* CFARepeatPatternDim */
			if (_reader->get2() == 6 && _reader->get2() == 6)
				filters = 9;
			break;
		case 33422:			/* CFAPattern */
			if (filters == 9)
			{
				for (size_t c = 0; c < 36; c++)
					((char *)xtrans)[c] = _reader->GetChar() & 3;
				break;
			}
		case 64777:			/* Kodak P-series */
			if ((plen = len) > 16)
				plen = 16;
			_reader->Read(cfa_pat, 1, plen);
			colors = cfa = 0;
			for (i = 0; i < plen && colors < 4; i++)
			{
				colors += !(cfa & (1 << cfa_pat[i]));
				cfa |= 1 << cfa_pat[i];
			}
			if (cfa == 070)
				memcpy(cfa_pc, "\003\004\005", 3);	/* CMY */
			if (cfa == 072)
				memcpy(cfa_pc, "\005\003\004\001", 4);	/* GMCY */
			goto guess_cfa_pc;
		case 33424:
		case 65024:
			_reader->Seek(_reader->get4() + base, SEEK_SET);
			parse_kodak_ifd(base);
			break;
		case 33434:			/* ExposureTime */
			tiff_ifd[ifd].shutter = shutter = _reader->getreal(type);
			break;
		case 33437:			/* FNumber */
			aperture = _reader->getreal(type);
			break;
		case 34306:			/* Leaf white balance */
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ 1] = 4096.0 / _reader->get2();
			break;
		case 34307:			/* Leaf CatchLight color matrix */
			_reader->Read(software, 1, 7);
			if (strncmp(software, "MATRIX", 6))
				break;
			colors = 4;
			for (raw_color = i = 0; i < 3; i++)
			{
				for (size_t c = 0; c < 4; c++)
					_reader->scanf("%f", &rgb_cam[i][c ^ 1]);
				if (!use_camera_wb)
					continue;
				num = 0;
				for (size_t c = 0; c < 4; c++)
					num += rgb_cam[i][c];
				for (size_t c = 0; c < 4; c++)
					rgb_cam[i][c] /= num;
			}
			break;
		case 34310:			/* Leaf metadata */
			parse_mos(_reader->GetPosition());
		case 34303:
			strcpy_s(make, LenMake, "Leaf");
			break;
		case 34665:			/* EXIF tag */
			_reader->Seek(_reader->get4() + base, SEEK_SET);
			parse_exif(base);
			break;
		case 34853:			/* GPSInfo tag */
			_reader->Seek(_reader->get4() + base, SEEK_SET);
			parse_gps(base);
			break;
		case 34675:			/* InterColorProfile */
		case 50831:			/* AsShotICCProfile */
			profile_offset = _reader->GetPosition();
			profile_length = len;
			break;
		case 37122:			/* CompressedBitsPerPixel */
			kodak_cbpp = _reader->get4();
			break;
		case 37386:			/* FocalLength */
			focal_len = _reader->getreal(type);
			break;
		case 37393:			/* ImageNumber */
			shot_order = _reader->getint(type);
			break;
		case 37400:			/* old Kodak KDC tag */
			for (raw_color = i = 0; i < 3; i++)
			{
				_reader->getreal(type);
				for (size_t c = 0; c < 3; c++)
					rgb_cam[i][c] = _reader->getreal(type);
			}
			break;
		case 40976:
			strip_offset = _reader->get4();
			switch (tiff_ifd[ifd].comp)
			{
			case 32770:
				load_raw = LoadRawType::samsung_load_raw;
				break;
			case 32772:
				load_raw = LoadRawType::samsung2_load_raw;
				break;
			case 32773:
				load_raw = LoadRawType::samsung3_load_raw;
				break;
			}
			break;
		case 46275:			/* Imacon tags */
			strcpy_s(make, LenMake, "Imacon");
			data_offset = _reader->GetPosition();
			ima_len = len;
			break;
		case 46279:
			if (!ima_len)
				break;
			_reader->Seek(38, SEEK_CUR);
		case 46274:
			_reader->Seek(40, SEEK_CUR);
			raw_width = _reader->get4();
			raw_height = _reader->get4();
			left_margin = _reader->get4() & 7;
			width = raw_width - left_margin - (_reader->get4() & 7);
			top_margin = _reader->get4() & 7;
			height = raw_height - top_margin - (_reader->get4() & 7);
			if (raw_width == 7262)
			{
				height = 5444;
				width = 7244;
				left_margin = 7;
			}
			_reader->Seek(52, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = _reader->getreal(11);
			_reader->Seek(114, SEEK_CUR);
			flip = (_reader->get2() >> 7) * 90;
			if (width * height * 6 == ima_len)
			{
				if (flip % 180 == 90)
					SWAP(width, height);
				raw_width = width;
				raw_height = height;
				left_margin = top_margin = filters = flip = 0;
			}
			sprintf_s(model, LenModel, "Ixpress %d-Mp", height*width / 1000000);
			load_raw = LoadRawType::imacon_full_load_raw;
			if (filters)
			{
				if (left_margin & 1) filters = 0x61616161;
				load_raw = LoadRawType::unpacked_load_raw;
			}
			maximum = 0xffff;
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
						sscanf_s(cp + 8, "%f %f %f", cam_mul, cam_mul + 1, cam_mul + 2);
			}
			break;
		case 50458:
			if (!make[0])
				strcpy_s(make, LenMake, "Hasselblad");
			break;
		case 50459:			/* Hasselblad tag */
			i = _reader->GetOrder();
			j = _reader->GetPosition();
			c = tiff_nifds;
			_reader->SetOrder(_reader->get2());
			_reader->Seek(j + (_reader->get2(), _reader->get4()), SEEK_SET);
			parse_tiff_ifd(j);
			maximum = 0xffff;
			tiff_nifds = c;
			_reader->SetOrder(i);
			break;
		case 50706:			/* DNGVersion */
			for (size_t c = 0; c < 4; c++)
				dng_version = (dng_version << 8) + _reader->GetChar();
			if (!make[0])
				strcpy_s(make, LenMake, "DNG");
			is_raw = 1;
			break;
		case 50708:			/* UniqueCameraModel */
			if (model[0])
				break;
			_reader->GetString(make, LenMake);
			if ((cp = strchr(make, ' ')))
			{
				strcpy_s(model, LenModel, cp + 1);
				*cp = 0;
			}
			break;
		case 50710:			/* CFAPlaneColor */
			if (filters == 9)
				break;
			if (len > 4)
				len = 4;
			colors = len;
			_reader->Read(cfa_pc, 1, colors);
		guess_cfa_pc:
			for (size_t c = 0; c < colors; c++)
				tab[cfa_pc[c]] = c;
			cdesc[colors] = 0;
			for (i = 16; i--; )
				filters = filters << 2 | tab[cfa_pat[i % plen]];
			filters -= !filters;
			break;
		case 50711:			/* CFALayout */
			if (_reader->get2() == 2)
				fuji_width = 1;
			break;
		case 291:
		case 50712:			/* LinearizationTable */
			linear_table(len);
			break;
		case 50713:			/* BlackLevelRepeatDim */
			cblack[4] = _reader->get2();
			cblack[5] = _reader->get2();
			if (cblack[4] * cblack[5] > sizeof cblack / sizeof *cblack - 6)
				cblack[4] = cblack[5] = 1;
			break;
		case 61450:
			cblack[4] = cblack[5] = MIN(sqrt(len), 64);
		case 50714:			/* BlackLevel */
			if (!(cblack[4] * cblack[5]))
				cblack[4] = cblack[5] = 1;
			for (size_t c = 0; c < (cblack[4] * cblack[5]); c++)
				cblack[6 + c] = _reader->getreal(type);
			black = 0;
			break;
		case 50715:			/* BlackLevelDeltaH */
		case 50716:			/* BlackLevelDeltaV */
			for (num = i = 0; i < (len & 0xffff); i++)
				num += _reader->getreal(type);
			black += num / len + 0.5;
			break;
		case 50717:			/* WhiteLevel */
			maximum = _reader->getint(type);
			break;
		case 50718:			/* DefaultScale */
			pixel_aspect = _reader->getreal(type);
			pixel_aspect /= _reader->getreal(type);
			break;
		case 50721:			/* ColorMatrix1 */
		case 50722:			/* ColorMatrix2 */
			for (size_t c = 0; c < colors; c++)
				for (j = 0; j < 3; j++)
					cm[c][j] = _reader->getreal(type);
			use_cm = true;
			break;
		case 50723:			/* CameraCalibration1 */
		case 50724:			/* CameraCalibration2 */
			for (i = 0; i < colors; i++)
				for (size_t c = 0; c < colors; c++)
					cc[i][c] = _reader->getreal(type);
			break;
		case 50727:			/* AnalogBalance */
			for (size_t c = 0; c < colors; c++)
				ab[c] = _reader->getreal(type);
			break;
		case 50728:			/* AsShotNeutral */
			for (size_t c = 0; c < colors; c++)
				asn[c] = _reader->getreal(type);
			break;
		case 50729:			/* AsShotWhiteXY */
			xyz[0] = _reader->getreal(type);
			xyz[1] = _reader->getreal(type);
			xyz[2] = 1 - xyz[0] - xyz[1];
			for (size_t c = 0; c < 3; c++)
				xyz[c] /= d65_white[c];
			break;
		case 50740:			/* DNGPrivateData */
			if (dng_version)
				break;
			parse_minolta(j = _reader->get4() + base);
			_reader->Seek(j, SEEK_SET);
			parse_tiff_ifd(base);
			break;
		case 50752:
			_reader->read_shorts(cr2_slice, 3);
			break;
		case 50829:			/* ActiveArea */
			top_margin = _reader->getint(type);
			left_margin = _reader->getint(type);
			height = _reader->getint(type) - top_margin;
			width = _reader->getint(type) - left_margin;
			break;
		case 50830:			/* MaskedAreas */
			for (i = 0; i < len && i < 32; i++)
				((int *)mask)[i] = _reader->getint(type);
			black = 0;
			break;
		case 51009:			/* OpcodeList2 */
			meta_offset = _reader->GetPosition();
			break;
		case 64772:			/* Kodak P-series */
			if (len < 13) break;
			_reader->Seek(16, SEEK_CUR);
			data_offset = _reader->get4();
			_reader->Seek(28, SEEK_CUR);
			data_offset += _reader->get4();
			load_raw = LoadRawType::packed_load_raw;
			break;
		case 65026:
			if (type == 2) _reader->GetString(model2, LenModel2);
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
			sony_decrypt(buf, sony_length / 4, 1, sony_key);

			CWriter* sfp = CWriter::CreateTempFile();
			sfp->Write(buf, sony_length, 1);
			sfp->Seek(0, SEEK_SET);
			parse_tiff_ifd(-sony_offset);
			delete sfp;
		}
	}
	for (i = 0; i < colors; i++)
		for (size_t c = 0; c < colors; c++)
			cc[i][c] *= ab[i];
	if (use_cm)
	{
		for (size_t c = 0; c < colors; c++)
			for (i = 0; i < 3; i++)
				for (cam_xyz[c][i] = j = 0; j < colors; j++)
					cam_xyz[c][i] += cc[c][j] * cm[j][i] * xyz[i];
		cam_xyz_coeff(cmatrix, cam_xyz);
	}
	if (asn[0])
	{
		cam_mul[3] = 0;
		for (size_t c = 0; c < colors; c++)
			cam_mul[c] = 1 / asn[c];
	}
	if (!use_cm)
		for (size_t c = 0; c < colors; c++)
			pre_mul[c] /= cc[c][c];
	return 0;
}

void CSimpleInfo::parse_exif(int base)
{
	unsigned tag, type, len, save;
	double expo;

	unsigned kodak = !strncmp(make, "EASTMAN", 7) && tiff_nifds < 3;
	unsigned entries = _reader->get2();
	while (entries--)
	{
		tiff_get(base, &tag, &type, &len, &save);
		switch (tag)
		{
		case 33434:
			tiff_ifd[tiff_nifds - 1].shutter = shutter = _reader->getreal(type);
			break;
		case 33437:
			aperture = _reader->getreal(type);
			break;
		case 34855:
			iso_speed = _reader->get2();
			break;
		case 36867:
		case 36868:
			get_timestamp(0);
			break;
		case 37377:
			if ((expo = -_reader->getreal(type)) < 128)
				tiff_ifd[tiff_nifds - 1].shutter = shutter = pow(2, expo);
			break;
		case 37378:
			aperture = pow(2, _reader->getreal(type) / 2);
			break;
		case 37386:
			focal_len = _reader->getreal(type);
			break;
		case 37500:
			parse_makernote(base, 0);
			break;
		case 40962:
			if (kodak) raw_width = _reader->get4();
			break;
		case 40963:
			if (kodak) raw_height = _reader->get4();
			break;
		case 41730:
			if (_reader->get4() == 0x20002)
			{
				exif_cfa = 0;
				for (size_t c = 0; c < 8; c += 2)
					exif_cfa |= _reader->GetChar() * 0x01010101 << c;
			}
		}
		_reader->Seek(save, SEEK_SET);
	}
}

void CSimpleInfo::parse_gps(int base)
{
	unsigned tag, type, len, save;

	unsigned entries = _reader->get2();
	while (entries--)
	{
		tiff_get(base, &tag, &type, &len, &save);
		switch (tag)
		{
		case 1: case 3: case 5:
			gpsdata[29 + tag / 2] = _reader->GetChar();
			break;
		case 2: case 4: case 7:
			for (size_t c = 0; c < 6; c++)
				gpsdata[tag / 3 * 6 + c] = _reader->get4();
			break;
		case 6:
			for (size_t c = 0; c < 6; c++)
				gpsdata[18 + c] = _reader->get4();
			break;
		case 18: case 29:
			_reader->GetString((char *)(gpsdata + 14 + tag / 3), MIN(len, 12));
		}
		_reader->Seek(save, SEEK_SET);
	}
}
void CSimpleInfo::parse_makernote(int base, int uptag)
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
	if (!strcmp(make, "Nokia")) return;
	_reader->Read(buf, 1, 10);
	if (!strncmp(buf, "KDK", 3) ||	/* these aren't TIFF tables */
		!strncmp(buf, "VER", 3) ||
		!strncmp(buf, "IIII", 4) ||
		!strncmp(buf, "MMMM", 4)) return;
	if (!strncmp(buf, "KC", 2) ||	/* Konica KD-400Z, KD-510Z */
		!strncmp(buf, "MLY", 3))
	{	/* Minolta DiMAGE G series */
		_reader->SetOrder(0x4d4d);
		while ((i = _reader->GetPosition()) < data_offset && i < 16384)
		{
			wb[0] = wb[2];  wb[2] = wb[1];  wb[1] = wb[3];
			wb[3] = _reader->get2();
			if (wb[1] == 256 && wb[3] == 256 &&
				wb[0] > 256 && wb[0] < 640 && wb[2] > 256 && wb[2] < 640)
				for (size_t c = 0; c < 4; c++)
					cam_mul[c] = wb[c];
		}
		goto quit;
	}
	if (!strcmp(buf, "Nikon"))
	{
		base = _reader->GetPosition();
		_reader->SetOrder(_reader->get2());
		if (_reader->get2() != 42)
			goto quit;
		unsigned offset = _reader->get4();
		_reader->Seek(offset - 8, SEEK_CUR);
	}
	else if (!strcmp(buf, "OLYMPUS") || !strcmp(buf, "PENTAX "))
	{
		base = _reader->GetPosition() - 10;
		_reader->Seek(-2, SEEK_CUR);
		_reader->SetOrder(_reader->get2());
		if (buf[0] == 'O')
			_reader->get2();
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
		if (!strncmp(make, "SAMSUNG", 7))
			base = _reader->GetPosition();
	}
	unsigned entries = _reader->get2();
	if (entries > 1000)
		return;
	short morder = _reader->GetOrder();
	while (entries--)
	{
		_reader->SetOrder(morder);
		tiff_get(base, &tag, &type, &len, &save);
		tag |= uptag << 16;
		if (tag == 2 && strstr(make, "NIKON") && !iso_speed)
			iso_speed = (_reader->get2(), _reader->get2());
		if (tag == 4 && len > 26 && len < 35)
		{
			if ((i = (_reader->get4(), _reader->get2())) != 0x7fff && !iso_speed)
				iso_speed = 50 * pow(2, i / 32.0 - 4);
			if ((i = (_reader->get2(), _reader->get2())) != 0x7fff && !aperture)
				aperture = pow(2, i / 64.0);
			if ((i = _reader->get2()) != 0xffff && !shutter)
				shutter = pow(2, (short)i / -32.0);
			wbi = (_reader->get2(), _reader->get2());
			shot_order = (_reader->get2(), _reader->get2());
		}
		if ((tag == 4 || tag == 0x114) && !strncmp(make, "KONICA", 6))
		{
			_reader->Seek(tag == 4 ? 140 : 160, SEEK_CUR);
			switch (_reader->get2())
			{
			case 72:  flip = 0;  break;
			case 76:  flip = 6;  break;
			case 82:  flip = 5;  break;
			}
		}
		if (tag == 7 && type == 2 && len > 20)
			_reader->GetString(model2, LenModel2);
		if (tag == 8 && type == 4)
			shot_order = _reader->get4();
		if (tag == 9 && !strcmp(make, "Canon"))
			_reader->Read(artist, LenArtist, 1);
		if (tag == 0xc && len == 4)
			for (size_t c = 0; c < 3; c++)
				cam_mul[(c << 1 | c >> 1) & 3] = _reader->getreal(type);
		if (tag == 0xd && type == 7 && _reader->get2() == 0xaaaa)
		{
			unsigned c = 2;
			for (i = 2; (unsigned short)c != 0xbbbb && i < len; i++)
				c = c << 8 | _reader->GetChar();
			while ((i += 4) < len - 5)
				if (_reader->get4() == 257 && (i = len) && (c = (_reader->get4(), _reader->GetChar())) < 3)
					flip = "065"[c] - '0';
		}
		if (tag == 0x10 && type == 4)
			unique_id = _reader->get4();
		if (tag == 0x11 && is_raw && !strncmp(make, "NIKON", 5))
		{
			_reader->Seek(_reader->get4() + base, SEEK_SET);
			parse_tiff_ifd(base);
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
				cam_mul[0] = _reader->get4() << 2;
				cam_mul[1] = _reader->get4() + _reader->get4();
				cam_mul[2] = _reader->get4() << 2;
			}
		}
		if (tag == 0x15 && type == 2 && is_raw)
			_reader->Read(model, LenModel, 1);
		if (strstr(make, "PENTAX"))
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
				cam_mul[c ^ (c >> 1) ^ 1] = _reader->get4();
		}
		if (tag == 0x3d && type == 3 && len == 4)
			for (size_t c = 0; c < 4; c++)
				cblack[c ^ c >> 1] = _reader->get2() >> (14 - tiff_bps);
		if (tag == 0x81 && type == 4)
		{
			data_offset = _reader->get4();
			_reader->Seek(data_offset + 41, SEEK_SET);
			raw_height = _reader->get2() * 2;
			raw_width = _reader->get2();
			filters = 0x61616161;
		}
		if ((tag == 0x81 && type == 7) ||
			(tag == 0x100 && type == 7) ||
			(tag == 0x280 && type == 1))
		{
			thumb_offset = _reader->GetPosition();
			thumb_length = len;
		}
		if (tag == 0x88 && type == 4 && (thumb_offset = _reader->get4()))
			thumb_offset += base;
		if (tag == 0x89 && type == 4)
			thumb_length = _reader->get4();
		if (tag == 0x8c || tag == 0x96)
			meta_offset = _reader->GetPosition();
		if (tag == 0x97)
		{
			for (i = 0; i < 4; i++)
				ver97 = ver97 * 10 + _reader->GetChar() - '0';
			switch (ver97)
			{
			case 100:
				_reader->Seek(68, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					cam_mul[(c >> 1) | ((c & 1) << 1)] = _reader->get2();
				break;
			case 102:
				_reader->Seek(6, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					cam_mul[c ^ (c >> 1)] = _reader->get2();
				break;
			case 103:
				_reader->Seek(16, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					cam_mul[c] = _reader->get2();
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
				cam_mul[c] = _reader->get4();
		}
		if (tag == 0xa4 && type == 3)
		{
			_reader->Seek(wbi * 48, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = _reader->get2();
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
				cam_mul[c ^ (c >> 1) ^ (i & 1)] = _reader->sget2(buf97 + (i & -2) + c * 2);
		}
		if (tag == 0x200 && len == 3)
			shot_order = (_reader->get4(), _reader->get4());
		if (tag == 0x200 && len == 4)
			for (size_t c = 0; c < 4; c++)
				cblack[c ^ c >> 1] = _reader->get2();
		if (tag == 0x201 && len == 4)
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ (c >> 1)] = _reader->get2();
		if (tag == 0x220 && type == 7)
			meta_offset = _reader->GetPosition();
		if (tag == 0x401 && type == 4 && len == 4)
			for (size_t c = 0; c < 4; c++)
				cblack[c ^ c >> 1] = _reader->get4();
		if (tag == 0xe01)
		{		/* Nikon Capture Note */
			_reader->SetOrder(0x4949);
			_reader->Seek(22, SEEK_CUR);
			for (unsigned offset = 22; offset + 22 < len; offset += 22 + i)
			{
				tag = _reader->get4();
				_reader->Seek(14, SEEK_CUR);
				i = _reader->get4() - 4;
				if (tag == 0x76a43207) flip = _reader->get2();
				else _reader->Seek(i, SEEK_CUR);
			}
		}
		if (tag == 0xe80 && len == 256 && type == 7)
		{
			_reader->Seek(48, SEEK_CUR);
			cam_mul[0] = _reader->get2() * 508 * 1.078 / 0x10000;
			cam_mul[2] = _reader->get2() * 382 * 1.173 / 0x10000;
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
					cmatrix[i][c] = ((short)_reader->get2()) / 256.0;
		if ((tag == 0x1012 || tag == 0x20400600) && len == 4)
			for (size_t c = 0; c < 4; c++)
				cblack[c ^ c >> 1] = _reader->get2();
		if (tag == 0x1017 || tag == 0x20400100)
			cam_mul[0] = _reader->get2() / 256.0;
		if (tag == 0x1018 || tag == 0x20400100)
			cam_mul[2] = _reader->get2() / 256.0;
		if (tag == 0x2011 && len == 2)
		{
		get2_256:
			_reader->SetOrder(0x4d4d);
			cam_mul[0] = _reader->get2() / 256.0;
			cam_mul[2] = _reader->get2() / 256.0;
		}
		if ((tag | 0x70) == 0x2070 && (type == 4 || type == 13))
			_reader->Seek(_reader->get4() + base, SEEK_SET);
		if (tag == 0x2020 && !strncmp(buf, "OLYMP", 5))
			parse_thumb_note(base, 257, 258);
		if (tag == 0x2040)
			parse_makernote(base, 0x2040);
		if (tag == 0xb028)
		{
			_reader->Seek(_reader->get4() + base, SEEK_SET);
			parse_thumb_note(base, 136, 137);
		}
		if (tag == 0x4001 && len > 500)
		{
			i = len == 582 ? 50 : len == 653 ? 68 : len == 5120 ? 142 : 126;
			_reader->Seek(i, SEEK_CUR);
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ (c >> 1)] = _reader->get2();
			for (i += 18; i <= len; i += 10)
			{
				_reader->get2();
				for (size_t c = 0; c < 4; c++)
					sraw_mul[c ^ (c >> 1)] = _reader->get2();
				if (sraw_mul[1] == 1170) break;
			}
		}
		if (tag == 0x4021 && _reader->get4() && _reader->get4())
			for (size_t c = 0; c < 4; c++)
				cam_mul[c] = 1024;
		if (tag == 0xa021)
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ (c >> 1)] = _reader->get4();
		if (tag == 0xa028)
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ (c >> 1)] -= _reader->get4();
		if (tag == 0xb001)
			unique_id = _reader->get2();
	next:
		_reader->Seek(save, SEEK_SET);
	}
quit:
	_reader->SetOrder(sorder);
}

void CSimpleInfo::parse_thumb_note(int base, unsigned toff, unsigned tlen)
{
	unsigned tag, type, len, save;

	unsigned entries = _reader->get2();
	while (entries--)
	{
		tiff_get(base, &tag, &type, &len, &save);
		if (tag == toff) thumb_offset = _reader->get4() + base;
		if (tag == tlen) thumb_length = _reader->get4();
		_reader->Seek(save, SEEK_SET);
	}
}

void CSimpleInfo::get_timestamp(int reversed)
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
		timestamp = mktime(&t);
}

void CSimpleInfo::tiff_get(unsigned base, unsigned *tag, unsigned *type, unsigned *len, unsigned *save)
{
	*tag = _reader->get2();
	*type = _reader->get2();
	*len = _reader->get4();
	*save = _reader->GetPosition() + 4;
	if (*len * ("11124811248484"[*type < 14 ? *type : 0] - '0') > 4)
		_reader->Seek(_reader->get4() + base, SEEK_SET);
}

void CSimpleInfo::apply_tiff()
{
	int max_samp = 0, ties = 0, raw = -1, thm = -1;

	thumb_misc = 16;
	if (thumb_offset)
	{
		_reader->Seek(thumb_offset, SEEK_SET);
		jhead jh(*_reader, *this, true);
		if (jh._success)
		{
			thumb_misc = jh.jh.bits;
			thumb_width = jh.jh.wide;
			thumb_height = jh.jh.high;
		}
	}
	for (size_t i = tiff_nifds; i--; )
	{
		if (tiff_ifd[i].shutter)
			shutter = tiff_ifd[i].shutter;
		tiff_ifd[i].shutter = shutter;
	}
	for (size_t i = 0; i < tiff_nifds; i++)
	{
		if (max_samp < tiff_ifd[i].samples)
			max_samp = tiff_ifd[i].samples;
		if (max_samp > 3) max_samp = 3;
		int os = raw_width*raw_height;
		int ns = tiff_ifd[i].width*tiff_ifd[i].height;
		if (tiff_bps)
		{
			os *= tiff_bps;
			ns *= tiff_ifd[i].bps;
		}
		if ((tiff_ifd[i].comp != 6 || tiff_ifd[i].samples != 3) &&
			(tiff_ifd[i].width | tiff_ifd[i].height) < 0x10000 &&
			ns && ((ns > os && (ties = 1)) ||
			(ns == os && shot_select == ties++)))
		{
			raw_width = tiff_ifd[i].width;
			raw_height = tiff_ifd[i].height;
			tiff_bps = tiff_ifd[i].bps;
			tiff_compress = tiff_ifd[i].comp;
			data_offset = tiff_ifd[i].offset;
			tiff_flip = tiff_ifd[i].flip;
			tiff_samples = tiff_ifd[i].samples;
			tile_width = tiff_ifd[i].tile_width;
			tile_length = tiff_ifd[i].tile_length;
			shutter = tiff_ifd[i].shutter;
			raw = i;
		}
	}
	if (is_raw == 1 && ties)
		is_raw = ties;
	if (!tile_width)
		tile_width = INT_MAX;
	if (!tile_length)
		tile_length = INT_MAX;
	for (size_t i = tiff_nifds; i--; )
		if (tiff_ifd[i].flip)
			tiff_flip = tiff_ifd[i].flip;
	if (raw >= 0 && !load_raw)
	{
		switch (tiff_compress)
		{
		case 32767:
			if (tiff_ifd[raw].bytes == raw_width*raw_height)
			{
				tiff_bps = 12;
				load_raw = LoadRawType::sony_arw2_load_raw;
				break;
			}
			if (tiff_ifd[raw].bytes * 8 != raw_width*raw_height*tiff_bps)
			{
				raw_height += 8;
				load_raw = LoadRawType::sony_arw_load_raw;
				break;
			}
			load_flags = 79;
		case 32769:
			load_flags++;
		case 32770:
		case 32773:
			goto slr;
		case 0:  case 1:
			if (!strncmp(make, "OLYMPUS", 7) &&
				tiff_ifd[raw].bytes * 2 == raw_width*raw_height * 3)
				load_flags = 24;
			if (tiff_ifd[raw].bytes * 5 == raw_width*raw_height * 8)
			{
				load_flags = 81;
				tiff_bps = 12;
			} slr:
			switch (tiff_bps)
			{
			case  8:
				load_raw = LoadRawType::eight_bit_load_raw;
				break;
			case 12:
				if (tiff_ifd[raw].phint == 2)
					load_flags = 6;
				load_raw = LoadRawType::packed_load_raw;
				break;
			case 14:
				load_flags = 0;
			case 16:
				load_raw = LoadRawType::unpacked_load_raw;
				if (!strncmp(make, "OLYMPUS", 7) && tiff_ifd[raw].bytes * 7 > raw_width*raw_height)
					load_raw = LoadRawType::olympus_load_raw;
			}
			break;
		case 6:  case 7:  case 99:
			load_raw = LoadRawType::lossless_jpeg_load_raw;
			break;
		case 262:
			load_raw = LoadRawType::kodak_262_load_raw;
			break;
		case 34713:
			if ((raw_width + 9) / 10 * 16 * raw_height == tiff_ifd[raw].bytes)
			{
				load_raw = LoadRawType::packed_load_raw;
				load_flags = 1;
			}
			else if (raw_width*raw_height * 3 == tiff_ifd[raw].bytes * 2)
			{
				load_raw = LoadRawType::packed_load_raw;
				if (model[0] == 'N') load_flags = 80;
			}
			else if (raw_width*raw_height * 3 == tiff_ifd[raw].bytes)
			{
				load_raw = LoadRawType::nikon_yuv_load_raw;
				gamma_curve(1 / 2.4, 12.92, 1, 4095);
				memset(cblack, 0, sizeof cblack);
				filters = 0;
			}
			else if (raw_width*raw_height * 2 == tiff_ifd[raw].bytes)
			{
				load_raw = LoadRawType::unpacked_load_raw;
				load_flags = 4;
				_reader->SetOrder(0x4d4d);
			}
			else
				load_raw = LoadRawType::nikon_load_raw;
			break;
		case 65535:
			load_raw = LoadRawType::pentax_load_raw;
			break;
		case 65000:
			switch (tiff_ifd[raw].phint)
			{
			case 2:
				load_raw = LoadRawType::kodak_rgb_load_raw;   filters = 0;
				break;
			case 6:
				load_raw = LoadRawType::kodak_ycbcr_load_raw; filters = 0;
				break;
			case 32803:
				load_raw = LoadRawType::kodak_65000_load_raw;
			}
		case 32867: case 34892:
			break;
		default: is_raw = 0;
		}
	}
	if (!dng_version)
	{
		if ((tiff_samples == 3 && tiff_ifd[raw].bytes && tiff_bps != 14 &&
			(tiff_compress & -16) != 32768)
			|| (tiff_bps == 8 && strncmp(make, "Phase", 5) &&
				!strcasestr(make, "Kodak") && !strstr(model2, "DEBUG RAW")))
		{
			is_raw = 0;
		}
	}
	for (size_t i = 0; i < tiff_nifds; i++)
	{
		if (i != raw && tiff_ifd[i].samples == max_samp &&
			tiff_ifd[i].width * tiff_ifd[i].height / (SQR(tiff_ifd[i].bps) + 1) >
			thumb_width *       thumb_height / (SQR(thumb_misc) + 1)
			&& tiff_ifd[i].comp != 34892)
		{
			thumb_width = tiff_ifd[i].width;
			thumb_height = tiff_ifd[i].height;
			thumb_offset = tiff_ifd[i].offset;
			thumb_length = tiff_ifd[i].bytes;
			thumb_misc = tiff_ifd[i].bps;
			thm = i;
		}
	}
	if (thm >= 0)
	{
		thumb_misc |= tiff_ifd[thm].samples << 5;
		switch (tiff_ifd[thm].comp)
		{
		case 0:
			write_thumb = WriteThumbType::layer_thumb;
			break;
		case 1:
			if (tiff_ifd[thm].bps <= 8)
				write_thumb = WriteThumbType::ppm_thumb;
			else if (!strcmp(make, "Imacon"))
				write_thumb = WriteThumbType::ppm16_thumb;
			else
				thumb_load_raw = LoadRawType::kodak_thumb_load_raw;
			break;
		case 65000:
			thumb_load_raw = tiff_ifd[thm].phint == 6 ?
				LoadRawType::kodak_ycbcr_load_raw : LoadRawType::kodak_rgb_load_raw;
		}
	}
}

/*
Many cameras have a "debug mode" that writes JPEG and raw
at the same time.  The raw file has no header, so try to
to open the matching JPEG file and read its metadata.
*/
void CSimpleInfo::parse_external_jpeg()
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

		parse_tiff(12);
		thumb_offset = 0;
		is_raw = 1;
		delete _reader;

		_reader = save;
	}
	if (!timestamp)
		fprintf(stderr, ("Failed to read metadata from %s\n"), jname);
}

/*
All matrices are from Adobe DNG Converter unless otherwise noted.
*/
void CSimpleInfo::adobe_coeff(const char *make, const char *model)
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
		{ "Nikon E2100", 0, 0,	/* copied from Z2, new white balance */
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
			if (table[i].black)   black = (unsigned short)table[i].black;
			if (table[i].maximum) maximum = (unsigned short)table[i].maximum;
			if (table[i].trans[0])
			{
				raw_color = 0;
				for (size_t j = 0; j < 12; j++)
					((double *)cam_xyz)[j] = table[i].trans[j] / 10000.0;
				cam_xyz_coeff(rgb_cam, cam_xyz);
			}
			break;
		}
	}
}

void CSimpleInfo::cam_xyz_coeff(float rgb_cam[3][4], double cam_xyz[4][3])
{
	double cam_rgb[4][3];

	for (size_t i = 0; i < colors; i++)		/* Multiply out XYZ colorspace */
	{
		for (size_t j = 0; j < 3; j++)
		{
			cam_rgb[i][j] = 0;
			for (size_t k = 0; k < 3; k++)
			{
				cam_rgb[i][j] += cam_xyz[i][k] * xyz_rgb[k][j];
			}
		}
	}

	for (size_t i = 0; i < colors; i++)
	{		/* Normalize cam_rgb so that */
		double num = 0;
		for (size_t j = 0; j < 3; j++)		/* cam_rgb * (1,1,1) is (1,1,1,1) */
			num += cam_rgb[i][j];
		for (size_t j = 0; j < 3; j++)
			cam_rgb[i][j] /= num;
		pre_mul[i] = 1 / num;
	}
	double inverse[4][3];
	pseudoinverse(cam_rgb, inverse, colors);
	for (size_t i = 0; i < 3; i++)
		for (size_t j = 0; j < colors; j++)
			rgb_cam[i][j] = inverse[j][i];
}

void CSimpleInfo::pseudoinverse(double(*in)[3], double(*out)[3], int size)
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

float CSimpleInfo::find_green(int bps, int bite, int off0, int off1)
{
	UINT64 bitbuf = 0;
	unsigned short img[2][2064];

	for (size_t c = 0; c < 2; c++)
	{
		_reader->Seek(c ? off1 : off0, SEEK_SET);
		int vbits = 0;
		for (int col = 0; col < width; col++)
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
	for (size_t c = 0; c < (width - 1); c++)
	{
		sum[c & 1] += ABS(img[0][c] - img[1][c + 1]);
		sum[~c & 1] += ABS(img[1][c] - img[0][c + 1]);
	}
	return 100 * log(sum[0] / sum[1]);
}

void CSimpleInfo::simple_coeff(int index)
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

	raw_color = 0;
	for (size_t i = 0; i < 3; i++)
		for (size_t c = 0; c < colors; c++)
			rgb_cam[i][c] = table[index][i*colors + c];
}

void CSimpleInfo::gamma_curve(double pwr, double ts, int mode, int imax)
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
		memcpy(gamm, g, sizeof gamm);
		return;
	}
	for (size_t i = 0; i < 0x10000; i++)
	{
		curve[i] = 0xffff;
		double r;
		if ((r = (double)i / imax) < 1)
			curve[i] = 0x10000 * (mode
				? (r < g[3] ? r*g[1] : (g[0] ? pow(r, g[0])*(1 + g[4]) - g[4] : log(r)*g[2] + 1))
				: (r < g[2] ? r / g[1] : (g[0] ? pow((r + g[4]) / (1 + g[4]), 1 / g[0]) : exp((r - 1) / g[2]))));
	}
}

short CSimpleInfo::guess_byte_order(int words)
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

int CSimpleInfo::canon_s2is()
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
int CSimpleInfo::nikon_e995()
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
int CSimpleInfo::nikon_e2100()
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

void CSimpleInfo::nikon_3700()
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
			strcpy_s(make, LenMake, table[i].make);
			strcpy_s(model, LenModel, table[i].model);
		}
}

/*
Separates a Minolta DiMAGE Z2 from a Nikon E4300.
*/
int CSimpleInfo::minolta_z2()
{
	char tail[424];

	_reader->Seek(-sizeof tail, SEEK_END);
	_reader->Read(tail, 1, sizeof tail);
	int nz = 0;
	for (size_t i = 0; i < sizeof tail; i++)
		if (tail[i]) nz++;
	return nz > 20;
}

void CSimpleInfo::parse_phase_one(int base)
{
	float romm_cam[3][3];
	char *cp;

	memset(&ph1, 0, sizeof ph1);
	_reader->Seek(base, SEEK_SET);
	_reader->SetOrder(_reader->get4() & 0xffff);
	if (_reader->get4() >> 8 != 0x526177)
		return;		/* "Raw" */
	_reader->Seek(_reader->get4() + base, SEEK_SET);
	unsigned entries = _reader->get4();
	_reader->get4();
	while (entries--)
	{
		unsigned tag = _reader->get4();
		unsigned type = _reader->get4();
		unsigned len = _reader->get4();
		unsigned data = _reader->get4();
		unsigned save = _reader->GetPosition();
		_reader->Seek(base + data, SEEK_SET);
		switch (tag)
		{
		case 0x100:
			flip = "0653"[data & 3] - '0';
			break;
		case 0x106:
			for (size_t i = 0; i < 9; i++)
				((float *)romm_cam)[i] = _reader->getreal(11);
			romm_coeff(romm_cam);
			break;
		case 0x107:
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = _reader->getreal(11);
			break;
		case 0x108:
			raw_width = data;
			break;
		case 0x109:
			raw_height = data;
			break;
		case 0x10a:
			left_margin = data;
			break;
		case 0x10b:
			top_margin = data;
			break;
		case 0x10c:
			width = data;
			break;
		case 0x10d:
			height = data;
			break;
		case 0x10e:
			ph1.format = data;
			break;
		case 0x10f:
			data_offset = data + base;
			break;
		case 0x110:
			meta_offset = data + base;
			meta_length = len;
			break;
		case 0x112:
			ph1.key_off = save - 4;
			break;
		case 0x210:
			ph1.tag_210 = _reader->int_to_float(data);
			break;
		case 0x21a:
			ph1.tag_21a = data;
			break;
		case 0x21c:
			strip_offset = data + base;
			break;
		case 0x21d:
			ph1.black = data;
			break;
		case 0x222:
			ph1.split_col = data;
			break;
		case 0x223:
			ph1.black_col = data + base;
			break;
		case 0x224:
			ph1.split_row = data;
			break;
		case 0x225:
			ph1.black_row = data + base;
			break;
		case 0x301:
			model[63] = 0;
			_reader->Read(model, 1, 63);
			if ((cp = strstr(model, " camera"))) *cp = 0;
		}
		_reader->Seek(save, SEEK_SET);
	}
	load_raw = ph1.format < 3 ? LoadRawType::phase_one_load_raw : LoadRawType::phase_one_load_raw_c;
	maximum = 0xffff;
	strcpy_s(make, LenMake, "Phase One");
	if (model[0]) return;
	switch (raw_height)
	{
	case 2060:
		strcpy_s(model, LenModel, "LightPhase");
		break;
	case 2682:
		strcpy_s(model, LenModel, "H 10");
		break;
	case 4128:
		strcpy_s(model, LenModel, "H 20");
		break;
	case 5488:
		strcpy_s(model, LenModel, "H 25");
		break;
	}
}

/*
Parse a CIFF file, better known as Canon CRW format.
*/
void CSimpleInfo::parse_ciff(int offset, int length, int depth)
{
	int wbi = -1;
	unsigned short key[] = { 0x410, 0x45f3 };

	_reader->Seek(offset + length - 4, SEEK_SET);
	int tboff = _reader->get4() + offset;
	_reader->Seek(tboff, SEEK_SET);
	int nrecs = _reader->get2();
	if ((nrecs | depth) > 127) return;
	while (nrecs--)
	{
		int type = _reader->get2();
		int len = _reader->get4();
		int save = _reader->GetPosition() + 4;
		_reader->Seek(offset + _reader->get4(), SEEK_SET);
		if ((((type >> 8) + 8) | 8) == 0x38)
			parse_ciff(_reader->GetPosition(), len, depth + 1); /* Parse a sub-table */
		if (type == 0x0810)
			_reader->Read(artist, 64, 1);
		if (type == 0x080a)
		{
			_reader->Read(make, 64, 1);
			_reader->Seek(strlen(make) - 63, SEEK_CUR);
			_reader->Read(model, 64, 1);
		}
		if (type == 0x1810)
		{
			width = _reader->get4();
			height = _reader->get4();
			pixel_aspect = _reader->int_to_float(_reader->get4());
			flip = _reader->get4();
		}
		if (type == 0x1835)			/* Get the decoder table */
			tiff_compress = _reader->get4();
		if (type == 0x2007)
		{
			thumb_offset = _reader->GetPosition();
			thumb_length = len;
		}
		if (type == 0x1818)
		{
			shutter = pow(2, -_reader->int_to_float((_reader->get4(), _reader->get4())));
			aperture = pow(2, _reader->int_to_float(_reader->get4()) / 2);
		}
		if (type == 0x102a)
		{
			iso_speed = pow(2, (_reader->get4(), _reader->get2()) / 32.0 - 4) * 50;
			aperture = pow(2, (_reader->get2(), (short)_reader->get2()) / 64.0);
			shutter = pow(2, -((short)_reader->get2()) / 32.0);
			wbi = (_reader->get2(), _reader->get2());
			if (wbi > 17)
				wbi = 0;
			_reader->Seek(32, SEEK_CUR);
			if (shutter > 1e6)
				shutter = _reader->get2() / 10.0;
		}
		if (type == 0x102c)
		{
			if (_reader->get2() > 512)
			{		/* Pro90, G1 */
				_reader->Seek(118, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					cam_mul[c ^ 2] = _reader->get2();
			}
			else
			{				/* G2, S30, S40 */
				_reader->Seek(98, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					cam_mul[c ^ (c >> 1) ^ 1] = _reader->get2();
			}
		}
		if (type == 0x0032)
		{
			if (len == 768)
			{			/* EOS D30 */
				_reader->Seek(72, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					cam_mul[c ^ (c >> 1)] = 1024.0 / _reader->get2();
				if (!wbi) cam_mul[0] = -1;	/* use my auto white balance */
			}
			else if (!cam_mul[0])
			{
				int x;
				if (_reader->get2() == key[0])		/* Pro1, G6, S60, S70 */
				{
					x = (strstr(model, "Pro1") ? "012346000000000000" : "01345:000000006008")[wbi] - '0' + 2;
				}
				else
				{				/* G3, G5, S45, S50 */
					x = "023457000000006000"[wbi] - '0';
					key[0] = key[1] = 0;
				}
				_reader->Seek(78 + x * 8, SEEK_CUR);
				for (size_t c = 0; c < 4; c++)
					cam_mul[c ^ (c >> 1) ^ 1] = _reader->get2() ^ key[c & 1];
				if (!wbi) cam_mul[0] = -1;
			}
		}
		if (type == 0x10a9)
		{		/* D60, 10D, 300D, and clones */
			if (len > 66) wbi = "0134567028"[wbi] - '0';
			_reader->Seek(2 + wbi * 8, SEEK_CUR);
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ (c >> 1)] = _reader->get2();
		}
		if (type == 0x1030 && (0x18040 >> wbi & 1))
			ciff_block_1030();		/* all that don't have 0x10a9 */
		if (type == 0x1031)
		{
			raw_width = (_reader->get2(), _reader->get2());
			raw_height = _reader->get2();
		}
		if (type == 0x5029)
		{
			focal_len = len >> 16;
			if ((len & 0xffff) == 2) focal_len /= 32;
		}
		if (type == 0x5813) flash_used = _reader->int_to_float(len);
		if (type == 0x5814) canon_ev = _reader->int_to_float(len);
		if (type == 0x5817) shot_order = len;
		if (type == 0x5834) unique_id = len;
		if (type == 0x580e) timestamp = len;
		if (type == 0x180e) timestamp = _reader->get4();
#ifdef LOCALTIME
		if ((type | 0x4000) == 0x580e)
			timestamp = mktime(gmtime(&timestamp));
#endif
		_reader->Seek(save, SEEK_SET);
	}
}

/*
CIFF block 0x1030 contains an 8x8 white sample.
Load this into white[][] for use in scale_colors().
*/
void CSimpleInfo::ciff_block_1030()
{
	static const unsigned short key[] = { 0x410, 0x45f3 };

	if ((_reader->get2(), _reader->get4()) != 0x80008 || !_reader->get4())
		return;
	int bpp = _reader->get2();
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
				bitbuf = bitbuf << 16 | (_reader->get2() ^ key[i++ & 1]);
				vbits += 16;
			}
			white[row][col] = bitbuf >> (vbits -= bpp) & ~(-1 << bpp);
		}
	}
}

void CSimpleInfo::parse_fuji(int offset)
{
	_reader->Seek(offset, SEEK_SET);
	unsigned entries = _reader->get4();
	if (entries > 255)
		return;
	while (entries--)
	{
		unsigned tag = _reader->get2();
		unsigned len = _reader->get2();
		unsigned save = _reader->GetPosition();
		if (tag == 0x100)
		{
			raw_height = _reader->get2();
			raw_width = _reader->get2();
		}
		else if (tag == 0x121)
		{
			height = _reader->get2();
			if ((width = _reader->get2()) == 4284)
				width += 3;
		}
		else if (tag == 0x130)
		{
			fuji_layout = _reader->GetChar() >> 7;
			fuji_width = !(_reader->GetChar() & 8);
		}
		else if (tag == 0x131)
		{
			filters = 9;
			for (size_t c = 0; c < 36; c++)
				xtrans_abs[0][35 - c] = _reader->GetChar() & 3;
		}
		else if (tag == 0x2ff0)
		{
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ 1] = _reader->get2();
		}
		else if (tag == 0xc000)
		{
			short c = _reader->GetOrder();
			_reader->SetOrder(0x4949);
			while ((tag = _reader->get4()) > raw_width);
			width = tag;
			height = _reader->get4();
			_reader->SetOrder(c);
		}
		_reader->Seek(save + len, SEEK_SET);
	}
	height <<= fuji_layout;
	width >>= fuji_layout;
}

int CSimpleInfo::parse_jpeg(int offset)
{
	int len, save, hlen, mark;

	_reader->Seek(offset, SEEK_SET);
	if (_reader->GetChar() != 0xff || _reader->GetChar() != 0xd8)
		return 0;

	while (_reader->GetChar() == 0xff && (mark = _reader->GetChar()) != 0xda)
	{
		_reader->SetOrder(0x4d4d);
		len = _reader->get2() - 2;
		save = _reader->GetPosition();
		if (mark == 0xc0 || mark == 0xc3 || mark == 0xc9)
		{
			_reader->GetChar();
			raw_height = _reader->get2();
			raw_width = _reader->get2();
		}
		_reader->SetOrder(_reader->get2());
		hlen = _reader->get4();
		if (_reader->get4() == 0x48454150)		/* "HEAP" */
			parse_ciff(save + hlen, len - hlen, 0);
		if (parse_tiff(save + 6)) apply_tiff();
		_reader->Seek(save + len, SEEK_SET);
	}
	return 1;
}

void CSimpleInfo::parse_riff()
{
	char tag[4];
	static const char mon[12][4] =
	{ "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

	_reader->SetOrder(0x4949);
	_reader->Read(tag, 4, 1);
	unsigned size = _reader->get4();
	unsigned end = _reader->GetPosition() + size;
	if (!memcmp(tag, "RIFF", 4) || !memcmp(tag, "LIST", 4))
	{
		_reader->get4();
		while (_reader->GetPosition() + 7 < end && !_reader->Eof())
			parse_riff();
	}
	else if (!memcmp(tag, "nctg", 4))
	{
		while (_reader->GetPosition() + 7 < end)
		{
			unsigned i = _reader->get2();
			size = _reader->get2();
			if ((i + 1) >> 1 == 10 && size == 20)
				get_timestamp(0);
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
				timestamp = mktime(&t);
		}
	}
	else
		_reader->Seek(size, SEEK_CUR);
}

void CSimpleInfo::parse_qt(int end)
{
	unsigned size;
	char tag[4];

	_reader->SetOrder(0x4d4d);
	while (_reader->GetPosition() + 7 < end)
	{
		unsigned save = _reader->GetPosition();
		if ((size = _reader->get4()) < 8)
			return;
		_reader->Read(tag, 4, 1);
		if (!memcmp(tag, "moov", 4) ||
			!memcmp(tag, "udta", 4) ||
			!memcmp(tag, "CNTH", 4))
			parse_qt(save + size);
		if (!memcmp(tag, "CNDA", 4))
			parse_jpeg(_reader->GetPosition());
		_reader->Seek(save + size, SEEK_SET);
	}
}

void CSimpleInfo::parse_smal(int offset, int fsize)
{
	_reader->Seek(offset + 2, SEEK_SET);
	_reader->SetOrder(0x4949);
	int ver = _reader->GetChar();
	if (ver == 6)
		_reader->Seek(5, SEEK_CUR);
	if (_reader->get4() != fsize)
		return;
	if (ver > 6)
		data_offset = _reader->get4();
	raw_height = height = _reader->get2();
	raw_width = width = _reader->get2();
	strcpy_s(make, LenMake, "SMaL");
	sprintf_s(model, LenModel, "v%d %dx%d", ver, width, height);
	if (ver == 6)
		load_raw = LoadRawType::smal_v6_load_raw;
	if (ver == 9)
		load_raw = LoadRawType::smal_v9_load_raw;
}

void CSimpleInfo::parse_cine()
{
	_reader->SetOrder(0x4949);
	_reader->Seek(4, SEEK_SET);
	is_raw = _reader->get2() == 2;
	_reader->Seek(14, SEEK_CUR);
	is_raw *= _reader->get4();
	unsigned off_head = _reader->get4();
	unsigned off_setup = _reader->get4();
	unsigned off_image = _reader->get4();
	timestamp = _reader->get4();
	unsigned i = _reader->get4();
	if (i)
		timestamp = i;
	_reader->Seek(off_head + 4, SEEK_SET);
	raw_width = _reader->get4();
	raw_height = _reader->get4();
	switch (_reader->get2(), _reader->get2())
	{
	case 8:
		load_raw = LoadRawType::eight_bit_load_raw;
		break;
	case 16:
		load_raw = LoadRawType::unpacked_load_raw;
	}
	_reader->Seek(off_setup + 792, SEEK_SET);
	strcpy_s(make, LenMake, "CINE");
	sprintf_s(model, LenModel, "%d", _reader->get4());
	_reader->Seek(12, SEEK_CUR);
	switch ((i = _reader->get4()) & 0xffffff)
	{
	case 3:
		filters = 0x94949494;
		break;
	case 4:
		filters = 0x49494949;
		break;
	default:  is_raw = 0;
	}
	_reader->Seek(72, SEEK_CUR);
	switch ((_reader->get4() + 3600) % 360)
	{
	case 270:
		flip = 4;
		break;
	case 180:
		flip = 1;
		break;
	case  90:
		flip = 7;
		break;
	case   0:
		flip = 2;
	}
	cam_mul[0] = _reader->getreal(11);
	cam_mul[2] = _reader->getreal(11);
	maximum = ~(-1 << _reader->get4());
	_reader->Seek(668, SEEK_CUR);
	shutter = _reader->get4() / 1000000000.0;
	_reader->Seek(off_image, SEEK_SET);
	if (shot_select < is_raw)
		_reader->Seek(shot_select * 8, SEEK_CUR);
	data_offset = (INT64)_reader->get4() + 8;
	data_offset += (INT64)_reader->get4() << 32;
}

void CSimpleInfo::parse_redcine()
{
	_reader->SetOrder(0x4d4d);
	is_raw = 0;
	_reader->Seek(52, SEEK_SET);
	width = _reader->get4();
	height = _reader->get4();
	_reader->Seek(0, SEEK_END);
	unsigned i = _reader->GetPosition() & 511;
	_reader->Seek(-(i), SEEK_CUR);
	if (_reader->get4() != i || _reader->get4() != 0x52454f42)
	{
		fprintf(stderr, ("%s: Tail is missing, parsing from head...\n"), _reader->GetFileName());
		_reader->Seek(0, SEEK_SET);
		unsigned len;
		while ((len = _reader->get4()) != EOF)
		{
			if (_reader->get4() == 0x52454456)
				if (is_raw++ == shot_select)
					data_offset = _reader->GetPosition() - 8;
			_reader->Seek(len - 8, SEEK_CUR);
		}
	}
	else
	{
		unsigned rdvo = _reader->get4();
		_reader->Seek(12, SEEK_CUR);
		is_raw = _reader->get4();
		_reader->Seek(rdvo + 8 + shot_select * 4, SEEK_SET);
		data_offset = _reader->get4();
	}
}

void CSimpleInfo::parse_rollei()
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
			thumb_offset = atoi(val);
		if (!strcmp(line, "X  "))
			raw_width = atoi(val);
		if (!strcmp(line, "Y  "))
			raw_height = atoi(val);
		if (!strcmp(line, "TX "))
			thumb_width = atoi(val);
		if (!strcmp(line, "TY "))
			thumb_height = atoi(val);
	} while (strncmp(line, "EOHD", 4));
	data_offset = thumb_offset + thumb_width * thumb_height * 2;
	t.tm_year -= 1900;
	t.tm_mon -= 1;
	if (mktime(&t) > 0)
		timestamp = mktime(&t);
	strcpy_s(make, LenMake, "Rollei");
	strcpy_s(model, LenModel, "d530flex");
	write_thumb = WriteThumbType::rollei_thumb;
}

void CSimpleInfo::parse_sinar_ia()
{
	_reader->SetOrder(0x4949);
	_reader->Seek(4, SEEK_SET);
	int entries = _reader->get4();
	_reader->Seek(_reader->get4(), SEEK_SET);
	char str[8];
	while (entries--)
	{
		int off = _reader->get4();
		_reader->get4();
		_reader->Read(str, 8, 1);
		if (!strcmp(str, "META"))   meta_offset = off;
		if (!strcmp(str, "THUMB")) thumb_offset = off;
		if (!strcmp(str, "RAW0"))   data_offset = off;
	}
	_reader->Seek(meta_offset + 20, SEEK_SET);
	_reader->Read(make, 64, 1);
	make[63] = 0;

	char* cp;
	if ((cp = strchr(make, ' ')))
	{
		strcpy_s(model, LenModel, cp + 1);
		*cp = 0;
	}
	raw_width = _reader->get2();
	raw_height = _reader->get2();
	load_raw = LoadRawType::unpacked_load_raw;
	thumb_width = (_reader->get4(), _reader->get2());
	thumb_height = _reader->get2();
	write_thumb = WriteThumbType::ppm_thumb;
	maximum = 0x3fff;
}

void CSimpleInfo::parse_minolta(int base)
{
	int high = 0, wide = 0;
	short sorder = _reader->GetOrder();

	_reader->Seek(base, SEEK_SET);
	if (_reader->GetChar() || _reader->GetChar() - 'M' || _reader->GetChar() - 'R')
		return;
	_reader->SetOrder(_reader->GetChar() * 0x101);
	int offset = base + _reader->get4() + 8;
	int save;
	while ((save = _reader->GetPosition()) < offset)
	{
		int tag = 0;
		for (size_t i = 0; i < 4; i++)
			tag = tag << 8 | _reader->GetChar();
		int len = _reader->get4();
		int i;
		switch (tag)
		{
		case 0x505244:				/* PRD */
			_reader->Seek(8, SEEK_CUR);
			high = _reader->get2();
			wide = _reader->get2();
			break;
		case 0x574247:				/* WBG */
			_reader->get4();
			i = strcmp(model, "DiMAGE A200") ? 0 : 3;
			for (size_t c = 0; c < 4; c++)
				cam_mul[c ^ (c >> 1) ^ i] = _reader->get2();
			break;
		case 0x545457:				/* TTW */
			parse_tiff(_reader->GetPosition());
			data_offset = offset;
		}
		_reader->Seek(save + len + 8, SEEK_SET);
	}
	raw_height = high;
	raw_width = wide;
	_reader->SetOrder(sorder);
}

void CSimpleInfo::parse_foveon()
{
	int entries, img = 0, off, len, tag, save, i, wide, high, pent, poff[256][2];
	char name[64], value[64];

	_reader->SetOrder(0x4949);			/* Little-endian */
	_reader->Seek(36, SEEK_SET);
	flip = _reader->get4();
	_reader->Seek(-4, SEEK_END);
	_reader->Seek(_reader->get4(), SEEK_SET);
	if (_reader->get4() != 0x64434553)
		return;	/* SECd */
	entries = (_reader->get4(), _reader->get4());
	while (entries--)
	{
		off = _reader->get4();
		len = _reader->get4();
		tag = _reader->get4();
		save = _reader->GetPosition();
		_reader->Seek(off, SEEK_SET);
		if (_reader->get4() != (0x20434553 | (tag << 24)))
			return;
		switch (tag)
		{
		case 0x47414d49:			/* IMAG */
		case 0x32414d49:			/* IMA2 */
			_reader->Seek(8, SEEK_CUR);
			pent = _reader->get4();
			wide = _reader->get4();
			high = _reader->get4();
			if (wide > raw_width && high > raw_height)
			{
				switch (pent)
				{
				case  5:
					load_flags = 1;
				case  6:
					load_raw = LoadRawType::foveon_sd_load_raw;
					break;
				case 30:
					load_raw = LoadRawType::foveon_dp_load_raw;
					break;
				default:
					load_raw = LoadRawType::unknown_load_raw;
				}
				raw_width = wide;
				raw_height = high;
				data_offset = off + 28;
				is_foveon = 1;
			}
			_reader->Seek(off + 28, SEEK_SET);
			if (_reader->GetChar() == 0xff && _reader->GetChar() == 0xd8
				&& thumb_length < len - 28)
			{
				thumb_offset = off + 28;
				thumb_length = len - 28;
				write_thumb = WriteThumbType::jpeg_thumb;
			}
			if (++img == 2 && !thumb_length)
			{
				thumb_offset = off + 24;
				thumb_width = wide;
				thumb_height = high;
				write_thumb = WriteThumbType::foveon_thumb;
			}
			break;
		case 0x464d4143:			/* CAMF */
			meta_offset = off + 8;
			meta_length = len - 28;
			break;
		case 0x504f5250:			/* PROP */
			pent = (_reader->get4(), _reader->get4());
			_reader->Seek(12, SEEK_CUR);
			off += pent * 8 + 24;
			if ((unsigned)pent > 256) pent = 256;
			for (i = 0; i < pent * 2; i++)
				((int *)poff)[i] = off + _reader->get4() * 2;
			for (i = 0; i < pent; i++)
			{
				foveon_gets(poff[i][0], name, 64);
				foveon_gets(poff[i][1], value, 64);
				if (!strcmp(name, "ISO"))
					iso_speed = atoi(value);
				if (!strcmp(name, "CAMMANUF"))
					strcpy_s(make, LenMake, value);
				if (!strcmp(name, "CAMMODEL"))
					strcpy_s(model, LenModel, value);
				if (!strcmp(name, "WB_DESC"))
					strcpy_s(model2, LenModel2, value);
				if (!strcmp(name, "TIME"))
					timestamp = atoi(value);
				if (!strcmp(name, "EXPTIME"))
					shutter = atoi(value) / 1000000.0;
				if (!strcmp(name, "APERTURE"))
					aperture = atof(value);
				if (!strcmp(name, "FLENGTH"))
					focal_len = atof(value);
			}
#ifdef LOCALTIME
			timestamp = mktime(gmtime(&timestamp));
#endif
		}
		_reader->Seek(save, SEEK_SET);
	}
}

char* CSimpleInfo::foveon_gets(int offset, char* str, int len)
{
	_reader->Seek(offset, SEEK_SET);
	size_t i;
	for (i = 0; i < len - 1; i++)
		if ((str[i] = _reader->get2()) == 0)
			break;
	str[i] = 0;
	return str;
}

void CSimpleInfo::parse_kodak_ifd(int base)
{
	int wbi = -2, wbtemp = 6500;
	float mul[3] = { 1,1,1 };
	static const int wbtag[] = { 64037,64040,64039,64041,-1,-1,64042 };

	unsigned entries = _reader->get2();
	if (entries > 1024)
		return;

	unsigned tag, type, len, save;
	while (entries--)
	{
		tiff_get(base, &tag, &type, &len, &save);
		if (tag == 1020)
			wbi = _reader->getint(type);
		if (tag == 1021 && len == 72)
		{		/* WB set in software */
			_reader->Seek(40, SEEK_CUR);
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = 2048.0 / _reader->get2();
			wbi = -2;
		}
		if (tag == 2118)
			wbtemp = _reader->getint(type);
		if (tag == 2120 + wbi && wbi >= 0)
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = 2048.0 / _reader->getreal(type);
		if (tag == 2130 + wbi)
			for (size_t c = 0; c < 3; c++)
				mul[c] = _reader->getreal(type);
		if (tag == 2140 + wbi && wbi >= 0)
		{
			for (size_t c = 0; c < 3; c++)
			{
				float num = 0;
				for (int i = 0; i < 4; i++)
					num += _reader->getreal(type) * pow(wbtemp / 100.0, i);
				cam_mul[c] = 2048 / (num * mul[c]);
			}
		}
		if (tag == 2317)
			linear_table(len);
		if (tag == 6020)
			iso_speed = _reader->getint(type);
		if (tag == 64013)
			wbi = _reader->GetChar();
		if ((unsigned)wbi < 7 && tag == wbtag[wbi])
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = _reader->get4();
		if (tag == 64019)
			width = _reader->getint(type);
		if (tag == 64020)
			height = (_reader->getint(type) + 1) & -2;
		_reader->Seek(save, SEEK_SET);
	}
}

void CSimpleInfo::parse_mos(int offset)
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
		if (_reader->get4() != 0x504b5453)
			break;
		_reader->get4();
		_reader->Read(data, 1, 40);
		int skip = _reader->get4();
		int from = _reader->GetPosition();
		if (!strcmp(data, "JPEG_preview_data"))
		{
			thumb_offset = from;
			thumb_length = skip;
		}
		if (!strcmp(data, "icc_camera_profile"))
		{
			profile_offset = from;
			profile_length = skip;
		}
		if (!strcmp(data, "ShootObj_back_type"))
		{
			_reader->scanf("%d", &i);
			if ((unsigned)i < sizeof mod / sizeof(*mod))
				strcpy_s(model, LenModel, mod[i]);
		}
		if (!strcmp(data, "icc_camera_to_tone_matrix"))
		{
			for (size_t i = 0; i < 9; i++)
				((float *)romm_cam)[i] = _reader->int_to_float(_reader->get4());
			romm_coeff(romm_cam);
		}
		if (!strcmp(data, "CaptProf_color_matrix"))
		{
			for (size_t i = 0; i < 9; i++)
				_reader->scanf("%f", (float *)romm_cam + i);
			romm_coeff(romm_cam);
		}
		if (!strcmp(data, "CaptProf_number_of_planes"))
			_reader->scanf("%d", &planes);
		if (!strcmp(data, "CaptProf_raw_data_rotation"))
			_reader->scanf("%d", &flip);
		if (!strcmp(data, "CaptProf_mosaic_pattern"))
		{
			for (size_t c = 0; c < 4; c++)
			{
				_reader->scanf("%d", &i);
				if (i == 1) frot = c ^ (c >> 1);
			}
			if (!strcmp(data, "ImgProf_rotation_angle"))
			{
				_reader->scanf("%d", &i);
				flip = i - flip;
			}
		}
		if (!strcmp(data, "NeutObj_neutrals") && !cam_mul[0])
		{
			for (size_t c = 0; c < 4; c++)
				_reader->scanf("%d", neut + c);
			for (size_t c = 0; c < 3; c++)
				cam_mul[c] = (float)neut[0] / neut[c + 1];
		}
		if (!strcmp(data, "Rows_data"))
			load_flags = _reader->get4();
		parse_mos(from);
		_reader->Seek(skip + from, SEEK_SET);
	}
	if (planes)
		filters = (planes == 1) * 0x01010101 *
		(unsigned char) "\x94\x61\x16\x49"[(flip / 90 + frot) & 3];
}



void CSimpleInfo::sony_decrypt(unsigned *data, int len, int start, int key)
{
	static unsigned pad[128], p;

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

void CSimpleInfo::linear_table(unsigned len)
{
	if (len > 0x1000)
		len = 0x1000;
	_reader->read_shorts(curve, len);
	for (size_t i = len; i < 0x1000; i++)
		curve[i] = curve[i - 1];
	maximum = curve[0xfff];
}

void CSimpleInfo::romm_coeff(float romm_cam[3][3])
{
	static const float rgb_romm[3][3] =	/* ROMM == Kodak ProPhoto */
	{ { 2.034193, -0.727420, -0.306766 },
	{ -0.228811,  1.231729, -0.002922 },
	{ -0.008565, -0.153273,  1.161839 } };
	int i, j, k;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			for (cmatrix[i][j] = k = 0; k < 3; k++)
				cmatrix[i][j] += rgb_romm[i][k] * romm_cam[k][j];
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

unsigned CSimpleInfo::getbithuff(int nbits, unsigned short *huff)
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
		!(reset = zero_after_ff && c == 0xff && _reader->GetChar()))
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

