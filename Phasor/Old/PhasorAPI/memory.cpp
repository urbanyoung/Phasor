#include "memory.h"
#include "api_readers.h"
#include "../../Common/MyString.h"
#include <limits>
using namespace Common;
using namespace Manager;

// Not sure why std::numeric_limits<T>::max conflicts with these defines
// But it is.. 
#undef max
#undef min

static const int kMaxStringElements = 80;

// Parses the input arguments into the destination address and (if specified)
// the bit offset.
struct s_number_read_info
{
	LPBYTE address;
	int bit_offset;
	size_t i ;
	// kMaxArgsNormalRead is the maximum number of arguments that can
	// be passed in a normal (byte level) read. A bit read is always 
	// kMaxArgsNormalRead + 1.
	s_number_read_info(const Object::unique_deque& args, bool is_read_bit,
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
template <typename T> struct s_number_write_info : s_number_read_info
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
	s_number_write_info(CallHandler& handler, 
		const Object::unique_deque& args, bool is_write_bit)
		: s_number_read_info(args, is_write_bit, 3)
	{
		double value = ReadNumber<double>(*args[i++]);

		if (!CheckTypeLimits<T>(value, &data))
			handler.RaiseError("attempted to write value outside of type's range.");

		if (is_write_bit) {
			BYTE b = read_data<BYTE>(handler, address);
			if (data == 1) data = (BYTE)(b | (1 << bit_offset)); 
			else {
				data = (BYTE)(b & ~(1 << bit_offset));
				//data = (BYTE)(b ^ (1 << bit_offset)); 
			}
		}
	}
};

// shouldn't use bool
template <> struct s_number_write_info<bool> {};

// Helper functions for reading/writing strings.
template <typename T> void copy_string(const T* src, int elem_count, T* dest);
template <> void copy_string<char>(const char* src, int elem_count, char* dest)
{
	strcpy_s(dest, elem_count, src);
}
template <> void copy_string<wchar_t>(const wchar_t* src, int elem_count, wchar_t* dest)
{
	wcscpy_s(dest, elem_count, src);
}

// ------------------------------------------------------------------------
// Reading functions
template <class T>
T readstring(typename T::value_type* src, int size_bytes)
{
	typedef T::value_type char_type;
	char_type buff[81];

	int buffer_char_count = sizeof(buff)/sizeof(char_type);
	int read_char_count = size_bytes / sizeof(char_type);
	int use_count = buffer_char_count < read_char_count ? buffer_char_count : read_char_count;

	copy_string<char_type>(src, use_count, buff);

	// make sure it's null terminated
	buff[use_count - 1] = 0;

	return buff;
}

template <class T> T reader(LPBYTE address, DWORD)
{
	return *(T*)address;
}

template <> std::string reader<std::string>(LPBYTE address, DWORD size_bytes)
{
	return readstring<std::string>((char*)address, size_bytes);
}

template <> std::wstring reader<std::wstring>(LPBYTE address, DWORD size_bytes)
{
	return readstring<std::wstring>((wchar_t*)address, size_bytes);
}

// Reads from the specified memory address and raises a script error if 
// the address is invalid.
template <class T> T read_data(CallHandler& handler, LPBYTE destAddress,
	DWORD size=sizeof(T))
{
	T data;

	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(destAddress), size, PAGE_EXECUTE_READ, &oldProtect))
	{
		data = reader<T>(destAddress, size);
		VirtualProtect(UlongToPtr(destAddress), size, oldProtect, &oldProtect);
	} else {
		std::string err = m_sprintf("Attempting read to invalid memory address %08X",
			destAddress);
		handler.RaiseError(err);
	}
	return data;
}

// ------------------------------------------------------------------------
// Writer functions
template <class T> 
void writer(LPBYTE address, T data, DWORD)
{
	*(T*)(address) = data;
}
template <> void writer<std::string>(LPBYTE address, std::string data, DWORD)
{
	copy_string<char>(data.c_str(), data.size(), (char*)address);
}
template <> void writer<std::wstring>(LPBYTE address, std::wstring data, DWORD)
{
	copy_string<wchar_t>(data.c_str(), data.size()+1, (wchar_t*)address);
}

template <class T> void write_data(CallHandler& handler, LPBYTE destAddress,
	T data, DWORD size = sizeof(T))
{
	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(destAddress), size, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		writer<T>(destAddress, data, size);
		VirtualProtect(UlongToPtr(destAddress), size, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), UlongToPtr(destAddress), size); 
	} else {
		std::string err = m_sprintf("Attempting write to invalid memory address %08X",
			destAddress);
		handler.RaiseError(err);
	}
}

// ------------------------------------------------------------------------
// Reader/writer functions used by rest of the script.
// 

// Reads from the specified memory address and raises a script error if the
// address. The result is added to results.
template <typename T>
void read_number(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_number_read_info r(args, false);
	T value = read_data<T>(handler, r.address);
	results.push_back(std::unique_ptr<Object>(new ObjNumber(value)));
}

template <typename T>
void read_string(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	LPBYTE address = (LPBYTE)ReadNumber<DWORD>(*args[0]);
	DWORD length = 0;
	if (args.size() == 2) length = ReadNumber<DWORD>(*args[1]);
	if (length > kMaxStringElements) handler.RaiseError("max string length is 80");

	DWORD byte_length = length == 0 ? kMaxStringElements : length;
	byte_length *= sizeof(T::value_type);

	T str = read_data<T>(handler, address, byte_length);
	AddResultString(str, results);
}

// Writes to the specified memory address and raises a script error if the
// address is invalid or if they value being written is outside the type's
// range.
template <class T>
void write_number(CallHandler& handler, Object::unique_deque& args, bool write_bit=false)
{
	s_number_write_info<T> w(handler, args, write_bit);
	write_data<T>(handler, w.address, w.data);
}

template <class T>
void write_string(CallHandler& handler, LPBYTE address, T str)
{
	write_data<T>(handler, address, str, (str.size() + 1)*sizeof(T::value_type));
}

// -------------------------------------------------------------------------
// 
void l_readbit(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_number_read_info r(args, true);
	BYTE b = read_data<BYTE>(handler, r.address);
	bool bit = (b & (1 << r.bit_offset)) != 0;
	results.push_back(std::unique_ptr<Object>(new ObjBool(bit)));
}

void l_readbyte(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<BYTE>(handler, args, results);
}

void l_readchar(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<char>(handler, args, results);
}

void l_readword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<WORD>(handler, args, results);
}

void l_readshort(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<short>(handler, args, results);
}

void l_readdword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<DWORD>(handler, args, results);
}

void l_readint(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<int>(handler, args, results);
}

void l_readfloat(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<float>(handler, args, results);
}

void l_readdouble(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_number<double>(handler, args, results);
}

void l_readstring(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_string<std::string>(handler, args, results);
}

void l_readwidestring(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	read_string<std::wstring>(handler, args, results);
}

void l_writebit(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<BYTE>(handler, args, true);
}

void l_writebyte(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<BYTE>(handler, args);
}

void l_writechar(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<char>(handler, args);
}

void l_writeword(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<WORD>(handler, args);
}

void l_writeshort(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<short>(handler, args);
}

void l_writedword(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<DWORD>(handler, args);
}

void l_writeint(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<int>(handler, args);
}

void l_writefloat(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<float>(handler, args);
}

void l_writedouble(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	write_number<double>(handler, args);
}

void l_writestring(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	LPBYTE address = (LPBYTE)ReadNumber<DWORD>(*args[0]);
	std::string str = ReadRawString(*args[1]);
	write_string<std::string>(handler, address, str);
}

void l_writewidestring(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	LPBYTE address = (LPBYTE)ReadNumber<DWORD>(*args[0]);
	std::wstring str = WidenString(ReadRawString(*args[1]));
	write_string<std::wstring>(handler, address, str);
}