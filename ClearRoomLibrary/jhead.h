#pragma once

#include "CReader.h"

namespace Unmanaged
{
	struct jhead
	{
		int algo, bits, high, wide, clrs, sraw, psv, restart, vpred[6];
		unsigned short quant[64], idct[64], *huff[20], *free[20], *row;

		static int ljpeg_start(jhead &jh, CReader& reader, unsigned int dngVersion, bool info_only);
		static void ljpeg_end(struct jhead& jh);
	};
}