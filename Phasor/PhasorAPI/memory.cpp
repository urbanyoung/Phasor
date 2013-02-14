#include "memory.h"
#include "api_readers.h"
#include "../Common/MyString.h"
#include <limits>
using namespace Common;
using namespace Manager;

// Not sure why std::numeric_limits<T>::max conflicts with these defines
// But it is.. 
#undef max
#undef min

// Parses the input arguments into the destination address and (if specified)
// the bit offset.
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
		address = (LPBYTE)ReadNumber<DWORD>(*args[i++]);
		if (!is_read_bit) {
			if (args.size() == kMaxArgsNormalRead) AddToAddress(*args[i++]);
		} else { // reading bit so extract the bit offset
			if (args.size() == kMaxArgsNormalRead + 1) AddToAddress(*args[i++]);
			bit_offset = ReadNumber<int>(*args[i++]);
			address += bit_offset / 8;
			bit_offset %= 8;
		}		
	}

	void AddToAddress(const Object& num)
	{
		address += ReadNumber<DWORD>(num);
	}
};

// Parse and validate input arguments ready for writing.
template <typename T> struct s_write_info : s_read_info
{
	T data;

	// Check the value is within the expected type's range.
	template <typename Type>
	bool CheckTypeLimits(double value, Type* out)
	{
		static const T max_value = std::numeric_limits<Type>::max();
		static const T min_value = std::numeric_limits<Type>::min();
		*out = (Type)value;
		return value <= max_value && value >= min_value;
	}

	template <> bool CheckTypeLimits<bool>(double value, bool* out)
	{
		*out = value == 1;
		return value == 1 || value == 0;
	}

	template <> bool CheckTypeLimits<float>(double value, float* out)
	{
		static const float max_value = std::numeric_limits<float>::max();
		static const float min_value = -max_value;
		*out = (float)value;
		return value <= max_value && value >= min_value;
	}

	template <>	bool CheckTypeLimits<double>(double value, double* out)
	{
		*out = value;
		return true;
	}

	// Can throw if value is out of range.
	s_write_info(CallHandler& handler, 
		const Object::unique_deque& args, bool is_write_bit)
		: s_read_info(args, is_write_bit, 3)
	{
		double value = ReadNumber<double>(*args[i++]);

		if (!CheckTypeLimits<T>(value, &data))
			handler.RaiseError("attempted to write value outside of type's range.");

		if (is_write_bit) {
			BYTE b = read_data<BYTE>(handler, address);
			if (data == 1) data = (BYTE)(b | (1 << bit_offset)); 
			else data = (BYTE)(b ^ (1 << bit_offset)); 
		}
	}
};

// shouldn't use bool
template <> struct s_write_info<bool> {};

// Reads from the specified memory address and raises a script error if 
// the address is invalid.
template <class T> T read_data(CallHandler& handler, LPBYTE destAddress)
{
	T data;

	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(destAddress), sizeof(T), PAGE_EXECUTE_READ, &oldProtect))
	{
		data = *(T*)(destAddress);
		VirtualProtect(UlongToPtr(destAddress), sizeof(T), oldProtect, &oldProtect);
	} else {
		std::string err = m_sprintf("Attempting read to invalid memory address %08X",
			destAddress);
		handler.RaiseError(err);
	}
	return data;
}

// Reads from the specified memory address and raises a script error if the
// address. The result is added to results.
template <typename T>
void read_type(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_read_info r(args, false);
	T value = read_data<T>(handler, r.address);
	results.push_back(std::unique_ptr<Object>(new ObjNumber(value)));
}

// Writes to the specified memory address and raises a script error if the
// address is invalid or if they value being written is outside the type's
// range.
template <class T>
void write_type(CallHandler& handler, Object::unique_deque& args, bool write_bit=false)
{
	s_write_info<T> w(handler, args, write_bit);

	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(w.address), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		*(T*)(w.address) = w.data;
		VirtualProtect(UlongToPtr(w.address), sizeof(T), oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), UlongToPtr(w.address), sizeof(T)); 
	} else {
		std::string err = m_sprintf("Attempting write to invalid memory address %08X",
			w.address);
		handler.RaiseError(err);
	}
}

// -------------------------------------------------------------------------
// 
void l_readbit(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_read_info r(args, true);
	BYTE b = read_data<BYTE>(handler, r.address);
	bool bit = ((b & (1 << r.bit_offset)) >> r.bit_offset) == 1;
	results.push_back(std::unique_ptr<Object>(new ObjBool(bit)));
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
	write_type<BYTE>(handler, args, true);
}

void l_writebyte(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<BYTE>(handler, args);
}

void l_writechar(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<char>(handler, args);
}

void l_writeword(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<WORD>(handler, args);
}

void l_writeshort(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<short>(handler, args);
}

void l_writedword(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<DWORD>(handler, args);
}

void l_writeint(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<int>(handler, args);
}

void l_writefloat(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<float>(handler, args);
}

void l_writedouble(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_type<double>(handler, args);
}