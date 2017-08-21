#pragma once

namespace Unmanaged
{
	class CWriter
	{
	public:
		CWriter(const char *fileName);
		~CWriter();
	protected:
		CWriter();

	public:
		static CWriter* CreateTempFile();

	private:
		FILE* _ifp;
		short _order;
		unsigned _dataError;
		char* _fileName;

	public:
		const char* GetFileName();

		void SetOrder(short order);
		short GetOrder();

		int Seek(long offset, int origin);
		size_t Write(void* buffer, size_t elementSize, size_t elementCount);
		long GetPosition(); //const;
		int Eof();
	};
}
