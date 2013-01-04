#include "Streams.h"
#include "MyString.h"
#include <vector>

COutStream::COutStream() : no_flush(false)
{
	str.reserve(kDefaultBufferSize); // start with 8kb
}

COutStream::~COutStream()
{
	// derived classes should flush
	//Flush();
}

void COutStream::Reserve(size_t size)
{
	str.clear();
	str.reserve(size); // start with 8kb
}

void COutStream::Flush()
{
	if (!str.size()) return;
	if (Write(str) || str.size() > (1 << 16)) {
		// clear the string on success or if it's > 64kb
		Reserve(kDefaultBufferSize);
	}
}

COutStream& COutStream::operator<<(const endl_tag&)
{
	*this << NEW_LINE_CHAR;
	if (!no_flush) Flush();
	return *this;
}

COutStream & COutStream::operator<<(const std::string& string)
{
	str += WidenString(string);
	return *this;
}

COutStream & COutStream::operator<<(const std::wstring& string)
{
	str += string;
	return *this;
}

COutStream & COutStream::operator<<(const char *string)
{
	return *this << WidenString(string);
}

COutStream & COutStream::operator<<(const wchar_t *string)
{
	str += string;
	return *this;
}

COutStream & COutStream::operator<<(wchar_t c)
{
	str += c;
	return *this;
}

COutStream & COutStream::operator<<(int number)
{
	wchar_t str[32];
	swprintf_s(str, NELEMS(str), L"%i", number);
	this->str += str;
	return *this;
}

COutStream & COutStream::operator<<(DWORD number)
{
	wchar_t str[32] = {0};
	swprintf_s(str, NELEMS(str), L"%u", number);
	this->str += str;
	return *this;
}

COutStream & COutStream::operator<<(double number)
{
	wchar_t str[32];
	swprintf_s(str, NELEMS(str), L"%.4d", number);
	this->str += str;
	return *this;
}

void COutStream::print(const char* format, ...)
{
	va_list ArgList;
	va_start(ArgList, format);
	std::string str = FormatVarArgs(format, ArgList);
	va_end(ArgList);
	*this << str << endl;
}

void COutStream::wprint(const wchar_t* format, ...)
{
	va_list ArgList;
	va_start(ArgList, format);
	std::wstring str = FormatVarArgsW(format, ArgList);
	va_end(ArgList);
	*this << str << endl;
}

// -----------------------------------------------------------------------
/*COutFileStream::COutFileStream()
{
}

COutFileStream::~COutFileStream()
{
	Flush();
}

bool COutFileStream::Open(const std::wstring& file)
{
	return this->file.Open(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);
}

bool COutFileStream::Write(const std::wstring& str)
{
	if (!file.IsOpen()) return false;
	file.SeekEnd();
	const wchar_t* c_str = str.c_str();
	static DWORD written;
	return file.Write((BYTE*)str.c_str(), sizeof(c_str[0])*str.size(), &written);
}
*/