#include "memory.h"
#include "../Common/MyString.h"
#include <limits>
using namespace Common;
using namespace Manager;

// Not sure why std::numeric_limits<T>::max conflicts with these defines
// But it is.. 
#undef max
#undef min

// Attempt to read data at the specified address
template <class T> bool read_data_raw(LPBYTE destAddress, T & data)
{
	bool success = false;
	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(destAddress), sizeof(T), PAGE_EXECUTE_READ, &oldProtect))
	{
		data = *(T*)(destAddress);
		VirtualProtect(UlongToPtr(destAddress), sizeof(T), oldProtect, &oldProtect);
		success = true;
	}

	return success;	
}

// Attempt to write data at the specified address
template <class T> bool write_data_raw(LPBYTE destAddress, T data)
{
	bool success = false;
	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(destAddress), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		*(T*)(destAddress) = data;
		VirtualProtect(UlongToPtr(destAddress), sizeof(T), oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), UlongToPtr(destAddress), sizeof(T)); 
		success = true;
	}
	return success;
}

template <class T> T read_data(CallHandler& handler, LPBYTE dest_address)
{
	T data;
	if (!read_data_raw<T>(dest_address, data)) {
		std::string err = m_sprintf("Attempting read to invalid memory address %08X",
			dest_address);
		handler.RaiseError(err);
	}
	return data;
}

struct s_read_info
{
	LPBYTE address;
	int bit_offset;
	size_t i ;
	// kMaxArgsNormalRead is the maximum number of arguments that can
	// be passed in a normal (byte level) read. A bit read is always 
	// kMaxArgsNormalRead + 1.
	s_read_info(const Object::unique_deque& args, bool is_read_bit,
		int kMaxArgsNormalRead = 2)
		: i(0)
	{		
		address = (LPBYTE)Read(*args[i++]);
		if (!is_read_bit) {
			if (args.size() == kMaxArgsNormalRead) AddToAddress(*args[i++]);
		} else { // reading bit so extract param
			if (args.size() == kMaxArgsNormalRead + 1) AddToAddress(*args[i++]);
			bit_offset = Read(*args[i++]);
			address += bit_offset / 8;
			bit_offset %= 8;
		}		
	}

	DWORD Read(const Object& num)
	{
		ObjNumber& address_offset = (ObjNumber&)num;
		return (DWORD)address_offset.GetValue();
	}
	void AddToAddress(const Object& num)
	{
		address += Read(num);
	}
};

template <typename T>
// Read the write information and then perform the write.
struct s_write_info : s_read_info
{
	template <typename Type>
	bool CheckTypeLimits(double value, Type* out)
	{
		static const T max_value = std::numeric_limits<Type>::max();
		static const T min_value = std::numeric_limits<Type>::min();
		*out = (Type)value;
		return value <= max_value && value >= min_value;
	}

	template <>
	bool CheckTypeLimits<bool>(double value, bool* out)
	{
		*out = value == 1;
		return value == 1 || value == 0;
	}

	template <>
	bool CheckTypeLimits<float>(double value, float* out)
	{
		static const float max_value = std::numeric_limits<float>::max();
		static const float min_value = -max_value;
		*out = (float)value;
		return value <= max_value && value >= min_value;
	}

	template <>
	bool CheckTypeLimits<double>(double value, double* out)
	{
		*out = value;
		return true;
	}

	s_write_info(CallHandler& handler, 
		const Object::unique_deque& args, bool is_write_bit)
		: s_read_info(args, is_write_bit, 3)
	{
		ObjNumber& num = (ObjNumber&)*args[i++];
		double value = num.GetValue();

		T data;
		if (!CheckTypeLimits<T>(value, &data))
			handler.RaiseError("attempted to write value outside of type's range.");
		
		if (!write_data_raw<T>(address, data)) {
			std::string err = m_sprintf("Attempting write to invalid memory address %08X",
				address);
			handler.RaiseError(err);
		}
	}
};

void l_readbit(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_read_info r(args, true);
	BYTE b = read_data<BYTE>(handler, r.address);
	bool bit = ((b & (1 << r.bit_offset)) >> r.bit_offset) == 1;
	results.push_back(std::unique_ptr<Object>(new ObjBool(bit)));
}

template <typename T>
void read_type(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_read_info r(args, false);
	T value = read_data<T>(handler, r.address);
	results.push_back(std::unique_ptr<Object>(new ObjNumber(value)));
}

void l_readbyte(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<BYTE>(handler, args, results);
}
void l_readchar(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<char>(handler, args, results);
}
void l_readword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<WORD>(handler, args, results);
}
void l_readshort(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<short>(handler, args, results);
}
void l_readdword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<DWORD>(handler, args, results);
}
void l_readint(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<int>(handler, args, results);
}
void l_readfloat(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<float>(handler, args, results);
}
void l_readdouble(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_type<double>(handler, args, results);
}
void l_writebit(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<bool> w(handler, args, true);
}
void l_writebyte(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<BYTE> w(handler, args, false);
}
void l_writechar(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<char> w(handler, args, false);
}
void l_writeword(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<WORD> w(handler, args, false);
}
void l_writeshort(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<short> w(handler, args, false);
}
void l_writedword(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<DWORD> w(handler, args, false);
}
void l_writeint(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<int> w(handler, args, false);
}
void l_writefloat(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<float> w(handler, args, false);
}
void l_writedouble(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	s_write_info<double> w(handler, args, false);
}
