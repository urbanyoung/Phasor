#include "Logging.h"
#include "../Common/MyString.h"

// ----------------------------------------------------------------------
//
void CLoggingStream::SetNames(const std::wstring& directory, 
	const std::wstring& fileName)
{
	this->fileDirectory = directory;
	this->fileName = fileName;
	this->filePath = fileDirectory + fileName + L".log";	
}

void CLoggingStream::Initialize(const std::wstring& directory,
	const std::wstring& fileName)
{
	byteSize = 0;
	errorOffset = 0;
	SetNames(directory, fileName);
}

CLoggingStream::CLoggingStream(const std::wstring& dir, const std::wstring& file)
	: COutStream()
{
	Initialize(dir, file);
}

CLoggingStream::CLoggingStream(const CLoggingStream& other)
	: COutStream()
{
	Initialize(other.fileDirectory, other.fileName);
}

CLoggingStream::~CLoggingStream()
{
	Flush();
}

void CLoggingStream::CheckAndMove(DWORD curSize, const SYSTEMTIME& st)
{
	if (byteSize == 0) return;
	DWORD checkSize = byteSize + errorOffset;
	if (curSize >= checkSize) {
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

void CLoggingStream::SetMoveInfo(const std::wstring& move_to, DWORD kbSize)
{
	moveDirectory = move_to;
	NDirectory::NormalizeDirectory(moveDirectory);
	byteSize = kbSize * 1024;
}

void CLoggingStream::SetOutFile(const std::wstring& directory,
	const std::wstring& fileName)
{
	SetNames(directory, fileName);
}

void CLoggingStream::SetOutFile(const std::wstring& fileName)
{
	SetNames(this->fileDirectory, fileName);
}

bool CLoggingStream::Write(const std::wstring& str)
{
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