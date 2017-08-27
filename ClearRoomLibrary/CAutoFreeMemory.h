#pragma once

namespace Unmanaged
{
	class CAutoFreeMemory
	{
	public:
		CAutoFreeMemory(void* ptr);
		CAutoFreeMemory(void* ptr, bool verify);

		~CAutoFreeMemory();

		void ChangePointer(void* ptr, bool verify = true, bool releaseOld = true);
		void Release();

	private:
		void* _ptr;
	};
}
