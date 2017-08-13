#pragma once

namespace Unmanaged
{
	class CDecoder
	{
	private:
		CDecoder();
		~CDecoder();

	public:
		static unsigned short * make_decoder_ref(const unsigned char **source);
		static unsigned short * make_decoder(const unsigned char *source);
	};
}
