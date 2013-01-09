#include "Logging.h"
#include "../Common/MyString.h"
#include "../Common/FileIO.h"

// ----------------------------------------------------------------------
//
void CLoggingStream::SetNames(const std::wstring& fileDirectory, 
	const std::wstring& fileName, const std::wstring& moveDirectory)
{
	this->fileDirectory = fileDirectory;
	this->moveDirectory = moveDirectory;
	this->fileName = fileName;
	this->filePath = fileDirectory + fileName + L".log";	
}

void CLoggingStream::Initialize(const std::wstring& directory,
	const std::wstring& fileName, const std::wstring& moveDirectory)
{
	byteSize = 0;
	errorOffset = 0;
	bTimestamp = true;
	SetNames(directory, fileName, moveDirectory);
}

CLoggingStream::CLoggingStream(const std::wstring& fileDirectory, 
	const std::wstring& fileName, const std::wstring& moveDirectory)
	: COutStream()
{
	Initialize(fileDirectory, fileName, moveDirectory);
}

CLoggingStream::CLoggingStream(const CLoggingStream& other)
	: COutStream()
{
	Initialize(other.fileDirectory, other.fileName, other.moveDirectory);
}

CLoggingStream::~CLoggingStream()
{
	Flush();
}

void CLoggingStream::CheckAndMove(DWORD curSize)
{
	if (byteSize == 0) return;
	DWORD checkSize = byteSize + errorOffset;
	if (curSize >= checkSize) {
		SYSTEMTIME st = {0};		
		GetLocalTime(&st);
		std::wstring newfile = m_swprintf(
			L"%s%s_%02i-%02i-%02i_%02i-%02i-%02i.log",
			moveDirectory.c_str(), fileName.c_str(),
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		// If the move fails we don't want the overhead of trying again
		// for every write, so we try again later.
		if (CFile::Move(filePath, newfile, true))
			errorOffset = 0;
		else
			errorOffset += (1 << 12); // 4kB
	}
}

void CLoggingStream::SetMoveSize(DWORD kbSize)
{
	byteSize = kbSize * 1024;
}

void CLoggingStream::SetMoveDirectory(const std::wstring& move_to)
{
	moveDirectory = move_to;
	NDirectory::NormalizeDirectory(moveDirectory);
}

void CLoggingStream::SetOutFile(const std::wstring& directory,
	const std::wstring& fileName)
{
	SetNames(directory, fileName, moveDirectory);
}

void CLoggingStream::SetOutFile(const std::wstring& fileName)
{
	SetNames(this->fileDirectory, fileName, moveDirectory);
}

void CLoggingStream::EnableTimestamp(bool state)
{
	bTimestamp = state;
}

std::wstring CLoggingStream::PrependTimestamp(const std::wstring& str)
{
	SYSTEMTIME st = {0};		
	GetLocalTime(&st);

	return m_swprintf(L"%04i/%02i/%02i %02i:%02i:%02i  %s",
		st.wYear, st.wMonth, st.wDay, st.wHour,st.wMinute,st.wSecond,
		str.c_str());
}

bool CLoggingStream::Write(const std::wstring& str)
{
	if (str.size() == 0) return true; // always successful to do nothing
	std::wstring output;

	if (bTimestamp) output = PrependTimestamp(str);
	else output = str;

	COutFile h_file;
	if (!h_file.Open(filePath, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, OPEN_ALWAYS))
		return false; // can't open file
	h_file.SeekEnd();

	const wchar_t* c_str = output.c_str();
	static DWORD written;

	if (!h_file.Write((BYTE*)c_str, 
		sizeof(c_str[0])*output.size(), &written))
		return false; // can't write data

	DWORD fileSize = h_file.GetFileSize();
	h_file.Close(); // done with it now

	CheckAndMove(fileSize);
	return true;
}

