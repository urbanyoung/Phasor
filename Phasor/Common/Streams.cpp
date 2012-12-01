#include "Streams.h"
#include "MyString.h"
#include <vector>

COutStream::COutStream() : no_flush(false)
{
}

COutStream::~COutStream()
{
	// derived classes should flush
	//Flush();
}

void COutStream::Flush()
{
	std::wstring line = ss.str();
	if (!line.size()) return;
	if (Write(line) || line.size() > (1 << 16)) {
		// clear the stringstream on success or if it's > 64kb
		ss.str(std::wstring());
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
	ss << WidenString(string);
	return *this;
}

COutStream & COutStream::operator<<(const std::wstring& string)
{
	ss << string;
	return *this;
}

COutStream & COutStream::operator<<(const char *string)
{
	return *this << WidenString(string);
}

COutStream & COutStream::operator<<(const wchar_t *string)
{
	ss << string;
	return *this;
}

COutStream & COutStream::operator<<(wchar_t c)
{
	ss << c;
	return *this;
}

COutStream & COutStream::operator<<(int number)
{
	wchar_t str[32];
	swprintf_s(str, NELEMS(str), L"%i", number);
	ss << str;
	return *this;
}

COutStream & COutStream::operator<<(DWORD number)
{
	wchar_t str[32] = {0};
	swprintf_s(str, NELEMS(str), L"%u", number);
	ss << str;
	return *this;
}

COutStream & COutStream::operator<<(double number)
{
	wchar_t str[32];
	swprintf_s(str, NELEMS(str), L"%.4d", number);
	ss << str;
	return *this;
}

// -----------------------------------------------------------------------
COutFileStream::COutFileStream()
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
