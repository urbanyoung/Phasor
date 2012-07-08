#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <windows.h>
#include <string>

namespace common 
{
	std::string NarrowString(std::wstring);
	std::wstring WidenString(std::string);


	// Class: StreamBuilder
	// ------------------------------------------------------------------------
	class StreamBuilder
	{
	private:
		LPBYTE stream;
		unsigned long index, bufferSize;
		static const unsigned long DEFAULT_BUFFER_SIZE = 16384;

	public:
		StreamBuilder()
		{
			init(DEFAULT_BUFFER_SIZE);	
		}

		StreamBuilder(int size)
		{
			init(size);
		}

		void init(int size)
		{
			stream = new BYTE[size];
			index = 0;
			bufferSize = size;
		}

		~StreamBuilder()
		{
			delete[] stream;
		}

		// Append data of primitive types
		template <class T>
		unsigned long Append(T data)
		{
			unsigned long startIndex = index;
			if (sizeof(T) + index > bufferSize)
			{
				std::string err = __FUNCTION__ + std::string(" attempted to write past stream bounds.");
				throw std::exception(err.c_str());
			}

			*(T*)(stream + index) = data;
			index += sizeof(T);
			return startIndex;
		}

		// Append a string to the stream
		unsigned long AppendString(const char* data)
		{
			unsigned long startIndex = index;
			int len = strlen(data);

			if (len + index > bufferSize)
			{
				std::string err = __FUNCTION__ + std::string(" attempted to write past stream bounds.");
				throw std::exception(err.c_str());
			}

			strcpy((char*)(stream + index), data);
			index += len;
			return startIndex;
		}

		// Appends an array of data
		unsigned long AppendArray(LPBYTE data, DWORD dwSize)
		{
			unsigned long startIndex = index;
			if (dwSize + index > bufferSize)
			{
				std::string err = __FUNCTION__ + std::string(" attempted to write past stream bounds.");
				throw std::exception(err.c_str());
			}

			memcpy(stream + index, data, dwSize);
			index += dwSize;
			return startIndex;
		}

		// Returns the data stream
		LPBYTE getStream() { return stream;}

		// Returns the stream size
		DWORD getStreamSize() { return index; }
	};

}
