#pragma once

struct tiff_ifd
{
	int width, height, bps, comp, phint, offset, flip, samples, bytes;
	int tile_width, tile_length;
	float shutter;
};
