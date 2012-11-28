#include "Streams.h"
#include "MyString.h"
#include <vector>

COutStream::~COutStream()
{
	// derived classes should flush
	//Flush();
}

void COutStream::Flush()
{
	std::wstring line = ss.str();
	if (!line.size()) return;
	if (Write(line) || line.size() > (1 << 15)) {
		// clear the stringstream on success or if it's > 32kb
		ss.str(std::wstring());
	}
}

COutStream& COutStream::operator<<(const endl_tag&)
{
	*this << NEW_LINE_CHAR;
	Flush();
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

// ----------------------------------------------------------------------
CLoggingStream::CLoggingStream(const std::wstring& file) : filePath(file),
	byteSize(0)
{
	// todo: set fileName and fileExtension
}

CLoggingStream::~CLoggingStream()
{
	Flush();
}

void CLoggingStream::CheckAndMove(DWORD curSize, const SYSTEMTIME& st)
{
	if (curSize >= byteSize) {
		std::wstring newfile = m_swprintf(
			L"%s\\%s_%02i-%02i-%02i_%02i-%02i-%02i.log",
			moveDirectory.c_str(), fileName.c_str(),
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		
		//std::wstring newfile = ss.str();
		
		// attempt to move the file (ignoring any errors)
		CFile::Move(filePath, newfile, true);		
	}
}

void CLoggingStream::SetMoveInfo(const std::wstring& move_to, DWORD kbSize)
{
	moveDirectory = move_to;
	byteSize = kbSize * 1024;
}

bool CLoggingStream::Write(const std::wstring& str)
{
	// todo: add timestamps etc here
	SYSTEMTIME st = {0};		
	GetLocalTime(&st);

	std::wstring output = m_swprintf(L"%04i/%02i/%02i %02i:%02i:%02i %s",
		st.wYear, st.wMonth, st.wDay, st.wHour,st.wMinute,st.wSecond,
		str.c_str());

	COutFile h_file;
	if (!h_file.Open(filePath, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, OPEN_ALWAYS))
		return false; // can't open file
	h_file.SeekEnd();

	const wchar_t* c_str = output.c_str();
	static DWORD written;
	
	if (!h_file.Write((BYTE*)output.c_str(), 
		sizeof(c_str[0])*output.size(), &written))
		return false; // can't write data

	DWORD fileSize = h_file.GetFileSize();
	h_file.Close(); // done with it now

	CheckAndMove(fileSize,st);
	return true;
}